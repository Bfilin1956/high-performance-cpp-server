#pragma once
#include <queue>
#include <memory>
#include <boost/asio.hpp>

#include <attaServer/protocol/Protocol.h>
#include <attaServer/logger/Logger.h>
#include <attaServer/network/ConnectionManager.h>

#include <attaServer/protocol/Reader.h>
#include <attaServer/protocol/Writer.h>

namespace asio = boost::asio;
using boost::asio::awaitable;
using boost::asio::ip::tcp;
using boost::asio::buffer;
using boost::asio::detached;
using boost::asio::co_spawn;
using boost::asio::use_awaitable;
using boost::asio::thread_pool;

class Session : public std::enable_shared_from_this<Session>{
    tcp::socket socket_;
    asio::strand<asio::any_io_executor> strand_;
    asio::steady_timer watchdog_timer_;
    std::queue<std::vector<std::byte>> q_;
    asio::streambuf buffer_;
    bool writing_{false};
    std::atomic<bool> closed_{false};
    std::atomic<bool> ping_in_flight_{false};
    std::chrono::steady_clock::time_point last_rx_{};
    std::chrono::steady_clock::time_point last_tx_{};
    std::chrono::seconds ping_interval_ = std::chrono::seconds(15);
    std::chrono::seconds timeout_ = std::chrono::seconds(45);

    uint64_t session_id_{};
public:
    explicit Session(tcp::socket socket)
    : socket_(std::move(socket)),
      strand_(asio::make_strand(socket_.get_executor())),
      watchdog_timer_(strand_)
    {}
    void start() {
        auto self = shared_from_this();

        session_id_ = ConnectionManager::registerSession(self);

        co_spawn(strand_,[self]()->awaitable<void> {
            co_await self->read();
        }, detached);
        co_spawn(strand_,[self]()->awaitable<void> {
            co_await self->watch_dog();
        }, detached);
    }
    void send(std::vector<std::byte> data) {
        auto self = shared_from_this();

        asio::post(strand_,
            [self, data = std::move(data)]() mutable {
                self->q_.push(std::move(data));

                if (!self->writing_) {
                    self->writing_ = true;

                    asio::co_spawn(
                        self->strand_,
                        self->write(),
                        asio::detached
                    );
                }
            });
    }
    awaitable<void> watch_dog() {
        while (true) {
            watchdog_timer_.expires_after(ping_interval_);
            boost::system::error_code ec;
            co_await watchdog_timer_.async_wait(asio::redirect_error(use_awaitable, ec));

            if (ec == asio::error::operation_aborted)
                co_return;

            if (closed_) co_return;

            auto idle = std::chrono::steady_clock::now() - last_rx_;

            if (idle > timeout_) {
                Logger::info("Client timeout");
                close();
                co_return;
            }

            if (idle > ping_interval_ && !ping_in_flight_.exchange(true)) {
                //send_ping();
            }
        }
    }
private:
    awaitable<void> read() {
        boost::system::error_code ec;
        while (true){
            Header header{};

            co_await asio::async_read(socket_,
                          asio::buffer(&header, sizeof(Header)),
                          asio::bind_executor(strand_, asio::redirect_error(asio::use_awaitable, ec)));

            if (ec) {
                close();
                co_return;
            }

            Protocol::ReadHeader(&header);

            if (!Protocol::isValid(&header)) {
                Logger::warn("Bad header");
                close();
                co_return;
            }

            std::vector<std::byte> body;
            body.resize(header.size);
            co_await asio::async_read(socket_,
                asio::buffer(body.data(), body.size()),
                asio::bind_executor(strand_, asio::redirect_error(asio::use_awaitable, ec)));

            if (ec) {
                close();
                co_return;
            }

            Reader reader(reinterpret_cast<char *>(body.data()), body.size());


            last_rx_ = std::chrono::steady_clock::now();
            Logger::info("Success reading");
        }
    }
    awaitable<void> write() {
        boost::system::error_code ec;
        while (!q_.empty()) {

            co_await asio::async_write(socket_,
                asio::buffer(q_.front().data(),
                q_.front().size()),
                asio::bind_executor(strand_, asio::redirect_error(asio::use_awaitable, ec)));
            if (ec) {
                close();
                co_return;
            }
            q_.pop();
            last_tx_ = std::chrono::steady_clock::now();
            Logger::info("Success writing");
        }

        writing_ = false;
    }
    void close() {
        auto self = shared_from_this();

        boost::system::error_code ecEp;
        const auto ep = socket_.remote_endpoint(ecEp);
        if (closed_.exchange(true))
            return;

        boost::system::error_code ignore;
        socket_.cancel(ignore);
        watchdog_timer_.cancel();
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
        socket_.close(ignore);

        ConnectionManager::deleteSession(session_id_);

        if (!ecEp) return;
        else Logger::info("Session close. id: {}, ip: {}/{}", session_id_, ep.address().to_string(), ep.port());
    }
};

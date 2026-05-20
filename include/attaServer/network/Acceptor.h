//
// Created by r13x on 5/7/26.
//

#ifndef ATTA1_ACCEPTOR_H
#define ATTA1_ACCEPTOR_H

#include <attaServer/network/Session.h>
#include <attaServer/logger/Logger.h>

class Acceptor {
    tcp::acceptor acceptor_;
    std::string ip_;
    unsigned short port_;
public:
    explicit Acceptor(const asio::any_io_executor& ex, const std::string& ip = "127.0.0.1", const short port = 9000)
    :  acceptor_(ex), ip_(std::move(ip)), port_(port)
    {
        auto addr = boost::asio::ip::make_address(ip_);
        acceptor_.open(tcp::v4());
        acceptor_.set_option(tcp::acceptor::reuse_address(true));
        acceptor_.bind({addr, port_});
        acceptor_.listen();

    }
    void start()  {
        co_spawn(acceptor_.get_executor(), do_accept(), detached);
    }
    awaitable<void> do_accept() {
        boost::system::error_code ec;
        while (true){
            auto socket = co_await  acceptor_.async_accept(asio::redirect_error(use_awaitable, ec));
            if (ec == boost::asio::error::address_in_use) {
                Logger::error("address_in_use");
                co_return;
            }
            if (ec) {
                Logger::error("{}",ec.message());
                continue;
            }

            std::make_shared<Session>(std::move(socket))->start();
            Logger::info("Client connected");
        }
    }
};

#endif //ATTA1_ACCEPTOR_H

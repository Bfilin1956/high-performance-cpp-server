#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>
#include <array>
#include <atomic>

class Session;

static constexpr size_t MAX_BUCKETS_ = 16; //64 || 128 for 50k

class ConnectionManager {
    struct Bucket_ {
        std::mutex mtx;
        std::unordered_map<uint64_t, std::shared_ptr<Session>> clients_;
    };

    static ConnectionManager& instance() {
        static ConnectionManager inst;
        return inst;
    }
    Bucket_& find_bucket(const uint64_t id) {
        return buckets_[id % MAX_BUCKETS_];
    }

public:
    static void init() {
    }
    static uint64_t registerSession(std::shared_ptr<Session> session) {
        uint64_t id = instance().idCounter_.fetch_add(1, std::memory_order_relaxed);

        auto&[mtx, clients_] = instance().find_bucket(id);
        std::lock_guard<std::mutex> lock(mtx);
        clients_.emplace(id, std::move(session));
        return id;
    }
    static void deleteSession(const uint64_t id) {
        auto&[mtx, clients_] = instance().find_bucket(id);
        std::lock_guard<std::mutex> lock(mtx);
        clients_.erase(id);
    }
    static std::shared_ptr<Session> getSession(const uint64_t id) {
        auto&[mtx, clients_] = instance().find_bucket(id);
        std::lock_guard<std::mutex> lock(mtx);
        const auto it = clients_.find(id);
        return (it != clients_.end()) ? it->second : nullptr;
    }
private:
    std::array<Bucket_, MAX_BUCKETS_> buckets_;
    std::atomic<uint64_t> idCounter_{1};
};


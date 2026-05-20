//
// Created by r13x on 5/8/26.
//

#ifndef ATTA1_BUFFERPOOL_H
#define ATTA1_BUFFERPOOL_H

#include <vector>
#include <mutex>
#include <memory>

class BufferPool {
    std::vector<std::vector<std::byte>> pool_;
    std::mutex mtx_;
    const size_t buffer_size_ = 1024*2;

public:
    static BufferPool& instance() {
        static BufferPool inst;
        return inst;
    }

    std::vector<std::byte> acquire() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (pool_.empty()) {
            return std::vector<std::byte>(buffer_size_);
        }
        auto buf = std::move(pool_.back());
        pool_.pop_back();
        return buf;
    }

    void release(std::vector<std::byte>&& buf) {
        std::lock_guard<std::mutex> lock(mtx_);
        pool_.push_back(std::move(buf));
    }
};

#endif //ATTA1_BUFFERPOOL_H
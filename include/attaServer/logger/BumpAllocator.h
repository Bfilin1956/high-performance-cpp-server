//
// Created by r13x on 5/14/26.
//

#ifndef ATTA1_POOLALLOCATOR_H
#define ATTA1_POOLALLOCATOR_H

#include <atomic>
#include <cassert>
#include <cstddef>
#include <sys/mman.h>

class PollAllocator {
    void* addr_{};
    size_t capacity_{};
    alignas(64) std::atomic<size_t> offset_{};
public:
    explicit PollAllocator(size_t size) :
    capacity_(size) {
        addr_ = mmap(nullptr, capacity_, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE,0,0);
        if (addr_ == MAP_FAILED) addr_ = nullptr;
    }
    ~PollAllocator() {
        if (!addr_)return;
        munmap(addr_, capacity_);
    }
    void* alloc(size_t size, size_t align = 8) {
        assert((align & (align-1)) == 0);
        while (true) {
            size_t old = offset_.load(std::memory_order_relaxed);
            const size_t current = (old + align - 1) & ~(align - 1);
            const auto next = current + size;
            if (next > capacity_) return nullptr;
            if (offset_.compare_exchange_weak(old, next, std::memory_order_relaxed, std::memory_order_relaxed)) return static_cast<char *>(addr_) + current;
        }
    }
    void reset() {offset_.store(0, std::memory_order_release);}
    [[nodiscard]] bool empty() const {return offset_.load(std::memory_order_relaxed) == 0;}
    [[nodiscard]] bool isValid() const {return addr_ ? true : false;}
    [[nodiscard]] size_t used() const {return offset_.load(std::memory_order_relaxed);}
    [[nodiscard]] size_t remaining() const { return capacity_ - offset_.load(std::memory_order_relaxed);}
    [[nodiscard]] size_t size() const { return offset_.load(std::memory_order_relaxed);}
    [[nodiscard]] char* getAddrChar() const {return static_cast<char *>(addr_);}
};

#endif //ATTA1_POOLALLOCATOR_H
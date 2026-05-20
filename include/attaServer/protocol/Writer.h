//
// Created by r13x on 5/8/26.
//

#ifndef ATTA1_WRITER_H
#define ATTA1_WRITER_H
#include <cstdint>
#include <cstring>
#include <endian.h>
#include <span>
#include <stdexcept>
#include <string_view>
#include <netinet/in.h>

class Writer {
    char* addr_;
    uint32_t size_;
    uint32_t offset_{};

    template<typename T>
    static T ToNetwork(T x) {
        static_assert(std::is_integral_v<T>, "Integral required");
        if constexpr (sizeof(T) == 2) return static_cast<T>(htons(static_cast<uint16_t>(x)));
        else if constexpr (sizeof(T) == 4) return static_cast<T>(htonl(static_cast<uint32_t>(x)));
        else if constexpr (sizeof(T) == 8) return static_cast<T>(htobe64(static_cast<uint64_t>(x)));
        else static_assert(sizeof(T) == 2||sizeof(T) == 4||sizeof(T) == 8,"Unsupported integer size");
    }

public:
    Writer(char* addr, uint32_t size) : addr_(addr), size_(size){}

    template<typename T>
    void write(T value) {
        if (offset_ + sizeof(T) > size_) throw std::runtime_error("Write out of bounds");
        if constexpr (std::is_integral_v<T> && sizeof(T) > 1) value = ToNetwork(value);
        memcpy(addr_ + offset_, &value,sizeof(T));
        offset_ += sizeof(T);
    }

    void writeString(const std::string_view s) {
        uint32_t len = s.size();
        if (offset_ + len+ sizeof(uint32_t) > size_) throw std::runtime_error("Write out of bounds(string)");
        write<uint32_t>(len);
        memcpy(addr_+offset_, s.data(), len);
        offset_ += len;
    }

    void writeRaw(std::span<const std::byte> raw) {
        uint32_t len = raw.size();
        if (offset_ + len+ sizeof(uint32_t) > size_) throw std::runtime_error("Write out of bounds(string)");
        write<uint32_t>(len);
        memcpy(addr_+offset_, raw.data(), len);
        offset_ += len;
    }

    [[nodiscard]] void* data() const {return addr_;}
    [[nodiscard]] uint32_t size() const {return offset_;}
    [[nodiscard]] std::span<const std::byte> span() const {return {reinterpret_cast<const std::byte *>(addr_), size_};}
};

#endif //ATTA1_WRITER_H
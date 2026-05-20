//
// Created by r13x on 5/8/26.
//

#ifndef ATTA1_READER_H
#define ATTA1_READER_H

#include <cstring>
#include <netinet/in.h>

class Reader {
    char* addr_;
    uint32_t offset_{};
    uint32_t size_{};

    template<typename T>
    static T FromNetwork(T x) {
        static_assert(std::is_integral_v<T>, "Integral required");
        if constexpr (sizeof(T) == 2) return static_cast<T>(ntohs(static_cast<uint16_t>(x)));
        else if constexpr (sizeof(T) == 4) return static_cast<T>(ntohl(static_cast<uint32_t>(x)));
        else if constexpr (sizeof(T) == 8) return static_cast<T>(be64toh(static_cast<uint64_t>(x)));
        else static_assert(sizeof(T) == 2||sizeof(T) == 4||sizeof(T) == 8,"Unsupported integer size");
    }

public:
    explicit Reader(char* addr, const uint32_t size) : addr_(addr), size_(size){}

    template<typename T>
    T read() {
        if (offset_ + sizeof(T) > size_) throw std::runtime_error("Out of bounds");
        T val;
        memcpy(&val, addr_+offset_, sizeof(T));
        offset_ += sizeof(T);
        if constexpr (std::is_integral_v<T> && sizeof(T) > 1)
            return FromNetwork(val);
        return val;
    }

    std::string_view readString(const uint32_t len) {
        if (offset_ + len > size_) throw std::runtime_error("String out of bounds");
        offset_ += len;
        return {addr_ + offset_ - len,len};
    }
    std::string_view readString() {
        auto* ptr = addr_ + offset_;
        const auto remaining = size_ - offset_;
        auto* end = static_cast<char *>(memchr(ptr, '\0', remaining));
        if (!end) throw std::runtime_error("Null terminator not found");
        const size_t len = end - ptr;
        offset_ += len + 1;
        return {ptr, len};
    }
};

#endif //ATTA1_READER_H
#pragma once
#include <cstdint>

#include <attaServer/protocol/ProtocolHeader.h>

class Protocol {
    template<typename T>
    static T FromNetwork(T x) {
        static_assert(std::is_integral_v<T>, "Integral required");
        if constexpr (sizeof(T) == 2) return static_cast<T>(ntohs(static_cast<uint16_t>(x)));
        else if constexpr (sizeof(T) == 4) return static_cast<T>(ntohl(static_cast<uint32_t>(x)));
        else if constexpr (sizeof(T) == 8) return static_cast<T>(be64toh(static_cast<uint64_t>(x)));
        else static_assert(sizeof(T) == 2||sizeof(T) == 4||sizeof(T) == 8,"Unsupported integer size");
    }

    template<typename T>
    static T ToNetwork(T x) {
        static_assert(std::is_integral_v<T>, "Integral required");
        if constexpr (sizeof(T) == 2) return static_cast<T>(htons(static_cast<uint16_t>(x)));
        else if constexpr (sizeof(T) == 4) return static_cast<T>(htonl(static_cast<uint32_t>(x)));
        else if constexpr (sizeof(T) == 8) return static_cast<T>(htobe64(static_cast<uint64_t>(x)));
        else static_assert(sizeof(T) == 2||sizeof(T) == 4||sizeof(T) == 8,"Unsupported integer size");
    }

public:
    static bool isValid(const Header* header) {
        if (header->magic != magic_) return false;
        if (header->size > maxSize_) return false;
        return true;
    }

    static void ReadHeader(Header* header) {
        header->magic = FromNetwork(header->magic);
        header->version = FromNetwork(header->version);
        header->type = FromNetwork(header->type);
        header->size = FromNetwork(header->size);
        header->flags = FromNetwork(header->flags);
    }

    static void WriteHeader(Header* header, const uint16_t type, const uint32_t size,const uint16_t flags)
    {
        header->magic = magic_;
        header->version = version_;
        header->type = type;
        header->size = size;
        header->flags = flags;
    }
};

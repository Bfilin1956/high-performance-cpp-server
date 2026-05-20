//
// Created by r13x on 5/18/26.
//

#ifndef ATTA1_PROTOCOLHEADER_H
#define ATTA1_PROTOCOLHEADER_H

#include <cstdint>

constexpr uint32_t magic_{0x43525831};
constexpr uint32_t version_{1};
constexpr uint32_t maxSize_{1024*1024};

#pragma pack(push, 1)
struct Header {
    uint32_t magic;      // protocol id / validation
    uint16_t version;    // protocol version
    uint16_t type;       // packet opcode/type
    uint32_t size;       // payload size
    uint32_t flags;      // compression/encryption/etc
};
#pragma pack(pop)
static_assert(sizeof(Header) == 16);

#endif //ATTA1_PROTOCOLHEADER_H
#pragma once
#include <cstdint>
#include <vector>
#include <enet/enet.h>
struct NetPacket {
    uint64_t server_time;      // 服务器收到包的时间
    ENetPeer* peer;            // 客户端
    std::vector<uint8_t> data; // 原始字节
};

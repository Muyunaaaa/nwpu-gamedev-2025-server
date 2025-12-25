#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include <enet/enet.h>
#include "network/NetChannel.h"
using ClientID = size_t;

enum class NetPacketType : uint8_t {
    Connect,
    Disconnect,
    Packet
};

struct RecvPacket {
    NetPacketType type;
    ClientID client;
    NetChannel channel; // 信道
    uint64_t recv_time_ns; // 单调时间
    std::vector<uint8_t> payload;
};

struct SendPacket {
    ClientID client;
    NetChannel channel; // 信道
    bool reliable;
    std::span<const uint8_t> data;//这里传入内存视图，限制拷贝和修改
    SendPacket() = default;
    SendPacket(
        ClientID c,
        NetChannel ch,
        std::span<const uint8_t> d,
        bool r = true
    ): client(c), channel(ch), reliable(r), data(d) {
    }
};

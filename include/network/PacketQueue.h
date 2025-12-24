//
// Created by 冯于洋 on 25-12-20.
//

#pragma once
#include <queue>
#include "network/NetPacket.h"


class PacketQueue {
private:
    std::queue<NetPacket> packets;

public:
    PacketQueue() = default;

    ~PacketQueue() = default;

    std::queue<NetPacket>& getPackets() {
        return packets;
    }

    std::queue<NetPacket> getPackets() const {
        return packets;
    }

    void updateAndClear();
};

//
// Created by 冯于洋 on 25-12-22.
//

#ifndef COMMON_H
#define COMMON_H
// common.cpp
#include "network/NetPacket.h"
#include "network/PacketQueue.h"

PacketQueue PollNetwork(ENetHost* host);
void parseAndClassify(NetPacket net_packet);
#endif //COMMON_H

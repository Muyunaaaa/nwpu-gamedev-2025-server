//
// Created by 冯于洋 on 25-12-20.
//

#include "enet/enet.h"
#include "network/NetPacket.h"
#include "util/time/getTime.h"
#include "network/PacketQueue.h"
#include "network/common.h"

//将此时在信道缓冲区的所有数据包接受并放入PacketQueue中
//这里仅仅处理游戏中的数据包，不处理连接断开等其他事件
PacketQueue PollNetwork(ENetHost* host) {
    ENetEvent event;
    PacketQueue packet_queue;
    while (enet_host_service(host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            NetPacket pkt;
            pkt.server_time = now_ms();
            pkt.peer = event.peer;
            pkt.data.assign(
                event.packet->data,
                event.packet->data + event.packet->dataLength
            );

            packet_queue.getPackets().push(pkt);

            enet_packet_destroy(event.packet);
        }
    }
    return packet_queue;
}

/*
 * TODO:解析并分类数据包，并执行相关句柄
 */
void parseAndClassify(NetPacket net_packet) {

}
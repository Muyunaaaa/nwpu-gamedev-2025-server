//
// Created by 冯于洋 on 25-12-20.
//

#include "enet/enet.h"
#include "network/NetPacket.h"
#include "util/getTime.h"
#include "network/NetWorkService.h"
#include "spdlog/spdlog.h"

int myu::NetWork::move_packet_sequence_max = -1;
int myu::NetWork::fire_packet_sequence_max = -1;

void myu::NetWork::init() {
    if (enet_initialize() != 0) {
        spdlog::error("无法初始化 ENet！");
        throw std::runtime_error("无法初始化 ENet！");
    }
    host = enet_host_create(
        &address,
        Config::network::MAX_CLIENTS,
        Config::network::MAX_EACH_CLIENT_CHANNELS,
        0,
        0
    );
}

void myu::NetWork::pollAllPackets() {
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: {
                ClientID id = static_cast<ClientID>(
                    reinterpret_cast<uintptr_t>(event.peer->data)
                );
                RecvPacket pkt;
                pkt.type = NetPacketType::Packet;
                pkt.client = id;
                pkt.channel = static_cast<NetChannel>(event.channelID);
                pkt.recv_time_ns = time::now_ns();

                pkt.payload.assign(
                    event.packet->data,
                    event.packet->data + event.packet->dataLength
                );

                spdlog::info("收到来自客户端 {} 的数据包，长度为 {} 字节", id, pkt.payload.size());
                recv_packet_queue.enqueue(std::move(pkt));

                enet_packet_destroy(event.packet);
                break;
            }

            case ENET_EVENT_TYPE_CONNECT: {
                ClientID id = nextClientId++;
                peerToId[event.peer] = id;
                idToPeer[id] = event.peer;

                event.peer->data = reinterpret_cast<void *>(
                    static_cast<uintptr_t>(id)
                );
                spdlog::info("客户端 {} 已连接", id);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT: {
                ClientID id = reinterpret_cast<uintptr_t>(event.peer->data);
                recv_packet_queue.enqueue(RecvPacket{
                    .type = NetPacketType::Disconnect,
                    .client = id,
                    .recv_time_ns = time::now_ns()
                });
                peerToId.erase(event.peer);
                idToPeer.erase(id);
                event.peer->data = nullptr;
                break;
            }
        }
    }
}


void myu::NetWork::startNetworkThread() {
    if (running.load()) {
        spdlog::warn("网络线程已在运行中");
        return;
    }

    if (!host) {
        spdlog::error("ENetHost 未初始化，是否忘记调用 init() ?");
        throw std::runtime_error("ENetHost is null");
    }

    running.store(true);

    networkThread = std::thread([this]() {
        spdlog::info("网络线程启动");

        while (running.load(std::memory_order_relaxed)) {
            pollAllPackets();
            sendAllPackets();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }

        spdlog::info("网络线程退出");
    });
}


void myu::NetWork::stopNetworkThread() {
    if (!running.exchange(false)) {
        return;
    }

    if (networkThread.joinable()) {
        networkThread.join();
    }

    spdlog::info("网络线程已停止");
}


myu::NetWork::~NetWork() {
    stopNetworkThread();

    if (host) {
        enet_host_destroy(host);
        host = nullptr;
    }

    enet_deinitialize();
}

bool myu::NetWork::popPacket(RecvPacket &out) {
    return recv_packet_queue.try_dequeue(out);
}

bool myu::NetWork::pushPacket(const SendPacket &in) {
    return send_packet_queue.enqueue(in);
}

bool myu::NetWork::_sendOnePacket(const SendPacket &packet) {
    auto it = idToPeer.find(packet.client);
    if (it == idToPeer.end()) {
        spdlog::warn("尝试向未知客户端 {} 发送数据包", packet.client);
        return false;
    }
    ENetPeer *peer = it->second;

    enet_uint32 flags = packet.reliable
                            ? ENET_PACKET_FLAG_RELIABLE
                            : 0;

    ENetPacket *enet_packet = enet_packet_create(
        packet.data.data(),
        packet.data.size(),
        flags
    );

    if (!enet_packet) {
        spdlog::error("enet_packet_create 失败");
        return false;
    }

    int ret = enet_peer_send(
        peer,
        static_cast<enet_uint8>(packet.channel),
        enet_packet
    );

    if (ret != 0) {
        spdlog::error("enet_peer_send 失败，client={}", packet.client);
        enet_packet_destroy(enet_packet);
        return false;
    }

    return true;
}

bool myu::NetWork::sendAllPackets() {
    SendPacket packet;
    bool sent = false;
    while (send_packet_queue.try_dequeue(packet)) {
        _sendOnePacket(packet);
        sent = true;
    }
    enet_host_flush(host);
    return sent;
}

//这里的广播只是将发送的数据包压入队列，必须要使用sendAllPacket才能真正发送
bool myu::NetWork::broadcast(const SendPacket &packet) {
    bool ok = true;

    for (const auto &[clientId, peer]: idToPeer) {
        SendPacket pkt = packet;
        pkt.client = clientId;
        if (!pushPacket(pkt)) {
            spdlog::error("向客户端 {} 广播数据包失败", clientId);
            ok = false;
        }
    }
    return ok;
}

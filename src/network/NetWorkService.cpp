//
// Created by 冯于洋 on 25-12-20.
//

#include "enet/enet.h"
#include "network/NetPacket.h"
#include "util/getTime.h"
#include "network/NetWorkService.h"
#include "spdlog/spdlog.h"


void myu::NetWork::init() {
    if (enet_initialize() != 0) {
        spdlog::error("无法初始化 ENet！");
        throw std::runtime_error("无法初始化 ENet！");
    }
    host = enet_host_create(
        &address,
        MAX_CLIENTS,
        MAX_EACH_CLIENT_CHANNELS,
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

                packet_queue.enqueue(std::move(pkt));

                enet_packet_destroy(event.packet);
                spdlog::info("收到来自客户端 {} 的数据包，长度为 {} 字节", id, pkt.payload.size());
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
                packet_queue.enqueue(RecvPacket{
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
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            ); // 防止空转吃满 CPU
        }

        spdlog::info("网络线程退出");
    });
}


void myu::NetWork::stopNetworkThread() {
    if (!running.exchange(false)) {
        return; // 本来就没在跑
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
    //TODO: 实现出队列
}

bool myu::NetWork::sendPacket(const SendPacket &packet) {
    //TODO: 实现发送数据包
}

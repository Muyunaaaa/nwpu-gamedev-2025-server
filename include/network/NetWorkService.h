//
// Created by 冯于洋 on 25-12-22.
//

#pragma once
#include <unordered_map>

#include "network/NetPacket.h"
#include "concurrentqueue.h"

namespace myu {
    struct NetWork {
        static constexpr int SERVER_PORT = 1234;
        static constexpr int SERVER_HOST = ENET_HOST_ANY;
        static constexpr int MAX_CLIENTS = 32;
        static constexpr int MAX_EACH_CLIENT_CHANNELS = 2;

        moodycamel::ConcurrentQueue<RecvPacket> packet_queue;

        std::unordered_map<ClientID, ENetPeer*> idToPeer;
        std::unordered_map<ENetPeer*, ClientID> peerToId;

        ClientID nextClientId = 1;

        ENetAddress address = {
            .host = SERVER_HOST,
            .port = SERVER_PORT
        };
        ENetHost *host = nullptr;

        std::thread networkThread;
        std::atomic<bool> running{false};

        NetWork() = default;
        ~NetWork();

        void init();
        //开一个网络线程
        void startNetworkThread();
        void stopNetworkThread();
        //轮询所有数据包，将数据包放在网络队列中
        void pollAllPackets();
        bool popPacket(RecvPacket& out);
        bool sendPacket(const SendPacket& packet);
    };
}

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

        moodycamel::ConcurrentQueue<RecvPacket> recv_packet_queue;
        moodycamel::ConcurrentQueue<SendPacket> send_packet_queue;

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
        void startNetworkThread();
        void stopNetworkThread();
        void pollAllPackets();
        bool popPacket(RecvPacket& out);
        bool pushPacket(const SendPacket& packet);
        bool _sendOnePacket(const SendPacket& packet);
        bool sendAllPackets();
        bool broadcast(const SendPacket& packet);
    };
}

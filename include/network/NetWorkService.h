//
// Created by 冯于洋 on 25-12-22.
//

#pragma once
#include <unordered_map>

#include "network/NetPacket.h"
#include "concurrentqueue.h"

namespace myu {
    struct NetWork {
        // 1. 私有化构造函数
    private:
        NetWork() = default;
        ~NetWork();

        // 2. 禁用拷贝构造和赋值操作
    public:
        NetWork(const NetWork&) = delete;
        NetWork& operator=(const NetWork&) = delete;
        NetWork(NetWork&&) = delete;
        NetWork& operator=(NetWork&&) = delete;

        // 3. 提供全局唯一的访问点
        static NetWork& getInstance() {
            static NetWork instance; // 静态局部变量，线程安全且仅初始化一次
            return instance;
        }

        // --- 以下是原本的成员变量和方法 ---
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

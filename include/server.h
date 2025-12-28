#pragma once

#include "network/NetWorkService.h"
#include <atomic>

#include "protocol/receiveGamingPacket_generated.h"

class Server {
public:
    static Server& instance() {
        static Server instance;
        return instance;
    }

    void start();
    void run();
    void stop();

    void onClientConnect(ClientID client);
    void onClientDisconnect(ClientID client);
    void onClientPacket(const RecvPacket &pkt);
    void dispatchNetMessage(ClientID client, const myu::net::NetMessage *msg);

    size_t getTick() const;
private:
    Server() = default;
    ~Server() = default;
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;


    std::atomic<bool> running{false};
    std::atomic<size_t> tick{0};
};

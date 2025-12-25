#pragma once

#include "network/NetWorkService.h"
#include <atomic>

#include "protocol/receiveGamingPacket_generated.h"

class Server {
public:
    void start();
    void run();
    void stop();
    void onClientConnect(ClientID client);
    void onClientDisconnect(ClientID client);
    void onClientPacket(const RecvPacket &pkt);
    void dispatchNetMessage(ClientID client,const myu::net::NetMessage *msg);

private:
    std::atomic<bool> running{false};
    std::atomic<size_t> tick{0};
};

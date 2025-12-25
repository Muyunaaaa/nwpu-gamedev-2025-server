#include "Server.h"
#include "spdlog/spdlog.h"
#include <thread>

#include "flatbuffers/verifier.h"
#include "protocol/receiveGamingPacket_generated.h"

void Server::start() {
    network.init();
    network.startNetworkThread();
    running = true;
}

void Server::run() {
    spdlog::info("Server main loop started");

    using clock = std::chrono::steady_clock;
    constexpr auto tick_dt = std::chrono::milliseconds(16);

    auto next_tick = clock::now();

    while (running) {
        next_tick += tick_dt;

        RecvPacket pkt;
        switch (pkt.type) {
            case NetPacketType::Connect:
                onClientConnect(pkt.client);
                break;

            case NetPacketType::Disconnect:
                onClientDisconnect(pkt.client);
                break;

            case NetPacketType::Packet:
                onClientPacket(pkt);
                break;
        }

        std::this_thread::sleep_until(next_tick);
    }
}

void Server::stop() {
    running = false;
    network.stopNetworkThread();
}

void Server::onClientConnect(ClientID client) {
    //TODO:需要将连接数据包解包
    spdlog::info("Client {} connected", client);
}

void Server::onClientDisconnect(ClientID client) {
    spdlog::info("Client {} disconnected", client);
}

void Server::onClientPacket(const RecvPacket &pkt) {
    if (pkt.payload.empty()) {
        spdlog::warn("Empty packet from client {}", pkt.client);
        return;
    }

    flatbuffers::Verifier verifier(
        pkt.payload.data(),
        pkt.payload.size()
    );

    if (!myu::net::VerifyNetMessageBuffer(verifier)) {
        spdlog::warn("Invalid NetMessage from client {}", pkt.client);
        return;
    }

    const myu::net::NetMessage *msg =
        myu::net::GetNetMessage(pkt.payload.data());

    dispatchNetMessage(pkt.client, msg);
}

void Server::dispatchNetMessage(
    ClientID client,
    const myu::net::NetMessage *msg
) {
    const auto *header = msg->header();

    if (!header) {
        spdlog::warn("Packet without header from {}", client);
        return;
    }

    switch (header->type()) {
        case myu::net::PacketType::PacketType_MOVE: {
            //handleMove(client, msg->packet_as_MovePacket());
            break;
        }

        case myu::net::PacketType::PacketType_FIRE: {
            //handleFire(client, msg->packet_as_FirePacket());
            break;
        }

        case myu::net::PacketType::PacketType_PLAYER_STATE: {
            //handlePlayerState(client, msg->packet_as_PlayerStatePacket());
            break;
        }
        case myu::net::PacketType::PacketType_SYNC_REQ: {
            //handleSyncReq(client, msg);
            break;
        }

        default:
            spdlog::warn("Unknown PacketType {} from {}",
                         int(header->type()), client);
        break;
    }
}
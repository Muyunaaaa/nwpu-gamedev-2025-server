#include "Server.h"
#include "spdlog/spdlog.h"
#include <thread>

#include "flatbuffers/verifier.h"
#include "protocol/receiveGamingPacket_generated.h"
#include "state/MatchController.h"
#include "state/RoomContext.h"
#include "state/WaitState.h"
#include "state/WarmupState.h"

void Server::start() {
    network.init();
    network.startNetworkThread();
    running = true;
}

void Server::run() {
    spdlog::info("Server main loop started");

    MatchController matchController;
    //FIXME:MEMLEAK
    matchController.ChangeState(new WaitState());

    //TODO:tick计算逻辑需要调整,需要tick计数器
    using clock = std::chrono::steady_clock;
    constexpr auto tick_dt = std::chrono::milliseconds(16);

    auto next_tick = clock::now();

    while (running) {
        next_tick += tick_dt;
        RecvPacket pkt;
        //TODO:这里要防止一直循环，需要根据实际情况限制获取数据包数量
        //处理数据包
        while (network.popPacket(pkt)) {
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
        }

        //FIXME:更新状态机状态
        matchController.Tick(0.016f); // 固定时间步长为16ms

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
    const auto *payload = msg->packet_as_FirePacket();

    if (!header) {
        spdlog::warn("Packet without header from {}", client);
        return;
    }

    switch (msg->packet_type()) {
        case myu::net::PacketUnion::PacketUnion_MovePacket: {
            //handleMove(client, msg->packet_as_MovePacket());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_FirePacket: {
            //handleFire(client, msg->packet_as_FirePacket());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_PlayerStatePacket: {
            //handlePlayerState(client, msg->packet_as_PlayerStatePacket());
            break;
        }
        case myu::net::PacketUnion::PacketUnion_PlayerInfo: {
            const auto *info = msg->packet_as_PlayerInfo();

            uint32_t player_id = client; // 假设 client 是唯一的 ClientID
            std::string player_name = info->name()->c_str();
            bool is_ready = info->ready();

            auto &room = RoomContext::getInstance();

            auto it = room.players.find(player_id);
            if (it == room.players.end()) {
                Team assigned_team = room.getTeamWithLessPlayers();

                PlayerInfo new_player;
                new_player.id = player_id;
                new_player.name = player_name;
                new_player.isReady = is_ready;
                new_player.team = assigned_team;

                room.players[player_id] = new_player;

                spdlog::info("New player joined: {} (ID: {}) assigned to team {}",
                             player_name, player_id, (int) assigned_team);
            } else {
                it->second.isReady = is_ready;
                it->second.name = player_name;

                spdlog::debug("Player {} (ID: {}) updated ready status to {}",
                              player_name, player_id, is_ready);
            }
            break;
        }

        default:
            spdlog::warn("Unknown PacketType {} from {}",
                         int(msg->packet_type()), client);
            break;
    }
}

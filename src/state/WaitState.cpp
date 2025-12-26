#include "state/WaitState.h"

#include "Server.h"
#include "state/GameState.h"
#include "state/MatchController.h"
#include "state/RoomContext.h"
#include "state/WarmupState.h"

#include "protocol/Main_generated.h"
#include "protocol/PlayerInfo_generated.h"

#include "util/generateUUID.h"
#include "util/getTime.h"

void WaitState::OnEnter(MatchController *controller) {
    // controller->BroadcastMessage("Waiting for players to join...");
    spdlog::info("Waiting for players to join...");
}

void WaitState::Update(MatchController *controller, float deltaTime) {
    auto &roomCtx = RoomContext::getInstance();

    // 玩家刚加入时的广播
    for (auto id : roomCtx.players_just_joined) {
        spdlog::info("player {} joined the room, broadcasting", roomCtx.players[id].name);

        flatbuffers::FlatBufferBuilder fbb;

        auto playerJoinedInfo = moe::net::CreatePlayerJoinedEvent(
            fbb,
            fbb.CreateString(""), // uuid 目前未设计，留空
            fbb.CreateString(roomCtx.players[id].name.c_str()),
            static_cast<uint16_t>(id)
        );

        auto event = moe::net::CreateGameEvent(
            fbb,
            moe::net::EventData::EventData_PlayerJoinedEvent,
            playerJoinedInfo.Union()
        );

        auto header = moe::net::CreateReceivedHeader(
            fbb,
            Server::instance().getTick(),
            myu::time::now_ms()
        );

        auto msg = moe::net::CreateReceivedNetMessage(
            fbb,
            header,
            moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
            event.Union()
        );

        fbb.Finish(msg);

        controller->BroadcastMessage(fbb.GetBufferSpan());
    }

    roomCtx.players_just_joined.clear();

    // 人数已满，进入 Warmup
    if (roomCtx.getReadyCount() == RoomContext::TARGET_PLAYERS) {

        controller->ChangeState(std::make_unique<WarmupState>());

        struct PlayerSnapshot {
            uint16_t temp_id;
            std::string name;
            moe::net::PlayerTeam team;
        };

        std::vector<PlayerSnapshot> snapshots;
        snapshots.reserve(roomCtx.players.size());

        for (const auto &[id, p] : roomCtx.players) {
            snapshots.push_back(PlayerSnapshot{
                static_cast<uint16_t>(id),
                p.name,
                static_cast<moe::net::PlayerTeam>(p.team)
            });
        }

        for (const auto &self : snapshots) {

            flatbuffers::FlatBufferBuilder fbb;

            std::vector<flatbuffers::Offset<moe::net::PlayerInfo>> fb_players;
            fb_players.reserve(snapshots.size());

            flatbuffers::Offset<moe::net::PlayerInfo> whoami;

            for (const auto &p : snapshots) {

                auto pi = moe::net::CreatePlayerInfo(
                    fbb,
                    fbb.CreateString("none uuid"),
                    fbb.CreateString(p.name.c_str()),
                    p.temp_id,
                    p.team
                );

                fb_players.push_back(pi);

                if (p.temp_id == self.temp_id) {
                    whoami = pi;
                }
            }

            auto players_vec = fbb.CreateVector(fb_players);

            auto all_player_info = moe::net::CreateAllPlayerInfo(
                fbb,
                players_vec,
                whoami
            );

            auto game_started_event =
                moe::net::CreateGameStartedEvent(fbb, all_player_info);

            auto header = moe::net::CreateReceivedHeader(
                fbb,
                Server::instance().getTick(),
                myu::time::now_ms()
            );

            auto event = moe::net::CreateGameEvent(
                fbb,
                moe::net::EventData::EventData_GameStartedEvent,
                game_started_event.Union()
            );

            auto msg = moe::net::CreateReceivedNetMessage(
                fbb,
                header,
                moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
                event.Union()
            );

            fbb.Finish(msg);


            //TODO:解决临时id和clientID的对应，这里先不考虑重连的问题
            ClientID client_id = static_cast<ClientID>(self.temp_id);
            SendPacket send_packet(
                client_id,
                NetChannel::CH_RELIABLE,
                std::span<const uint8_t>(fbb.GetBufferPointer(), fbb.GetSize()),
                true
            );

            myu::NetWork::getInstance().pushPacket(send_packet);
        }
    }
}

void WaitState::OnExit(MatchController *controller) {
    //spdlog::warn("等待阶段不应该从外部退出");
}

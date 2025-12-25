#include "state/WaitState.h"

#include "state/GameState.h"
#include "state/MatchController.h"
#include "state/RoomContext.h"
#include "state/WarmupState.h"

#include "protocol/Main_generated.h"

void WaitState::OnEnter(MatchController *controller){
    // controller->BroadcastMessage("Waiting for players to join...");
    spdlog::info("Waiting for players to join...");
}

void WaitState::Update(MatchController *controller, float deltaTime) {
    // todo: server tick and timestamp
    //!!!!!!!!! urgent fix

    auto& roomCtx = RoomContext::getInstance();
    for (auto id : roomCtx.players_just_joined) {
        spdlog::info("player {} joined the room, broadcasting", roomCtx.players[id].name);
        flatbuffers::FlatBufferBuilder fbb;
        auto playerJoinedInfo = moe::net::CreatePlayerJoinedEvent(
            fbb,
            fbb.CreateString(""),
            fbb.CreateString(roomCtx.players[id].name.c_str()),
            id
        );
        // fuck microsoft
        auto event = moe::net::CreateGameEvent(
            fbb,
            moe::net::EventData::EventData_PlayerJoinedEvent,
            playerJoinedInfo.Union()
        );

        auto header = moe::net::CreateReceivedHeader(
            fbb, 0, 0
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
    // important !!

    if (RoomContext::getInstance().getReadyCount() == RoomContext::TARGET_PLAYERS) {
        controller->ChangeState(new WarmupState());
        spdlog::info("玩家人数已满，准备进入购买阶段");
    }
}

void WaitState::OnExit(MatchController *controller){
    //spdlog::warn("等待阶段不应该从外部退出");
}

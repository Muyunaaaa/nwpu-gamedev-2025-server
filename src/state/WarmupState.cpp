#include "state/GameState.h"
#include "state/MatchController.h"
#include "state/WarmupState.h"

#include "Server.h"
#include "core/GameContext.h"
#include "protocol/Events_generated.h"
#include "protocol/Main_generated.h"
#include "spdlog/spdlog.h"
#include "state/RoomContext.h"
#include "state/RoundState.h"
#include "util/getTime.h"


void WarmupState::OnEnter(MatchController *ctrl) {
    timerMs = Config::match::PURCHASE_TIMER.count();
    for (auto& player : GameContext::Instance().Players()) {
        flatbuffers::FlatBufferBuilder fbb;
        uint16_t round_number = ctrl->currentRound;
        auto purchase_event = moe::net::CreateRoundPurchaseStartedEvent(
            fbb,
            round_number,
            player.second.money
            );
        auto event = moe::net::CreateGameEvent(
            fbb,
            moe::net::EventData::EventData_RoundPurchaseStartedEvent,
            purchase_event.Union()
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
        SendPacket packet = SendPacket(player.first, CH_RELIABLE, fbb.GetBufferSpan(), true);
        myu::NetWork::getInstance().pushPacket(packet);
    }

    spdlog::info("第{}局",ctrl->currentRound);
    spdlog::info("游戏开始，进入购买阶段(10s)");
    ctrl->enablePurchase();
    ctrl->disableFire();
    ctrl->disableMove();
}

void WarmupState::Update(MatchController *ctrl, float deltaTime) {
    timerMs -= deltaTime;
    if (timerMs <= 0) {
         ctrl->ChangeState(std::make_unique<RoundState>()); // 切换到战斗状态
    }
}

void WarmupState::OnExit(MatchController *ctrl) {
}

#include "state/MatchEndState.h"

#include "Server.h"
#include "core/GameContext.h"
#include "protocol/Main_generated.h"
#include "util/getTime.h"

void MatchEndState::OnEnter(MatchController *controller) {
    //校验是否应该在该阶段
    timerMs = MatchController::END_UNTIL_SERVER_CLOSE_TIMER.count();
    GameContext::Instance().Reset();
    moe::net::PlayerTeam winner_team;
    switch (winner) {
        case 0: {
            spdlog::error("MatchEndState entered but no winner set!");
            break;
        }
        case 1: {
            winner_team = moe::net::PlayerTeam::PlayerTeam_TEAM_CT;
            break;
        }
        case 2: {
            winner_team = moe::net::PlayerTeam::PlayerTeam_TEAM_T;
            break;
        }
        case 3: {
            winner_team = moe::net::PlayerTeam::PlayerTeam_TEAM_NONE;
            break;
        }
        default: {
            spdlog::error("错误的获胜方判断结果!");
        }
    }

    //消息发送
    flatbuffers::FlatBufferBuilder fbb;
    auto server_close_event = moe::net::CreateGameEndedEvent(
        fbb,
        fbb.CreateString("对局已结束，服务器将在15s后关闭"),
        winner_team
    );
    auto event = moe::net::CreateGameEvent(
        fbb,
        moe::net::EventData::EventData_GameEndedEvent,
        server_close_event.Union()
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

    if (winner == 3) {
        spdlog::info("平局");
    }else {
        spdlog::info("获胜方{}",winner);
    }
    spdlog::info("对局已结束，服务器将在15s后关闭");
    controller->disableFire();
    controller->disableMove();
    controller->disablePurchase();
}

void MatchEndState::Update(MatchController *controller, float deltaTime) {
    timerMs -= deltaTime;
    if (timerMs <= 0) {
        Server::instance().stop();
    }
}

void MatchEndState::OnExit(MatchController *controller) {
    //不会被执行
}

MatchEndState::MatchEndState(uint16_t winner) {
    this->winner = winner;
}



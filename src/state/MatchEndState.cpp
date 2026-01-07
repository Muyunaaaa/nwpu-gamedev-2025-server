#include "state/MatchEndState.h"

#include "config.h"
#include "server.h"
#include "core/GameContext.h"
#include "protocol/Main_generated.h"
#include "util/getTime.h"
#include "stats/ComputeStats.h"


void MatchEndState::OnEnter(MatchController *controller) {
    //校验是否应该在该阶段
    timerMs = Config::match::END_UNTIL_SERVER_CLOSE_TIMER.count();
    moe::net::PlayerTeam winner_team;
    PlayerTeam winner_teamL;
    switch (winner) {
        case 0: {
            spdlog::error("MatchEndState entered but no winner set!");
            break;
        }
        case 1: {
            winner_team = moe::net::PlayerTeam::PlayerTeam_TEAM_CT;
            winner_teamL = PlayerTeam::CT;
            break;
        }
        case 2: {
            winner_team = moe::net::PlayerTeam::PlayerTeam_TEAM_T;
            winner_teamL = PlayerTeam::T;
            break;
        }
        case 3: {
            winner_team = moe::net::PlayerTeam::PlayerTeam_TEAM_NONE;
            winner_teamL = PlayerTeam::NONE;
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

    //输出到csv
    ComputeStats::GetInstance().LoadFromPlayerStates(GameContext::Instance().Players());
    std::string filePath = "match_stats_" + std::to_string(myu::time::now_ms()) + ".csv";
    if (ComputeStats::GetInstance().ExportCSV(filePath,winner_teamL)) {
        spdlog::info("比赛统计数据已导出到 {}", filePath);
    } else {
        spdlog::error("比赛统计数据导出失败");
    }

    GameContext::Instance().Reset();
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



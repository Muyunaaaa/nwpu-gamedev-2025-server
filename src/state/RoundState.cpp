#include "state/RoundState.h"

#include "Server.h"
#include "core/GameContext.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "protocol/Events_generated.h"
#include "protocol/Main_generated.h"
#include "spdlog/spdlog.h"
#include "state/MatchController.h"
#include "state/MatchEndState.h"
#include "state/WarmupState.h"
#include "util/getTime.h"

//工具函数
void getWinnerAndBroadcastAndChangeState(MatchController *controller) {
    flatbuffers::FlatBufferBuilder fbb;
    PlayerTeam winner = controller->winner_team;
    uint16_t round_number = controller->currentRound;
    auto winner_event = moe::net::CreateRoundEndedEvent(
        fbb,
        round_number,
        parseToNetPlayerTeam(winner)
    );
    auto event = moe::net::CreateGameEvent(
        fbb,
        moe::net::EventData::EventData_RoundEndedEvent,
        winner_event.Union()
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
    if (controller->checkMatchWin() == 0) {
        //还未结束
        controller->ChangeState(std::make_unique<WarmupState>());
    }else if (controller->checkMatchWin() == 1) {
        //CT胜利
        controller->ChangeState(std::make_unique<MatchEndState>(1));
    }else if (controller->checkMatchWin() == 2) {
        //T胜利
        controller->ChangeState(std::make_unique<MatchEndState>(2));
    }else if (controller->checkMatchWin() == 3) {
        //T平局
        controller->ChangeState(std::make_unique<MatchEndState>(3));
    }else {
        spdlog::error("比赛胜负判断出现错误");
    }

}

//主要实现
void RoundState::OnEnter(MatchController *controller) {
    timerMs = Config::match::ROUND_TIMER.count();
    flatbuffers::FlatBufferBuilder fbb;
    uint16_t round_number = controller->currentRound;
    auto round_start_event = moe::net::CreateRoundPurchaseStartedEvent(fbb, round_number);
    auto event = moe::net::CreateGameEvent(
        fbb,
        moe::net::EventData::EventData_RoundStartedEvent,
        round_start_event.Union()
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
    spdlog::info("购买阶段结束，进入交火阶段");
    //权限控制
    controller->disablePurchase();
    controller->enableFire();
    controller->enableMove();

    //回合控制
    controller->initRound();
    controller->roundStart();
}

void RoundState::Update(MatchController *controller, float deltaTime) {
    if (controller->c4_planted) {
        if (!controller->c4_planted_and_counting) {
        spdlog::info("c4已安放，炸弹计时器开始");
            timerMs = Config::match::C4_TIMER.count();
            controller->c4_planted_and_counting = true;
        }
    }
    if (controller->c4_defused) {
        spdlog::info("C4已拆除，反恐精英获胜");
        controller->ctWin();
        getWinnerAndBroadcastAndChangeState(controller);
    }
    if (GameContext::Instance().countLifes(PlayerTeam::T) == 0
        && controller->c4_planted == false) {
        controller->ctWin();
        spdlog::info("恐怖分子全部阵亡，反恐精英获胜");
        getWinnerAndBroadcastAndChangeState(controller);
    }
    if (GameContext::Instance().countLifes(PlayerTeam::CT) == 0) {
        controller->tWin();
        spdlog::info("反恐精英全部阵亡，恐怖分子获胜");
        getWinnerAndBroadcastAndChangeState(controller);
    }
    timerMs -= deltaTime;
    if (timerMs <= 0) {
        if (controller->c4_planted) {
            controller->tWin();
            spdlog::info("C4爆炸，恐怖分子获胜");
            getWinnerAndBroadcastAndChangeState(controller);
        } else {
            controller->ctWin();
            spdlog::info("回合时间耗尽,反恐精英获胜");
            getWinnerAndBroadcastAndChangeState(controller);
        }
    }
}

void RoundState::OnExit(MatchController *controller) {
    spdlog::info("交火阶段结束，第{}回合结束",controller->currentRound);
    GameContext::Instance().flushShotRecords();
    GameContext::Instance().resetARound();
    controller->roundEnd();
    controller->resetRound();
    controller->currentRound++;
}

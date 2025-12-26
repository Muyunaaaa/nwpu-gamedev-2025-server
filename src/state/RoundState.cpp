#include "state/RoundState.h"

#include "Server.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "protocol/Events_generated.h"
#include "protocol/Main_generated.h"
#include "spdlog/spdlog.h"
#include "state/MatchController.h"
#include "state/MatchEndState.h"
#include "state/WaitState.h"
#include "util/getTime.h"

//工具函数
void broadcastPlanted(MatchController *controller) {
    flatbuffers::FlatBufferBuilder fbb;
    uint16_t planter_id = controller->planter_id;
    uint16_t bomb_site = controller->c4_planted_site;
    auto plant_event = moe::net::CreateBombPlantedEvent(
        fbb,
        planter_id,
        bomb_site
    );
    auto event = moe::net::CreateGameEvent(
        fbb,
        moe::net::EventData::EventData_BombPlantedEvent,
        plant_event.Union()
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

void broadcastDefused(MatchController *controller) {
    flatbuffers::FlatBufferBuilder fbb;
    uint16_t defuser_id = controller->defuser_id;
    auto defuse_event = moe::net::CreateBombDefusedEvent(
        fbb,
        defuser_id
    );
    auto event = moe::net::CreateGameEvent(
        fbb,
        moe::net::EventData::EventData_BombDefusedEvent,
        defuse_event.Union()
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

void getWinnerAndBroadcastAndChangeState(MatchController *controller) {
    flatbuffers::FlatBufferBuilder fbb;
    uint16_t winner = controller->winner_team;
    uint16_t round_number = controller->currentRound;
    auto winner_event = moe::net::CreateRoundEndedEvent(
        fbb,
        round_number,
        static_cast<moe::net::PlayerTeam>(winner)
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
    //TODO:这里还需要根据总局数判断是否结束比赛
    if (controller->checkMatchWin() == 0) {
        //还未结束
        controller->ChangeState(std::make_unique<WaitState>());
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
    timerMs = MatchController::ROUND_TIMER.count();
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
        spdlog::info("c4已安放，炸弹计时器开始");
        timerMs = MatchController::C4_TIMER.count();
        broadcastPlanted(controller);
    }
    if (controller->c4_defused) {
        spdlog::info("C4已拆除，反恐精英获胜");
        broadcastDefused(controller);
        controller->ctWin();
        getWinnerAndBroadcastAndChangeState(controller);
    }
    if (controller->ct_alive == 0) {
        controller->tWin();
        spdlog::info("反恐精英全部阵亡，恐怖分子获胜");
        getWinnerAndBroadcastAndChangeState(controller);
    }
    if (controller->t_alive == 0) {
        controller->ctWin();
        spdlog::info("恐怖分子全部阵亡，反恐精英获胜");
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
    controller->roundEnd();
    controller->resetRound();
    controller->currentRound++;
}

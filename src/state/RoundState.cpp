#include "state/RoundState.h"

#include "server.h"
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
    } else if (controller->checkMatchWin() == 1) {
        //CT胜利
        controller->ChangeState(std::make_unique<MatchEndState>(1));
    } else if (controller->checkMatchWin() == 2) {
        //T胜利
        controller->ChangeState(std::make_unique<MatchEndState>(2));
    } else if (controller->checkMatchWin() == 3) {
        //T平局
        controller->ChangeState(std::make_unique<MatchEndState>(3));
    } else {
        spdlog::error("比赛胜负判断出现错误");
    }
}

//主要实现
void RoundState::OnEnter(MatchController *controller) {
    timerMs = Config::match::ROUND_TIMER.count();
    //广播倒计时事件
    flatbuffers::FlatBufferBuilder count_down_fbb;
    auto count_down_event = moe::net::CreateCountDownEvent(
            count_down_fbb,
            static_cast<uint32_t>(timerMs)
        );
    auto event = moe::net::CreateGameEvent(
        count_down_fbb,
        moe::net::EventData::EventData_CountDownEvent,
        count_down_event.Union()
    );
    auto header = moe::net::CreateReceivedHeader(
        count_down_fbb,
        Server::instance().getTick(),
        myu::time::now_ms()
    );
    auto msg = moe::net::CreateReceivedNetMessage(
        count_down_fbb,
        header,
        moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
        event.Union()
    );
    count_down_fbb.Finish(msg);
    controller->BroadcastMessage(count_down_fbb.GetBufferSpan());

    uint16_t round_number = controller->currentRound;
    //给每个玩家发送枪械配置
    for (auto &player: GameContext::Instance().Players()) {
        flatbuffers::FlatBufferBuilder fbb;
        moe::net::Weapon primary_weapon = moe::net::Weapon::Weapon_WEAPON_NONE;
        moe::net::Weapon secondary_weapon = moe::net::Weapon::Weapon_WEAPON_NONE;
        WeaponInstance *primary = player.second.primary.get();
        WeaponInstance *secondary = player.second.secondary.get();
        if (primary) {
            primary_weapon = parseToNetWeapon(player.second.primary->config->weapon_id);
        }
        if (secondary) {
            secondary_weapon = parseToNetWeapon(player.second.secondary->config->weapon_id);
        }
        auto round_start_event = moe::net::CreateRoundStartedEvent(
            fbb,
            primary_weapon,
            secondary_weapon,
            round_number
        );
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
        SendPacket packet = SendPacket(player.first, CH_RELIABLE, fbb.GetBufferSpan(), true);
        myu::NetWork::getInstance().pushPacket(packet);
    }
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
            flatbuffers::FlatBufferBuilder count_down_fbb;
            //广播倒计时事件
            auto count_down_event = moe::net::CreateCountDownEvent(
                    count_down_fbb,
                    static_cast<uint32_t>(timerMs)
                );
            auto event = moe::net::CreateGameEvent(
                count_down_fbb,
                moe::net::EventData::EventData_CountDownEvent,
                count_down_event.Union()
            );
            auto header = moe::net::CreateReceivedHeader(
                count_down_fbb,
                Server::instance().getTick(),
                myu::time::now_ms()
            );
            auto msg = moe::net::CreateReceivedNetMessage(
                count_down_fbb,
                header,
                moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
                event.Union()
            );
            count_down_fbb.Finish(msg);
            controller->BroadcastMessage(count_down_fbb.GetBufferSpan());
        }
    }
    if (controller->c4_defused) {
        spdlog::info("C4已拆除，反恐精英获胜");
        controller->ctWin();
        getWinnerAndBroadcastAndChangeState(controller);
        return;
    }
    if (GameContext::Instance().countLifes(PlayerTeam::T) == 0
        && controller->c4_planted == false) {
        controller->ctWin();
        spdlog::info("恐怖分子全部阵亡，反恐精英获胜");
        getWinnerAndBroadcastAndChangeState(controller);
        return;
    }
    if (GameContext::Instance().countLifes(PlayerTeam::CT) == 0) {
        controller->tWin();
        spdlog::info("反恐精英全部阵亡，恐怖分子获胜");
        getWinnerAndBroadcastAndChangeState(controller);
        return;
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
    spdlog::info("交火阶段结束，第{}回合结束", controller->currentRound);
    // GameContext::Instance().flushShotRecords();
    GameContext::Instance().resetARound();
    controller->roundEnd();
    controller->resetRound();
    controller->currentRound++;
}

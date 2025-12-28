#include "state/MatchController.h"

#include "Server.h"
#include "core/GameContext.h"
#include "network/NetWorkService.h"
#include "protocol/receiveGamingPacket_generated.h"
#include "state/RoundState.h"
#include "state/WaitState.h"


void MatchController::ChangeState(std::unique_ptr<GameState> newState) {
    if (currentState) {
        currentState->OnExit(this);
    }
    currentState = std::move(newState);
    if (currentState) {
        currentState->OnEnter(this);
    }
}

void MatchController::BroadcastMessage(std::span<const uint8_t> msg) {
    SendPacket pkt(-1, CH_RELIABLE, msg, true);
    myu::NetWork::getInstance().broadcast(pkt);
}

void MatchController::Tick(float deltaTime) {
    if (currentState) {
        currentState->Update(this, deltaTime);
    }
    if (currentState!= nullptr && dynamic_cast<WaitState*>(currentState.get()) == nullptr) {
        GameContext::Instance().playerSnyc();
    }
}

void MatchController::enableFire() {
    a_fire = true;
}

void MatchController::disableFire() {
    a_fire = false;
}

bool MatchController::fire_able() const {
    return a_fire;
}


void MatchController::enableMove() {
    a_move = true;
}

void MatchController::disableMove() {
    a_move = false;
}

bool MatchController::move_able() const{
    return a_move;
}

void MatchController::enablePurchase() {
    a_purchase = true;
}

void MatchController::disablePurchase() {
    a_purchase = false;
}

bool MatchController::purchase_able() const{
    return a_purchase;
}

void MatchController::gameEnd() {
    game_over = true;
}

void MatchController::gameStart() {
    game_over = false;
}

void MatchController::plantC4(PlantSite _c4_plant_site) {
    c4_plant_site = _c4_plant_site;
    c4_planted = true;
}

void MatchController::roundStart() {
    round_running = true;
}

void MatchController::roundEnd() {
    round_running = false;
}

void MatchController::defuseC4() {
    c4_planted = false;
    c4_defused = true;
}

void MatchController::ctWin() {
    winner_team = PlayerTeam::CT;
    ct_win_round++;
}

void MatchController::tWin() {
    winner_team = PlayerTeam::T;
    t_win_round++;
}

void MatchController::initRound() {
    c4_plant_site = PlantSite::None;
    c4_defused = false;
    c4_planted = false;
    round_running = false;//初始化先不启动回合
    winner_team = PlayerTeam::NONE;
}

void MatchController::resetRound() {
    c4_plant_site = PlantSite::None;
    c4_defused = false;
    c4_planted = false;
    round_running = false;
    winner_team = PlayerTeam::NONE;
}

uint16_t MatchController::checkMatchWin() {
    int max_rounds = Config::room::MAX_ROUNDS;
    if (max_rounds % 2 != 0) {
        spdlog::error("Max rounds must be even");
    }
    int rounds_to_win = max_rounds / 2 + 1;
    if (ct_win_round >= rounds_to_win) {
        return 1;
    }else if (t_win_round >= rounds_to_win) {
        return 2;
    }else if (ct_win_round < rounds_to_win
        && t_win_round >= rounds_to_win && currentRound == max_rounds) {
        return 3;
    }else {
        return 0;
    }
}

bool MatchController::plant_able() const {
    //TODO:后续需要判断炸弹安放的位置，但我们这里先不管,我们这里认为安放炸弹的位置合理
    if (c4_plant_site == PlantSite::None) {
        spdlog::info("玩家试图在无效位置安放炸弹");
        return false;
    }
    //是否在交火阶段
    if (dynamic_cast<RoundState*>(currentState.get()) == nullptr) {
        return false;
    }
    //是否已经被安装炸弹
    if (c4_planted) {
        return false;
    }
    //不考虑是否被拆除，无意义
    return true;
    //假设客户端中只有T会发送请求，这里先不处理
}

bool MatchController::defuse_able() const {
    //TODO:后续需要判断炸弹安放的位置，但我们这里先不管,我们这里认为安放炸弹的位置合理
    //是否在交火阶段
    if (dynamic_cast<RoundState*>(currentState.get()) == nullptr) {
        return false;
    }
    //是否已经被安放炸弹
    if (!c4_planted) {
        return false;
    }
    return true;
}







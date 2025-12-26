#include "state/MatchController.h"

#include "network/NetWorkService.h"
#include "protocol/receiveGamingPacket_generated.h"

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
}

void MatchController::enableFire() {
    a_fire = true;
}

void MatchController::disableFire() {
    a_fire = false;
}

bool MatchController::fire_able() {
    return a_fire;
}

void MatchController::enableMove() {
    a_move = true;
}

void MatchController::disableMove() {
    a_move = false;
}

bool MatchController::move_able() {
    return a_move;
}

void MatchController::enablePurchase() {
    a_purchase = true;
}

void MatchController::disablePurchase() {
    a_purchase = false;
}

bool MatchController::purchase_able() {
    return a_purchase;
}

void MatchController::gameEnd() {
    game_over = true;
}

void MatchController::gameStart() {
    game_over = false;
}

void MatchController::plantC4() {
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
    winner_team = 1;
}

void MatchController::tWin() {
    winner_team = 2;
}

void MatchController::initRound() {
    players_alive = 4;
    ct_alive = 2;
    t_alive = 2;
    planter_id = -1;
    defuser_id = -1;
    c4_defused = false;
    c4_planted = false;
    round_running = false;//初始化先不启动回合
    winner_team = 0;
}

void MatchController::resetRound() {
    players_alive = 4;
    ct_alive = 2;
    t_alive = 2;
    planter_id = -1;
    defuser_id = -1;
    c4_defused = false;
    c4_planted = false;
    round_running = false;
    winner_team = 0;
}

void MatchController::killACt() {
    if (ct_alive > 0) {
        ct_alive--;
    }
}

void MatchController::killAT() {
    if (t_alive > 0) {
        t_alive--;
    }
}







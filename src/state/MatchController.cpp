#include "state/MatchController.h"

#include "network/NetWorkService.h"
#include "protocol/receiveGamingPacket_generated.h"

void MatchController::ChangeState(GameState *newState) {
    if (currentState) {
        currentState->OnExit(this);
        delete currentState;
    }
    currentState = newState;
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

void MatchController::CheckWinCondition() {

}


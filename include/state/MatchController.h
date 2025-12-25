#pragma once
#include <cstdint>
#include <span>
#include <string>
#include "state/GameState.h"

class MatchController {
private:
    GameState* currentState = nullptr;
    int currentRound = 1;
    int players_alive = 0;

public:
    void ChangeState(GameState* newState);

    void Tick(float deltaTime);
    void BroadcastMessage(std::span<const uint8_t> message);
    void CheckWinCondition();
    //TODO:补充其他功能
};

#pragma once
#include "MatchController.h"
#include "state/GameState.h"
class MatchEndState : public GameState {
private:
    float timerMs = MatchController::END_UNTIL_SERVER_CLOSE_TIMER.count();
public:
    uint16_t winner = 0;//0:未有胜利方(error) 1:CT胜利 2:T胜利 3:平局
    void OnEnter(MatchController *controller) override;
    void Update(MatchController *controller, float deltaTime) override;
    void OnExit(MatchController *controller) override;
    MatchEndState() = default;
    MatchEndState(uint16_t winner);
    ~MatchEndState() override = default;
};
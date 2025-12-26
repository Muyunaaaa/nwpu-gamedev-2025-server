#pragma once
#include "MatchController.h"
#include "state/GameState.h"
class RoundState : public GameState {
private:
    float timerMs = MatchController::ROUND_TIMER.count();
public:
    void OnEnter(MatchController *controller) override;
    void Update(MatchController *controller, float deltaTime) override;
    void OnExit(MatchController *controller) override;
    RoundState() = default;
    ~RoundState() override = default;
};

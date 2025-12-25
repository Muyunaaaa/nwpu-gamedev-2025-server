#include "state/GameState.h"
#include "state/MatchController.h"
#include "state/RoomContext.h"
#include "state/WarmupState.h"

class WaitState : public GameState {
public:
    void OnEnter(MatchController *controller) override {
        controller->BroadcastMessage("Waiting for players to join...");
    }
    void Update(MatchController *controller, float deltaTime) override {
        if (RoomContext::getInstance().getReadyCount() == RoomContext::TARGET_PLAYERS) {
            controller->ChangeState(new WarmupState());
        }
    }
    void OnExit(MatchController *controller) override {
        spdlog::warn("等待阶段不应该从外部退出");
    }
    WaitState() = default;
};

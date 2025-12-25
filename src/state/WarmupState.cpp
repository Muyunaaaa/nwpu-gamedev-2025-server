#include "state/GameState.h"
#include "state/MatchController.h"
class WarmupState : public GameState {
    float timer = 10.0f;
public:
    void OnEnter(MatchController* ctrl) override {
        timer = 10.0f;
        ctrl->BroadcastMessage("Round Start: Freeze Time (10s)");
        // ctrl->EnableBuying(true);  // 开启购买窗口
        // ctrl->LockPlayerMovement(true); // 锁定移动
    }

    void Update(MatchController* ctrl, float deltaTime) override {
        // timer -= deltaTime;
        // if (timer <= 0) {
        //     ctrl->ChangeState(new ActionState()); // 切换到战斗状态
        // }
    }

    void OnExit(MatchController* ctrl) override {
        // ctrl->EnableBuying(false); // 关闭购买
        // ctrl->LockPlayerMovement(false); // 允许移动
    }
};
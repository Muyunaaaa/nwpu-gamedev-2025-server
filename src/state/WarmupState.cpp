#include "state/GameState.h"
#include "state/MatchController.h"
#include "state/WarmupState.h"

#include "spdlog/spdlog.h"


void WarmupState::OnEnter(MatchController *ctrl){
    timer = 10.0f;
    // ctrl->BroadcastMessage("Round Start: Freeze Time (10s)");
    spdlog::info("Round Start: Freeze Time (10s)");
    // ctrl->EnableBuying(true);  // 开启购买窗口
    // ctrl->LockPlayerMovement(true); // 锁定移动
}

void WarmupState::Update(MatchController *ctrl, float deltaTime){
    // timer -= deltaTime;
    // if (timer <= 0) {
    //     ctrl->ChangeState(new ActionState()); // 切换到战斗状态
    // }
}

void WarmupState::OnExit(MatchController *ctrl){
    // ctrl->EnableBuying(false); // 关闭购买
    // ctrl->LockPlayerMovement(false); // 允许移动
}

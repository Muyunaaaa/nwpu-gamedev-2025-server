#pragma once
class MatchController;

class GameState {
public:
    virtual ~GameState() = default;
    virtual void OnEnter(MatchController* controller) = 0; // 进入状态
    virtual void Update(MatchController* controller, float deltaTime) = 0; // 每帧/每秒更新
    virtual void OnExit(MatchController* controller) = 0;  // 离开状态
};
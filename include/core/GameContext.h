#pragma once
#include <unordered_map>
#include <spdlog/spdlog.h>

#include "entity/PlayerState.h"

class GameContext {
private:
    GameContext() {
        spdlog::info("GameContext Initialized");
    }

public:
    static GameContext& Instance() {
        static GameContext instance;
        return instance;
    }

    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;

    void InitFromRoom();   // 从 RoomContext 拷贝
    //每一局重置状态
    void stateDetection();//用于检测玩家是否死亡
    void setPlayerDied(ClientID playerID);
    void reducePlayerHealth(ClientID playerID, float damage);
    void setPlayerAlive(ClientID playerID);
    void addPostionHistory(ClientID playerID, const myu::math::Vec3& position);
    void addmoneyToPlayer(ClientID playerID, int amount);
    void addKill(ClientID playerID);
    void addDeath(ClientID playerID);
    void addplant(ClientID playerID);
    void adddefuse(ClientID playerID);
    void resetARound();
    void Reset();          // 游戏结束

    PlayerState* GetPlayer(ClientID id);
    const std::unordered_map<ClientID, PlayerState>& Players() const;

private:
    std::unordered_map<ClientID, PlayerState> players_;
};

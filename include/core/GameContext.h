#pragma once
#include <unordered_map>
#include <spdlog/spdlog.h>

#include "entity/PlayerState.h"

class GameContext {
private:
    GameContext() {
        spdlog::info("GameContext Initialized");
    }

    void setPlayerDied(ClientID playerID);
    void addMoneyToPlayer(ClientID playerID, int amount);
    void addKillAndReward(ClientID playerID);
    void addDeath(ClientID playerID);
public:
    static GameContext& Instance() {
        static GameContext instance;
        return instance;
    }

    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;

    void InitFromRoom();   // 从 RoomContext 拷贝
    //每一局重置状态
    void playerShotted(ClientID Attacker,ClientID Victim,float damage);
    // void setPlayerAlive(ClientID playerID);
    void addPositionHistory(ClientID playerID, const myu::math::Vec3& position);
    void addPlantAndReward(ClientID playerID);
    void addDefuseAndReward(ClientID playerID);
    int countLifes();
    int countLifes(PlayerTeam team);

    void resetARound();
    void Reset();          // 游戏结束

    PlayerState* GetPlayer(ClientID id);
    const std::unordered_map<ClientID, PlayerState>& Players() const;

private:
    std::unordered_map<ClientID, PlayerState> players_;
};

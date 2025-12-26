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
    void Reset();          // 游戏结束

    PlayerState* GetPlayer(ClientID id);
    const std::unordered_map<ClientID, PlayerState>& Players() const;

private:
    std::unordered_map<ClientID, PlayerState> players_;
};

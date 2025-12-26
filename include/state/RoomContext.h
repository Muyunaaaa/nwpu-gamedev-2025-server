#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

#include "spdlog/spdlog.h"

enum Team {
    TEAM_UNASSIGNED = 0,
    TEAM_CT = 1,
    TEAM_T = 2
};

struct PlayerInfo {
    uint32_t id;
    std::string name;
    Team team = TEAM_UNASSIGNED;
    bool isReady;      // 准备状态
};

/*
 * RoomContext 作为单例类，存储房间内的全局信息和状态，管理游戏未开始前的状态，不管理每一回合的状态
 */
class RoomContext {
private:
    RoomContext() {
        spdlog::info("RoomContext Initialized");
    }
public:
    static constexpr int TARGET_PLAYERS = 2;
    static constexpr int MAX_ROUNDS = 2;

    std::unordered_map<uint32_t, PlayerInfo> players;
    std::vector<uint32_t> players_just_joined;

    static RoomContext& getInstance() {
        static RoomContext instance;
        return instance;
    }

    RoomContext(const RoomContext&) = delete;
    RoomContext& operator=(const RoomContext&) = delete;

    int getReadyCount();
    int getTotalPlayerCount();
    Team getTeamWithLessPlayers();
};

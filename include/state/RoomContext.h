#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

#include "game/PlayerTeam.h"
#include "network/NetPacket.h"
#include "spdlog/spdlog.h"


struct PlayerInfo {
    ClientID id;
    std::string name;
    PlayerTeam team = PlayerTeam::NONE;
    bool isReady;      // 准备状态
};

/*
 * RoomContext 作为单例类，仅仅储存和管理游戏开始前的房间状态,
 * 当进入游戏后,该上下文的玩家信息将全部拷贝到新的玩家信息管理类中
 */
class RoomContext {
private:
    RoomContext() {
        spdlog::info("RoomContext Initialized");
    }
public:
    std::unordered_map<ClientID, PlayerInfo> players;
    std::vector<ClientID> players_just_joined;

    static RoomContext& getInstance() {
        static RoomContext instance;
        return instance;
    }

    RoomContext(const RoomContext&) = delete;
    RoomContext& operator=(const RoomContext&) = delete;

    int getReadyCount();
    int getTotalPlayerCount();
    PlayerTeam getTeamWithLessPlayers();
};

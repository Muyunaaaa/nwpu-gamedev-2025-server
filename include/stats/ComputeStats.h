#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include "game/PlayerTeam.h"
#include "entity/PlayerState.h"

using ClientID = uint64_t;

/**
 * @brief 单个玩家结算数据
 */
struct PlayerStats
{
    ClientID clientId;
    std::string nickname;
    PlayerTeam team;

    // 战绩
    uint32_t kills   = 0;
    uint32_t deaths  = 0;

    // 行为
    uint32_t plants  = 0;
    uint32_t defuses = 0;

    // 伤害
    float totalDamage = 0.0f;

    // 计算字段
    float kd     = 0.0f;
    float rating = 0.0f;
};

/**
 * @brief 比赛统计计算器（单例）
 */
class ComputeStats
{
public:
    static ComputeStats& GetInstance();

    ComputeStats(const ComputeStats&) = delete;
    ComputeStats& operator=(const ComputeStats&) = delete;

    /**
     * @brief 从 PlayerState 加载并计算
     */
    void LoadFromPlayerStates(
        const std::unordered_map<ClientID, PlayerState>& players);

    /**
     * @brief 导出 CSV
     */
    bool ExportCSV(const std::string& filePath,PlayerTeam winner_team) const;

    const std::unordered_map<ClientID, PlayerStats>& GetAllStats() const;

private:
    ComputeStats() = default;

private:
    float ComputeKD(uint32_t kills, uint32_t deaths) const;
    float ComputeRating(const PlayerStats& stats) const;

    float ComputeTotalDamage(const PlayerState& player) const;

private:
    std::unordered_map<ClientID, PlayerStats> m_stats;
};
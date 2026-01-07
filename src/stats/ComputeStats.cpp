#include "stats/ComputeStats.h"

#include <fstream>
#include <iomanip>

#include "core/GameContext.h"
#include "state/MatchController.h"

ComputeStats &ComputeStats::GetInstance() {
    static ComputeStats instance;
    return instance;
}

void ComputeStats::LoadFromPlayerStates(
    const std::unordered_map<ClientID, PlayerState>& players)
{
    m_stats.clear();

    for (const auto& [id, player] : players)
    {
        PlayerStats stats;
        stats.clientId = player.client_id;
        stats.nickname = player.name;
        stats.team     = player.team;

        stats.kills   = player.kills;
        stats.deaths  = player.deaths;
        stats.plants  = player.plants;
        stats.defuses = player.defuse;

        stats.totalDamage = ComputeTotalDamage(player);

        stats.kd     = ComputeKD(stats.kills, stats.deaths);
        stats.rating = ComputeRating(stats);

        m_stats.emplace(stats.clientId, std::move(stats));
    }
}

bool ComputeStats::ExportCSV(const std::string &filePath,PlayerTeam winner_team) const
{
    std::ofstream file(filePath);
    if (!file.is_open()) {
        spdlog::error("csv文件输出失败: {}", filePath);
        return false;
    }

    if (winner_team == PlayerTeam::NONE) {
        spdlog::error("数据统计时不应出现未有胜利方的情况");
        return false;
    }

    file << "# WinningTeam=" << toString(winner_team) << "\n";
    file << "ClientID,Nickname,Team,Kills,Deaths,KD,Damage,Plants,Defuses,Rating\n";

    for (const auto &[id, s]: m_stats)
    {
        file << s.clientId << ","
             << s.nickname << ","
             << toString(s.team) << ","
             << s.kills << ","
             << s.deaths << ","
             << std::fixed << std::setprecision(2) << s.kd << ","
             << std::fixed << std::setprecision(1) << s.totalDamage << ","
             << s.plants << ","
             << s.defuses << ","
             << std::fixed << std::setprecision(2) << s.rating << ","
             << "\n";
    }

    return true;
}

const std::unordered_map<ClientID, PlayerStats> &
ComputeStats::GetAllStats() const {
    return m_stats;
}


float ComputeStats::ComputeKD(uint32_t kills, uint32_t deaths) const {
    if (deaths == 0)
        return static_cast<float>(kills);

    return static_cast<float>(kills) / static_cast<float>(deaths);
}

float ComputeStats::ComputeTotalDamage(const PlayerState &player) const {
    float damage = 0.0f;
    for (const auto &record: player.damage_records) {
        damage += record.damage;
    }
    return damage;
}

float ComputeStats::ComputeRating(const PlayerStats& s) const
{
    constexpr float AVG_KPR = 0.75f;
    constexpr float AVG_DPR = 0.75f;
    constexpr float AVG_DMG = 70.0f;
    constexpr float AVG_OBJ = 0.2f;

    float rounds = std::max(1.0f, static_cast<float>(Config::room::MAX_ROUNDS));

    float kpr   = s.kills / rounds;
    float dpr   = s.deaths / rounds;
    float dmgpr = s.totalDamage / rounds;

    float objCount = (s.team == PlayerTeam::T)
        ? s.plants
        : s.defuses;

    float objpr = objCount / rounds;

    float k   = (kpr - AVG_KPR) / AVG_KPR;
    float d   = (AVG_DPR - dpr) / AVG_DPR;
    float dmg = (dmgpr - AVG_DMG) / AVG_DMG;
    float obj = (objpr - AVG_OBJ) / AVG_OBJ;

    float raw =
        0.45f * k +
        0.25f * dmg +
        0.20f * d +
        0.10f * obj;

    float rating = 3.0f / (1.0f + std::exp(-(raw - 0.7f)));

    return rating;
}
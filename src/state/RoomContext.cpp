#include "state/RoomContext.h"
int RoomContext::getReadyCount() {
    int count = 0;
    for (auto& [id, info] : players) {
        if (info.isReady) count++;
    }
    return count;
}

int RoomContext::getTotalPlayerCount() {
    return players.size();
}

Team RoomContext::getTeamWithLessPlayers() {
    int countT = 0;
    int countCT = 0;

    for (const auto& [id, player] : players) {
        if (player.team == Team::TEAM_T) countT++;
        else if (player.team == Team::TEAM_CT) countCT++;
    }

    if (countT < countCT) return Team::TEAM_T;
    if (countCT < countT) return Team::TEAM_CT;

    return Team::TEAM_T;
}



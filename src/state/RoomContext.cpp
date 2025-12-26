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

PlayerTeam RoomContext::getTeamWithLessPlayers() {
    int countT = 0;
    int countCT = 0;

    for (const auto& [id, player] : players) {
        if (player.team == PlayerTeam::T) countT++;
        else if (player.team == PlayerTeam::CT) countCT++;
    }

    if (countT < countCT) return PlayerTeam::T;
    if (countCT < countT) return PlayerTeam::CT;

    return PlayerTeam::T;
}



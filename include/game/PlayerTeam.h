//
// Created by Administrator on 25-12-26.
//

#ifndef PLAYERTEAM_H
#define PLAYERTEAM_H
#include "protocol/Main_generated.h"

enum class PlayerTeam {
    NONE = 0,
    CT = 1,
    T = 2,
};

inline moe::net::PlayerTeam parseToNetPlayerTeam(PlayerTeam team) {
    switch (team) {
        case PlayerTeam::CT:
            return moe::net::PlayerTeam::PlayerTeam_TEAM_CT;
        case PlayerTeam::T:
            return moe::net::PlayerTeam::PlayerTeam_TEAM_T;
        case PlayerTeam::NONE:
            return moe::net::PlayerTeam::PlayerTeam_TEAM_NONE;
        default:
            return moe::net::PlayerTeam::PlayerTeam_TEAM_NONE;
    }
}

inline std::string toString(PlayerTeam team) {
    switch (team) {
        case PlayerTeam::CT:
            return "CT";
        case PlayerTeam::T:
            return "T";
        case PlayerTeam::NONE:
            return "NONE";
        default:
            return "NONE";
    }
}
#endif //PLAYERTEAM_H

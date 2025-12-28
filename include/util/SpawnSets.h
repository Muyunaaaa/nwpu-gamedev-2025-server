#pragma once

#include <vector>
#include <algorithm>
#include <random>
#include "config.h"

struct SpawnSets {
    std::vector<myu::math::Vec3> ct_spawns;
    std::vector<myu::math::Vec3> t_spawns;
};

class SpawnUtils {
public:
    static SpawnSets generateRandomSpawnSets() {
        SpawnSets sets;

        for(int i = 0; i < 5; ++i) {
            sets.ct_spawns.push_back(Config::room::DUST2_CT_SPAWNS[i]);
            sets.t_spawns.push_back(Config::room::DUST2_T_SPAWNS[i]);
        }

        static std::random_device rd;
        static std::mt19937 g(rd());

        std::shuffle(sets.ct_spawns.begin(), sets.ct_spawns.end(), g);
        std::shuffle(sets.t_spawns.begin(), sets.t_spawns.end(), g);

        return sets;
    }
};
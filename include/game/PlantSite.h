//
// Created by Administrator on 25-12-27.
//

#ifndef PLANTSITE_H
#define PLANTSITE_H
#include <cstdint>

#include "protocol/PlayerTeam_generated.h"

enum class PlantSite : uint16_t {
    None = 0,
    A = 1,
    B = 2
};

inline uint16_t parseToNetBombSite(PlantSite site) {
    switch (site) {
        case PlantSite::A:
            return 0;
        case PlantSite::B:
            return 1;
        case PlantSite::None:
            return 2;
        default:
            return -1;
    }
}

inline PlantSite parseNetBombSiteToBombSite(uint16_t site) {
    switch (site) {
        case 0:
            return PlantSite::A;
        case 1:
            return PlantSite::B;
        default:
            return PlantSite::None;
    }
}
#endif //PLANTSITE_H

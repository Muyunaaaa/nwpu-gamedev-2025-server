#ifndef PLANTZONE_H
#define PLANTZONE_H
#include "math/common.h"
struct PlantZone {
    myu::math::Vec3 min_point;
    myu::math::Vec3 max_point;

    bool contains(const myu::math::Vec3& pos) const {
        return pos.x >= min_point.x && pos.x <= max_point.x &&
               pos.y >= min_point.y && pos.y <= max_point.y &&
               pos.z >= min_point.z && pos.z <= max_point.z;
    }
};
#endif //PLANTZONE_H

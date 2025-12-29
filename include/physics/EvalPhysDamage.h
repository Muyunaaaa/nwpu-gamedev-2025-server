#pragma once
#include "PhysicsEnginee.h"
#include "math/common.h"
#include "network/NetPacket.h"

using BodyID = JPH::BodyID;
class MapOnlyBroadPhaseFilter : public JPH::BroadPhaseLayerFilter {
public:
    virtual bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override {
        return inLayer == myu::Physics::Details::BroadPhaseLayers::NON_MOVING;
    }
};

class MapOnlyObjectFilter : public JPH::ObjectLayerFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer) const override {
        // 假设你的静态环境层 ID 是 LAYER_STATIC
        return inLayer == myu::Physics::Details::Layers::NON_MOVING;
    }
};

struct PhysRaycastHit {
    myu::math::Vec3 fire_point;
    myu::math::Vec3 fire_dir;
    ClientID shooter_id;

    PhysRaycastHit() = delete;
    PhysRaycastHit(myu::math::Vec3 p, myu::math::Vec3 d, ClientID shooter_id) {
        this->fire_dir = d;
        this->fire_point = p;
        this->shooter_id = shooter_id;
    }

    int isHit(uint64_t shot_tick);
};

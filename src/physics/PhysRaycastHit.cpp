#include "physics/EvalPhysDamage.h"
#include "Physics/JoltIncludes.hpp"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics//Collision/ShapeCast.h>

#include "core/GameContext.h"
#include "physics/PhysicsEnginee.h"

//返回值-1代表未击中玩家，否则返回玩家ID
int PhysRaycastHit::isHit(uint64_t shot_tick) {
    // 1. 初始化数据
    JPH::RVec3 ray_origin = JPH::RVec3(fire_point.x, fire_point.y, fire_point.z);
    float max_distance = 500.0f;
    JPH::Vec3 ray_direction = JPH::Vec3(fire_dir.x, fire_dir.y, fire_dir.z) * max_distance;

    JPH::RayCast ray{JPH::Vec3(ray_origin), ray_direction};
    JPH::RRayCast r_ray(ray_origin, ray_direction);

    auto shooter = GameContext::Instance().GetPlayer(shooter_id);
    if (!shooter) {
        spdlog::error("PhysRaycastHit: Invalid shooter_id {}", shooter_id);
        return -1;
    }
    auto shooter_team = shooter->team;

    // 2. 环境阻挡检测
    JPH::RayCastResult mapHit;
    MapOnlyBroadPhaseFilter bp_filter;
    MapOnlyObjectFilter obj_filter;

    bool hitMap = myu::PhysicsEngine::getInstance().getPhysicsSystem().GetNarrowPhaseQuery().CastRay(
        r_ray, mapHit, bp_filter, obj_filter, {}
    );

    float maxFraction = hitMap ? mapHit.mFraction : 1.0f;

    // 调试日志：如果击中了地图，记录阻挡距离
    if (hitMap) {
        spdlog::debug("Shot [{}] blocked by map at fraction: {:.3f}", shooter_id, maxFraction);
    }

    ClientID hitPlayerID = -1;
    float closestPlayerFraction = maxFraction;

    // 3. 遍历其他玩家进行回溯判定
    for (auto& player_pair : GameContext::Instance().Players()) {
        ClientID target_id = player_pair.first;
        auto& player_state = player_pair.second;

        if (player_state.health <= 0) {
            continue;
        }

        if (player_state.team == shooter_team || target_id == shooter_id) {
            continue;
        }

        JPH::RVec3 historyPos;
        auto& history = player_state.position_history;
        bool found = false;

        // 查找历史 Tick
        for (size_t i = 0; i < history.size(); ++i) {
            auto& current = history[i];
            auto& next = (i + 1 < history.size()) ? history[i + 1] : current;

            if (shot_tick >= current.tick && shot_tick <= next.tick) {
                float alpha = (next.tick == current.tick) ? 0.0f :
                              (float)(shot_tick - current.tick) / (float)(next.tick - current.tick);

                historyPos = JPH::RVec3(
                    current.position.x + (next.position.x - current.position.x) * alpha,
                    current.position.y + (next.position.y - current.position.y) * alpha,
                    current.position.z + (next.position.z - current.position.z) * alpha
                );
                found = true;
                break;
            }
        }

        if (!found) {
            // 警告日志：回溯失败通常意味着客户端延迟过高或服务器历史缓存不足
            if (history.size() > 0) {
                spdlog::warn("Backtrack failed for target {} at tick {}. Using oldest snapshot (tick {}).",
                             target_id, shot_tick, history.back().tick);
                auto& last = history.back();
                historyPos = JPH::RVec3(last.position.x, last.position.y, last.position.z);
            } else {
                spdlog::error("Backtrack failed: Target {} has empty position history.", target_id);
                continue;
            }
        }

        // 4. 执行物理空间判定
        auto character = player_state.physics_controller.get()->getCharacter();
        JPH::RMat44 transform = JPH::RMat44::sTranslation(historyPos);
        JPH::RMat44 invTransform = transform.Inversed();

        JPH::RayCast localRay;
        localRay.mOrigin = JPH::Vec3(invTransform * r_ray.mOrigin);
        localRay.mDirection = JPH::Vec3(invTransform.Multiply3x3(r_ray.mDirection));

        JPH::RayCastResult shapeHit;
        JPH::SubShapeIDCreator id_creator;

        if (character->GetShape()->CastRay(localRay, id_creator, shapeHit)) {
            // 判定是否比之前的命中更近（地图或其他玩家）
            if (shapeHit.mFraction < closestPlayerFraction) {
                closestPlayerFraction = shapeHit.mFraction;
                hitPlayerID = target_id;
            }
        }
    }

    // 5. 最终判定结果日志
    if (hitPlayerID != -1) {
        spdlog::info("SUCCESS: Shooter {} hit Target {} at tick {} (fraction: {:.3f})",
                     shooter_id, hitPlayerID, shot_tick, closestPlayerFraction);
    } else if (hitMap) {
        spdlog::debug("MISS: Shooter {} hit the wall.", shooter_id);
    }

    return (int)hitPlayerID;
}
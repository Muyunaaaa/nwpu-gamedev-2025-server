#pragma once
#include "glm/vec3.hpp"
#include "math/common.h"
#include <cmath>

struct InputIntent {
    glm::vec3 dir; //移动方向，归一化
    // bool jump;
    // bool crouch; //下蹲
    // bool sprint; //前行
    float yaw_radian;
    float pitch_radian;
    myu::math::Vec3 getHeadDirection() const {
        float vx = std::sin(yaw_radian);
        float vz = std::cos(yaw_radian);
        return myu::math::Vec3(vx, 0.0f, vz);
    }
    InputIntent(
        const glm::vec3& move_dir,
        float yaw_rad,
        float pitch_rad
    )
        : dir(move_dir),
          // jump(jump_request),
          // crouch(crouch_request),
          // sprint(sprint_request),
          yaw_radian(yaw_rad),
          pitch_radian(pitch_rad)
    {}
};

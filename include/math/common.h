#pragma once
#include "protocol/Main_generated.h"
#include "Physics/JoltIncludes.hpp"

namespace myu {
    namespace math {
        struct Vec2 {
            float x;
            float y;
        };

        struct Vec3 {
            float x;
            float y;
            float z;
            constexpr Vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f)
            : x(x), y(y), z(z) {}
            inline moe::net::Vec3 ToMOEVec3() const {
                return moe::net::Vec3(x, y, z);
            }
            inline static myu::math::Vec3 JPHToMyuVec3(const JPH::RVec3& vec){
                return myu::math::Vec3(vec.GetX(), vec.GetY(), vec.GetZ());
            }
        };
    }
}

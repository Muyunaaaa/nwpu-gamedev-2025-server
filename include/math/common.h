#pragma once
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
        };
    }
}

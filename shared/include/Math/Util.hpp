#pragma once

#include "Core/Common.hpp"

MOE_BEGIN_NAMESPACE

namespace Math {
    template<typename T>
    T min(T a, T b) {
        return (a < b) ? a : b;
    }

    template<typename T>
    T max(T a, T b) {
        return (a > b) ? a : b;
    }

    template<typename T>
    T clamp(T value, T minVal, T maxVal) {
        if (value < minVal) {
            return minVal;
        }
        if (value > maxVal) {
            return maxVal;
        }
        return value;
    }

    template<typename T>
    T lerp(const T& a, const T& b, float t) {
        return a + (b - a) * t;
    }

    template<typename T>
    T smoothstep(const T& edge0, const T& edge1, float x) {
        float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
}// namespace Math

MOE_END_NAMESPACE
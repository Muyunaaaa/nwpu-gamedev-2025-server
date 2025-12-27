#pragma once

#include "Core/Common.hpp"
#include "Math/Common.hpp"

MOE_BEGIN_NAMESPACE

struct Rect {
public:
    Rect(glm::vec2 pos = glm::vec2(0.0), glm::vec2 size = glm::vec2(0.0))
        : pos(pos), size(size) {}

    glm::vec2 pos;
    glm::vec2 size;
};

MOE_END_NAMESPACE
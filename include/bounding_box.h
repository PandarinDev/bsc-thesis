#pragma once

#include <glm/vec2.hpp>

namespace inf {

    struct BoundingBox2D {

        glm::ivec2 lower;
        glm::ivec2 higher;

        BoundingBox2D(const glm::ivec2& lower, const glm::ivec2& higher);
        BoundingBox2D(int low_x, int low_y, int high_x, int high_y);

        bool contains(const glm::ivec2& coordinate) const;

    };

}
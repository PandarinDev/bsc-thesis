#pragma once

#include "bounding_box.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace inf {

    enum class RoadDirection {
        HORIZONTAL_UP,
        HORIZONTAL_DOWN,
        VERTICAL_LEFT,
        VERTICAL_RIGHT,
        CROSSING_UP_LEFT,
        CROSSING_UP_RIGHT,
        CROSSING_DOWN_RIGHT,
        CROSSING_DOWN_LEFT
    };

    struct RoadUtils {

        RoadUtils() = delete;

        static glm::vec3 road_direction_to_world_direction(RoadDirection direction);

    };

    struct DistrictRoad {

        RoadDirection direction;
        glm::ivec2 position;

        DistrictRoad(RoadDirection direction, const glm::ivec2& position);

        bool is_crossing() const;
        BoundingBox3D get_bounding_box(const glm::vec3& district_position) const;

    };

}
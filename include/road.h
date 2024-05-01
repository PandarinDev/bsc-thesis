#pragma once

#include "bounding_box.h"
#include "gfx/mesh.h"

#include <glm/vec2.hpp>

#include <vector>
#include <unordered_map>

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

    struct DistrictRoad {

        RoadDirection direction;
        glm::ivec2 position;
        bool traversable;
        const gfx::Mesh* mesh;

        DistrictRoad(RoadDirection direction, const glm::ivec2& position, bool traversable, const gfx::Mesh* mesh);

        bool is_crossing() const;
        BoundingBox3D get_bounding_box(const glm::vec3& district_position) const;

    };

    struct RoadUtils {

        RoadUtils() = delete;

        static glm::ivec2 road_direction_to_grid_direction(RoadDirection direction);
        static std::vector<std::vector<glm::ivec2>> get_possible_continuations(
            const std::unordered_map<glm::ivec2, DistrictRoad>& roads,
            const glm::ivec2& current_position,
            RoadDirection direction);

    };

}
#include "road.h"

namespace inf {

    glm::vec3 RoadUtils::road_direction_to_world_direction(RoadDirection direction) {
        switch (direction) {
            case RoadDirection::HORIZONTAL_UP:
                return glm::vec3(-1.0f, 0.0f, 0.0f);
            case RoadDirection::HORIZONTAL_DOWN:
                return glm::vec3(1.0f, 0.0f, 0.0f);
            case RoadDirection::VERTICAL_LEFT:
                return glm::vec3(0.0f, 0.0f, 1.0f);
            case RoadDirection::VERTICAL_RIGHT:
                return glm::vec3(0.0f, 0.0f, -1.0f);
            default: return glm::vec3();
        }
    }

    DistrictRoad::DistrictRoad(RoadDirection direction, const glm::ivec2& position) :
        direction(direction), position(position) {}

    bool DistrictRoad::is_crossing() const {
        return direction == RoadDirection::CROSSING_DOWN_LEFT ||
            direction == RoadDirection::CROSSING_DOWN_RIGHT ||
            direction == RoadDirection::CROSSING_UP_LEFT ||
            direction == RoadDirection::CROSSING_UP_RIGHT;
    }

    BoundingBox3D DistrictRoad::get_bounding_box(const glm::vec3& district_position) const {
        // TODO: This function is currently unused, only needed for diagnostics/debugging
        const auto center = glm::vec3(district_position.x + position.x, 0.0f, district_position.z + position.y);
        const auto min = glm::vec3(center.x - 0.1f, center.y - 0.1f, center.z - 0.1f);
        const auto max = glm::vec3(center.x + 0.1f, center.y + 0.1f, center.z + 0.1f);
        return BoundingBox3D(min, max);
    }

}
#include "road.h"
#include "utils/hash_utils.h"

#include <stdexcept>

namespace inf {

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

    glm::ivec2 RoadUtils::road_direction_to_grid_direction(RoadDirection direction) {
        switch (direction) {
            case RoadDirection::HORIZONTAL_UP: return glm::ivec2(-1, 0);
            case RoadDirection::HORIZONTAL_DOWN: return glm::ivec2(1, 0);
            case RoadDirection::VERTICAL_LEFT: return glm::ivec2(0, 1);
            case RoadDirection::VERTICAL_RIGHT: return glm::ivec2(0, -1);
            default: throw std::runtime_error("Unhandled road direction.");
        }
    }

    std::vector<std::vector<glm::ivec2>> RoadUtils::get_possible_continuations(
        const std::unordered_map<glm::ivec2, DistrictRoad>& roads,
        const glm::ivec2& current_position) {
        // It can happen in unlucky scenarios (when a car is put at the edge of a district
        // facing outwards of the district) that the first time this function is called the
        // vehicle is already out-of-bounds, so check if the current position is valid.
        // TODO: This should not be needed anymore once dead-end elimination is implemented.
        const auto road_it = roads.find(current_position);
        if (road_it == roads.cend()) {
            return {};
        }
        const auto& road = road_it->second;
        
        // For straight roads the choices are simpler. We can always go straight, just need
        // to check if we can also turn left (which is not considered as stepping on a crossing
        // if there is no opportunity to turn right).
        if (!road.is_crossing()) {
            std::vector<std::vector<glm::ivec2>> result;
            const auto forward_position = current_position + RoadUtils::road_direction_to_grid_direction(road.direction);
            // Only add forward if it does not go out of bounds
            if (roads.find(forward_position) != roads.cend()) {
                result.push_back({ forward_position });
            }
            // TODO: Check if we can turn left
            return result;
        }

        // In crossings we need to decide between keeping straight, turning right or turning left (if possible)
        std::vector<std::vector<glm::ivec2>> result;
        if (road.direction == RoadDirection::CROSSING_DOWN_RIGHT) {
            // Turning right is always possible
            result.push_back({ current_position + glm::ivec2(1, 0) });
            // Check if turning left is possible
            if (const auto it = roads.find(current_position + glm::ivec2(-1, -1)); it != roads.cend() && it->second.direction == RoadDirection::CROSSING_UP_LEFT) {
                result.push_back({ current_position + glm::ivec2(-1, -1), current_position + glm::ivec2(-2, -1) });
            }
            // Check if going straight is possible
            if (roads.find(current_position + glm::ivec2(0, -2)) != roads.cend()) {
                result.push_back({ current_position + glm::ivec2(0, -1), current_position + glm::ivec2(0, -2) });
            }
            return result;
        }
        if (road.direction == RoadDirection::CROSSING_UP_RIGHT) {
            // Turning right is always possible
            result.push_back({ current_position + glm::ivec2(0, -1) });
            // Check if turning left is possible
            if (const auto it = roads.find(current_position + glm::ivec2(-1, 1)); it != roads.cend() && it->second.direction == RoadDirection::CROSSING_DOWN_LEFT) {
                result.push_back({ current_position + glm::ivec2(-1, 1), current_position + glm::ivec2(-1, 2) });
            }
            // Check if going straight is possible
            if (roads.find(current_position + glm::ivec2(-2, 0)) != roads.cend()) {
                result.push_back({ current_position + glm::ivec2(-1, 0), current_position + glm::ivec2(-2, 0) });
            }
            return result;
        }
        if (road.direction == RoadDirection::CROSSING_UP_LEFT) {
            // Turning right is always possible
            result.push_back({ current_position + glm::ivec2(-1, 0) });
            // Check if turning left is possible
            if (const auto it = roads.find(current_position + glm::ivec2(1, -1)); it != roads.cend() && it->second.direction == RoadDirection::CROSSING_DOWN_RIGHT) {
                result.push_back({ current_position + glm::ivec2(1, -1), current_position + glm::ivec2(2, -1) });
            }
            // Check if going straight is possible
            if (roads.find(current_position + glm::ivec2(0, 2)) != roads.cend()) {
                result.push_back({ current_position + glm::ivec2(0, 1), current_position + glm::ivec2(0, 2) });
            }
            return result;
        }
        if (road.direction == RoadDirection::CROSSING_DOWN_LEFT) {
            // Turning right is always possible
            result.push_back({ current_position + glm::ivec2(0, 1) });
            // Check if turning left is possible
            if (const auto it = roads.find(current_position + glm::ivec2(1, -1)); it != roads.cend() && it->second.direction == RoadDirection::CROSSING_UP_RIGHT) {
                result.push_back({ current_position + glm::ivec2(1, -1), current_position + glm::ivec2(1, -2) });
            }
            // Check if going straight is possible
            if (roads.find(current_position + glm::ivec2(2, 0)) != roads.cend()) {
                result.push_back({ current_position + glm::ivec2(1, 0), current_position + glm::ivec2(2, 0) });
            }
            return result;
        }

        throw std::runtime_error("Unhandled road direction in possible continuation computation.");
    }

}
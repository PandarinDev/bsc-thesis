#include "road.h"
#include "utils/hash_utils.h"

#include <stdexcept>

namespace inf {

    DistrictRoad::DistrictRoad(RoadDirection direction, const glm::ivec2& position, bool traversable, const gfx::Mesh* mesh) :
        direction(direction), position(position), traversable(traversable), mesh(mesh) {}

    bool DistrictRoad::is_crossing() const {
        return direction == RoadDirection::CROSSING_DOWN_LEFT ||
            direction == RoadDirection::CROSSING_DOWN_RIGHT ||
            direction == RoadDirection::CROSSING_UP_LEFT ||
            direction == RoadDirection::CROSSING_UP_RIGHT;
    }

    BoundingBox3D DistrictRoad::get_bounding_box(const glm::vec3& district_position) const {
        // TODO: This function is currently unused, only needed for diagnostics/debugging
        const auto center = glm::vec3(district_position.x + position.x + 0.5f, 0.0f, district_position.z + position.y + 0.5f);
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
        const glm::ivec2& current_position,
        RoadDirection direction) {
        // It can happen in unlucky scenarios (when a car is put at the edge of a district
        // facing outwards of the district) that the first time this function is called the
        // vehicle is already out-of-bounds, so check if the current position is valid.
        // TODO: This should not be needed anymore once dead-end elimination is implemented.
        const auto road_it = roads.find(current_position);
        if (road_it == roads.cend()) {
            return {};
        }
        const auto& road = road_it->second;
        
        const auto get_traversable_road_at = [&roads](const glm::ivec2& position) -> const DistrictRoad* {
            const auto it = roads.find(position);
            return (it != roads.cend() && it->second.traversable)
                ? &it->second
                : nullptr;
        };

        // For straight roads the choices are simpler. We can always go straight, just need
        // to check if we can also turn left (which is not considered as stepping on a crossing
        // if there is no opportunity to turn right).
        if (!road.is_crossing()) {
            std::vector<std::vector<glm::ivec2>> result;
            const auto grid_direction = RoadUtils::road_direction_to_grid_direction(road.direction);
            // Only add forward if it does not go out of bounds
            if (get_traversable_road_at(current_position + grid_direction) &&
                get_traversable_road_at(current_position + grid_direction * 2)) {
                result.push_back({ current_position + grid_direction });
            }
            
            // Check if we can turn left
            if (road.direction == RoadDirection::VERTICAL_RIGHT) {
                if (const auto crossing = get_traversable_road_at(current_position + glm::ivec2(-1, -1));
                    crossing && crossing->direction == RoadDirection::CROSSING_UP_LEFT &&
                    get_traversable_road_at(current_position + glm::ivec2(-2, -1))) {
                    result.push_back({
                        current_position + glm::ivec2(-1, -1),
                        current_position + glm::ivec2(-2, -1)
                    });
                }
            }
            else if (road.direction == RoadDirection::VERTICAL_LEFT) {
                if (const auto it = get_traversable_road_at(current_position + glm::ivec2(1, 1));
                    it && it->direction == RoadDirection::CROSSING_DOWN_RIGHT &&
                    get_traversable_road_at(current_position + glm::ivec2(2, 1))) {
                    result.push_back({
                        current_position + glm::ivec2(1, 1),
                        current_position + glm::ivec2(2, 1)
                    });
                }
            }
            else if (road.direction == RoadDirection::HORIZONTAL_UP) {
                if (const auto it = get_traversable_road_at(current_position + glm::ivec2(-1, 1));
                    it && it->direction == RoadDirection::CROSSING_DOWN_LEFT &&
                    get_traversable_road_at(current_position + glm::ivec2(-1, 2))) {
                    result.push_back({
                        current_position + glm::ivec2(-1, 1),
                        current_position + glm::ivec2(-1, 2)
                    });
                }
            }
            else if (road.direction == RoadDirection::HORIZONTAL_DOWN) {
                if (const auto it = get_traversable_road_at(current_position + glm::ivec2(1, -1));
                    it && it->direction == RoadDirection::CROSSING_UP_RIGHT &&
                    get_traversable_road_at(current_position + glm::ivec2(1, -2))) {
                    result.push_back({
                        current_position + glm::ivec2(1, -1),
                        current_position + glm::ivec2(1, -2)
                    });
                }
            }

            return result;
        }

        // In crossings we need to decide between keeping straight, turning right or turning left (if possible)
        std::vector<std::vector<glm::ivec2>> result;
        if (road.direction == RoadDirection::CROSSING_DOWN_RIGHT) {
            // Turning right is always possible if it does not go out of bounds
            if (get_traversable_road_at(current_position + glm::ivec2(1, 0))) {
                result.push_back({ current_position + glm::ivec2(1, 0) });
            }
            // Check if turning left is possible
            if (const auto it = get_traversable_road_at(current_position + glm::ivec2(-1, -1));
                it && (it->direction == RoadDirection::CROSSING_UP_LEFT || it->direction == RoadDirection::HORIZONTAL_UP) &&
                get_traversable_road_at(current_position + glm::ivec2(-2, -1))) {
                result.push_back({ current_position + glm::ivec2(-1, -1), current_position + glm::ivec2(-2, -1) });
            }
            // Check if going straight is possible
            if (const auto it = get_traversable_road_at(current_position + glm::ivec2(0, -2));
                it && it->direction == direction) {
                result.push_back({ current_position + glm::ivec2(0, -1), current_position + glm::ivec2(0, -2) });
            }
            return result;
        }
        if (road.direction == RoadDirection::CROSSING_UP_RIGHT) {
            // Turning right is always possible if it does not go out of bounds
            if (get_traversable_road_at(current_position + glm::ivec2(0, -1))) {
                result.push_back({ current_position + glm::ivec2(0, -1) });
            }
            // Check if turning left is possible
            if (const auto it = get_traversable_road_at(current_position + glm::ivec2(-1, 1));
                it && (it->direction == RoadDirection::CROSSING_DOWN_LEFT || it->direction == RoadDirection::VERTICAL_LEFT) &&
                get_traversable_road_at(current_position + glm::ivec2(-1, 2))) {
                result.push_back({ current_position + glm::ivec2(-1, 1), current_position + glm::ivec2(-1, 2) });
            }
            // Check if going straight is possible
            if (const auto it = get_traversable_road_at(current_position + glm::ivec2(-2, 0));
                it && it->direction == direction) {
                result.push_back({ current_position + glm::ivec2(-1, 0), current_position + glm::ivec2(-2, 0) });
            }
            return result;
        }
        if (road.direction == RoadDirection::CROSSING_UP_LEFT) {
            // Turning right is always possible if it does not go out of bounds
            if (get_traversable_road_at(current_position + glm::ivec2(-1, 0))) {
                result.push_back({ current_position + glm::ivec2(-1, 0) });
            }
            // Check if turning left is possible
            if (const auto it = get_traversable_road_at(current_position + glm::ivec2(1, -1));
                it && (it->direction == RoadDirection::CROSSING_DOWN_RIGHT || it->direction == RoadDirection::HORIZONTAL_DOWN) &&
                get_traversable_road_at(current_position + glm::ivec2(2, -1))) {
                result.push_back({ current_position + glm::ivec2(1, -1), current_position + glm::ivec2(2, -1) });
            }
            // Check if going straight is possible
            if (const auto it = get_traversable_road_at(current_position + glm::ivec2(0, 2));
                it && it->direction == direction) {
                result.push_back({ current_position + glm::ivec2(0, 1), current_position + glm::ivec2(0, 2) });
            }
            return result;
        }
        if (road.direction == RoadDirection::CROSSING_DOWN_LEFT) {
            // Turning right is always possible if it does not go out of bounds
            if (get_traversable_road_at(current_position + glm::ivec2(0, 1))) {
                result.push_back({ current_position + glm::ivec2(0, 1) });
            }
            // Check if turning left is possible
            if (const auto it = get_traversable_road_at(current_position + glm::ivec2(1, -1));
                it && (it->direction == RoadDirection::CROSSING_UP_RIGHT || it->direction == RoadDirection::VERTICAL_RIGHT) &&
                get_traversable_road_at(current_position + glm::ivec2(1, -2))) {
                result.push_back({ current_position + glm::ivec2(1, -1), current_position + glm::ivec2(1, -2) });
            }
            // Check if going straight is possible
            if (const auto it = get_traversable_road_at(current_position + glm::ivec2(2, 0));
                it && it->direction == direction) {
                result.push_back({ current_position + glm::ivec2(1, 0), current_position + glm::ivec2(2, 0) });
            }
            return result;
        }

        throw std::runtime_error("Unhandled road direction in possible continuation computation.");
    }

}
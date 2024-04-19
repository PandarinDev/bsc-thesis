#include "district.h"
#include "gfx/renderer.h"
#include "utils/random_utils.h"

#include <limits>

namespace inf {

    DistrictLot::DistrictLot(
        const glm::ivec2& position,
        const glm::ivec2& dimensions,
        const glm::vec3& bb_color,
        std::optional<wfc::Building>&& building) :
        position(position), dimensions(dimensions), bb_color(bb_color), building(std::move(building)) {}

    BoundingBox3D DistrictLot::get_bounding_box(const glm::vec3& district_position) const {
        const auto min = glm::vec3(district_position.x + position.x, 0.0f, district_position.z + position.y);
        const auto max_y = building ? building->get_bounding_box().max.y : 0.1f;
        const auto max = glm::vec3(min.x + dimensions.x, max_y, min.z + dimensions.y);
        return BoundingBox3D(min, max);
    }

    District::District(
        DistrictType type,
        const glm::ivec2& grid_position,
        const glm::ivec2& dimensions,
        const glm::vec3& bb_color) :
        type(type), grid_position(grid_position), dimensions(dimensions),
        position(), bb_color(bb_color), bounding_box(
            glm::vec3(position.x, 0.0f, position.z),
            glm::vec3(position.x + dimensions.x, DISTRICT_BB_HEIGHT, position.z + dimensions.y)) {}

    void District::update(RandomGenerator& rng, float delta_time) {
        for (auto& vehicle : vehicles) {
            vehicle.update(rng, roads, delta_time);
        }
    }

    void District::update_caches() {
        // Update grass data
        grass_positions.clear();
        for (const auto& lot : lots) {
            for (int x = 0; x < lot.dimensions.x; ++x) {
                for (int y = 0; y < lot.dimensions.y; ++y) {
                    grass_positions.emplace_back(glm::vec3(
                        position.x + lot.position.x + x + 0.5f,
                        position.y + 0.5f,
                        position.z + lot.position.y + y + 0.5f));
                    grass_rotations.emplace_back(0.0f);
                }
            }
        }

        // Update road positions
        road_positions.clear();
        road_crossing_positions.clear();
        for (const auto& [_, road] : roads) {
            const auto& direction = road.direction;
            const auto road_position =  glm::vec3(
                position.x + road.position.x + 0.5f,
                position.y + 0.5f,
                position.z + road.position.y + 0.5f);
            switch (direction) {
                // Straight roads
                case RoadDirection::VERTICAL_LEFT:
                    road_positions.emplace_back(road_position);
                    road_rotations.emplace_back(0.0f);
                    break;
                case RoadDirection::VERTICAL_RIGHT:
                    road_positions.emplace_back(road_position);
                    road_rotations.emplace_back(glm::radians(180.f));
                    break;
                case RoadDirection::HORIZONTAL_UP:
                    road_positions.emplace_back(road_position);
                    road_rotations.emplace_back(glm::radians(90.0f));
                    break;
                case RoadDirection::HORIZONTAL_DOWN:
                    road_positions.emplace_back(road_position);
                    road_rotations.emplace_back(glm::radians(270.0f));
                    break;
                // Crossings
                case RoadDirection::CROSSING_DOWN_LEFT:
                    road_crossing_positions.emplace_back(road_position);
                    road_crossing_rotations.emplace_back(0.0f);
                    break;
                case RoadDirection::CROSSING_DOWN_RIGHT:
                    road_crossing_positions.emplace_back(road_position);
                    road_crossing_rotations.emplace_back(glm::radians(270.0f));
                    break;
                case RoadDirection::CROSSING_UP_RIGHT:
                    road_crossing_positions.emplace_back(road_position);
                    road_crossing_rotations.emplace_back(glm::radians(180.0f));
                    break;
                case RoadDirection::CROSSING_UP_LEFT:
                    road_crossing_positions.emplace_back(road_position);
                    road_crossing_rotations.emplace_back(glm::radians(90.0f));
                    break;
            }
        }
    }

    const glm::ivec2& District::get_grid_position() const {
        return grid_position;
    }

    const glm::ivec2& District::get_dimensions() const {
        return dimensions;
    }

    const glm::vec3& District::get_position() const {
        return position;
    }

    void District::set_position(const glm::vec3& position) {
        this->position = position;
        this->bounding_box = BoundingBox3D(
            glm::vec3(position.x, 0.0f, position.z),
            glm::vec3(position.x + dimensions.x, DISTRICT_BB_HEIGHT, position.z + dimensions.y)); // TODO: The height here should be computed
        for (auto& lot : lots) {
            if (lot.building) {
                const auto lot_bb = lot.get_bounding_box(position);
                const auto& building_bb = lot.building->get_local_bounding_box();
                const auto half_width_difference = (lot_bb.width() - building_bb.width()) * 0.5f;
                const auto half_depth_difference = (lot_bb.depth() - building_bb.depth()) * 0.5f;
                const auto building_position = glm::vec3(
                    lot_bb.min.x - building_bb.min.x + half_width_difference,
                    0.0f,
                    lot_bb.max.z - building_bb.max.z - half_depth_difference);
                lot.building->set_position(building_position);
            }
        }
    }

    BoundingBox3D District::compute_bounding_box() const {
        return bounding_box;
    }

    const std::vector<DistrictLot>& District::get_lots() const {
        return lots;
    }

    const std::unordered_map<glm::ivec2, DistrictRoad>& District::get_roads() const {
        return roads;
    }

    std::unordered_map<glm::ivec2, const DistrictRoad*> District::get_roads_at_edges() const {
        std::unordered_map<glm::ivec2, const DistrictRoad*> result;
        for (const auto& [position, road] : roads) {
            if (position.x == 0 ||
                position.x == dimensions.x - 1 ||
                position.y == 0 ||
                position.y == dimensions.y - 1) {
                result.emplace(position, &road);
            }
        }
        return result;
    }

    const std::vector<Vehicle>& District::get_vehicles() const {
        return vehicles;
    }

    void District::add_lot(DistrictLot&& lot) {
        lots.emplace_back(std::move(lot));
    }

    void District::add_road(DistrictRoad&& road) {
        const auto position = road.position;
        roads.emplace(position, std::move(road));
    }

    void District::add_vehicle(Vehicle&& vehicle) {
        vehicles.emplace_back(std::move(vehicle));
    }

    void District::render(gfx::Renderer& renderer) {
        // Render ground objects (such as roads and foliage)
        const auto& grass_mesh = wfc::GroundPatterns::get_pattern("grass").mesh;
        const auto& road = wfc::GroundPatterns::get_pattern("road").mesh;
        const auto& road_crossing = wfc::GroundPatterns::get_pattern("road_crossing").mesh;

        renderer.render_instanced(grass_mesh, grass_positions, grass_rotations);
        renderer.render_instanced(road, road_positions, road_rotations);
        renderer.render_instanced(road_crossing, road_crossing_positions, road_crossing_rotations);

        // Vehicle positions need to be recomputed on every frame due to moving
        vehicle_positions.clear();
        vehicle_rotations.clear();
        for (const auto& vehicle : vehicles) {
            const auto [position, rotation] = vehicle.get_world_position_and_rotation(this->position);
            vehicle_positions.emplace_back(position);
            vehicle_rotations.emplace_back(rotation);
        }
        if (!vehicle_positions.empty()) {
            // TODO: Group by vehicle type instead (since there is 1 mesh per vehicle type)
            renderer.render_instanced_caster(*vehicles[0].mesh, vehicle_positions, vehicle_rotations);
        }

        // Render lot buildings
        const auto frustum = renderer.get_frustum_in_view_space();
        const auto transform = renderer.get_view_matrix();
        for (const auto& lot : lots) {
            const auto lot_bb = lot.get_bounding_box(position);
            const auto obb = lot_bb.to_oriented(transform);
            if (!frustum.is_inside(obb)) {
                continue;
            }
            const auto& building = lot.building;
            renderer.render(lot_bb, lot.bb_color);
            if (building) {
                renderer.render(building->get_mesh());
                // renderer.render(building->get_bounding_box(), glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }
        renderer.render(compute_bounding_box(), bb_color);
    }

    BoundingBox3D District::get_left_district_bb() const {
        return BoundingBox3D(
            bounding_box.min + glm::vec3(-DISTRICT_SIZE - ROAD_GAP, 0.0f, 0.0f),
            bounding_box.max + glm::vec3(-DISTRICT_SIZE - ROAD_GAP, 0.0f, 0.0f));
    }

    BoundingBox3D District::get_right_district_bb() const {
        return BoundingBox3D(
            bounding_box.min + glm::vec3(DISTRICT_SIZE + ROAD_GAP, 0.0f, 0.0f),
            bounding_box.max + glm::vec3(DISTRICT_SIZE + ROAD_GAP, 0.0f, 0.0f));
    }

    BoundingBox3D District::get_above_district_bb() const {
        return BoundingBox3D(
            bounding_box.min + glm::vec3(0.0f, 0.0f, -DISTRICT_SIZE - ROAD_GAP),
            bounding_box.max + glm::vec3(0.0f, 0.0f, -DISTRICT_SIZE - ROAD_GAP));
    }

    BoundingBox3D District::get_below_district_bb() const {
        return BoundingBox3D(
            bounding_box.min + glm::vec3(0.0f, 0.0f, DISTRICT_SIZE + ROAD_GAP),
            bounding_box.max + glm::vec3(0.0f, 0.0f, DISTRICT_SIZE + ROAD_GAP));
    }

}

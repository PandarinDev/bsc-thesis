#include "generator.h"
#include "gfx/geometry.h"

#include <cmath>
#include <stdexcept>
#include <functional>
#include <unordered_map>

namespace inf {

    WorldGenerator::WorldGenerator(RandomGenerator& random_engine, const gfx::Renderer& renderer) :
        random_engine(random_engine), renderer(renderer) {}

    World WorldGenerator::generate_initial() {
        World world;
        auto& district = world.districts.emplace(glm::ivec2(0, 0), generate_district(glm::ivec2(0, 0))).first->second;

        // Center the district compared to where the camera initially intersects the ground plane
        const auto& camera = renderer.get_camera();
        const gfx::Ray camera_ray(camera.get_position(), camera.get_direction());
        const gfx::Plane ground_plane(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 0.0f);
        const auto maybe_intersection = camera_ray.intersect(ground_plane);
        if (maybe_intersection) {
            const auto intersection = glm::floor(camera_ray.point_at(maybe_intersection.value()));
            const auto district_bb = district.compute_bounding_box();
            const auto district_width = district_bb.width();
            const auto district_depth = district_bb.depth();
            district.set_position(glm::vec3(intersection.x - district_width * 0.5f, 0.0f, intersection.z - district_depth * 0.5f));
        }

        // Make sure the district caches have been initialized
        district.update_caches();

        return world;
    }

    void WorldGenerator::populate_world(World& world) {
        // TODO: This should be done recursively instead to avoid scenarios when a large chunk of
        // districts would become visible at once but we only generate one per frame. Realistically
        // this is only a problem in freecam situations and even then it is not a big deal.
        for (const auto& entry : world.districts) {
            const auto& district = entry.second;
            const auto& grid_position = entry.first;
            const auto& world_position = district.get_position();
            const auto district_bb = district.compute_bounding_box();
            // Left
            if (!world.has_district_at(grid_position + glm::ivec2(-1, 0)) &&
                renderer.is_in_view(district_bb.get_block_to_the_left())) {
                const auto new_district_grid_position = grid_position + glm::ivec2(-1, 0);
                auto& new_district = world.districts.emplace(new_district_grid_position, generate_district(new_district_grid_position)).first->second;
                const auto new_district_bb = new_district.compute_bounding_box();
                new_district.set_position(glm::vec3(world_position.x - new_district_bb.width(), world_position.y, world_position.z));
                new_district.update_caches();
            }
            // Right
            if (!world.has_district_at(grid_position + glm::ivec2(1, 0)) &&
                renderer.is_in_view(district_bb.get_block_to_the_right())) {
                const auto new_district_grid_position = grid_position + glm::ivec2(1, 0);
                auto& new_district = world.districts.emplace(new_district_grid_position, generate_district(new_district_grid_position)).first->second;
                new_district.set_position(glm::vec3(world_position.x + district_bb.width(), world_position.y, world_position.z));
                new_district.update_caches();
            }
            // Top
            if (!world.has_district_at(grid_position + glm::ivec2(0, 1)) &&
                renderer.is_in_view(district_bb.get_block_above())) {
                const auto new_district_grid_position = grid_position + glm::ivec2(0, 1);
                auto& new_district = world.districts.emplace(new_district_grid_position, generate_district(new_district_grid_position)).first->second;
                const auto new_district_bb = new_district.compute_bounding_box();
                new_district.set_position(glm::vec3(world_position.x, world_position.y, world_position.z - new_district_bb.depth()));
                new_district.update_caches();
            }
            // Bottom
            if (!world.has_district_at(grid_position + glm::ivec2(0, -1)) &&
                renderer.is_in_view(district_bb.get_block_below())) {
                const auto new_district_grid_position = grid_position + glm::ivec2(0, -1);
                auto& new_district = world.districts.emplace(new_district_grid_position, generate_district(new_district_grid_position)).first->second;
                new_district.set_position(glm::vec3(world_position.x, world_position.y, world_position.z + district_bb.depth()));
                new_district.update_caches();
            }
        }
    }

    District WorldGenerator::generate_district(const glm::ivec2& grid_position) {
        static constexpr auto district_width = 100;
        static constexpr auto district_depth = 100;
        std::uniform_real_distribution<float> color_distribution(0.0f, 1.0f);
        auto district = District(DistrictType::RESIDENTAL, grid_position, glm::ivec2(district_width, district_depth),
            glm::vec3(color_distribution(random_engine), color_distribution(random_engine), color_distribution(random_engine)));
        // Slice up the district into lots
        static constexpr auto min_lot_width = 8;
        static constexpr auto max_lot_width = 10;
        std::uniform_int_distribution<int> lot_width_distribution(min_lot_width, max_lot_width);
        static constexpr auto min_lot_depth = 6;
        static constexpr auto max_lot_depth = 8;
        std::uniform_int_distribution<int> lot_depth_distribution(min_lot_depth, max_lot_depth);
        std::vector<glm::ivec4> partitions{ glm::vec4{ 0, 0, district_width, district_depth } };
        std::unordered_map<glm::ivec2, DistrictRoad> roads;
        const auto is_partition_sufficiently_sized = [](const glm::ivec4& partition) {
            const auto width = partition.z - partition.x;
            if (width > max_lot_width) {
                return false;
            }
            const auto depth = partition.w - partition.y;
            if (depth > max_lot_depth) {
                return false;
            }
            // We do not need to check for smaller than minimum sizes, because the slicing
            // algorithm should guarantee that we never slice into smaller partitions than
            // what is the minimum size for a lot.
            return true;
        };
        const auto are_partitions_sufficiently_sized = [&]() {
            for (const auto& partition : partitions) {
                if (!is_partition_sufficiently_sized(partition)) {
                    return false;
                }
            }
            return true;
        };
        // Lot gap is used to introduce gaps between lots where roads will be placed
        static constexpr auto lot_gap = 2;
        while (!are_partitions_sufficiently_sized()) {
            std::vector<glm::ivec4> new_partitions;
            for (const auto& partition : partitions) {
                const auto width = partition.z - partition.x;
                const auto depth = partition.w - partition.y;
                // Cut partition vertically if needed
                if (width > max_lot_width) {
                    std::uniform_int_distribution<int> slice_distribution(
                        static_cast<int>(width * 0.4f),
                        static_cast<int>(width * 0.6f));
                    const auto slice_at = slice_distribution(random_engine);
                    new_partitions.emplace_back(partition.x, partition.y, partition.x + slice_at, partition.w);
                    new_partitions.emplace_back(partition.x + slice_at + lot_gap, partition.y, partition.z, partition.w);
                    // Add a vertical road strip along the created gap
                    for (int offset = partition.y; offset <= partition.w; ++offset) {
                        const auto road_position_left = glm::ivec2(partition.x + slice_at, offset);
                        const auto road_position_right = road_position_left + glm::ivec2(1, 0);
                        const auto road_it = roads.find(road_position_left);
                        roads.emplace(road_position_left, DistrictRoad(RoadDirection::VERTICAL_LEFT, road_position_left));
                        roads.emplace(road_position_right, DistrictRoad(RoadDirection::VERTICAL_RIGHT, road_position_right));
                    }
                }
                // Cut partition horizontally if needed
                else if (depth > max_lot_depth) {
                    std::uniform_int_distribution<int> slice_distribution(
                        static_cast<int>(depth * 0.4f),
                        static_cast<int>(depth * 0.6f));
                    const auto slice_at = slice_distribution(random_engine);
                    new_partitions.emplace_back(partition.x, partition.y, partition.z, partition.y + slice_at);
                    new_partitions.emplace_back(partition.x, partition.y + slice_at + lot_gap, partition.z, partition.w);
                    // Add a horizontal road strip along the created gap
                    for (int offset = partition.x; offset <= partition.z; ++offset) {
                        const auto road_position_up = glm::ivec2(offset, partition.y + slice_at);
                        const auto road_position_down = road_position_up + glm::ivec2(0, 1);
                        const auto road_it = roads.find(road_position_up);
                        roads.emplace(road_position_up, DistrictRoad(RoadDirection::HORIZONTAL_UP, road_position_up));
                        roads.emplace(road_position_down, DistrictRoad(RoadDirection::HORIZONTAL_DOWN, road_position_down));
                    }
                }
                // Otherwise the partition is sufficiently sized and we simply move it the list of new partitions
                else {
                    new_partitions.emplace_back(partition);
                }
            }
            partitions = std::move(new_partitions);
        }

        // Post process roads to add crossings
        const auto has_road_direction = [&roads](const glm::ivec2& position, RoadDirection direction) {
            const auto it = roads.find(position);
            if (it == roads.cend()) {
                return false;
            }
            return it->second.direction == direction;
        };
        for (auto& entry : roads) {
            const auto& position = entry.first;
            const auto left_neighbor = position + glm::ivec2(-1, 0);
            const auto right_neighbor = position + glm::ivec2(1, 0);
            const auto up_neighbor = position + glm::ivec2(0, -1);
            const auto down_neighbor = position + glm::ivec2(0, 1);
            if (has_road_direction(left_neighbor, RoadDirection::HORIZONTAL_UP) &&
                has_road_direction(up_neighbor, RoadDirection::VERTICAL_LEFT)) {
                entry.second.direction = RoadDirection::CROSSING_UP_LEFT;
            }
            else if (has_road_direction(right_neighbor, RoadDirection::HORIZONTAL_UP) &&
                has_road_direction(up_neighbor, RoadDirection::VERTICAL_RIGHT)) {
                entry.second.direction = RoadDirection::CROSSING_UP_RIGHT;
            }
            else if (has_road_direction(left_neighbor, RoadDirection::HORIZONTAL_DOWN) &&
                has_road_direction(down_neighbor, RoadDirection::VERTICAL_LEFT)) {
                entry.second.direction = RoadDirection::CROSSING_DOWN_LEFT;
            }
            else if (has_road_direction(right_neighbor, RoadDirection::HORIZONTAL_DOWN) &&
                has_road_direction(down_neighbor, RoadDirection::VERTICAL_RIGHT)) {
                entry.second.direction = RoadDirection::CROSSING_DOWN_RIGHT;
            }
        }

        // Turn partitions into lots by generating buildings on them
        for (const auto& partition : partitions) {
            const auto width = partition.z - partition.x;
            const auto depth = partition.w - partition.y;
            const auto patterns = wfc::BuildingPatterns::get_patterns(width, depth);
            const wfc::BuildingPattern* pattern = nullptr;
            if (!patterns.empty()) {
                std::uniform_int_distribution<int> pattern_distribution(0, static_cast<int>(patterns.size() - 1));
                pattern = patterns[pattern_distribution(random_engine)];
            }
            // If the dimensions are not suitable for any pattern for the district the lot remains vacant, otherwise generate building that is guaranteed to fit
            district.add_lot(DistrictLot(
                glm::ivec2(partition),
                glm::ivec2(width, depth),
                glm::vec3(color_distribution(random_engine), color_distribution(random_engine), color_distribution(random_engine)),
                pattern
                    ? std::make_optional(generate_building(*pattern, width - 1, depth - 1))
                    : std::nullopt));
        }

        // Add created roads to the district
        for (auto& entry : roads) {
            district.add_road(std::move(entry.second));
        }

        return district;
    }

    wfc::Building WorldGenerator::generate_building(const wfc::BuildingPattern& pattern, int max_width, int max_depth) {
        return pattern.instantiate(
            random_engine,
            &renderer.get_logical_device(),
            &renderer.get_memory_allocator(),
            max_width,
            max_depth);
    }

}
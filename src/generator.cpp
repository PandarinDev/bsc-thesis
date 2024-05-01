#include "generator.h"
#include "gfx/geometry.h"
#include "gfx/particles.h"
#include "utils/random_utils.h"

#include <array>
#include <cmath>
#include <stdexcept>
#include <functional>
#include <unordered_map>

namespace inf {

    WorldGenerator::WorldGenerator(RandomGenerator& random_engine, const gfx::Renderer& renderer) :
        random_engine(random_engine), renderer(renderer) {}

    World WorldGenerator::generate_initial(const Timer& timer) {
        const auto rain_particles_factory = [this](int num_rain_particles) {
            return gfx::ParticleSystem(
                &gfx::ParticleMeshes::get_rain_mesh(),
                random_engine,
                renderer.get_frustum_in_world_space(),
                num_rain_particles);
        };

        World world(timer, rain_particles_factory);
        auto& district = world.add_district(glm::ivec2(0, 0), generate_district(glm::ivec2(0, 0)));

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
        else {
            throw std::runtime_error("Ground intersection not found.");
        }

        // Make sure the district caches have been initialized
        district.update_caches();

        return world;
    }

    void WorldGenerator::populate_world(World& world) {
        // TODO: This should be done recursively instead to avoid scenarios when a large chunk of
        // districts would become visible at once but we only generate one per frame. Realistically
        // this is only a problem in freecam situations and even then it is not a big deal.
        const auto frustum = renderer.get_frustum_in_view_space();
        const auto transformation = renderer.get_view_matrix();
        for (const auto& entry : world.get_districts()) {
            const auto& district = entry.second;
            const auto& grid_position = entry.first;
            const auto& world_position = district.get_position();
            const auto district_bb = district.compute_bounding_box();
            // Left
            const auto left_position = grid_position + glm::ivec2(-1, 0);
            if (!world.has_district_at(left_position) &&
                frustum.is_inside(district.get_left_district_bb().to_oriented(transformation))) {
                auto& new_district = world.add_district(left_position, generate_district(left_position));
                const auto new_district_bb = new_district.compute_bounding_box();
                new_district.set_position(glm::vec3(world_position.x - new_district_bb.width() - District::ROAD_GAP, world_position.y, world_position.z));
                new_district.update_caches();
            }
            // Right
            const auto right_position = grid_position + glm::ivec2(1, 0);
            if (!world.has_district_at(right_position) &&
                frustum.is_inside(district.get_right_district_bb().to_oriented(transformation))) {
                auto& new_district = world.add_district(right_position, generate_district(right_position));
                new_district.set_position(glm::vec3(world_position.x + district_bb.width() + District::ROAD_GAP, world_position.y, world_position.z));
                new_district.update_caches();
            }
            // Top
            const auto top_position = grid_position + glm::ivec2(0, 1);
            if (!world.has_district_at(top_position) &&
                frustum.is_inside(district.get_above_district_bb().to_oriented(transformation))) {
                auto& new_district = world.add_district(top_position, generate_district(top_position));
                const auto new_district_bb = new_district.compute_bounding_box();
                new_district.set_position(glm::vec3(world_position.x, world_position.y, world_position.z - new_district_bb.depth() - District::ROAD_GAP));
                new_district.update_caches();
            }
            // Bottom
            const auto bottom_position = grid_position + glm::ivec2(0, -1);
            if (!world.has_district_at(bottom_position) &&
                frustum.is_inside(district.get_below_district_bb().to_oriented(transformation))) {
                auto& new_district = world.add_district(bottom_position, generate_district(bottom_position));
                new_district.set_position(glm::vec3(world_position.x, world_position.y, world_position.z + district_bb.depth() + District::ROAD_GAP));
                new_district.update_caches();
            }
        }
    }

    District WorldGenerator::generate_district(const glm::ivec2& grid_position) {
        std::uniform_real_distribution<float> color_distribution(0.0f, 1.0f);
        const auto bb_color = glm::vec3(color_distribution(random_engine), color_distribution(random_engine), color_distribution(random_engine));
        auto district = District(DistrictType::RESIDENTAL, grid_position, glm::ivec2(District::DISTRICT_SIZE, District::DISTRICT_SIZE), bb_color);
        // Slice up the district into lots
        static constexpr auto min_lot_width = 8;
        static constexpr auto max_lot_width = 10;
        std::uniform_int_distribution<int> lot_width_distribution(min_lot_width, max_lot_width);
        static constexpr auto min_lot_depth = 6;
        static constexpr auto max_lot_depth = 8;
        std::uniform_int_distribution<int> lot_depth_distribution(min_lot_depth, max_lot_depth);
        std::vector<glm::ivec4> partitions{ glm::vec4{ 0, 0, District::DISTRICT_SIZE, District::DISTRICT_SIZE } };
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
        // Road gap is used to introduce gaps between lots where roads will be placed
        static constexpr auto road_gap = 2;
        const auto road_mesh = &wfc::GroundPatterns::get_pattern("road").mesh;
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
                    new_partitions.emplace_back(partition.x + slice_at + road_gap, partition.y, partition.z, partition.w);
                    // Add a vertical road strip along the created gap
                    for (int offset = partition.y; offset < partition.w; ++offset) {
                        const auto road_position_left = glm::ivec2(partition.x + slice_at, offset);
                        const auto road_position_right = road_position_left + glm::ivec2(1, 0);
                        roads.emplace(road_position_left, DistrictRoad(RoadDirection::VERTICAL_LEFT, road_position_left, false, road_mesh));
                        roads.emplace(road_position_right, DistrictRoad(RoadDirection::VERTICAL_RIGHT, road_position_right, false, road_mesh));
                    }
                }
                // Cut partition horizontally if needed
                else if (depth > max_lot_depth) {
                    std::uniform_int_distribution<int> slice_distribution(
                        static_cast<int>(depth * 0.4f),
                        static_cast<int>(depth * 0.6f));
                    const auto slice_at = slice_distribution(random_engine);
                    new_partitions.emplace_back(partition.x, partition.y, partition.z, partition.y + slice_at);
                    new_partitions.emplace_back(partition.x, partition.y + slice_at + road_gap, partition.z, partition.w);
                    // Add a horizontal road strip along the created gap
                    for (int offset = partition.x; offset < partition.z; ++offset) {
                        const auto road_position_up = glm::ivec2(offset, partition.y + slice_at);
                        const auto road_position_down = road_position_up + glm::ivec2(0, 1);
                        roads.emplace(road_position_up, DistrictRoad(RoadDirection::HORIZONTAL_UP, road_position_up, false, road_mesh));
                        roads.emplace(road_position_down, DistrictRoad(RoadDirection::HORIZONTAL_DOWN, road_position_down, false, road_mesh));
                    }
                }
                // Otherwise the partition is sufficiently sized and we simply move it the list of new partitions
                else {
                    new_partitions.emplace_back(partition);
                }
            }
            partitions = std::move(new_partitions);
        }

        // Post process roads to set their directions (which is not trivial to do before, because we need to account for crossings)
        // and to set their traversability depending on whether they are located on the edges of a district.
        std::vector<glm::ivec4> non_edge_partitions;
        for (const auto& partition : partitions) {
            if (partition.x != 0 &&
                partition.y != 0 &&
                partition.z != District::DISTRICT_SIZE &&
                partition.w != District::DISTRICT_SIZE) {
                non_edge_partitions.emplace_back(partition);
            }
        }
        set_road_directions_and_traversability(roads, non_edge_partitions);

        // Place vehicles randomly onto traversable roads that are not crossings
        static constexpr auto num_vehicles_per_district = 50;
        std::vector<const DistrictRoad*> road_vector;
        road_vector.reserve(roads.size());
        for (const auto& [_, road] : roads) {
            if (road.traversable && !road.is_crossing()) {
                road_vector.emplace_back(&road);
            }
        }
        const auto roads_to_place_vehicles_on = utils::RandomUtils::choose(random_engine, road_vector, num_vehicles_per_district);
        std::vector<Vehicle> vehicles;
        for (const auto& road_ptr : roads_to_place_vehicles_on) {
            const auto& road = **road_ptr;
            std::deque<glm::ivec2> targets = { road.position + RoadUtils::road_direction_to_grid_direction(road.direction) };
            const auto& vehicle_pattern = VehiclePatterns::get_random_pattern(random_engine);
            vehicles.emplace_back(vehicle_pattern.instantiate(
                random_engine, &renderer.get_logical_device(), &renderer.get_memory_allocator(), road.position, targets));
        }

        // Turn partitions into lots by generating buildings on them
        for (const auto& partition : partitions) {
            const auto width = partition.z - partition.x;
            const auto depth = partition.w - partition.y;
            const auto patterns = wfc::BuildingPatterns::get_patterns(width, depth);
            const wfc::BuildingPattern* pattern = nullptr;
            if (!patterns.empty()) {
                // Sum the weights and use them to form a distribution
                int sum_weights = 0;
                for (const auto& entry : patterns) {
                    sum_weights += entry->weight;
                }
                std::uniform_int_distribution<int> pattern_distribution(1, sum_weights);
                int result = pattern_distribution(random_engine);
                int accumulator = 0;
                for (const auto& entry : patterns) {
                    if (result > accumulator && result <= entry->weight + accumulator) {
                        pattern = entry;
                        break;
                    }
                    accumulator += entry->weight;
                }
            }
            // If the dimensions are not suitable for any pattern for the district the lot remains vacant, otherwise generate building that is guaranteed to fit
            auto building = pattern
                    ? std::make_optional(generate_building(*pattern, width - 1, depth - 1))
                    : std::nullopt;
            
            DistrictFoliage foliage;
            const auto building_dimensions = building ? building->get_dimensions() : glm::ivec3();
            // TODO: Foliage placement logic could be improved quite a bit, but this works for now
            static constexpr auto foliage_chance = 0.25f;
            std::uniform_real_distribution<float> random_dist(0.0f, 1.0f);
            if (building_dimensions.x < width - 2) {
                std::uniform_int_distribution<int> vertical_dist(1, depth - 1);
                // Potentially put foliage on the left
                if (random_dist(random_engine) < foliage_chance) {
                    const auto& foliage_pattern = wfc::GroundPatterns::get_random_foliage_pattern(random_engine);
                    foliage[&foliage_pattern].emplace_back(glm::vec3(1.0f, 0.0f, vertical_dist(random_engine)));
                }
                // Potentially to put foliage on the right
                if (random_dist(random_engine) < foliage_chance) {
                    const auto& foliage_pattern = wfc::GroundPatterns::get_random_foliage_pattern(random_engine);
                    foliage[&foliage_pattern].emplace_back(glm::vec3(width - 1.0f, 0.0f, vertical_dist(random_engine)));
                }
            }
            if (building_dimensions.z < depth - 2) {
                std::uniform_int_distribution<int> horizontal_dist(1, width - 1);
                // Potentially put foliage on top
                if (random_dist(random_engine) < foliage_chance) {
                    const auto& foliage_pattern = wfc::GroundPatterns::get_random_foliage_pattern(random_engine);
                    foliage[&foliage_pattern].emplace_back(glm::vec3(horizontal_dist(random_engine), 0.0f, 1.0f));
                }
                // Potentially put foliage on bottom
                if (random_dist(random_engine) < foliage_chance) {
                    const auto& foliage_pattern = wfc::GroundPatterns::get_random_foliage_pattern(random_engine);
                    foliage[&foliage_pattern].emplace_back(glm::vec3(horizontal_dist(random_engine), 0.0f, depth - 1.0f));
                }
            }

            district.add_lot(DistrictLot(
                glm::ivec2(partition),
                glm::ivec2(width, depth),
                glm::vec3(color_distribution(random_engine), color_distribution(random_engine), color_distribution(random_engine)),
                std::move(building),
                std::move(foliage)));
        }

        // Add created roads to the district
        for (auto& entry : roads) {
            district.add_road(std::move(entry.second));
        }

        // Add created vehicles to the district
        for (auto& vehicle : vehicles) {
            district.add_vehicle(std::move(vehicle));
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

    bool WorldGenerator::has_road_direction(
        const std::unordered_map<glm::ivec2, DistrictRoad>& roads,
        const glm::ivec2& position,
        RoadDirection direction) {
        const auto it = roads.find(position);
        return it != roads.cend() && it->second.direction == direction;
    }

    void WorldGenerator::set_road_directions_and_traversability(
        std::unordered_map<glm::ivec2, DistrictRoad>& roads,
        const std::vector<glm::ivec4>& non_edge_partitions) {
        // Set direction and mesh based on neighboring roads
        for (auto& entry : roads) {
            const auto& position = entry.first;
            auto& road = entry.second;
            const auto left_neighbor = position + glm::ivec2(-1, 0);
            const auto right_neighbor = position + glm::ivec2(1, 0);
            const auto up_neighbor = position + glm::ivec2(0, -1);
            const auto down_neighbor = position + glm::ivec2(0, 1);
            if (has_road_direction(roads, left_neighbor, RoadDirection::HORIZONTAL_UP) &&
                has_road_direction(roads, up_neighbor, RoadDirection::VERTICAL_LEFT)) {
                road.direction = RoadDirection::CROSSING_UP_LEFT;
                road.mesh = &wfc::GroundPatterns::get_random_crossing_pattern(random_engine).mesh;
            }
            else if (has_road_direction(roads, right_neighbor, RoadDirection::HORIZONTAL_UP) &&
                has_road_direction(roads, up_neighbor, RoadDirection::VERTICAL_RIGHT)) {
                road.direction = RoadDirection::CROSSING_UP_RIGHT;
                road.mesh = &wfc::GroundPatterns::get_random_crossing_pattern(random_engine).mesh;
            }
            else if (has_road_direction(roads, left_neighbor, RoadDirection::HORIZONTAL_DOWN) &&
                has_road_direction(roads, down_neighbor, RoadDirection::VERTICAL_LEFT)) {
                road.direction = RoadDirection::CROSSING_DOWN_LEFT;
                road.mesh = &wfc::GroundPatterns::get_random_crossing_pattern(random_engine).mesh;
            }
            else if (has_road_direction(roads, right_neighbor, RoadDirection::HORIZONTAL_DOWN) &&
                has_road_direction(roads, down_neighbor, RoadDirection::VERTICAL_RIGHT)) {
                road.direction = RoadDirection::CROSSING_DOWN_RIGHT;
                road.mesh = &wfc::GroundPatterns::get_random_crossing_pattern(random_engine).mesh;
            }
        }

        // Set traversability. The easiest way to do this is to set the traversability of all roads to false
        // earlier during generation and set traversability to true around every non-edge partition.
        const auto set_traversable = [&roads](const std::vector<glm::ivec2>& neighbors) {
            for (const auto& neighbor : neighbors) {
                const auto it = roads.find(neighbor);
                if (it != roads.cend()) {
                    it->second.traversable = true;
                }
            }
        };
        for (const auto& partition : non_edge_partitions) {
            std::vector<glm::ivec2> neighbors;
            for (int x = partition.x - 2; x < partition.z + 2; ++x) {
                neighbors.emplace_back(glm::ivec2(x, partition.y - 1));
                neighbors.emplace_back(glm::ivec2(x, partition.y - 2));
                neighbors.emplace_back(glm::ivec2(x, partition.w));
                neighbors.emplace_back(glm::ivec2(x, partition.w + 1));
            }
            for (int y = partition.y - 2; y < partition.w + 2; ++y) {
                neighbors.emplace_back(glm::ivec2(partition.x - 1, y));
                neighbors.emplace_back(glm::ivec2(partition.x - 2, y));
                neighbors.emplace_back(glm::ivec2(partition.z, y));
                neighbors.emplace_back(glm::ivec2(partition.z + 1, y));
            }
            set_traversable(neighbors);
        }
    }

}
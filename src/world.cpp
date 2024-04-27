#include "world.h"
#include "utils/random_utils.h"

#include <array>
#include <magic_enum.hpp>

namespace inf {

    // TODO: Move these to context and make them user controllable
    static constexpr float WEATHER_CHANGE_CHECK_FREQUENCY_SECONDS = 10.0f;
    static constexpr float WEATHER_CHANGE_CHANCE_PERCENTAGE = 0.5f;

    static const std::array<int, magic_enum::enum_count<RainIntensity>()> rain_intensity_to_num_particles {
        0,
        1000,
        3000,
        5000
    };

    World::World(const Timer& timer, std::function<gfx::ParticleSystem(int)> rain_particle_factory) :
        timer(timer), dirty(true), weather(Weather::SUNNY),
        rain_intensity(RainIntensity::NONE), rain_particle_factory(rain_particle_factory),
        last_weather_change_check(static_cast<float>(timer.get_time())) {}

    bool World::has_district_at(const glm::ivec2& position) const {
        return districts.find(position) != districts.cend();
    }

    District& World::add_district(const glm::ivec2& position, District&& district) {
        // Flag as dirty to recompute caches before rendering
        dirty = true;
        return districts.emplace(position, std::move(district)).first->second;
    }

    const std::unordered_map<glm::ivec2, District>& World::get_districts() const {
        return districts;
    }

    std::size_t World::get_number_of_districts() const {
        return districts.size();
    }

    std::size_t World::get_number_of_buildings() const {
        std::size_t accumulator = 0;
        for (const auto& entry : districts) {
            const auto& district = entry.second;
            // This uses the assumption that there is exactly one building per lot
            accumulator += district.get_lots().size();
        }
        return accumulator;
    }

    BoundingBox3D World::compute_bounding_box() const {
        BoundingBox3D result;
        for (const auto& entry : districts) {
            const auto& district = entry.second;
            result.update(district.compute_bounding_box());
        }
        return result;
    }

    void World::update(const gfx::Renderer& renderer, RandomGenerator& rng, float delta_time) {
        const auto frustum = renderer.get_frustum_in_view_space();
        const auto transformation = renderer.get_view_matrix();
        // Remove districts that are not visible anymore
        std::vector<glm::ivec2> keys_to_remove;
        for (const auto& entry : districts) {
            const auto& district = entry.second;
            const auto district_bb = district.compute_bounding_box();
            if (!frustum.is_inside(district_bb.to_oriented(transformation))) {
                keys_to_remove.emplace_back(entry.first);
            }
        }
        for (const auto& key : keys_to_remove) {
            districts.erase(key);
        }
        if (!keys_to_remove.empty()) {
            dirty = true;
        }

        // Update the remaining districts
        for (auto& [_, district] : districts) {
            district.update(rng, delta_time);
        }

        // Potentially change weather
        const auto time = static_cast<float>(timer.get_time());
        if (time - last_weather_change_check > WEATHER_CHANGE_CHECK_FREQUENCY_SECONDS) {
            std::uniform_real_distribution<float> weather_change_distribution(0.0f, 1.0f);
            const auto result = weather_change_distribution(rng);
            if (result <= WEATHER_CHANGE_CHANCE_PERCENTAGE) {
                const auto possible_new_weathers = get_possible_new_weathers();
                std::uniform_int_distribution<std::size_t> weather_index_distribution(0, possible_new_weathers.size() - 1);
                const auto weather_index = weather_index_distribution(rng);
                const auto [new_weather, new_rain_intensity] = possible_new_weathers[weather_index];
                on_weather_change(new_weather, new_rain_intensity);
            }
            last_weather_change_check = time;
        }

        // Update particle system
        if (rain_particles) {
            rain_particles->update(renderer.get_frustum_in_world_space(), delta_time);
        }
    }

    void World::render(gfx::Renderer& renderer) {
        for (auto& entry : districts) {
            auto& district = entry.second;
            district.render(renderer);
        }

        // Render roads between districts
        const auto& road = wfc::GroundPatterns::get_pattern("road").mesh;
        const auto& crossing = wfc::GroundPatterns::get_pattern("road_crossing").mesh;
        renderer.render_instanced(road, road_positions, road_rotations);
        renderer.render_instanced(crossing, crossing_positions, crossing_rotations);
        
        // Render rain particles
        if (rain_particles) {
            renderer.render_particles(*rain_particles->mesh, rain_particles->positions);
        }
    }

    bool World::is_dirty() const {
        return dirty;
    }

    void World::update_caches() {
        road_positions.clear();
        road_rotations.clear();
        crossing_positions.clear();
        crossing_rotations.clear();

        for (const auto& [grid_position, district] : districts) {
            const auto roads_at_edges = district.get_roads_at_edges();

            // Place a road to the right of each district to connect their roads seamlessly
            const auto right_district_it = districts.find(grid_position + glm::ivec2(1, 0));
            const auto right_district = (right_district_it != districts.cend()) ? &right_district_it->second : nullptr;
            const auto right_roads_at_edges = right_district
                ? right_district->get_roads_at_edges()
                : std::unordered_map<glm::ivec2, const DistrictRoad*>{};
            place_vertical_road(&district, roads_at_edges, right_roads_at_edges);

            // Place a road to the top of each district to connect their roads seamlessly
            const auto top_district_it = districts.find(grid_position + glm::ivec2(0, -1));
            const auto top_district = (top_district_it != districts.cend()) ? &top_district_it->second : nullptr;
            const auto top_roads_at_edges = top_district
                ? top_district->get_roads_at_edges()
                : std::unordered_map<glm::ivec2, const DistrictRoad*>{};
            place_horizontal_road(&district, top_roads_at_edges, roads_at_edges);

            // Seal the corners
            const auto& world_position = district.get_position();
            const auto& dimensions = district.get_dimensions();
            // Bottom left
            crossing_positions.emplace_back(glm::vec3(world_position.x - 0.5f, world_position.y + 0.5f, world_position.z - 0.5f));
            crossing_rotations.emplace_back(glm::radians(270.0f));
            // Bottom right
            crossing_positions.emplace_back(glm::vec3(world_position.x + dimensions.x + 0.5f, world_position.y + 0.5f, world_position.z - 0.5f));
            crossing_rotations.emplace_back(0.0f);
            // Top left
            crossing_positions.emplace_back(glm::vec3(world_position.x - 0.5f, world_position.y + 0.5f, world_position.z + dimensions.y + 0.5f));
            crossing_rotations.emplace_back(glm::radians(180.0f));
            // Top right
            crossing_positions.emplace_back(glm::vec3(world_position.x + dimensions.x + 0.5f, world_position.y + 0.5f, world_position.z + dimensions.y + 0.5f));
            crossing_rotations.emplace_back(glm::radians(90.0f));
        }

        // Clear the dirty flag to avoid recomputing caches when not necessary
        dirty = false;
    }

    void World::place_vertical_road(
            const District* left,
            const std::unordered_map<glm::ivec2, const DistrictRoad*>& left_roads_at_edges,
            const std::unordered_map<glm::ivec2, const DistrictRoad*>& right_roads_at_edges) {
        const auto& world_position = left->get_position();
        const auto& dimensions = left->get_dimensions();
        const auto base_position = glm::vec3(world_position.x + dimensions.x + 0.5f, world_position.y + 0.5f, world_position.z + 0.5f);
        for (int z_offset = 0; z_offset < dimensions.y; ++z_offset) {
            const auto& left_position = glm::vec3(base_position.x, base_position.y, base_position.z + z_offset);
            const auto right_position = glm::vec3(left_position.x + 1.0f, left_position.y, left_position.z);

            // Check if the left road needs to be a crossing
            if (const auto it = left_roads_at_edges.find(glm::ivec2(left->get_dimensions().x - 1, z_offset)); it != left_roads_at_edges.cend()) {
                crossing_positions.emplace_back(left_position);
                crossing_rotations.emplace_back(it->second->direction == RoadDirection::HORIZONTAL_UP ? glm::radians(90.f) : 0.0f);
            }
            else {
                road_positions.emplace_back(left_position);
                road_rotations.emplace_back(0.0f);    
            }

            // Check if the right road needs to be a crossing
            if (const auto it = right_roads_at_edges.find(glm::ivec2(0, z_offset)); it != right_roads_at_edges.cend()) {
                crossing_positions.emplace_back(right_position);
                crossing_rotations.emplace_back(it->second->direction == RoadDirection::HORIZONTAL_DOWN ? glm::radians(270.0f) : glm::radians(180.0f));
            }
            else {
                road_positions.emplace_back(right_position);
                road_rotations.emplace_back(glm::radians(180.0f));
            }
        }
    }


    void World::place_horizontal_road(
            const District* bottom,
            const std::unordered_map<glm::ivec2, const DistrictRoad*>& top_roads_at_edges,
            const std::unordered_map<glm::ivec2, const DistrictRoad*>& bottom_roads_at_edges) {
        const auto& world_position = bottom->get_position();
        const auto& dimensions = bottom->get_dimensions();
        for (int x_offset = 0; x_offset < dimensions.x; ++x_offset) {
                const auto bottom_position = glm::vec3(
                    world_position.x + x_offset + 0.5f,
                    world_position.y + 0.5f,
                    world_position.z + dimensions.y + 0.5f);
                const auto top_position = glm::vec3(bottom_position.x, bottom_position.y, bottom_position.z + 1.0f);

                if (const auto it = bottom_roads_at_edges.find(glm::ivec2(x_offset, dimensions.y - 1)); it != bottom_roads_at_edges.cend()) {
                    crossing_positions.emplace_back(bottom_position);
                    crossing_rotations.emplace_back(it->second->direction == RoadDirection::VERTICAL_LEFT ? glm::radians(90.0f) : glm::radians(180.0f));
                }
                else {
                    road_positions.emplace_back(bottom_position);
                    road_rotations.emplace_back(glm::radians(90.0f));
                }

                if (const auto it = top_roads_at_edges.find(glm::ivec2(x_offset, 0)); it != top_roads_at_edges.cend()) {
                    crossing_positions.emplace_back(top_position);
                    crossing_rotations.emplace_back(it->second->direction == RoadDirection::VERTICAL_LEFT ? 0.0f : glm::radians(270.0f)); // TODO: Fix this
                }
                else {
                    road_positions.emplace_back(top_position);
                    road_rotations.emplace_back(glm::radians(270.0f));
                }
            }
    }

    std::vector<std::pair<Weather, RainIntensity>> World::get_possible_new_weathers() const {
        if (weather == Weather::SUNNY) {
            return {{ Weather::RAINY, RainIntensity::LIGHT }};
        }
        if (rain_intensity == RainIntensity::LIGHT) {
            return {
                { Weather::SUNNY, RainIntensity::NONE },
                { Weather::RAINY, RainIntensity::MODERATE }
            };
        }
        if (rain_intensity == RainIntensity::MODERATE) {
            return {
                { Weather::RAINY, RainIntensity::LIGHT },
                { Weather::RAINY, RainIntensity::HEAVY }
            };
        }
        return {{ Weather::RAINY, RainIntensity::MODERATE }};
    }

    void World::on_weather_change(Weather new_weather, RainIntensity new_rain_intensity) {
        if (new_weather == Weather::SUNNY) {
            rain_particles.reset();
        }
        else {
            const auto num_rain_particles = rain_intensity_to_num_particles[static_cast<std::size_t>(new_rain_intensity)];
            rain_particles = std::make_unique<gfx::ParticleSystem>(rain_particle_factory(num_rain_particles));
        }
        this->weather = new_weather;
        this->rain_intensity = new_rain_intensity;
    }

}

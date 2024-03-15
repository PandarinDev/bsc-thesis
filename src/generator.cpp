#include "generator.h"
#include "gfx/geometry.h"

#include <deque>
#include <cmath>
#include <stdexcept>
#include <unordered_map>

#include <iostream>

namespace inf {

    WorldGenerator::WorldGenerator(RandomGenerator& random_engine, const gfx::Renderer& renderer) :
        random_engine(random_engine), renderer(renderer) {}

    World WorldGenerator::generate_initial() {
        World world;
        auto& district = world.districts.emplace_back(DistrictType::RESIDENTAL);
        populate_district(district);
        return world;
    }

    void WorldGenerator::populate_district(District& district) {
        const auto& camera = renderer.get_camera();
        const gfx::Ray camera_ray(camera.get_position(), camera.get_direction());
        const gfx::Plane ground_plane(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 0.0f);
        const auto maybe_intersection = camera_ray.intersect(ground_plane);
        if (!maybe_intersection) {
            throw std::runtime_error("Camera does not intersect ground plane.");
        }
        const auto intersection = glm::floor(camera_ray.point_at(maybe_intersection.value()));

        // Place the initial building around which we'll start the generation
        {
            auto initial_building = generate_building();
            initial_building.set_position(intersection);
            district.add_building(std::move(initial_building));
        }
        const auto& buildings = district.get_buildings();
        for (std::size_t i = 0; i < buildings.size(); ++i) {
            const auto& building = buildings[i];
            const auto& center_position = building.get_position();
            const auto& center_bb = building.get_bounding_box();
            
            auto new_building = generate_building();

            const auto try_place = [&](const glm::vec3& position) {
                new_building.set_position(position);
                const auto new_building_bb = new_building.get_bounding_box();
                if (district.can_place(new_building_bb) && renderer.is_in_view(new_building_bb)) {
                    district.add_building(std::move(new_building));
                    new_building = generate_building();
                }
            };

            static constexpr auto gap = 2.0f;
            try_place(glm::vec3(std::floorf(center_position.x - new_building.get_bounding_box().width() - gap), 0.0f, center_position.z)); // Try to place the building to the left
            try_place(glm::vec3(std::ceilf(center_position.x + building.get_bounding_box().width() + gap), 0.0f, center_position.z));      // Try to place the building to the right
            try_place(glm::vec3(center_position.x, 0.0f, std::floorf(center_position.z - center_bb.depth() - gap)));                       // Try to place the building above
            try_place(glm::vec3(center_position.x, 0.0f, std::ceilf(center_position.z + new_building.get_bounding_box().depth() + gap)));  // Try to place the building below
        }
    }

    void WorldGenerator::populate_district_edges(District& district) {
        // Find the buildings on the edges of the district. This consists of iterating through
        // all of the buildings in the district and for every distinct Z position mark the
        // building with the minimum and maximum X values. Also keep all the buildings that
        // have minimal and maximal Z values.
        float min_z_value = std::numeric_limits<float>::max();
        float max_z_value = std::numeric_limits<float>::lowest();
        std::unordered_map<float, float> min_x_by_z;
        std::unordered_map<float, float> max_x_by_z;
        for (const auto& building : district.get_buildings()) {
            const auto& position = building.get_position();
            if (position.z < min_z_value) min_z_value = position.z;
            if (position.z > max_z_value) max_z_value = position.z;
            const auto min_it = min_x_by_z.find(position.z);
            if (min_it == min_x_by_z.cend() || min_it->second > position.x) {
                min_x_by_z[position.z] = position.x;
            }
            
            const auto max_it = max_x_by_z.find(position.z);
            if (max_it == max_x_by_z.cend() || max_it->second < position.x) {
                max_x_by_z[position.z] = position.x;
            }
        }

        const auto& buildings = district.get_buildings();
        // Since we are going to modify the buildings in the loop we cannot refer to them using pointers.
        // So instead we are referring to them using their indices, which works since we only ever add to the list here.
        std::vector<std::size_t> buildings_to_place_around;
        for (std::size_t i = 0; i < buildings.size(); ++i) {
            const auto& building = buildings[i];
            const auto& position = building.get_position();
            if (position.z == min_z_value || position.z == max_z_value) {
                buildings_to_place_around.emplace_back(i);
                continue;
            }
            const auto min_it = min_x_by_z.find(position.z);
            if (min_it != min_x_by_z.cend() && min_it->second == position.x) {
                buildings_to_place_around.emplace_back(i);
                continue;
            }
            const auto max_it = max_x_by_z.find(position.z);
            if (max_it != max_x_by_z.cend() && max_it->second == position.x) {
                buildings_to_place_around.emplace_back(i);
            }
        }

        // TODO: With some cleverness this could be generalized so that populate_district() and populate_district_edges() share the core of the loop.
        for (std::size_t i = 0; i < buildings_to_place_around.size(); ++i) {
            const auto& building_index = buildings_to_place_around[i];
            const auto& building = buildings[building_index];
            const auto& center_position = building.get_position();
            const auto& center_bb = building.get_bounding_box();
            
            auto new_building = generate_building();

            const auto try_place = [&](const glm::vec3& position) {
                new_building.set_position(position);
                const auto new_building_bb = new_building.get_bounding_box();
                if (district.can_place(new_building_bb) && renderer.is_in_view(new_building_bb)) {
                    district.add_building(std::move(new_building));
                    buildings_to_place_around.emplace_back(district.get_buildings().size() - 1);
                    new_building = generate_building();
                }
            };

            static constexpr auto gap = 2.0f;
            try_place(glm::vec3(std::floorf(center_position.x - new_building.get_bounding_box().width() - gap), 0.0f, center_position.z)); // Try to place the building to the left
            try_place(glm::vec3(std::ceilf(center_position.x + building.get_bounding_box().width() + gap), 0.0f, center_position.z));      // Try to place the building to the right
            try_place(glm::vec3(center_position.x, 0.0f, std::floorf(center_position.z - center_bb.depth() - gap)));                       // Try to place the building above
            try_place(glm::vec3(center_position.x, 0.0f, std::ceilf(center_position.z + new_building.get_bounding_box().depth() + gap)));  // Try to place the building below
        }
    }

    wfc::Building WorldGenerator::generate_building() {
        return wfc::BuildingPatterns::get_pattern("house")->instantiate(
            random_engine,
            &renderer.get_logical_device(),
            &renderer.get_memory_allocator());
    }

}
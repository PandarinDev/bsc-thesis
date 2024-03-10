#include "generator.h"
#include "gfx/geometry.h"

#include <deque>
#include <cmath>
#include <stdexcept>

namespace inf {

    World WorldGenerator::generate_initial(const gfx::Renderer& renderer) {
        World world;
        WorldGenerator generator(&world);
        generator.generate(renderer);
        return world;
    }

    WorldGenerator::WorldGenerator(World* world) : world(world) {
        std::random_device random_device;
        random_engine.seed(random_device());
    }

    void WorldGenerator::generate(const gfx::Renderer& renderer) {
        const auto& physical_device = renderer.get_physical_device();
        const auto logical_device = &renderer.get_logical_device();

        District district(DistrictType::RESIDENTAL);
        const auto& camera = renderer.get_camera();
        const gfx::Ray camera_ray(camera.get_position(), camera.get_direction());
        const gfx::Plane ground_plane(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 0.0f);
        const auto maybe_intersection = camera_ray.intersect(ground_plane);
        if (!maybe_intersection) {
            throw std::runtime_error("Camera does not intersect ground plane.");
        }
        const auto intersection = glm::floor(camera_ray.point_at(maybe_intersection.value()));
        const auto generate_building = [&]() {
            return wfc::BuildingPatterns::get_pattern("house")->instantiate(random_engine, physical_device, logical_device);
        };

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

            static constexpr auto gap = 1.0f;
            try_place(glm::vec3(center_position.x - new_building.get_bounding_box().width() - gap, 0.0f, center_position.z)); // Try to place the building to the left
            try_place(glm::vec3(center_position.x + building.get_bounding_box().width() + gap, 0.0f, center_position.z));     // Try to place the building to the right
            try_place(glm::vec3(center_position.x, 0.0f, center_position.z - center_bb.depth() - gap));                       // Try to place the building above
            try_place(glm::vec3(center_position.x, 0.0f, center_position.z + new_building.get_bounding_box().depth() + gap)); // Try to place the building below
        }

        world->districts.emplace_back(std::move(district));
    }

}
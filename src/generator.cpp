#include "generator.h"
#include "gfx/geometry.h"

#include <cmath>
#include <deque>
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

        std::uniform_int_distribution<int> shift_distribution(2, 3);
        District district(DistrictType::RESIDENTAL);
        const auto& camera = renderer.get_camera();
        const gfx::Ray camera_ray(camera.get_position(), camera.get_direction());
        const gfx::Plane ground_plane(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 0.0f);
        const auto maybe_intersection = camera_ray.intersect(ground_plane);
        if (!maybe_intersection) {
            throw std::runtime_error("Camera does not intersect ground plane.");
        }
        const auto intersection = camera_ray.point_at(maybe_intersection.value());

        std::deque<glm::vec2> coordinates_to_examine;
        coordinates_to_examine.emplace_front(glm::vec2(intersection.x, intersection.z));
        while (!coordinates_to_examine.empty()) {
            const auto coordinate = coordinates_to_examine.front();
            coordinates_to_examine.pop_front();
            auto house = wfc::BuildingPatterns::get_pattern("house")->instantiate(random_engine, physical_device, logical_device);
            house.set_position(glm::vec3(coordinate.x, 0.0f, coordinate.y));
            // If the building is not visible do not place it or examine it's neighbors
            if (!renderer.is_in_view(house.get_bounding_box())) {
                continue;
            }
            // TODO: Compute bb and and neighbors to coordinates to examine
            district.buildings.emplace_back(std::move(house));
        }

        // Generate ground under the entire district
        const auto district_bb = district.compute_bounding_box();
        const auto ground_width = std::ceilf(district_bb.max.x);
        const auto ground_depth = -std::ceilf(district_bb.min.z);
        for (int x = 0; x < ground_width; ++x) {
            for (int z = 0; z < ground_depth; ++z) {
                auto ground = wfc::Ground(
                    wfc::GroundPatterns::get_pattern("grass").mesh,
                    glm::vec3(static_cast<float>(x), 0.0f, -static_cast<float>(z)));
                district.grounds.emplace_back(std::move(ground));
            }
        }

        world->districts.emplace_back(std::move(district));
    }

}
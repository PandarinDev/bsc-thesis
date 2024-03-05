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
        const auto intersection = camera_ray.point_at(maybe_intersection.value());

        std::deque<BoundingBox3D> bbs_to_process;
        const auto generate_building = [&]() { return wfc::BuildingPatterns::get_pattern("house")->instantiate(random_engine, physical_device, logical_device); };
        const auto has_collision = [&](const BoundingBox3D& bb) {
            for (const auto& building : district.buildings) {
                if (building.get_bounding_box().collides(bb)) {
                    return true;
                }
            }
            return false;
        };
        // TODO: All of these place functions should be refactored to a single function
        const auto place_left = [&](const BoundingBox3D& bb) {
            static constexpr auto gap = 0.5f;
            auto left_building = generate_building();
            left_building.set_position(glm::vec3(bb.min.x - left_building.get_bounding_box().width() - gap, 0.0f, bb.max.z));
            const auto left_bb = left_building.get_bounding_box();
            if (renderer.is_in_view(left_bb) && !has_collision(left_bb)) {
                district.buildings.emplace_back(std::move(left_building));
                bbs_to_process.emplace_back(left_bb);
            }
        };
        const auto place_right = [&](const BoundingBox3D& bb) {
            static constexpr auto gap = 0.5f;
            auto right_building = generate_building();
            // TODO: This right here is incorrect (maybe place_left as well). Easy to verify by increasing to 1.5 and seeing how it fixes generation to the right
            right_building.set_position(glm::vec3(bb.max.x + gap, 0.0f, bb.max.z));
            const auto right_bb = right_building.get_bounding_box();
            if (renderer.is_in_view(right_bb) && !has_collision(right_bb)) {
                district.buildings.emplace_back(std::move(right_building));
                bbs_to_process.emplace_back(right_bb);
            }
        };
        const auto place_up = [&](const BoundingBox3D& bb) {
            static constexpr auto gap = 0.5f;
            auto up_building = generate_building();
            up_building.set_position(glm::vec3(bb.min.x, 0.0f, bb.min.z - up_building.get_bounding_box().depth() - gap));
            const auto up_bb = up_building.get_bounding_box();
            if (renderer.is_in_view(up_bb) && !has_collision(up_bb)) {
                district.buildings.emplace_back(std::move(up_building));
                bbs_to_process.emplace_back(up_bb);
            }
        };
        const auto place_down = [&](const BoundingBox3D& bb) {
            static constexpr auto gap = 0.5f;
            auto down_building = generate_building();
            down_building.set_position(glm::vec3(bb.min.x, 0.0f, bb.max.z + down_building.get_bounding_box().depth() + gap));
            const auto down_bb = down_building.get_bounding_box();
            if (renderer.is_in_view(down_bb) && !has_collision(down_bb)) {
                district.buildings.emplace_back(std::move(down_building));
                bbs_to_process.emplace_back(down_bb);
            }
        };

        auto initial_house = generate_building();
        initial_house.set_position(glm::vec3(intersection.x, 0.0f, intersection.z));
        const auto initial_bb = initial_house.get_bounding_box();
        district.buildings.emplace_back(std::move(initial_house));
        bbs_to_process.emplace_back(initial_bb);
        while (!bbs_to_process.empty()) {
            auto bb = std::move(bbs_to_process.front());
            bbs_to_process.pop_front();
            place_left(bb);
            place_right(bb);
            place_up(bb);
            place_down(bb);
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
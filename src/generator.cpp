#include "generator.h"

#include <cmath>

namespace inf {

    World WorldGenerator::generate_initial(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        World world;
        WorldGenerator generator(&world);
        generator.generate(physical_device, logical_device);
        return world;
    }

    WorldGenerator::WorldGenerator(World* world) : world(world) {
        std::random_device random_device;
        random_engine.seed(random_device());
    }

    void WorldGenerator::generate(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        // TODO: For now we generate a district that is able to hold up to 5x5 buildings
        // Later on we want districts to be dynamically generated next to eachother, infinitely.
        District district(DistrictType::RESIDENTAL, std::make_pair(5, 5));
        std::uniform_int_distribution<int> shift_distribution(2, 3);
        std::vector<float> x_shifts(district.capacity.second);
        for (int x = 0; x < district.capacity.first; ++x) {
            float z_shift = 0.0f;
            float max_x = 0.0f;
            for (int z = 0; z < district.capacity.second; ++z) {
                auto house = wfc::BuildingPatterns::get_pattern("house")->instantiate(random_engine, physical_device, logical_device);
                house.set_position(glm::vec3(x_shifts[z], 0.0f, z_shift));
                const auto house_bb = house.get_bounding_box();
                district.buildings.emplace_back(std::move(house));
                x_shifts[z] = house_bb.max.x + shift_distribution(random_engine);
                z_shift = house_bb.min.z - shift_distribution(random_engine);
                if (house_bb.max.x > max_x) {
                    max_x = house_bb.max.x;
                }
            }
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
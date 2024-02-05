#include "generator.h"
#include "wfc/building/house.h"

namespace inf {

    static constexpr auto WORLD_INITIAL_WIDTH = 50;
    static constexpr auto WORLD_INITIAL_HEIGHT = 50;

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
        // TODO: For now we focus on building generation, so always generate a single district with a single building
        District district(DistrictType::RESIDENTAL, std::make_pair(1, 1));
        district.buildings.emplace_back(wfc::building::HouseGenerator::generate(
            random_engine,
            physical_device,
            logical_device
        ));
        world->districts.emplace_back(std::move(district));
    }

}
#pragma once

#include "common.h"
#include "world.h"
#include "gfx/vk/device.h"

namespace inf {

    struct WorldGenerator {

        static World generate_initial(
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);

        WorldGenerator(World* world);

        void generate(
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);

    private:

        World* world;
        RandomGenerator random_engine;

    };

}

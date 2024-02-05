#pragma once

#include "common.h"
#include "world.h"
#include "camera.h"
#include "wfc/rule.h"
#include "gfx/geometry.h"
#include "gfx/vk/device.h"

#include <vector>
#include <memory>
#include <random>

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

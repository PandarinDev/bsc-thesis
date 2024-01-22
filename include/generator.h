#pragma once

#include "world.h"
#include "camera.h"
#include "wfc/rule.h"
#include "gfx/geometry.h"
#include "gfx/vk/device.h"

#include <vector>
#include <memory>
#include <random>

namespace inf {

    using WorldRule = wfc::Rule<World, Cell>;

    struct WorldGenerator {

        static World generate_initial(
            const gfx::Frustum& frustum,
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);

        WorldGenerator(
            World* world,
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);

        void collapse(Cell& cell);

    private:

        World* world;
        std::mt19937 random_engine;
        std::vector<std::unique_ptr<WorldRule>> rules;

    };

}
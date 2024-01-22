#pragma once

#include "world.h"
#include "gfx/mesh.h"
#include "wfc/rule.h"

namespace inf::wfc::building {

    struct FlowerShopRule : public Rule<World, Cell> {

        FlowerShopRule(
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);

        bool matches(const World& world, const Cell& cell) override;
        void apply(World& world, Cell& cell) override;

    private:

        const gfx::vk::PhysicalDevice& physical_device;
        const gfx::vk::LogicalDevice* logical_device;

    };

    struct FlowerShopMeshGenerator {

        FlowerShopMeshGenerator() = delete;

        static gfx::Mesh generate_mesh(
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);

    };

}

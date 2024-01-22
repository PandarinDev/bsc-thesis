#include "generator.h"
#include "wfc/building/house.h"
#include "wfc/building/flower_shop.h"

namespace inf {

    World WorldGenerator::generate_initial(
        const gfx::Frustum& frustum,
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        // TODO: Figure out how to determine the visible coordinates. This problem is not trivial as the
        // size of the building blocks is not uniform. Some buildings are wider and/or taller than others.
        std::vector<glm::ivec3> visible_coordinates;
        for (int x = -3; x <= 3; ++x) {
            visible_coordinates.emplace_back(x, 0.0f, -5.0f);
        }
        World world;
        WorldGenerator generator(&world, physical_device, logical_device);
        for (const auto& coordinate : visible_coordinates) {
            generator.collapse(world.get_or_create_cell(coordinate));
        }

        return world;
    }

    WorldGenerator::WorldGenerator(
        World* world,
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) : world(world) {
        std::random_device random_device;
        random_engine.seed(random_device());
        rules.emplace_back(std::make_unique<wfc::building::HouseRule>(physical_device, logical_device));
        rules.emplace_back(std::make_unique<wfc::building::FlowerShopRule>(physical_device, logical_device));
    }

    void WorldGenerator::collapse(Cell& cell) {
        std::vector<WorldRule*> matching_rules;
        for (const auto& rule : rules) {
            if (rule->matches(*world, cell)) {
                matching_rules.emplace_back(rule.get());
            }
        }
        // TODO: Take into account the rule weights
        std::uniform_int_distribution<std::size_t> distribution(0, matching_rules.size() - 1);
        auto& chosen_rule = matching_rules[distribution(random_engine)];
        chosen_rule->apply(*world, cell);
    }

}
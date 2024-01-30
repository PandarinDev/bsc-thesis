#include "generator.h"
#include "wfc/building/house.h"
#include "wfc/building/flower_shop.h"

namespace inf {

    static constexpr auto WORLD_INITIAL_WIDTH = 50;
    static constexpr auto WORLD_INITIAL_HEIGHT = 50;

    World WorldGenerator::generate_initial(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        // TODO: Later on we only want to generate part of the world that is initially visible in the frustum.
        // However, for now while we are exploring generation it is better to generate a meaningfully sized
        // chunk of the world fully.
        World world(WORLD_INITIAL_WIDTH, WORLD_INITIAL_HEIGHT);
        WorldGenerator generator(&world, physical_device, logical_device);
        generator.generate();

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

    void WorldGenerator::generate() {
        // TODO: For now we just mark the entire starting chunk as a residental district, but this is obviously not desired
        world->add_district(District(
            DistrictType::RESIDENTAL,
            BoundingBox2D(0, 0, WORLD_INITIAL_WIDTH, WORLD_INITIAL_HEIGHT)));

        // Collapse each cell in the initial cell set
        for (std::size_t x = 0; x < WORLD_INITIAL_WIDTH; ++x) {
            for (std::size_t y = 0; y < WORLD_INITIAL_HEIGHT; ++y) {
                auto& cell = world->get_or_create_cell(glm::ivec3(x, 0, y));
                collapse(cell);
            }
        }
    }

    void WorldGenerator::collapse(Cell& cell) {
        std::vector<WorldRule*> matching_rules;
        for (const auto& rule : rules) {
            if (rule->matches(*world, cell)) {
                matching_rules.emplace_back(rule.get());
            }
        }
        if (matching_rules.empty()) {
            return;
        }
        // TODO: Take into account the rule weights
        std::uniform_int_distribution<std::size_t> distribution(0, matching_rules.size() - 1);
        auto& chosen_rule = matching_rules[distribution(random_engine)];
        chosen_rule->apply(*world, cell);
    }

}
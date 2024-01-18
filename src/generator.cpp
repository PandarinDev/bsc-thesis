#include "generator.h"
#include "wfc/building/house.h"

namespace inf {

    World WorldGenerator::generate_initial(const gfx::Frustum& frustum) {
        // TODO: Do intersection checking for every row in the frustum and decide
        // which cells are going to be visible. Collapse rules for cells that will
        // be visible initially.
        std::vector<glm::ivec3> visible_coordinates {
            { -1, 0, 3 },
            { 0, 0, 3 },
            { 1, 0, 3 }
        };
        World world;
        WorldGenerator generator(&world);
        for (const auto& coordinate : visible_coordinates) {
            generator.collapse(world.get_or_create_cell(coordinate));
        }

        return world;
    }

    WorldGenerator::WorldGenerator(const World* world) : world(world) {
        std::random_device random_device;
        random_engine.seed(random_device());
        rules.emplace_back(std::make_unique<wfc::building::HouseRule>());
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
        chosen_rule->apply(cell);
    }

}
#pragma once

#include "common.h"

#include <limits>
#include <functional>
#include <unordered_set>
#include <unordered_map>

namespace inf::wfc {

    template<typename ContextType, typename RuleType>
    void wfc_collapse(
        RandomGenerator& rng,
        ContextType& context,
        const std::vector<RuleType>& rules) {
        using InstanceType = typename ContextType::InstanceType;
        std::unordered_set<InstanceType*> uncollapsed_cells;
        for (auto& cell : context.cells) {
            uncollapsed_cells.emplace(&cell);
        }

        using Entropy = std::unordered_map<InstanceType*, std::vector<const RuleType*>>;
        while (!uncollapsed_cells.empty()) {
            Entropy entropy;
            for (auto cell : uncollapsed_cells) {
                for (auto& rule : rules) {
                    if (rule.matches(context, *cell)) {
                        auto it = entropy.find(cell);
                        if (it == entropy.cend()) {
                            it = entropy.emplace(
                                std::piecewise_construct,
                                std::forward_as_tuple(cell),
                                std::forward_as_tuple(std::vector<const RuleType*>{})).first;
                        }
                        it->second.emplace_back(&rule);
                    }
                }
            }

            std::size_t min_entropy = std::numeric_limits<std::size_t>::max();
            for (const auto& entry : entropy) {
                const auto& cell_rules = entry.second;
                if (cell_rules.size() < min_entropy) {
                    min_entropy = cell_rules.size();
                }
            }
            // If there are no matching rules for the rest of the cells exit early
            if (min_entropy == std::numeric_limits<std::size_t>::max()) {
                break;
            }

            std::vector<InstanceType*> min_entropy_cells;
            for (const auto& entry : entropy) {
                if (entry.second.size() == min_entropy) {
                    min_entropy_cells.emplace_back(entry.first);
                }
            }
            std::uniform_int_distribution<std::size_t> cell_distribution(0, min_entropy_cells.size() - 1);
            auto cell = min_entropy_cells[cell_distribution(rng)];

            std::uniform_int_distribution<std::size_t> rule_distribution(0, min_entropy - 1);
            const auto rule = entropy[cell][rule_distribution(rng)];
            rule->apply(context, *cell);
            uncollapsed_cells.erase(cell);
        }
    }

}
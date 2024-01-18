#pragma once

#include "world.h"
#include "wfc/rule.h"

namespace inf::wfc::building {

    struct HouseRule final : public Rule<World, Cell> {

        bool matches(const World& world, const Cell& cell) override;
        void apply(Cell& cell) override;

    };

}
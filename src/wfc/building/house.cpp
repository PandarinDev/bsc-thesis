#include "wfc/building/house.h"
#include "gfx/assets.h"

#include <iostream>

namespace inf::wfc::building {

    bool HouseRule::matches(const World& world, const Cell& cell) {
        // TODO: Should decide based on whether the cell is in a residential district.
        return true;
    }

    void HouseRule::apply(Cell& cell) {
        cell.type = CellType::HOUSE;
    }

}
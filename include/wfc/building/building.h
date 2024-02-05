#pragma once

#include "gfx/mesh.h"

namespace inf::wfc {

    enum class BuildingType {
        UNDECIDED,
        HOUSE,
        FLOWER_SHOP
    };

    enum class Direction {
        NORTH,
        EAST,
        SOUTH,
        WEST
    };

    struct Building {

        BuildingType type;
        Direction facing_direction;
        gfx::Mesh mesh;

        Building(
            BuildingType type,
            Direction facing_direction,
            gfx::Mesh&& mesh);

    };

}

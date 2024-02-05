#include "wfc/building/building.h"

namespace inf::wfc {

    Building::Building(
        BuildingType type,
        Direction facing_direction,
        gfx::Mesh&& mesh) :
        type(type),
        facing_direction(facing_direction),
        mesh(std::move(mesh)) {}

}
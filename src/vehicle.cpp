#include "vehicle.h"

namespace inf {

    Vehicle::Vehicle(VehicleType type, const glm::ivec2& position, VehicleState state, const gfx::Mesh* mesh) :
        type(type), position(position), state(state), mesh(mesh) {}

}
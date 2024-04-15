#include "vehicle.h"

namespace inf {

    VehicleIntention::VehicleIntention(VehicleIntentionType intention, const glm::vec3& direction) :
        intention(intention), direction(direction) {}

    Vehicle::Vehicle(
        VehicleType type,
        const glm::ivec2& position,
        VehicleState state,
        const gfx::Mesh* mesh) :
        type(type), position(position), state(state), mesh(mesh), offset(0.0f),
        // Initial intention is irrelevant, as it will be overwritten when necessary
        intention(VehicleIntention(VehicleIntentionType::TURN_LEFT, glm::vec3())) {}

}
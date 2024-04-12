#pragma once

#include "gfx/mesh.h"

#include <glm/vec2.hpp>

namespace inf {

    enum class VehicleType {
        CAR
    };

    enum class VehicleState {
        HORIZONTAL_LEFT,
        HORIZONTAL_RIGHT,
        VERTICAL_UP,
        VERTICAL_DOWN
    };

    struct Vehicle {

        VehicleType type;
        glm::ivec2 position;
        VehicleState state;
        const gfx::Mesh* mesh;

        Vehicle(VehicleType type, const glm::ivec2& position, VehicleState state, const gfx::Mesh* mesh);

    };

}
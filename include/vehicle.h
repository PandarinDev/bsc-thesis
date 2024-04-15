#pragma once

#include "gfx/mesh.h"
#include "road.h"

#include <glm/vec2.hpp>

#include <deque>
#include <optional>

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

    enum class VehicleIntentionType {
        TURN_LEFT,
        TURN_RIGHT,
        KEEP_STRAIGHT
    };

    struct VehicleIntention {

        VehicleIntentionType intention;
        glm::vec3 direction;

        VehicleIntention(VehicleIntentionType intention, const glm::vec3& direction);

    };

    struct Vehicle {

        VehicleType type;
        glm::ivec2 position;
        VehicleState state;
        const gfx::Mesh* mesh;
        float offset;
        VehicleIntention intention;

        Vehicle(
            VehicleType type,
            const glm::ivec2& position,
            VehicleState state,
            const gfx::Mesh* mesh);

    };

}
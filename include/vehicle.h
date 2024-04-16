#pragma once

#include "gfx/mesh.h"
#include "road.h"
#include "common.h"

#include <glm/vec2.hpp>

#include <deque>
#include <unordered_map>

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
        std::deque<glm::ivec2> targets;
        const gfx::Mesh* mesh;
        float offset;

        Vehicle(
            VehicleType type,
            const glm::ivec2& position,
            const std::deque<glm::ivec2>& targets,
            const gfx::Mesh* mesh);

        void update(
            RandomGenerator& rng,
            const std::unordered_map<glm::ivec2, DistrictRoad>& roads,
            float delta_time);
        std::pair<glm::vec3, float> get_world_position_and_rotation(const glm::vec3& district_position) const;

    private:

        VehicleState compute_state() const;

    };

}
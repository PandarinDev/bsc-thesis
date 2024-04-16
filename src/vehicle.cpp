#include "vehicle.h"
#include "utils/hash_utils.h"
#include "utils/random_utils.h"

#include <glm/glm.hpp>

#include <string>
#include <stdexcept>

namespace inf {

    Vehicle::Vehicle(
        VehicleType type,
        const glm::ivec2& position,
        const std::deque<glm::ivec2>& targets,
        const gfx::Mesh* mesh) :
        type(type), position(position), targets(targets), mesh(mesh), offset(0.0f) {}

    void Vehicle::update(
        RandomGenerator& rng,
        const std::unordered_map<glm::ivec2, DistrictRoad>& roads,
        float delta_time) {
        static constexpr float vehicle_speed = 1.0f;
        if (targets.empty()) {
            return;
        }
        auto new_offset = offset + vehicle_speed * delta_time;
        if (new_offset > 1.0f) {
            position = targets.front();
            targets.pop_front();
            new_offset = std::fmod(new_offset, 1.0f);
            // If we removed the last target add a new one based on the current road direction
            if (targets.empty()) {
                const auto possible_continuations = RoadUtils::get_possible_continuations(roads, position);
                if (!possible_continuations.empty()) {
                    std::uniform_int_distribution<std::size_t> continuation_distribution(0, possible_continuations.size() - 1);
                    const auto& continuation = possible_continuations[continuation_distribution(rng)];
                    for (const auto& next : continuation) {
                        targets.emplace_back(next);
                    }
                }
            }
        }
        offset = new_offset;
    }

    std::pair<glm::vec3, float> Vehicle::get_world_position_and_rotation(const glm::vec3& district_position) const {
        const auto start = glm::vec3(position.x, 0.0f, position.y);
        float rotation = 0.0f;
        if (targets.empty()) {
            return std::make_pair(district_position + start, rotation);
        }
        const auto& target = targets.front();
        const auto end = glm::vec3(target.x, 0.0f, target.y);
        glm::vec3 world_position = district_position + glm::mix(start, end, offset);
        // TODO: These position offsets are needed to line up with the road position offsets (see road position in update_caches()).
        // This should be solved in a more sophisticated way, as this will not working when turning around corners.
        /*
        const auto state = compute_state();
        switch (state) {
            case VehicleState::HORIZONTAL_LEFT:
                world_position += glm::vec3(-offset, 0.0f, 0.65f);
                rotation = glm::radians(90.0f);
                break;
            case VehicleState::VERTICAL_UP:
                world_position += glm::vec3(0.35f, 0.0f, offset);
                rotation = glm::radians(180.0f);
                break;
            case VehicleState::HORIZONTAL_RIGHT:
                world_position += glm::vec3(offset, 0.0f, 0.35f);
                rotation = glm::radians(270.0f);
                break;
            case VehicleState::VERTICAL_DOWN:
                world_position += glm::vec3(0.65f, 0.0f, -offset);
                break;
        }
        */
        return std::make_pair(world_position, rotation);
    }

    VehicleState Vehicle::compute_state() const {
        if (targets.empty()) {
            throw std::runtime_error("Cannot compute vehicle state if targets are empty.");
        }
        const auto& next = targets.front();
        const auto direction = next - position;
        if (direction == glm::ivec2(1, 0)) {
            return VehicleState::HORIZONTAL_RIGHT;
        }
        if (direction == glm::ivec2(-1, 0)) {
            return VehicleState::HORIZONTAL_LEFT;
        }
        if (direction == glm::ivec2(0, 1)) {
            return VehicleState::VERTICAL_UP;
        }
        if (direction == glm::ivec2(0, -1)) {
            return VehicleState::VERTICAL_DOWN;
        }
        throw std::runtime_error("Unhandled vehicle direction in state computation ([" +
            std::to_string(direction.x) + ", " + std::to_string(direction.y) + "])");
    }

}
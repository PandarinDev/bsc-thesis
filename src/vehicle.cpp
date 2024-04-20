#include "vehicle.h"
#include "gfx/vk/buffer.h"
#include "utils/hash_utils.h"
#include "utils/random_utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <nlohmann/json.hpp>
#include <cpp-base64/base64.h>

#include <string>
#include <random>
#include <fstream>
#include <stdexcept>

namespace inf {

    Vehicle::Vehicle(
        const glm::ivec2& position,
        const std::deque<glm::ivec2>& targets,
        gfx::Mesh&& mesh) :
        position(position), targets(targets), mesh(std::move(mesh)), offset(0.0f) {}

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
        if (targets.empty()) {
            return std::make_pair(district_position + start, 0.0f);
        }
        const auto& target = targets.front();
        const auto end = glm::vec3(target.x, 0.0f, target.y);
        glm::vec3 world_position = district_position + glm::mix(start, end, offset);
        const auto direction = glm::normalize(end - start);
        // These offsets have to do with how the road vs car meshes are centered
        if (direction.x == 1) {
            world_position += glm::vec3(0.0f, 0.0f, 0.35f);
        }
        else if (direction.x == -1) {
            world_position += glm::vec3(0.0f, 0.0f, 0.65f);
        }
        if (direction.z == 1) {
            world_position += glm::vec3(0.65f, 0.0f, 0.0f);
        }
        else if (direction.z == -1) {
            world_position += glm::vec3(0.35f, 0.0f, 0.0f);
        }
        const auto rotation = std::atan2(direction.z, direction.x) - glm::pi<float>() * 0.5f;
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

    std::unordered_map<std::string, VehiclePattern> VehiclePatterns::patterns;

    VehiclePattern::VehiclePattern(
        std::vector<gfx::vk::VertexWithMaterialName>&& vertices,
        VehicleMaterials&& materials) :
        vertices(std::move(vertices)),
        materials(std::move(materials)) {}

    Vehicle VehiclePattern::instantiate(
        RandomGenerator& rng,
        const gfx::vk::LogicalDevice* device,
        const gfx::vk::MemoryAllocator* allocator,
        const glm::ivec2& position,
        const std::deque<glm::ivec2>& targets) const {
        // Choose a color for each material
        std::unordered_map<std::string, glm::vec3> chosen_materials;
        for (const auto& [name, candidates] : materials) {
            std::uniform_int_distribution<std::size_t> candidate_distribution(0, candidates.size() - 1);
            chosen_materials.emplace(name, candidates[candidate_distribution(rng)]);
        }

        std::vector<gfx::vk::Vertex> vertices;
        for (const auto& vertex : this->vertices) {
            vertices.emplace_back(vertex, chosen_materials.at(vertex.material_name));
        }

        const auto num_bytes = vertices.size() * sizeof(gfx::vk::Vertex);
        auto buffer = gfx::vk::MappedBuffer::create(device, allocator, gfx::vk::BufferType::VERTEX_BUFFER, num_bytes);
        buffer.upload(vertices.data(), num_bytes);
        const auto bb = gfx::vk::Vertex::compute_bounding_box(vertices);
        return Vehicle(position, targets, gfx::Mesh(std::move(buffer), vertices.size(), glm::mat4(1.0f), bb));
    }

    void VehiclePatterns::initialize(const std::filesystem::path& vehicles_path) {
        if (!std::filesystem::is_directory(vehicles_path)) {
            throw std::runtime_error("Directory at '" + vehicles_path.string() + "' does not exist.");
        }
        for (const auto& file : std::filesystem::directory_iterator(vehicles_path)) {
            if (!file.is_regular_file()) {
                continue;
            }
            const auto file_path = file.path();
            if (file_path.extension() != ".json") {
                continue;
            }
            std::ifstream file_handle(file_path);
            if (!file_handle) {
                throw std::runtime_error("Failed to open file at '" + file_path.string() + "'.");
            }
            VehicleMaterials materials;

            const auto json_contents = nlohmann::json::parse(file_handle);
            const auto name = json_contents["name"].get<std::string>();
            const auto data = json_contents["data"].get<std::string>();
            const auto& materials_obj = json_contents["materials"];
            for (const auto& entry : materials_obj.items()) {
                auto& material = materials.emplace(entry.key(), std::vector<glm::vec3>{}).first->second;
                for (const auto& candidate : entry.value()) {
                    material.emplace_back(candidate[0].get<float>(), candidate[1].get<float>(), candidate[2].get<float>());
                }
            }

            auto vertices = gfx::vk::VertexWithMaterialName::from_bytes(base64_decode(data));
            patterns.emplace(name, VehiclePattern(std::move(vertices), std::move(materials)));
        }
    }

    const VehiclePattern& VehiclePatterns::get_pattern(const std::string& name) {
        return patterns.at(name);
    }

}
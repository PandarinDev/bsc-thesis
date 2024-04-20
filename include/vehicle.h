#pragma once

#include "gfx/mesh.h"
#include "gfx/vk/vertex.h"
#include "gfx/vk/device.h"
#include "gfx/vk/memory_allocator.h"
#include "road.h"
#include "common.h"

#include <glm/vec2.hpp>

#include <deque>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace inf {

    enum class VehicleState {
        HORIZONTAL_LEFT,
        HORIZONTAL_RIGHT,
        VERTICAL_UP,
        VERTICAL_DOWN
    };

    struct Vehicle {

        glm::ivec2 position;
        std::deque<glm::ivec2> targets;
        gfx::Mesh mesh;
        float offset;

        Vehicle(
            const glm::ivec2& position,
            const std::deque<glm::ivec2>& targets,
            gfx::Mesh&& mesh);

        void update(
            RandomGenerator& rng,
            const std::unordered_map<glm::ivec2, DistrictRoad>& roads,
            float delta_time);
        std::pair<glm::vec3, float> get_world_position_and_rotation(const glm::vec3& district_position) const;

    private:

        VehicleState compute_state() const;

    };

    using VehicleMaterials = std::unordered_map<std::string, std::vector<glm::vec3>>;

    struct VehiclePattern {

        VehiclePattern(
            std::vector<gfx::vk::VertexWithMaterialName>&& vertices,
            VehicleMaterials&& materials);

        Vehicle instantiate(
            RandomGenerator& rng,
            const gfx::vk::LogicalDevice* device,
            const gfx::vk::MemoryAllocator* allocator,
            const glm::ivec2& position,
            const std::deque<glm::ivec2>& targets) const;

    private:

        std::vector<gfx::vk::VertexWithMaterialName> vertices;
        VehicleMaterials materials;

    };

    struct VehiclePatterns {

        VehiclePatterns() = delete;

        static void initialize(const std::filesystem::path& vehicles_path);
        static const VehiclePattern& get_random_pattern(RandomGenerator& rng);
        static const VehiclePattern& get_pattern(const std::string& name);

    private:

        static std::unordered_map<std::string, VehiclePattern> patterns;

    };

}
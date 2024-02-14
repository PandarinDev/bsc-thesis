#pragma once

#include "gfx/mesh.h"
#include "gfx/vk/device.h"
#include "gfx/vk/vertex.h"

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

namespace inf::wfc {

    struct Ground {

        gfx::Mesh& mesh;
        glm::vec3 position;

        Ground(gfx::Mesh& mesh, const glm::vec3& position);

    };

    struct GroundPattern {

        std::string name;
        gfx::Mesh mesh;

        GroundPattern(const std::string& name, gfx::Mesh&& mesh);

    };

    struct GroundPatterns {

        GroundPatterns() = delete;

        static void initialize(
            const std::filesystem::path& grounds_path,
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);
        static void deinitialize();

        static GroundPattern& get_pattern(const std::string& name);

    private:

        static std::unordered_map<std::string, GroundPattern> patterns;

    };

}
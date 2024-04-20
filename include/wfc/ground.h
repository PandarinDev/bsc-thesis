#pragma once

#include "common.h"
#include "gfx/mesh.h"
#include "gfx/vk/device.h"
#include "gfx/vk/vertex.h"
#include "gfx/vk/memory_allocator.h"

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
            const gfx::vk::LogicalDevice* logical_device,
            const gfx::vk::MemoryAllocator* allocator);
        static void deinitialize();

        static const GroundPattern& get_pattern(const std::string& name);
        static const GroundPattern& get_random_foliage_pattern(RandomGenerator& rng);

    private:

        static std::unordered_map<std::string, GroundPattern> patterns;
        static std::vector<const GroundPattern*> foliage_patterns;

    };

}
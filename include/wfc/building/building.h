#pragma once

#include "common.h"
#include "gfx/mesh.h"
#include "gfx/vk/device.h"
#include "gfx/vk/vertex.h"
#include "wfc/rule.h"

#include <glm/vec3.hpp>

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

namespace inf::wfc {

    enum class BuildingPatternFilterType {
        EDGE,
        CORNER
    };

    struct BuildingMesh;

    struct BuildingContext {};

    struct BuildingCell {
        glm::ivec3 position;
        bool is_corner;
        bool is_edge;
        const BuildingMesh* mesh;
    };

    struct Building {

        gfx::Mesh mesh;

        Building(gfx::Mesh&& mesh);

    };

    struct BuildingMesh {

        std::string name;
        std::vector<gfx::vk::Vertex> vertices;
        std::vector<BuildingPatternFilterType> filters;

        BuildingMesh(
            const std::string& name,
            std::vector<gfx::vk::Vertex>&& vertices,
            std::vector<BuildingPatternFilterType>&& filters);

        bool matches(const BuildingContext& context, const BuildingCell& cell) const;
        void apply(BuildingContext& context, BuildingCell& cell) const;

    };


    struct BuildingDimensions {

        Range2D<int> width;
        Range2D<int> height;
        Range2D<int> depth;

    };

    struct BuildingPattern {

        std::string name;
        BuildingDimensions dimensions;
        std::vector<BuildingMesh> meshes;

        BuildingPattern(const std::string& name, const BuildingDimensions& dimensions);

        Building instantiate(
            RandomGenerator& rng,
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);

    };

    struct BuildingPatterns {

        BuildingPatterns() = delete;

        static void initialize(const std::filesystem::path& buildings_path);
        static BuildingPattern* get_pattern(const std::string& name);

    private:

        static std::unordered_map<std::string, BuildingPattern> patterns;

    };

}

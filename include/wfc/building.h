#pragma once

#include "common.h"
#include "gfx/mesh.h"
#include "gfx/vk/device.h"
#include "gfx/vk/vertex.h"
#include "wfc/rule.h"
#include "bounding_box.h"

#include <glm/vec3.hpp>

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <filesystem>
#include <unordered_map>

namespace inf::wfc {

    enum class BuildingPatternFilterType {
        EDGE,
        CORNER
    };

    struct BuildingMesh;

    struct BuildingContext {

        int width;
        int height;
        int depth;

        BuildingContext(int width, int height, int depth);

    };

    struct BuildingCell {
        glm::ivec3 position;
        bool is_corner;
        bool is_edge;
        float rotate_y;
        const BuildingMesh* mesh;
    };

    struct EdgeBuildingPatternFilter {
        bool operator()(const BuildingContext&, const BuildingCell&) const;
    };

    struct CornerBuildingPatternFilter {
        bool operator()(const BuildingContext&, const BuildingCell&) const;
    };

    struct NegationBuildingPatternFilter;

    using BuildingPatternFilter = std::variant<
        EdgeBuildingPatternFilter,
        CornerBuildingPatternFilter,
        NegationBuildingPatternFilter>;

    struct NegationBuildingPatternFilter {

        // This needs to be pointer, otherwise the type starts depending on it's own size
        std::unique_ptr<BuildingPatternFilter> filter;

        NegationBuildingPatternFilter(std::unique_ptr<BuildingPatternFilter> filter);

        bool operator()(const BuildingContext&, const BuildingCell&) const;
    };

    struct Building {

        Building(gfx::Mesh&& mesh, const BoundingBox3D& bounding_box);
        Building(
            gfx::Mesh&& mesh,
            const BoundingBox3D& bounding_box,
            const glm::vec3& position);

        const gfx::Mesh& get_mesh() const;
        const BoundingBox3D& get_local_bounding_box() const;
        BoundingBox3D get_bounding_box() const;

        const glm::vec3& get_position() const;
        void set_position(const glm::vec3& position);

    private:

        gfx::Mesh mesh;
        BoundingBox3D bounding_box;
        glm::vec3 position;

    };

    struct AbsoluteHeightRestriction{
        int height;
    };
    struct NotTopHeightRestriction{};
    struct TopHeightRestriction{};
    struct NotBottomHeightRestriction{};
    struct BottomHeightRestriction{};
    using BuildingMeshHeightRestriction = std::variant<
        AbsoluteHeightRestriction,
        NotTopHeightRestriction,
        TopHeightRestriction,
        NotBottomHeightRestriction,
        BottomHeightRestriction>;

    struct BuildingMesh {

        std::string name;
        std::vector<gfx::vk::VertexWithMaterialName> vertices;
        std::vector<BuildingPatternFilter> filters;
        std::vector<BuildingMeshHeightRestriction> height_restrictions;

        BuildingMesh(
            const std::string& name,
            std::vector<gfx::vk::VertexWithMaterialName>&& vertices,
            std::vector<BuildingPatternFilter>&& filters,
            std::vector<BuildingMeshHeightRestriction>&& height_restrictions);

        bool matches(const BuildingContext& context, const BuildingCell& cell) const;
        void apply(BuildingContext& context, BuildingCell& cell) const;

    };

    struct BuildingDimensions {

        Range2D<int> width;
        Range2D<int> height;
        Range2D<int> depth;

    };

    using BuildingMaterials = std::unordered_map<std::string, std::vector<glm::vec3>>;

    struct BuildingPattern {

        std::string name;
        BuildingDimensions dimensions;
        BuildingMaterials materials;
        std::vector<BuildingMesh> meshes;

        BuildingPattern(
            const std::string& name,
            const BuildingDimensions& dimensions,
            BuildingMaterials&& materials);

        Building instantiate(
            RandomGenerator& rng,
            const gfx::vk::LogicalDevice* logical_device,
            const gfx::vk::MemoryAllocator* allocator,
            int max_width,
            int max_depth) const;

    };

    struct BuildingPatterns {

        BuildingPatterns() = delete;

        static void initialize(const std::filesystem::path& buildings_path);
        static std::vector<const BuildingPattern*> get_patterns(int max_width, int max_depth);
        static const BuildingPattern* get_pattern(const std::string& name);

    private:

        static std::unordered_map<std::string, BuildingPattern> patterns;

    };

}

#include "wfc/building/building.h"
#include "wfc/rule.h"
#include "gfx/vk/vertex.h"
#include "utils/string_utils.h"

#include <nlohmann/json.hpp>
#include <magic_enum.hpp>
#include <cpp-base64/base64.h>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <stdexcept>

namespace inf::wfc {

    std::unordered_map<std::string, BuildingPattern> BuildingPatterns::patterns;

    BuildingMesh::BuildingMesh(
        const std::string& name,
        std::vector<gfx::vk::Vertex>&& vertices,
        std::vector<BuildingPatternFilterType>&& filters) :
        name(name),
        vertices(std::move(vertices)),
        filters(std::move(filters)) {}

    bool BuildingMesh::matches(const BuildingContext&, const BuildingCell& cell) const {
        bool all_filters_passed = true;
        for (const auto& filter : filters) {
            switch (filter) {
                case BuildingPatternFilterType::CORNER:
                    all_filters_passed = all_filters_passed && cell.is_corner;
                    break;
                case BuildingPatternFilterType::EDGE:
                    all_filters_passed = all_filters_passed && cell.is_edge;
                    break;
            }
        }
        return all_filters_passed;
    }

    void BuildingMesh::apply(BuildingContext&, BuildingCell& cell) const {
        cell.mesh = this;
    }

    BuildingPattern::BuildingPattern(const std::string& name, const BuildingDimensions& dimensions) :
        name(name), dimensions(dimensions) {}

    Building BuildingPattern::instantiate(
        RandomGenerator& rng,
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        auto width = dimensions.width.to_distribution()(rng);
        auto height = dimensions.height.to_distribution()(rng);
        auto depth = dimensions.depth.to_distribution()(rng);

        BuildingContext context;
        std::vector<BuildingCell> cells;
        cells.reserve(width * height * depth);
        for (std::size_t x = 0; x < width; ++x) {
            for (std::size_t y = 0; y < height; ++y) {
                for (std::size_t z = 0; z < depth; ++z) {
                    BuildingCell cell{};
                    cell.position = glm::ivec3(x, y, z);
                    cell.is_corner =
                        (x == 0 || x == width - 1) &&
                        (z == 0 || z == depth - 1);
                    cell.is_edge = !cell.is_corner &&
                        (x == 0 || x == width - 1 ||
                        z == 0 || z == depth - 1);
                    // Apply corner rotations
                    if (cell.is_corner) {
                        if (x == width - 1 && z == 0) {
                            cell.rotate_y = glm::radians(90.0f);
                        }
                        else if (x == width - 1 && z == depth - 1) {
                            cell.rotate_y = glm::radians(180.0f);
                        }
                        else if (x == 0 && z == depth - 1) {
                            cell.rotate_y = glm::radians(270.0f);
                        }
                    }
                    // Apply edge rotations
                    else if (cell.is_edge) {
                        if (x == width - 1) {
                            cell.rotate_y = glm::radians(90.0f);
                        }
                        else if (z == depth - 1) {
                            cell.rotate_y = glm::radians(180.0f);
                        }
                        else if (x == 0) {
                            cell.rotate_y = glm::radians(270.0f);
                        }
                    }
                    cell.mesh = nullptr;
                    cells.push_back(cell);
                }
            }
        }
        wfc_collapse(rng, context, cells, meshes);
        

        // Generate a mesh from the resulting cells
        std::vector<gfx::vk::Vertex> vertices;
        for (const auto& cell : cells) {
            if (!cell.mesh) {
                continue;
            }
            glm::mat4 transformation(1.0f);
            transformation = glm::translate(transformation, glm::vec3(cell.position.x, cell.position.y, -cell.position.z));
            transformation = glm::rotate(transformation, cell.rotate_y, glm::vec3(0.0f, 1.0f, 0.0f));
            for (auto vertex : cell.mesh->vertices) {
                vertex.position = glm::vec3(transformation * glm::vec4(vertex.position, 1.0f));
                vertices.push_back(std::move(vertex));
            }
        }
        auto vertex_buffer = gfx::vk::MappedBuffer::create(
            physical_device,
            logical_device,
            gfx::vk::BufferType::VERTEX_BUFFER,
            sizeof(gfx::vk::Vertex) * vertices.size());
        vertex_buffer.upload(vertices);
        return Building(gfx::Mesh(std::move(vertex_buffer), vertices.size(), glm::mat4(1.0f)));
    }

    void BuildingPatterns::initialize(const std::filesystem::path& buildings_path) {
        if (!std::filesystem::is_directory(buildings_path)) {
            throw std::runtime_error("Directory at '" + buildings_path.string() + "' does not exist.");
        }
        for (const auto& file : std::filesystem::directory_iterator(buildings_path)) {
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
            const auto json_contents = nlohmann::json::parse(file_handle);
            const auto pattern_name = json_contents["name"].get<std::string>();
            // Parse dimensions
            BuildingDimensions dimensions;
            const auto& width = json_contents["dimensions"]["width"];
            const auto& height = json_contents["dimensions"]["height"];
            const auto& depth = json_contents["dimensions"]["depth"];
            dimensions.width.min = width["min"].get<int>();
            dimensions.width.max = width["max"].get<int>();
            dimensions.height.min = height["min"].get<int>();
            dimensions.height.max = height["max"].get<int>();
            dimensions.depth.min = depth["min"].get<int>();
            dimensions.depth.max = depth["max"].get<int>();

            auto& pattern = patterns.emplace(pattern_name, BuildingPattern(pattern_name, dimensions)).first->second;
            for (const auto& mesh_obj : json_contents["meshes"]) {
                const auto mesh_name = mesh_obj["name"].get<std::string>();

                // Parse mesh data
                const auto data = mesh_obj["data"].get<std::string>();
                // Data is base64 encoded, we need to decode it first then parse vertices from it
                auto vertices = gfx::vk::Vertex::from_bytes(base64_decode(data));

                // Parse mesh filters
                std::vector<BuildingPatternFilterType> filters;
                for (const auto& filter : mesh_obj["filters"]) {
                    const auto filter_str = utils::StringUtils::to_uppercase(filter.get<std::string>());
                    const auto maybe_filter = magic_enum::enum_cast<BuildingPatternFilterType>(filter_str);
                    if (!maybe_filter) {
                        throw std::runtime_error("Failed to parse filter '" + filter_str + "'.");
                    }
                    filters.emplace_back(maybe_filter.value());
                }

                // Store the building mesh
                pattern.meshes.emplace_back(mesh_name, std::move(vertices), std::move(filters));
            }
        }
    }

    BuildingPattern* BuildingPatterns::get_pattern(const std::string& pattern) {
        const auto it = patterns.find(pattern);
        if (it == patterns.cend()) {
            return nullptr;
        }
        return &it->second;
    }

    Building::Building(gfx::Mesh&& mesh) : mesh(std::move(mesh)) {}

}
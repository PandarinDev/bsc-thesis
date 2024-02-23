#include "wfc/building.h"
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

    bool EdgeBuildingPatternFilter::operator()(const BuildingContext&, const BuildingCell& cell) const {
        return cell.is_edge;
    }

    bool CornerBuildingPatternFilter::operator()(const BuildingContext&, const BuildingCell& cell) const {
        return cell.is_corner;
    }

    NegationBuildingPatternFilter::NegationBuildingPatternFilter(std::unique_ptr<BuildingPatternFilter> filter) :
        filter(std::move(filter)) {}

    bool NegationBuildingPatternFilter::operator()(const BuildingContext& context, const BuildingCell& cell) const {
        return std::visit([&](auto&& callable) { return !callable(context, cell); }, *filter);
    }

    BuildingMesh::BuildingMesh(
        const std::string& name,
        std::vector<gfx::vk::Vertex>&& vertices,
        std::vector<BuildingPatternFilter>&& filters,
        const std::optional<int>& height) :
        name(name),
        vertices(std::move(vertices)),
        filters(std::move(filters)),
        height(height) {}

    bool BuildingMesh::matches(const BuildingContext& context, const BuildingCell& cell) const {
        bool all_filters_passed = true;
        for (const auto& filter : filters) {
            all_filters_passed = all_filters_passed && std::visit([&](auto&& callable) { return callable(context, cell); }, filter);
        }
        // Apply height restriction if present
        if (height.has_value()) {
            all_filters_passed = all_filters_passed && cell.position.y == height.value();
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
        static constexpr auto float_max = std::numeric_limits<float>::max();
        static constexpr auto float_min = std::numeric_limits<float>::min();

        BoundingBox3D bounding_box(
            glm::vec3(float_max, float_max, float_max),
            glm::vec3(float_min, float_min, float_min));
        for (const auto& cell : cells) {
            if (!cell.mesh) {
                continue;
            }
            glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), cell.rotate_y, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 transformation = glm::translate(glm::mat4(1.0f), glm::vec3(cell.position.x, cell.position.y, -cell.position.z)) * rotation_matrix;
            for (auto vertex : cell.mesh->vertices) {
                vertex.position = glm::vec3(transformation * glm::vec4(vertex.position, 1.0f));
                vertex.normal = glm::vec3(rotation_matrix * glm::vec4(vertex.normal, 1.0f));
                vertices.push_back(std::move(vertex));
                bounding_box.update(vertex.position);
            }
        }
        auto vertex_buffer = gfx::vk::MappedBuffer::create(
            physical_device,
            logical_device,
            gfx::vk::BufferType::VERTEX_BUFFER,
            sizeof(gfx::vk::Vertex) * vertices.size());
        vertex_buffer.upload(vertices);
        return Building(gfx::Mesh(std::move(vertex_buffer), vertices.size(), glm::mat4(1.0f)), bounding_box);
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
                std::vector<BuildingPatternFilter> filters;
                for (const auto& filter_obj : mesh_obj["filters"]) {
                    auto filter_str = utils::StringUtils::to_uppercase(filter_obj.get<std::string>());
                    if (filter_str.empty()) {
                        continue;
                    }
                    // Check if the filter is negated
                    bool negated = filter_str[0] == '!';
                    if (negated) {
                        filter_str = filter_str.substr(1);
                    }
                    const auto maybe_filter_type = magic_enum::enum_cast<BuildingPatternFilterType>(filter_str);
                    if (!maybe_filter_type) {
                        throw std::runtime_error("Failed to parse filter type '" + filter_str + "'.");
                    }
                    const auto filter_type = *maybe_filter_type;
                    BuildingPatternFilter filter;
                    switch (filter_type) {
                        case BuildingPatternFilterType::CORNER:
                            filter = CornerBuildingPatternFilter();
                            break;
                        case BuildingPatternFilterType::EDGE:
                            filter = EdgeBuildingPatternFilter();
                            break;
                        default: throw std::runtime_error("Unhandled filter type for '" + filter_str + "'.");
                    }
                    if (negated) {
                        filter = NegationBuildingPatternFilter(std::make_unique<BuildingPatternFilter>(std::move(filter)));
                    }
                    filters.emplace_back(std::move(filter));
                }

                // Parse height restriction if present
                std::optional<int> height_restriction;
                if (mesh_obj.contains("height")) {
                    height_restriction = mesh_obj["height"].get<int>();
                }

                // Store the building mesh
                pattern.meshes.emplace_back(mesh_name, std::move(vertices), std::move(filters), height_restriction);
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

    Building::Building(gfx::Mesh&& mesh, const BoundingBox3D& bounding_box) :
        Building(std::move(mesh), bounding_box, glm::vec3()) {}

    Building::Building(gfx::Mesh&& mesh, const BoundingBox3D& bounding_box, const glm::vec3& position) :
        mesh(std::move(mesh)), bounding_box(bounding_box), position(position) {}

    const gfx::Mesh& Building::get_mesh() const {
        return mesh;
    }

    BoundingBox3D Building::get_bounding_box() const {
        return bounding_box.apply(mesh.get_model_matrix());
    }
    
    void Building::set_position(const glm::vec3& position) {
        this->position = position;
        mesh.set_model_matrix(glm::translate(glm::mat4(1.0f), position));
    }

}
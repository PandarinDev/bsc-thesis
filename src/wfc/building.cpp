#include "wfc/building.h"
#include "wfc/rule.h"
#include "gfx/vk/vertex.h"
#include "utils/string_utils.h"

#include <nlohmann/json.hpp>
#include <magic_enum.hpp>
#include <cpp-base64/base64.h>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <fstream>
#include <stdexcept>

namespace inf::wfc {

    std::unordered_map<std::string, BuildingPattern> BuildingPatterns::patterns;

    BuildingContext::BuildingContext(int width, int height, int depth) :
        width(width), height(height), depth(depth) {}

    bool BuildingContext::cell_contains(const glm::ivec3& position, std::string_view name) const {
        for (const auto& cell : cells) {
            if (cell.position == position) {
                const auto result = cell.mesh && cell.mesh->name == name;
                return result;
            }
        }
        return false;
    }

    bool EdgeBuildingPatternFilter::operator()(const BuildingContext&, const BuildingCell& cell) const {
        return cell.is_edge;
    }

    bool CornerBuildingPatternFilter::operator()(const BuildingContext&, const BuildingCell& cell) const {
        return cell.is_corner;
    }

    NextToBuildingPatternFilter::NextToBuildingPatternFilter(const std::string& mesh_name) : mesh_name(mesh_name) {}

    bool NextToBuildingPatternFilter::operator()(const BuildingContext& context, const BuildingCell& cell) const {
        std::array<glm::ivec3, 4> positions = {
            cell.position + glm::ivec3(-1, 0, 0), // Left
            cell.position + glm::ivec3(1, 0, 0),  // Right
            cell.position + glm::ivec3(0, 0, -1), // Top
            cell.position + glm::ivec3(0, 0, 1)   // Bottom
        };
        for (const auto& position : positions) {
            if (context.cell_contains(position, mesh_name)) {
                return true;
            }
        }
        return false;
    }

    NegationBuildingPatternFilter::NegationBuildingPatternFilter(std::unique_ptr<BuildingPatternFilter> filter) :
        filter(std::move(filter)) {}

    bool NegationBuildingPatternFilter::operator()(const BuildingContext& context, const BuildingCell& cell) const {
        return std::visit([&](auto&& callable) { return !callable(context, cell); }, *filter);
    }

    BuildingMesh::BuildingMesh(
        const std::string& name,
        std::vector<gfx::vk::VertexWithMaterialName>&& vertices,
        std::vector<BuildingPatternFilter>&& filters,
        std::vector<BuildingMeshHeightRestriction>&& height_restrictions) :
        name(name),
        vertices(std::move(vertices)),
        filters(std::move(filters)),
        height_restrictions(std::move(height_restrictions)) {}

    bool BuildingMesh::matches(const BuildingContext& context, const BuildingCell& cell) const {
        // Apply height restriction if present first, because it is a cheap first filter
        for (const auto& restriction : height_restrictions) {
            if (!std::visit([&](auto&& value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, AbsoluteHeightRestriction>) {
                    return cell.position.y == value.height;
                }
                if constexpr (std::is_same_v<T, TopHeightRestriction>) {
                    return cell.position.y == context.height - 1;
                }
                if constexpr (std::is_same_v<T, NotTopHeightRestriction>) {
                    return cell.position.y != context.height - 1;
                }
                if constexpr (std::is_same_v<T, BottomHeightRestriction>) {
                    return cell.position.y == 0;
                }
                if constexpr (std::is_same_v<T, NotBottomHeightRestriction>) {
                    return cell.position.y != 0;
                }
            }, restriction)) {
                return false;
            }
        }
        // Apply all other filter types next
        for (const auto& filter : filters) {
            if (!std::visit([&](auto&& callable) { return callable(context, cell); }, filter)) {
                return false;
            }
        }
        return true;
    }

    void BuildingMesh::apply(BuildingContext&, BuildingCell& cell) const {
        cell.mesh = this;
    }

    BuildingPattern::BuildingPattern(
        const std::string& name,
        const BuildingDimensions& dimensions,
        BuildingMaterials&& materials,
        int weight) :
        name(name), dimensions(dimensions), materials(std::move(materials)), weight(weight) {}

    Building BuildingPattern::instantiate(
        RandomGenerator& rng,
        const gfx::vk::LogicalDevice* logical_device,
        const gfx::vk::MemoryAllocator* allocator,
        int max_width,
        int max_depth) const {
        int width = max_width;
        int height = 1;
        int depth = max_depth;
        if (std::holds_alternative<AbsoluteBuildingDimensions>(dimensions)) {
            const auto& dimensions = std::get<AbsoluteBuildingDimensions>(this->dimensions);
            if (dimensions.width.min > max_width || dimensions.depth.min > max_depth) {
                throw std::runtime_error("Cannot fit building into the required maximum dimensions.");
            }
            width = std::uniform_int_distribution<int>(dimensions.width.min, std::min(dimensions.width.max, max_width))(rng);
            height = dimensions.height.to_distribution()(rng);
            depth = std::uniform_int_distribution<int>(dimensions.depth.min, std::min(dimensions.depth.max, max_depth))(rng);
        }

        // Create the context
        BuildingContext context(width, height, depth);
        // Initialize cell values (position, edge, is_corner, etc. needed for filters)
        context.cells.reserve(width * height * depth);
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                for (int z = 0; z < depth; ++z) {
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
                    context.cells.push_back(cell);
                }
            }
        }
        // Collapse cells
        wfc_collapse(rng, context, meshes);

        // Generate a mesh from the resulting cells
        std::vector<gfx::vk::Vertex> vertices;
        static constexpr auto float_max = std::numeric_limits<float>::max();
        static constexpr auto float_min = std::numeric_limits<float>::lowest();

        // Choose a material for each material from the candidates
        std::unordered_map<std::string, glm::vec3> chosen_materials;
        for (const auto& material : materials) {
            const auto& name = material.first;
            const auto& candidates = material.second;
            std::uniform_int_distribution<std::size_t> candidate_distribution(0, candidates.size() - 1);
            chosen_materials[name] = candidates[candidate_distribution(rng)];
        }

        BoundingBox3D bounding_box(
            glm::vec3(float_max, float_max, float_max),
            glm::vec3(float_min, float_min, float_min));
        for (const auto& cell : context.cells) {
            if (!cell.mesh) {
                continue;
            }
            glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), cell.rotate_y, glm::vec3(0.0f, 1.0f, 0.0f));
            // TODO: That +0.5f to the Y coordinate should be part of the models instead
            glm::mat4 transformation = glm::translate(glm::mat4(1.0f), glm::vec3(cell.position.x, cell.position.y + 0.5f, -cell.position.z)) * rotation_matrix;
            for (auto vertex : cell.mesh->vertices) {
                vertex.position = glm::vec3(transformation * glm::vec4(vertex.position, 1.0f));
                vertex.normal = glm::vec3(rotation_matrix * glm::vec4(vertex.normal, 1.0f));
                vertices.emplace_back(vertex, chosen_materials.at(vertex.material_name));
                bounding_box.update(vertex.position);
            }
        }

        auto vertex_buffer = gfx::vk::MappedBuffer::create(
            logical_device,
            allocator,
            gfx::vk::BufferType::VERTEX_BUFFER,
            sizeof(gfx::vk::Vertex) * vertices.size());
        vertex_buffer.upload(vertices.data(), vertices.size() * sizeof(gfx::vk::Vertex));
        return Building(gfx::Mesh(std::move(vertex_buffer), vertices.size(), glm::mat4(1.0f), bounding_box), glm::ivec3(width, height, depth));
    }

    AbsoluteBuildingDimensions::AbsoluteBuildingDimensions(
        const Range2D<int>& width,
        const Range2D<int>& height,
        const Range2D<int>& depth) :
        width(width), height(height), depth(depth) {}

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
            BuildingDimensions dimensions = AnyBuildingDimensions();
            if (json_contents.contains("dimensions")) {
                const auto& width = json_contents["dimensions"]["width"];
                const auto& height = json_contents["dimensions"]["height"];
                const auto& depth = json_contents["dimensions"]["depth"];
                Range2D<int> width_range;
                width_range.min = width["min"].get<int>();
                width_range.max = width["max"].get<int>();

                Range2D<int> height_range;
                height_range.min = height["min"].get<int>();
                height_range.max = height["max"].get<int>();

                Range2D<int> depth_range;
                depth_range.min = depth["min"].get<int>();
                depth_range.max = depth["max"].get<int>();
                dimensions = AbsoluteBuildingDimensions(width_range, height_range, depth_range);
            }

            // Parse materials
            BuildingMaterials materials;
            for (const auto& material_entry : json_contents["materials"].items()) {
                auto& material_candidates = materials.emplace(material_entry.key(), std::vector<glm::vec3>{}).first->second;
                for (const auto& candidate : material_entry.value()) {
                    material_candidates.emplace_back(candidate[0].get<float>(), candidate[1].get<float>(), candidate[2].get<float>());
                }
            }

            // Parse weight
            int weight = json_contents["weight"].get<int>();

            auto& pattern = patterns.emplace(pattern_name, BuildingPattern(
                pattern_name, dimensions, std::move(materials), weight)).first->second;
            for (const auto& mesh_obj : json_contents["meshes"]) {
                const auto mesh_name = mesh_obj["name"].get<std::string>();

                // Parse mesh data
                const auto data = mesh_obj["data"].get<std::string>();

                // Data is base64 encoded, we need to decode it first then parse vertices from it
                auto vertices = gfx::vk::VertexWithMaterialName::from_bytes(base64_decode(data));

                // Parse mesh filters
                std::vector<BuildingPatternFilter> filters;
                for (const auto& filter_obj : mesh_obj["filters"]) {
                    auto filter_str = filter_obj.get<std::string>();
                    if (filter_str.empty()) {
                        continue;
                    }

                    // Parse filter parameters
                    std::vector<std::string> filter_params;
                    const auto param_separator = filter_str.find(':');
                    if (param_separator != std::string::npos) {
                        const auto params_str = filter_str.substr(param_separator + 1);
                        std::vector<std::string_view> param_views = utils::StringUtils::split(params_str, ',');
                        for (const auto& param_view : param_views) {
                            filter_params.emplace_back(param_view);
                        }
                        filter_str = filter_str.substr(0, param_separator);
                    }

                    // Check if the filter is negated
                    bool negated = filter_str[0] == '!';
                    if (negated) {
                        filter_str = filter_str.substr(1);
                    }
                    const auto maybe_filter_type = magic_enum::enum_cast<BuildingPatternFilterType>(utils::StringUtils::to_uppercase(filter_str));
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
                        case BuildingPatternFilterType::NEXT_TO:
                            if (filter_params.empty()) {
                                throw std::runtime_error("Filter type 'next_to' used without parameters.");
                            }
                            filter = NextToBuildingPatternFilter(filter_params[0]);
                            break;
                        default: throw std::runtime_error("Unhandled filter type for '" + filter_str + "'.");
                    }
                    if (negated) {
                        filter = NegationBuildingPatternFilter(std::make_unique<BuildingPatternFilter>(std::move(filter)));
                    }
                    filters.emplace_back(std::move(filter));
                }

                // Parse height restriction if present
                std::vector<BuildingMeshHeightRestriction> height_restrictions;
                if (mesh_obj.contains("height")) {
                    const auto& height_obj = mesh_obj["height"];
                    const auto parse_restriction = [&height_restrictions](const auto& restriction) {
                        // Numeric height restrictions are treated as absolute values
                        if (restriction.is_number_integer()) {
                            height_restrictions.emplace_back(AbsoluteHeightRestriction{restriction.template get<int>()});
                        }
                        // Otherwise special values are strings
                        else if (restriction.is_string()) {
                            const auto height_str = restriction.template get<std::string>();
                            if (height_str == "top") {
                                height_restrictions.emplace_back(TopHeightRestriction());
                            }
                            else if (height_str == "!top") {
                                height_restrictions.emplace_back(NotTopHeightRestriction());
                            }
                            else if (height_str == "bottom") {
                                height_restrictions.emplace_back(BottomHeightRestriction());
                            }
                            else if (height_str == "!bottom") {
                                height_restrictions.emplace_back(NotBottomHeightRestriction());
                            }
                            else {
                                throw std::runtime_error("Uknown height restriction '" + height_str + "'.");
                            }
                        }
                    };
                    if (height_obj.is_array()) {
                        for (const auto& restriction : height_obj) {
                            parse_restriction(restriction);
                        }
                    }
                    else {
                        parse_restriction(height_obj);
                    }
                }

                // Store the building mesh
                pattern.meshes.emplace_back(mesh_name, std::move(vertices), std::move(filters), std::move(height_restrictions));
            }
        }
    }

    std::vector<const BuildingPattern*> BuildingPatterns::get_patterns(int width, int depth) {
        std::vector<const BuildingPattern*> result;
        for (const auto& entry : patterns) {
            if (std::holds_alternative<AnyBuildingDimensions>(entry.second.dimensions)) {
                result.emplace_back(&entry.second);
                continue;
            }
            const auto& dimensions = std::get<AbsoluteBuildingDimensions>(entry.second.dimensions);
            if (width > dimensions.width.min && depth > dimensions.depth.min) {
                result.emplace_back(&entry.second);
            }
        }
        return result;
    }

    const BuildingPattern* BuildingPatterns::get_pattern(const std::string& pattern) {
        const auto it = patterns.find(pattern);
        if (it == patterns.cend()) {
            return nullptr;
        }
        return &it->second;
    }

    Building::Building(gfx::Mesh&& mesh, const glm::ivec3& dimensions) :
        Building(std::move(mesh), dimensions, glm::vec3()) {}

    Building::Building(gfx::Mesh&& mesh, const glm::ivec3& dimensions, const glm::vec3& position) :
        mesh(std::move(mesh)), dimensions(dimensions), position(position) {}

    const gfx::Mesh& Building::get_mesh() const {
        return mesh;
    }

    const BoundingBox3D& Building::get_bounding_box_in_model_space() const {
        return mesh.get_bounding_box_in_model_space();
    }

    BoundingBox3D Building::get_bounding_box_in_world_space() const {
        return get_bounding_box_in_model_space().apply(mesh.get_model_matrix());
    }

    const glm::ivec3& Building::get_dimensions() const {
        return dimensions;
    }

    const glm::vec3& Building::get_position() const {
        return position;
    }

    void Building::set_position(const glm::vec3& position) {
        this->position = position;
        mesh.set_model_matrix(glm::translate(glm::mat4(1.0f), position));
    }

}
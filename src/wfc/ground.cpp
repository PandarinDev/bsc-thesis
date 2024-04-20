#include "wfc/ground.h"

#include <nlohmann/json.hpp>
#include <cpp-base64/base64.h>

#include <fstream>
#include <stdexcept>

namespace inf::wfc {

    std::unordered_map<std::string, GroundPattern> GroundPatterns::patterns;
    std::vector<const GroundPattern*> GroundPatterns::foliage_patterns;

    Ground::Ground(gfx::Mesh& mesh, const glm::vec3& position) :
        mesh(mesh), position(position) {}

    GroundPattern::GroundPattern(const std::string& name, gfx::Mesh&& mesh) :
        name(name), mesh(std::move(mesh)) {}

    void GroundPatterns::initialize(
        const std::filesystem::path& grounds_path,
        const gfx::vk::LogicalDevice* logical_device,
        const gfx::vk::MemoryAllocator* allocator) {
        if (!std::filesystem::is_directory(grounds_path)) {
            throw std::runtime_error("Directory at '" + grounds_path.string() + "' does not exist.");
        }
        for (const auto& file : std::filesystem::directory_iterator(grounds_path)) {
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
            const auto parse_pattern = [&](const auto& json_obj) {
                const auto pattern_name = json_obj["name"].template get<std::string>();
                const auto data = json_obj["data"].template get<std::string>();
                auto vertices = gfx::vk::Vertex::from_bytes(base64_decode(data));
                auto vertex_buffer = gfx::vk::MappedBuffer::create(
                    logical_device, allocator, gfx::vk::BufferType::VERTEX_BUFFER, vertices.size() * sizeof(gfx::vk::Vertex));
                vertex_buffer.upload(vertices.data(), vertices.size() * sizeof(gfx::vk::Vertex));
                auto mesh = gfx::Mesh(std::move(vertex_buffer), vertices.size(), glm::mat4(1.0f), gfx::vk::Vertex::compute_bounding_box(vertices));
                patterns.emplace(pattern_name, GroundPattern(pattern_name, std::move(mesh)));
            };

            if (json_contents.is_array()) {
                for (const auto& element : json_contents) {
                    parse_pattern(element);
                }
            }
            else {
                parse_pattern(json_contents);
            }
        }

        // Set foliage patterns that will be used to fill empty spaces around buildings
        foliage_patterns = {
            &GroundPatterns::get_pattern("tree"),
            &GroundPatterns::get_pattern("pinetree")
        };
    }

    void GroundPatterns::deinitialize() {
        patterns.clear();
    }

    const GroundPattern& GroundPatterns::get_pattern(const std::string& name) {
        return patterns.at(name);
    }

    const GroundPattern& GroundPatterns::get_random_foliage_pattern(RandomGenerator& rng) {
        std::uniform_int_distribution<std::size_t> foliage_distribution(0, foliage_patterns.size() - 1);
        return *foliage_patterns[foliage_distribution(rng)];
    }

}
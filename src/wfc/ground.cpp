#include "wfc/ground.h"

#include <nlohmann/json.hpp>
#include <cpp-base64/base64.h>

#include <fstream>
#include <stdexcept>

namespace inf::wfc {

    std::unordered_map<std::string, GroundPattern> GroundPatterns::patterns;

    Ground::Ground(gfx::Mesh& mesh, const glm::vec3& position) :
        mesh(mesh), position(position) {}

    GroundPattern::GroundPattern(const std::string& name, gfx::Mesh&& mesh) :
        name(name), mesh(std::move(mesh)) {}

    void GroundPatterns::initialize(
        const std::filesystem::path& grounds_path,
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
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
            const auto pattern_name = json_contents["name"].get<std::string>();
            const auto data = json_contents["data"].get<std::string>();
            auto vertices = gfx::vk::Vertex::from_bytes(base64_decode(data));
            auto vertex_buffer = gfx::vk::MappedBuffer::create(
                physical_device, logical_device, gfx::vk::BufferType::VERTEX_BUFFER, vertices.size() * sizeof(gfx::vk::Vertex));
            vertex_buffer.upload(vertices);
            auto mesh = gfx::Mesh(std::move(vertex_buffer), vertices.size(), glm::mat4(1.0f));
            patterns.emplace(pattern_name, GroundPattern(pattern_name, std::move(mesh)));
            // TODO: Parse and add filters to the pattern
        }
    }

    void GroundPatterns::deinitialize() {
        patterns.clear();
    }

    GroundPattern& GroundPatterns::get_pattern(const std::string& name) {
        return patterns.at(name);
    }

}
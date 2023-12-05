#include "gfx/obj_loader.h"
#include "gfx/vk/vertex.h"
#include "utils/string_utils.h"

#include <string>
#include <array>
#include <optional>
#include <stdexcept>

namespace inf::gfx {

    struct ObjFace {
        glm::vec3 color;
        std::array<std::size_t, 3> vertices;
        std::array<std::size_t, 3> normals;
    };

    MaterialMap ObjLoader::load_materials(std::string_view mtl_content) {
        MaterialMap result;
        const auto lines = utils::StringUtils::split(mtl_content, '\n');
        std::optional<std::string> current_material;
        for (const auto& line : lines) {
            // Handle new material
            if (line.substr(0, 7) == "newmtl ") {
                current_material = line.substr(7); 
            }
            // Handle material diffuse color
            if (line.substr(0, 3) == "Kd ") {
                if (!current_material.has_value()) {
                    throw std::runtime_error("Material diffuse color found without newmtl directive.");
                }
                const auto parts = utils::StringUtils::split(line, ' ');
                glm::vec3 color;
                color.r = std::stof(std::string(parts[1]));
                color.g = std::stof(std::string(parts[2]));
                color.b = std::stof(std::string(parts[3]));
                result.emplace(current_material.value(), color);
            }
        }

        return result;
    }

    Mesh ObjLoader::load_mesh(
        const vk::PhysicalDevice& physical_device,
        const vk::LogicalDevice* logical_device,
        const MaterialMap& materials,
        std::string_view obj_content) {
        const auto lines = utils::StringUtils::split(obj_content, '\n');
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<ObjFace> faces;
        std::optional<glm::vec3> current_material;

        for (const auto& line : lines) {
            const auto is_material = line.substr(0, 7) == "usemtl ";
            if (is_material) {
                const auto material_name = std::string(utils::StringUtils::split(line, ' ')[1]);
                const auto it = materials.find(material_name);
                if (it == materials.cend()) {
                    throw std::runtime_error("Material with name '" + material_name + "' not found while loading OBJ model.");
                }
                current_material = it->second;
            }

            const auto is_face = line.substr(0, 2) == "f ";
            if (is_face) {
                const auto parts = utils::StringUtils::split(line, ' ');
                const std::array<std::vector<std::string_view>, 3> subparts = {
                    utils::StringUtils::split(parts[1], '/'),
                    utils::StringUtils::split(parts[2], '/'),
                    utils::StringUtils::split(parts[3], '/')
                };
                if (!current_material.has_value()) {
                    throw std::runtime_error("OBJ file defines face without active material.");
                }

                ObjFace face;
                face.color = current_material.value();
                for (std::size_t i = 0; i < subparts.size(); ++i) {
                    // OBJ is indexed from 1, so subtract 1 from indices to index from 0
                    face.vertices[i] = std::stoull(std::string(subparts[i][0])) - 1;
                    face.normals[i] = std::stoull(std::string(subparts[i][1])) - 1;
                }
                faces.emplace_back(std::move(face));
                continue;
            }

            const auto is_vertex = line.substr(0, 2) == "v ";
            const auto is_normal = line.substr(0, 3) == "vn ";
            if (!is_vertex && !is_normal) {
                continue;
            }
            const auto parts = utils::StringUtils::split(line, ' ');
            glm::vec3 vec;
            vec.x = std::stof(std::string(parts[1]));
            vec.y = std::stof(std::string(parts[2]));
            vec.z = std::stof(std::string(parts[3]));
            auto& add_to = is_vertex ? vertices : normals;
            add_to.emplace_back(vec);
        }

        // Convert the faces to vertex data
        std::vector<vk::Vertex> vertex_buffer_data;
        for (const auto& face : faces) {
            for (std::size_t i = 0; i < face.vertices.size(); ++i) {
                vertex_buffer_data.emplace_back(
                    vertices.at(face.vertices[i]),
                    normals.at(face.normals[i]),
                    face.color
                );
            }
        }

        // Allocate the Vulkan buffer and return the mesh
        const auto size_bytes = sizeof(vk::Vertex) * vertex_buffer_data.size();
        auto buffer = vk::MappedBuffer::create(
            physical_device,
            logical_device,
            vk::BufferType::VERTEX_BUFFER,
            size_bytes);
        buffer.upload(vertex_buffer_data.data(), size_bytes);

        return Mesh(std::move(buffer), vertex_buffer_data.size(), glm::mat4(1.0f));
    }

}
#pragma once

#include "gfx/mesh.h"
#include "gfx/vk/device.h"

#include <glm/vec3.hpp>

#include <string>
#include <string_view>
#include <unordered_map>

namespace inf::gfx {

    using MaterialMap = std::unordered_map<std::string, glm::vec3>;

    struct ObjLoader {

        static MaterialMap load_materials(std::string_view mtl_content);
        static Mesh load_mesh(
            const vk::PhysicalDevice& physical_device,
            const vk::LogicalDevice* logical_device,
            const MaterialMap& materials,
            std::string_view obj_content);

        ObjLoader() = delete;

    };

}
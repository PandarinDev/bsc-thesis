#pragma once

#include "gfx/mesh.h"
#include "gfx/vk/device.h"
#include "world.h"

#include <unordered_map>

namespace inf::gfx {

    struct Assets {

        Assets() = delete;

        static void initialize_assets(
            const vk::PhysicalDevice& physical_device,
            const vk::LogicalDevice& logical_device);
        static void destroy_assets();

        static Mesh& get_mesh(CellType cell_type);

    private:

        static std::unordered_map<CellType, Mesh> meshes;

    };

}
#include "gfx/assets.h"
#include "gfx/obj_loader.h"
#include "utils/file_utils.h"

#include <stdexcept>

namespace inf::gfx {

    std::unordered_map<CellType, Mesh> Assets::meshes;

    void Assets::initialize_assets(
            const vk::PhysicalDevice& physical_device,
            const vk::LogicalDevice& logical_device) {
        const auto materials = ObjLoader::load_materials(
            utils::FileUtils::read_string("assets/meshes/house.mtl"));
        meshes.emplace(CellType::HOUSE, ObjLoader::load_mesh(
            physical_device,
            &logical_device,
            materials,
            utils::FileUtils::read_string("assets/meshes/house.obj")));
    }

    void Assets::destroy_assets() {
        meshes.clear();
    }

    Mesh& Assets::get_mesh(CellType type) {
        const auto it = meshes.find(type);
        if (it == meshes.cend()) {
            throw std::runtime_error("Failed to find mesh for cell type.");
        }
        return it->second;
    }

}
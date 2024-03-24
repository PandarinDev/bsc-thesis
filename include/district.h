#pragma once

#include "bounding_box.h"
#include "wfc/building.h"
#include "wfc/ground.h"
#include "utils/hash_utils.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace inf {

    namespace gfx {
        struct Renderer;
    }

    enum class DistrictType {
        RESIDENTAL
    };

    struct DistrictBuilding {

        glm::ivec2 position;        
        wfc::Building building;
        DistrictBuilding* top;
        DistrictBuilding* right;
        DistrictBuilding* bottom;
        DistrictBuilding* left;

        DistrictBuilding(const glm::ivec2& position, wfc::Building&& building);

    };

    using DistrictBuildings = std::unordered_map<glm::ivec2, DistrictBuilding>;

    struct District {

        District(DistrictType type);
        District(District&&) = default;
        District& operator=(District&&) = default;

        DistrictBuildings& get_buildings();
        const DistrictBuildings& get_buildings() const;
        DistrictBuilding* add_building(const glm::ivec2& position, wfc::Building&& building);
        BoundingBox3D compute_bounding_box() const;
        
        void update(const gfx::Renderer& renderer);
        void render(gfx::Renderer& renderer) const;

    private:

        DistrictType type;
        DistrictBuildings buildings;

    };

}

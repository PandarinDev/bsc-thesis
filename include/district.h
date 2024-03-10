#pragma once

#include "bounding_box.h"
#include "wfc/building.h"
#include "wfc/ground.h"
#include "utils/hash_utils.h"

#include <vector>
#include <utility>
#include <unordered_set>

namespace inf {

    namespace gfx {
        struct Renderer;
    }

    enum class DistrictType {
        RESIDENTAL
    };

    struct District {

        District(DistrictType type);
        District(District&&) = default;
        District& operator=(District&&) = default;

        const std::vector<wfc::Building>& get_buildings() const;
        void add_building(wfc::Building&& building);
        bool can_place(const BoundingBox3D& bb) const;
        BoundingBox3D compute_bounding_box() const;
        
        void update(const gfx::Renderer& renderer);
        void render(gfx::Renderer& renderer) const;

    private:

        DistrictType type;
        std::vector<wfc::Building> buildings;
        std::vector<wfc::Ground> grounds;
        std::unordered_set<glm::ivec3> occupied_blocks;

    };

}

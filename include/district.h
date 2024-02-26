#pragma once

#include "bounding_box.h"
#include "wfc/building.h"
#include "wfc/ground.h"

#include <vector>
#include <utility>

namespace inf {

    namespace gfx {
        struct Renderer;
    }

    enum class DistrictType {
        RESIDENTAL
    };

    struct District {

        DistrictType type;
        std::vector<wfc::Building> buildings;
        std::vector<wfc::Ground> grounds;

        District(DistrictType type);

        BoundingBox3D compute_bounding_box() const;
        void render(gfx::Renderer& renderer) const;

    };

}

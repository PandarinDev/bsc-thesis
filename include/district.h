#pragma once

#include "bounding_box.h"
#include "wfc/building/building.h"

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
        std::pair<int, int> capacity;
        std::vector<wfc::Building> buildings;

        District(
            DistrictType type,
            const std::pair<int, int>& capacity);

        void render(const gfx::Renderer& renderer) const;

    };

}

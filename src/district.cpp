#include "district.h"
#include "gfx/renderer.h"

namespace inf {

    District::District(
        DistrictType type,
        const std::pair<int, int>& capacity) :
        type(type),
        capacity(capacity) {}

    void District::render(const gfx::Renderer& renderer) const {
        for (const auto& building : buildings) {
            renderer.render(building.mesh);
        }
    }

}

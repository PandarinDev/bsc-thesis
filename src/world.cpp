#include "world.h"

namespace inf {

    void World::render(gfx::Renderer& renderer) {
        for (const auto& district : districts) {
            district.render(renderer);
        }
    }

}

#include "world.h"

namespace inf {

    void World::update(const gfx::Renderer& renderer) {
        for (auto& district : districts) {
            district.update(renderer);
        }
    }

    void World::render(gfx::Renderer& renderer) {
        for (const auto& district : districts) {
            district.render(renderer);
        }
    }

}

#pragma once

#include "district.h"
#include "gfx/renderer.h"

#include <unordered_map>

namespace inf {

    struct World {

        std::unordered_map<glm::ivec2, District> districts;

        bool has_district_at(const glm::ivec2& position) const;
        std::size_t get_number_of_districts() const;
        std::size_t get_number_of_buildings() const;

        void update(const gfx::Renderer& renderer);
        void render(gfx::Renderer& renderer);

    };

}
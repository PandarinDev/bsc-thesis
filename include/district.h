#pragma once

#include "bounding_box.h"
#include "wfc/building.h"
#include "wfc/ground.h"
#include "utils/hash_utils.h"

#include <vector>
#include <optional>
#include <unordered_set>

namespace inf {

    namespace gfx {
        struct Renderer;
    }

    enum class DistrictType {
        RESIDENTAL
    };

    struct DistrictLot {

        glm::ivec2 position;
        glm::ivec2 dimensions;
        glm::vec3 bb_color;
        std::optional<wfc::Building> building;

        DistrictLot(
            const glm::ivec2& position,
            const glm::ivec2& dimensions,
            const glm::vec3& bb_color,
            std::optional<wfc::Building>&& building);

        BoundingBox3D get_bounding_box(const glm::vec3& district_position) const;

    };

    struct District {

        District(
            DistrictType type,
            const glm::ivec2& grid_position,
            const glm::ivec2& dimensions,
            const glm::vec3& bb_color);
        District(District&&) = default;
        District& operator=(District&&) = default;

        const glm::ivec2& get_grid_position() const;
        const glm::vec3& get_position() const;
        void set_position(const glm::vec3& position);

        BoundingBox3D compute_bounding_box() const;
        const std::vector<DistrictLot>& get_lots() const;
        void add_lot(DistrictLot&& lot);

        void render(gfx::Renderer& renderer) const;

    private:

        DistrictType type;
        glm::ivec2 grid_position;
        glm::ivec2 dimensions;
        glm::vec3 position;
        glm::vec3 bb_color;
        BoundingBox3D bounding_box;
        std::vector<DistrictLot> lots;

    };

}

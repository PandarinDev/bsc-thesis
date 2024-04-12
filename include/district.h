#pragma once

#include "bounding_box.h"
#include "wfc/building.h"
#include "wfc/ground.h"
#include "vehicle.h"
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

    enum class RoadDirection {
        HORIZONTAL_UP,
        HORIZONTAL_DOWN,
        VERTICAL_LEFT,
        VERTICAL_RIGHT,
        CROSSING_UP_LEFT,
        CROSSING_UP_RIGHT,
        CROSSING_DOWN_RIGHT,
        CROSSING_DOWN_LEFT
    };

    struct DistrictRoad {

        RoadDirection direction;
        glm::ivec2 position;

        DistrictRoad(RoadDirection direction, const glm::ivec2& position);

        bool is_crossing() const;
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

        void update_caches();

        const glm::ivec2& get_grid_position() const;
        const glm::vec3& get_position() const;
        void set_position(const glm::vec3& position);

        BoundingBox3D compute_bounding_box() const;
        const std::vector<DistrictLot>& get_lots() const;
        const std::vector<DistrictRoad>& get_roads() const;
        const std::vector<Vehicle>& get_vehicles() const;
        void add_lot(DistrictLot&& lot);
        void add_road(DistrictRoad&& road);
        void add_vehicle(Vehicle&& vehicle);

        void render(gfx::Renderer& renderer);

    private:

        DistrictType type;
        glm::ivec2 grid_position;
        glm::ivec2 dimensions;
        glm::vec3 position;
        glm::vec3 bb_color;
        BoundingBox3D bounding_box;
        std::vector<DistrictLot> lots;
        std::vector<DistrictRoad> roads;
        std::vector<Vehicle> vehicles;
        // Cache positions for instanced rendering
        std::vector<glm::vec3> grass_positions;
        std::vector<float> grass_rotations;
        std::vector<glm::vec3> road_positions;
        std::vector<float> road_rotations;
        std::vector<glm::vec3> road_crossing_positions;
        std::vector<float> road_crossing_rotations;
        std::vector<glm::vec3> vehicle_positions;
        std::vector<float> vehicle_rotations;

    };

}

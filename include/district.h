#pragma once

#include "bounding_box.h"
#include "wfc/building.h"
#include "wfc/ground.h"
#include "road.h"
#include "vehicle.h"
#include "utils/hash_utils.h"

#include <vector>
#include <optional>
#include <unordered_map>

namespace inf {

    namespace gfx {
        struct Renderer;
    }

    enum class DistrictType {
        RESIDENTAL
    };

    using DistrictFoliage = std::unordered_map<const wfc::GroundPattern*, std::vector<glm::vec3>>;

    struct DistrictLot {

        glm::ivec2 position;
        glm::ivec2 dimensions;
        glm::vec3 bb_color;
        std::optional<wfc::Building> building;
        DistrictFoliage foliage;

        DistrictLot(
            const glm::ivec2& position,
            const glm::ivec2& dimensions,
            const glm::vec3& bb_color,
            std::optional<wfc::Building>&& building,
            DistrictFoliage&& foliage);

        BoundingBox3D get_bounding_box(const glm::vec3& district_position) const;

    };

    struct District {
    
        static constexpr int DISTRICT_SIZE = 100;
        static constexpr int DISTRICT_BB_HEIGHT = 10;
        static constexpr int ROAD_GAP = 2;

        District(
            DistrictType type,
            const glm::ivec2& grid_position,
            const glm::ivec2& dimensions,
            const glm::vec3& bb_color);
        District(District&&) = default;
        District& operator=(District&&) = default;

        void update(RandomGenerator& rng, float delta_time);
        void update_caches();

        const glm::ivec2& get_grid_position() const;
        const glm::ivec2& get_dimensions() const;
        const glm::vec3& get_position() const;
        void set_position(const glm::vec3& position);

        BoundingBox3D compute_bounding_box() const;
        const std::vector<DistrictLot>& get_lots() const;
        const std::unordered_map<glm::ivec2, DistrictRoad>& get_roads() const;
        std::unordered_map<glm::ivec2, const DistrictRoad*> get_roads_at_edges() const;
        const std::vector<Vehicle>& get_vehicles() const;
        void add_lot(DistrictLot&& lot);
        void add_road(DistrictRoad&& road);
        void add_vehicle(Vehicle&& vehicle);

        void render(gfx::Renderer& renderer);

        BoundingBox3D get_left_district_bb() const;
        BoundingBox3D get_right_district_bb() const;
        BoundingBox3D get_above_district_bb() const;
        BoundingBox3D get_below_district_bb() const;

    private:

        struct InstanceData {

            std::vector<glm::vec3> positions;
            std::vector<float> rotations;

            void clear() {
                positions.clear();
                rotations.clear();
            }

        };

        DistrictType type;
        glm::ivec2 grid_position;
        glm::ivec2 dimensions;
        glm::vec3 position;
        glm::vec3 bb_color;
        BoundingBox3D bounding_box;
        std::vector<DistrictLot> lots;
        std::unordered_map<glm::ivec2, DistrictRoad> roads;
        std::vector<Vehicle> vehicles;
        // Cache positions for instanced rendering
        InstanceData grass_instances;
        std::unordered_map<const gfx::Mesh*, InstanceData> road_instances;
        std::unordered_map<const wfc::GroundPattern*, InstanceData> foliage_instances;

    };

}

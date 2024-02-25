#include "district.h"
#include "gfx/renderer.h"

#include <limits>

namespace inf {

    District::District(
        DistrictType type,
        const std::pair<int, int>& capacity) :
        type(type),
        capacity(capacity) {}

    BoundingBox3D District::compute_bounding_box() const {
        static constexpr auto float_min = std::numeric_limits<float>::lowest();
        static constexpr auto float_max = std::numeric_limits<float>::max();
        BoundingBox3D result(
            glm::vec3(float_max, float_max, float_max),
            glm::vec3(float_min, float_min, float_min));
        for (const auto& building : buildings) {
            const auto building_bb = building.get_bounding_box();
            result.update(building_bb.min);
            result.update(building_bb.max);
        }
        return result;
    }

    void District::render(gfx::Renderer& renderer) const {
        // TODO: Ground meshes are not dynamically generated with WFC, so instead of assigning
        // a mesh to each there should be a mesh pool that the instances choose from.
        for (auto& ground : grounds) {
            ground.mesh.set_model_matrix(glm::translate(glm::mat4(1.0f), ground.position));
            renderer.render(ground.mesh);
        }
        for (const auto& building : buildings) {
            renderer.render(building.get_mesh());
        }
    }

}

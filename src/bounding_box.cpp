#include "bounding_box.h"

namespace inf {

    BoundingBox3D::BoundingBox3D(const glm::vec3& min, const glm::vec3& max) :
        min(min), max(max) {}

    glm::vec3 BoundingBox3D::center() const {
        return (min + max) * 0.5f;
    }

    void BoundingBox3D::update(const glm::vec3& position) {
        if (position.x < min.x) min.x = position.x;
        if (position.y < min.y) min.y = position.y;
        if (position.z < min.z) min.z = position.z;
        if (position.x > max.x) max.x = position.x;
        if (position.y > max.y) max.y = position.y;
        if (position.z > max.z) max.z = position.z;
    }

}
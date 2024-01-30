#include "bounding_box.h"

namespace inf {

    BoundingBox2D::BoundingBox2D(const glm::ivec2& lower, const glm::ivec2& higher) :
        lower(lower), higher(higher) {}

    BoundingBox2D::BoundingBox2D(int low_x, int low_y, int high_x, int high_y) :
        lower(low_x, low_y), higher(high_x, high_y) {}

    bool BoundingBox2D::contains(const glm::ivec2& coordinate) const {
        return coordinate.x >= lower.x &&
            coordinate.y >= lower.y &&
            coordinate.x <= higher.x &&
            coordinate.y <= higher.y;
    }

}
#include "district.h"

namespace inf {

    District::District(
        DistrictType type,
        const BoundingBox2D& bounding_box) :
        type(type),
        bounding_box(bounding_box) {}

}

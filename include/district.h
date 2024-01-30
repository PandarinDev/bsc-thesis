#pragma once

#include "bounding_box.h"

namespace inf {

    enum class DistrictType {
        RESIDENTAL
    };

    struct District {

        DistrictType type;
        BoundingBox2D bounding_box;

        District(
            DistrictType type,
            const BoundingBox2D& bounding_box);

    };

}

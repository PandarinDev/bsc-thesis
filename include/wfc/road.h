#pragma once

#include "gfx/mesh.h"

namespace inf::wfc {

    struct Road {

        gfx::Mesh mesh;

        Road(gfx::Mesh&& mesh);

    };

    struct RoadPatterns {

        RoadPatterns() = delete;

    };

}
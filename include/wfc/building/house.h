#pragma once

#include "common.h"
#include "wfc/building/building.h"
#include "gfx/vk/device.h"

namespace inf::wfc::building {

    struct HouseGenerator {

        HouseGenerator() = delete;

        static Building generate(
            RandomGenerator& rng,
            const gfx::vk::PhysicalDevice& physical_device,
            const gfx::vk::LogicalDevice* logical_device);

    };

}

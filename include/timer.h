#pragma once

#include <cstdint>

namespace inf {

    struct Timer {

        Timer();

        void tick();
        double get_time() const;
        double get_delta() const;
        std::uint32_t get_fps() const;

    private:

        std::uint32_t frames;
        std::uint32_t fps;
        double last_frame;
        double last_fps;
        double delta;

    };

}
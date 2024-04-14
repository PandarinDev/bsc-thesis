#pragma once

#include <functional>

namespace inf {

    enum class State {
        PANNING,
        FREECAM,
        ELEMENT_PICKING
    };

    struct Context {
        
        float day_of_time;

        Context(const std::function<void(bool)>& set_mouse_captured);

        void advance_day_of_time(float delta_time);
        State get_state() const;
        void set_state(State state);

    private:
        
        State state;
        std::function<void(bool)> set_mouse_captured;

    };

}
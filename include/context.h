#pragma once

#include <functional>

namespace inf {

    enum class State {
        PANNING,
        FREECAM,
        ELEMENT_PICKING
    };

    struct Context {
        
        float time_of_day;

        Context(const std::function<void(bool)>& set_mouse_captured);

        void advance_time_of_day(float delta_time);
        State get_state() const;
        void set_state(State state);

    private:
        
        State state;
        std::function<void(bool)> set_mouse_captured;

    };

}
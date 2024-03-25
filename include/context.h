#pragma once

#include <functional>

namespace inf {

    enum class State {
        PANNING,
        FREECAM,
        ELEMENT_PICKING
    };

    struct Context {
        
        Context(const std::function<void(bool)>& set_mouse_captured);

        State get_state() const;
        void set_state(State state);

    private:
        
        State state;
        std::function<void(bool)> set_mouse_captured;

    };

}
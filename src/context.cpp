#include "context.h"

namespace inf {

    Context::Context(const std::function<void(bool)>& set_mouse_captured) :
        state(State::PANNING), set_mouse_captured(set_mouse_captured) {}

    State Context::get_state() const {
        return state;
    }

    void Context::set_state(State state) {
        this->state = state;
        set_mouse_captured(state != State::ELEMENT_PICKING);
    }

}
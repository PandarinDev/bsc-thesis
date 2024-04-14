#include "context.h"

#include <cmath>

namespace inf {

    Context::Context(const std::function<void(bool)>& set_mouse_captured) :
        day_of_time(0.0f), state(State::PANNING), set_mouse_captured(set_mouse_captured) {}

    void Context::advance_day_of_time(float delta_time) {
        static constexpr float day_duration_seconds = 60.0f;
        static constexpr float day_of_time_per_second = 1.0f / day_duration_seconds;
        day_of_time = std::fmod(day_of_time + day_of_time_per_second * delta_time, 1.0f);
    }

    State Context::get_state() const {
        return state;
    }

    void Context::set_state(State state) {
        this->state = state;
        set_mouse_captured(state != State::ELEMENT_PICKING);
    }

}
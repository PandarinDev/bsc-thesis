#include "context.h"

#include <cmath>

namespace inf {

    Context::Context(const std::function<void(bool)>& set_mouse_captured) :
        time_of_day(0.5f), state(State::PANNING), set_mouse_captured(set_mouse_captured) {}

    void Context::advance_time_of_day(float delta_time) {
        static constexpr float day_duration_seconds = 60.0f;
        static constexpr float time_of_day_per_second = 1.0f / day_duration_seconds;
        time_of_day = std::fmod(time_of_day + time_of_day_per_second * delta_time, 1.0f);
    }

    State Context::get_state() const {
        return state;
    }

    void Context::set_state(State state) {
        this->state = state;
        set_mouse_captured(state != State::ELEMENT_PICKING);
    }

}
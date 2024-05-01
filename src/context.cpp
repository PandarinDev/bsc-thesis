#include "context.h"

#include <cmath>

namespace inf {

    Context::Context(const std::function<void(bool)>& set_mouse_captured) :
        time_of_day(0.5f), camera_speed(CAMERA_SPEED_INIITAL), fix_time_of_day(false), override_weather(false),
        weather_change_frequency_seconds(WEATHER_CHANGE_FREQUENCY_SECONDS_INITIAL),
        weather_change_chance_percentage(WEATHER_CHANGE_CHANCE_PERCENTAGE_INITIAL),
        show_diagnostics(false),
        show_debug_bbs(false),
        state(State::PANNING), set_mouse_captured(set_mouse_captured),
        weather_change_force_flag(false), weather(Weather::SUNNY), rain_intensity(RainIntensity::LIGHT) {}

    void Context::advance_time_of_day(float delta_time) {
        if (fix_time_of_day) {
            return;
        }
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

    void Context::force_weather_change(Weather weather, RainIntensity rain_intensity) {
        this->weather = weather;
        this->rain_intensity = rain_intensity;
        this->weather_change_force_flag = true;
    }

    bool Context::should_force_weather_change() {
        const auto result = weather_change_force_flag;
        weather_change_force_flag = false; // Reset the flag once it's been queried
        return result;
    }
    
    Weather Context::get_overriden_weather() const {
        return weather;
    }
    
    RainIntensity Context::get_overriden_rain_intensity() const {
        return rain_intensity;
    }

}
#pragma once

#include "weather.h"

#include <functional>

namespace inf {

    enum class State {
        PANNING,
        FREECAM,
        ELEMENT_PICKING
    };

    struct Context {
        
        // Camera speed
        static constexpr float CAMERA_SPEED_INIITAL = 1.0f;
        static constexpr float CAMERA_SPEED_MIN = 1.0f;
        static constexpr float CAMERA_SPEED_MAX = 3.0f;

        // Weather change frequency
        static constexpr float WEATHER_CHANGE_FREQUENCY_SECONDS_INITIAL = 10.0f;
        static constexpr float WEATHER_CHANGE_FREQUENCY_SECONDS_MIN = 5.0f;
        static constexpr float WEATHER_CHANGE_FREQUENCY_SECONDS_MAX = 30.0f;

        // Weather change likelyhood
        static constexpr float WEATHER_CHANGE_CHANCE_PERCENTAGE_INITIAL = 0.30f;
        static constexpr float WEATHER_CHANGE_CHANCE_PERCENTAGE_MIN = 0.0f;
        static constexpr float WEATHER_CHANGE_CHANCE_PERCENTAGE_MAX = 1.0f;

        float time_of_day;
        float camera_speed;
        bool fix_time_of_day;
        bool override_weather;
        float weather_change_frequency_seconds;
        float weather_change_chance_percentage;
        bool show_diagnostics;
        bool show_debug_bbs;

        Context(const std::function<void(bool)>& set_mouse_captured);

        void advance_time_of_day(float delta_time);
        State get_state() const;
        void set_state(State state);
        void force_weather_change(Weather weather, RainIntensity rain_intensity);
        bool should_force_weather_change();
        Weather get_overriden_weather() const;
        RainIntensity get_overriden_rain_intensity() const;

    private:
        
        State state;
        std::function<void(bool)> set_mouse_captured;
        bool weather_change_force_flag;
        Weather weather;
        RainIntensity rain_intensity;

    };

}
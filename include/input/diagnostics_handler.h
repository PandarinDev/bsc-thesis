#pragma once

#include "input/input_manager.h"

#include <functional>

namespace inf::input {

    struct DiagnosticsHandler final : public InputHandler {

        DiagnosticsHandler(const std::function<void(bool)>& toggle_function);

        void handle_input(
            const float delta_time,
            const KeyFunction& is_key_down,
            const KeyFunction& is_key_up,
            const glm::vec2& mouse_delta) override;

    private:

        bool enabled;
        std::function<void(bool)> toggle_function;

    };

}
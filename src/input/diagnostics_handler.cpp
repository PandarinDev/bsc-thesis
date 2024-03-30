#include "input/diagnostics_handler.h"

namespace inf::input {

    DiagnosticsHandler::DiagnosticsHandler(const std::function<void(bool)>& toggle_function) :
        enabled(false), toggle_function(toggle_function) {}

    void DiagnosticsHandler::handle_input(
        [[maybe_unused]] const float delta_time,
        [[maybe_unused]] const KeyFunction& is_key_down,
        const KeyFunction& is_key_up,
        [[maybe_unused]] const glm::vec2& mouse_coordinates,
        [[maybe_unused]] const glm::vec2& mouse_delta,
        [[maybe_unused]] const bool has_clicked) {
        if (is_key_up(GLFW_KEY_F11)) {
            enabled = !enabled;
            toggle_function(enabled);
        }
    }

}
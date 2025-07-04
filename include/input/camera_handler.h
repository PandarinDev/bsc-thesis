#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "context.h"
#include "camera.h"
#include "input/input_manager.h"

namespace inf::input {

    struct CameraHandler final : public InputHandler {

        CameraHandler(Context& context, Camera& camera);

        void handle_input(
            const float delta_time,
            const KeyFunction& is_key_down,
            const KeyFunction& is_key_up,
            const glm::vec2& mouse_coordinates,
            const glm::vec2& mouse_delta,
            const bool has_clicked) override;

    private:

        Context& context;
        Camera& camera;

    };

}
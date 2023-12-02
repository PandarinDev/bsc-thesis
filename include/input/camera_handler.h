#pragma once

#include "camera.h"
#include "input/input_manager.h"

namespace inf::input {

    struct CameraHandler final : public InputHandler {

        CameraHandler(Camera& camera);

        void handle_input(
            const float delta_time,
            const KeyFunction& is_key_down,
            const KeyFunction& is_key_up,
            const glm::vec2& mouse_delta) override;

    private:

        Camera& camera;
        bool enabled;

    };

}
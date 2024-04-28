#pragma once

#include "context.h"
#include "world.h"
#include "camera.h"
#include "input/input_manager.h"

#include <glm/matrix.hpp>

#include <functional>

namespace inf::input {

    struct ElementSelectionHandler final : public InputHandler {

        ElementSelectionHandler(
            Context& context,
            const World& world,
            const Camera& camera,
            const glm::mat4& projection_matrix);

        void handle_input(
            const float delta_time,
            const KeyFunction& is_key_down,
            const KeyFunction& is_key_up,
            const glm::vec2& mouse_coordinates,
            const glm::vec2& mouse_delta,
            const bool has_clicked) override;

    private:

        Context& context;
        [[maybe_unused]] const World& world;
        [[maybe_unused]] const Camera& camera;
        [[maybe_unused]] const glm::mat4 projection_matrix;

    };

}
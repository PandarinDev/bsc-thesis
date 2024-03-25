#pragma once

#include "window.h"
#include "timer.h"

#include <glm/vec2.hpp>

#include <vector>
#include <memory>
#include <functional>
#include <unordered_set>

namespace inf::input {

    using KeyFunction = std::function<bool(int)>;

    struct InputHandler {

        virtual ~InputHandler() = default;

        virtual void handle_input(
            const float delta_time,
            const KeyFunction& is_key_down,
            const KeyFunction& is_key_up,
            const glm::vec2& mouse_coordinates,
            const glm::vec2& mouse_delta,
            const bool has_clicked) = 0;

    };

    struct InputManager {

        InputManager(
            const Window& window,
            const Timer& timer);

        void update();
        void add_handler(std::unique_ptr<InputHandler> handler);

    private:

        const Window& window;
        const Timer& timer;
        std::vector<std::unique_ptr<InputHandler>> input_handlers;
        std::unordered_set<int> keys_down;
        std::unordered_set<int> keys_up;
        glm::vec2 last_mouse_coords;
        int last_left_mouse_button_state;

    };

}
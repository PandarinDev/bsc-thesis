#include "input/input_manager.h"

namespace inf::input {

    InputManager::InputManager(
        const Window& window,
        const Timer& timer) :
        window(window),
        timer(timer),
        last_left_mouse_button_state(GLFW_RELEASE) {}

    void InputManager::update() {
        // Clear the list of released keys from the previous frame
        keys_up.clear();

        const auto window_handle = window.get_handle();
        // Query the state of each key and update the state appropriately
        for (int key = GLFW_KEY_SPACE; key < GLFW_KEY_LAST; ++key) {
            const auto state = glfwGetKey(window_handle, key);
            if (state == GLFW_PRESS) {
                keys_down.emplace(key);
            }
            else if (state == GLFW_RELEASE) {
                // Check if the key was released exactly this frame
                if (keys_down.find(key) != keys_down.cend()) {
                    keys_up.emplace(key);
                    keys_down.erase(key);
                }
            }
        }

        // Compute mouse delta
        const auto window_size = window.get_size();
        double mouse_x, mouse_y;
        glfwGetCursorPos(window_handle, &mouse_x, &mouse_y);
        glm::vec2 mouse_coords(mouse_x, mouse_y);
        glm::vec2 normalized_mouse_coords(mouse_coords.x / window_size.x, mouse_coords.y / window_size.y);
        glm::vec2 mouse_delta = mouse_coords - last_mouse_coords;
        // Flip Y as GLFW uses Y+ down
        mouse_delta.y *= -1.0f;
        // Normalize mouse delta to the screen size
        mouse_delta.x /= window_size.x;
        mouse_delta.y /= window_size.y;
        last_mouse_coords = mouse_coords;

        // Notify all input handlers
        const auto delta_time = static_cast<float>(timer.get_delta());
        const auto is_key_down = [this](int key) {
            return keys_down.find(key) != keys_down.cend();
        };
        const auto is_key_up = [this](int key) {
            return keys_up.find(key) != keys_up.cend();
        };
        int left_mouse_button_state = glfwGetMouseButton(window_handle, GLFW_MOUSE_BUTTON_LEFT);
        bool has_clicked = last_left_mouse_button_state == GLFW_PRESS && left_mouse_button_state == GLFW_RELEASE;
        last_left_mouse_button_state = left_mouse_button_state;
        for (const auto& handler : input_handlers) {
            handler->handle_input(delta_time, is_key_down, is_key_up, normalized_mouse_coords, mouse_delta, has_clicked);
        }
    }

    void InputManager::add_handler(std::unique_ptr<InputHandler> handler) {
        input_handlers.emplace_back(std::move(handler));
    }

}
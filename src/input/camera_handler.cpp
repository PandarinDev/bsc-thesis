#include "input/camera_handler.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <algorithm>

namespace inf::input {

    static constexpr float CAMERA_SENSITIVITY = 1.0f;
    static constexpr glm::vec3 BACK_DIRECTION(0.0f, 0.0f, 1.0f);

    CameraHandler::CameraHandler(Context& context, Camera& camera) :
        context(context), camera(camera) {}

    void CameraHandler::handle_input(
        const float delta_time,
        const KeyFunction& is_key_down,
        const KeyFunction& is_key_up,
        [[maybe_unused]] const glm::vec2& mouse_coordinates,
        const glm::vec2& mouse_delta,
        [[maybe_unused]] const bool has_clicked) {
        if (is_key_up(GLFW_KEY_F)) {
            context.set_state(context.get_state() == State::FREECAM
                ? State::PANNING
                : State::FREECAM);
        }
        if (context.get_state() == State::PANNING) {
            // If direct controls are not enabled simply pan the camera slowly back
            camera.set_position(camera.get_position() + BACK_DIRECTION * delta_time * context.camera_speed);
        }
        if (context.get_state() != State::FREECAM) {
            return;
        }

        // Handle camera rotation
        auto direction = glm::normalize(camera.get_direction());
        auto pitch = std::asin(direction.y);
        auto yaw = std::atan2(direction.x, direction.z);

        // Modify Euler angles according to mouse movement
        pitch = std::clamp(pitch + mouse_delta.y * CAMERA_SENSITIVITY, static_cast<float>(-M_PI_4), static_cast<float>(M_PI_4));
        yaw -= mouse_delta.x * CAMERA_SENSITIVITY;

        // Build the new direction vector from the Euler angles
        direction.x = std::sin(yaw) * std::cos(pitch);
        direction.y = std::sin(pitch);
        direction.z = std::cos(yaw) * std::cos(pitch);
        camera.set_direction(direction);

        static const glm::vec3 up(0.0f, 1.0f, 0.0f);
        const auto right = glm::cross(direction, up);

        // Handle camera movement using the W,A,S,D and Q,E (up/down) keys
        glm::vec3 translation{};
        if (is_key_down(GLFW_KEY_W)) {
            translation += direction * context.camera_speed;
        }
        if (is_key_down(GLFW_KEY_S)) {
            translation -= direction * context.camera_speed;
        }
        if (is_key_down(GLFW_KEY_A)) {
            translation -= right * context.camera_speed;
        }
        if (is_key_down(GLFW_KEY_D)) {
            translation += right * context.camera_speed;
        }
        if (is_key_down(GLFW_KEY_Q)) {
            translation.y += context.camera_speed;
        }
        if (is_key_down(GLFW_KEY_E)) {
            translation.y -= context.camera_speed;
        }
        translation *= delta_time;
        camera.set_position(camera.get_position() + translation);
    }

}
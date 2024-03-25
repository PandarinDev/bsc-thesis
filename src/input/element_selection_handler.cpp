#include "input/element_selection_handler.h"

#include <limits>
#include <iostream>

namespace inf::input {

    ElementSelectionHandler::ElementSelectionHandler(
        Context& context,
        const District& district,
        const Camera& camera,
        const glm::mat4& projection_matrix) :
        context(context), district(district), projection_matrix(projection_matrix),
        camera(camera) {}

    void ElementSelectionHandler::handle_input(
        [[maybe_unused]] const float delta_time,
        [[maybe_unused]] const KeyFunction& is_key_down,
        [[maybe_unused]] const KeyFunction& is_key_up,
        const glm::vec2& mouse_coordinates,
        [[maybe_unused]] const glm::vec2& mouse_delta,
        const bool has_clicked) {
        if (is_key_up(GLFW_KEY_P)) {
            // If we are already in element picking mode return to panning
            if (context.get_state() == State::ELEMENT_PICKING) {
                context.set_state(State::PANNING);
                return;
            }
            // Otherwise enter element picking
            context.set_state(State::ELEMENT_PICKING);
        }
        if (context.get_state() != State::ELEMENT_PICKING || !has_clicked) {
            return;
        }

        // Mouse coordinates are normalized between [0, 1], but NDC is [-1, 1]
        const auto mouse_ndc = (mouse_coordinates - glm::vec2(0.5f, 0.5f)) * 2.0f;
        const auto projection_view_matrix = projection_matrix * camera.to_view_matrix();
        const auto& camera_position = camera.get_position();
        auto min_distance = std::numeric_limits<float>::max();
        const glm::ivec2* closest_intersection = nullptr;
        const wfc::Building* closest_building = nullptr;
        for (const auto& entry : district.get_buildings()) {
            const auto bb = entry.second.building.get_bounding_box();
            const auto bb_in_ndc = bb.apply_and_transform_to_ndc(projection_view_matrix);
            // Since the bounding box is now transformed to NDC we can simply do XY intersection checking
            if (mouse_ndc.x >= bb_in_ndc.min.x &&
                mouse_ndc.x <= bb_in_ndc.max.x &&
                mouse_ndc.y >= bb_in_ndc.min.y &&
                mouse_ndc.y <= bb_in_ndc.max.y) {
                // TODO: Comparing to center might not be ideal, instead maybe take the closest BB point to camera
                const auto distance = glm::length(camera_position - bb.center());
                if (distance < min_distance) {
                    min_distance = distance;
                    closest_intersection = &entry.first;
                    closest_building = &entry.second.building;
                }
            }
        }
        if (closest_intersection) {
            std::cout << "Closest intersection: [" << closest_intersection->x << ", " << closest_intersection->y << "]" << std::endl;
            const auto closest_bb = closest_building->get_bounding_box();
            std::cout << "Closest BB: [" << closest_bb.min.x << ", " << closest_bb.min.y << ", " << closest_bb.min.z << "], [" <<
                closest_bb.max.x << ", " << closest_bb.max.y << ", " << closest_bb.max.z << "]" << std::endl;
        }
    }

}

#include "window.h"
#include "camera.h"
#include "timer.h"
#include "world.h"
#include "context.h"
#include "generator.h"
#include "gfx/renderer.h"
#include "input/input_manager.h"
#include "input/camera_handler.h"
#include "input/diagnostics_handler.h"
#include "input/element_selection_handler.h"
#include "wfc/building.h"
#include "wfc/ground.h"
#include "utils/file_utils.h"

#include <iostream>
#include <stdexcept>

using namespace inf;
using namespace inf::gfx;
using namespace inf::input;
using namespace inf::utils;

int main() {
    try {
        Window window("Infinitown", RelativeWindowSize{ 0.75f }, false);
        Timer timer;
        InputManager input_manager(window, timer);
        Context context([handle = window.get_handle()](bool captured) {
            glfwSetInputMode(handle, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        });

        Camera camera(glm::vec3(0.0f, 7.0f, 2.0f), glm::vec3(0.0f, -0.6f, -1.0f));
        Renderer renderer(context, window, camera, timer);
        input_manager.add_handler(std::make_unique<CameraHandler>(context, camera));
        input_manager.add_handler(std::make_unique<DiagnosticsHandler>([&renderer](bool value) {
            renderer.set_show_diagnostics(value);
        }));

        const auto asset_load_start_time = timer.get_time();
        wfc::BuildingPatterns::initialize("assets/buildings");
        wfc::GroundPatterns::initialize("assets/grounds", &renderer.get_logical_device(), &renderer.get_memory_allocator());
        VehiclePatterns::initialize("assets/vehicles");
        ParticleMeshes::initialize(&renderer.get_logical_device(), &renderer.get_memory_allocator());
        const auto asset_load_elapsed_time = timer.get_time() - asset_load_start_time;
        std::cout << "Asset loading took " << asset_load_elapsed_time << " seconds." << std::endl;

        std::random_device random_device;
        RandomGenerator random_engine(random_device());
        const auto generation_start_time = timer.get_time();
        WorldGenerator generator(random_engine, renderer);
        World world = generator.generate_initial(timer);
        const auto generation_elapsed_time = timer.get_time() - generation_start_time;
        std::cout << "World generation took " << generation_elapsed_time << " seconds." << std::endl;

        input_manager.add_handler(std::make_unique<ElementSelectionHandler>(
            context,            
            world,
            camera,
            renderer.get_projection_matrix()));

        while (!window.should_close()) {
            timer.tick();
            window.poll_events();
            input_manager.update();
            const auto delta_time = static_cast<float>(timer.get_delta());
            context.advance_time_of_day(delta_time);
            world.update(renderer, random_engine, delta_time);
            generator.populate_world(world);
            if (world.is_dirty()) {
                world.update_caches();
            }
            renderer.begin_frame(world.get_number_of_districts(), world.get_number_of_buildings());
            world.render(renderer);
            renderer.end_frame();
        }
        // Wait until the device becomes idle (flushes queues) to destroy in a well-defined state
        renderer.get_logical_device().wait_until_idle();
        renderer.destroy_imgui();
        // Ground pattern and particle meshes are not dynamically generated, so they are statically stored.
        // Hence, they need to be cleaned up explicitly before shutdown to avoid validation layers complaining.
        wfc::GroundPatterns::deinitialize();
        ParticleMeshes::deinitialize();
        glfwTerminate();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error happened: " << e.what() << std::endl;
        return 1;
    }
}

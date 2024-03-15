#include "window.h"
#include "camera.h"
#include "timer.h"
#include "world.h"
#include "generator.h"
#include "gfx/renderer.h"
#include "input/input_manager.h"
#include "input/camera_handler.h"
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

        Camera camera(glm::vec3(0.0f, 4.0f, 2.0f), glm::vec3(0.0f, -0.5f, -1.0f));
        Renderer renderer(window, camera);
        input_manager.add_handler(std::make_unique<CameraHandler>(camera));

        const auto asset_load_start_time = timer.get_time();
        wfc::BuildingPatterns::initialize("assets/buildings");
        wfc::GroundPatterns::initialize("assets/grounds", &renderer.get_logical_device(), &renderer.get_memory_allocator());
        const auto asset_load_elapsed_time = timer.get_time() - asset_load_start_time;
        std::cout << "Asset loading took " << asset_load_elapsed_time << " seconds." << std::endl;

        std::random_device random_device;
        RandomGenerator random_engine(random_device());
        const auto generation_start_time = timer.get_time();
        WorldGenerator generator(random_engine, renderer);
        World world = generator.generate_initial();
        // TODO: When we'll have multiple districts this will obviously not work
        auto& district = world.districts[0];
        const auto generation_elapsed_time = timer.get_time() - generation_start_time;
        std::cout << "World generation took " << generation_elapsed_time << " seconds." << std::endl;

        while (!window.should_close()) {
            timer.tick();
            window.poll_events();
            input_manager.update();
            world.update(renderer);
            generator.populate_district_edges(district);
            renderer.begin_frame();
            world.render(renderer);
            renderer.end_frame(district.compute_bounding_box());
        }
        // Wait until the device becomes idle (flushes queues) to destroy in a well-defined state
        renderer.get_logical_device().wait_until_idle();
        // Ground pattern meshes are not dynamically generated, so they are statically stored.
        // Hence, they need to be cleaned up explicitly before shutdown to avoid validation layers complaining.
        wfc::GroundPatterns::deinitialize();
        glfwTerminate();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error happened: " << e.what() << std::endl;
        return 1;
    }
}

#include "window.h"
#include "camera.h"
#include "timer.h"
#include "world.h"
#include "generator.h"
#include "gfx/renderer.h"
#include "input/input_manager.h"
#include "input/camera_handler.h"
#include "utils/file_utils.h"

#include <iostream>

using namespace inf;
using namespace inf::gfx;
using namespace inf::input;
using namespace inf::utils;

int main() {
    Window window("Infinitown", RelativeWindowSize{ 0.75f }, false);
    Timer timer;
    InputManager input_manager(window, timer);

    Camera camera(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(0.0f, -0.5f, -1.0f));
    Renderer renderer(window, camera);
    input_manager.add_handler(std::make_unique<CameraHandler>(camera));

    const auto generation_start_time = timer.get_time();
    World world = WorldGenerator::generate_initial(
        renderer.get_physical_device(),
        &renderer.get_logical_device());
    const auto generation_elapsed_time = timer.get_time() - generation_start_time;
    std::cout << "World generation took " << generation_elapsed_time << " seconds." << std::endl;

    while (!window.should_close()) {
        timer.tick();
        window.poll_events();
        input_manager.update();
        renderer.begin_frame();
        world.render(renderer);
        renderer.end_frame();
    }
    // Wait until the device becomes idle (flushes queues) to destroy in a well-defined state
    renderer.get_logical_device().wait_until_idle();
    glfwTerminate();
    return 0;
}

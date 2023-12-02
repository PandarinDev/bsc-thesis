#include "window.h"
#include "camera.h"
#include "timer.h"
#include "world.h"
#include "gfx/renderer.h"
#include "input/input_manager.h"
#include "input/camera_handler.h"

using namespace inf;
using namespace inf::gfx;
using namespace inf::input;

int main() {
    Window window("Infinitown", 1600, 900, false);
    Timer timer;
    InputManager input_manager(window, timer);
    Camera camera(glm::vec3(0.0f, 2.0f, 0.0f), glm::normalize(glm::vec3(0.0f, -0.45f, -1.0f)));
    Renderer renderer(window, camera);
    input_manager.add_handler(std::make_unique<CameraHandler>(camera));

    // Build a test cube that we are going to render and instantiate the world
    World world(Cube::build(renderer.get_physical_device(), &renderer.get_logical_device(), 1.0f, glm::vec3(1.0f, 0.0f, 0.0f)));
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
// Standartne cpp kniznice
#include <iostream>

// Moje header files
#include "engine/renderer.hpp"
#include "engine/world.hpp"
#include "engine/particle.hpp"
#include "engine/camera.hpp"
#include "engine/entity.hpp"
#include "engine/controls.hpp"
#include "others/GLOBALS.hpp"
#include "others/utils.hpp"

World world(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE);
Renderer render(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT);
Camera camera(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT);
Player player();
// Controls controls(&player);

int main(int argc, char **argv)
{
    render.init();
    render.set_camera(&camera);
    render.enable_blending();
    render.enable_ortho_projection();

    render.print_render_info();

    while (!render.should_close())
    {
        // controls

        // world.update_world();

        // buffer handling

        render.render_everything();
    }

    render.cleanup();

    return 0;
}

void test()
{
    float verticies[] = {
        100.0f, 100.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        150.0f, 100.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        100.0f, 150.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        150.0f, 150.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    unsigned int indicies[] = {
        0, 1, 2,
        1, 2, 3
    };
}
// Standartne cpp kniznice
#include <iostream>

// Moje header files
#include "engine/renderer/renderer.hpp"
#include "engine/world.hpp"
#include "engine/particle.hpp"
#include "engine/camera.hpp"
#include "engine/entity.hpp"
#include "engine/controls.hpp"
#include "others/GLOBALS.hpp"
#include "others/utils.hpp"

Player player();
Camera camera(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT);
Renderer render(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT);
World world(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE);
// Controls controls(&player);

int main(int argc, char **argv)
{
    render.init();
    // render.set_camera(&camera);
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
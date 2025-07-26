// Standartne cpp kniznice
#include <iostream>

// Moje header files
#include "engine/renderer/renderer.hpp"
// #include "engine/world.hpp"
// #include "engine/particle.hpp"
// #include "engine/camera.hpp"
// #include "engine/entity.hpp"
// #include "engine/controls.hpp"
#include "others/GLOBALS.hpp"
// #include "others/utils.hpp"

// Player player;
// Controls controls(&player);
// Camera camera(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT);
Renderer render(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT);
// World world(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE);

int main(int argc, char **argv)
{
    // std::cout << "PRED INIT";
    render.init();

    render.enable_blending();
    render.enable_ortho_projection();
    
    // nefunguje vyjebat prec
    render.enable_pixel_perfect_rendering();
    
    // render.print_render_info();

    // controls.set_window(render.get_window());

    while (!render.should_close())
    {
        // controls
        // controls.handle_input();

        // render everything
        render.render_everything();

        // std::cout << "HELL OWORLD\n";
    }

    // render.cleanup();

    std::cout << "KONIEC PROGRAMU\n";
    return 0;
}
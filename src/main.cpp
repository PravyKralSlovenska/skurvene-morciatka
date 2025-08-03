// Standartne cpp kniznice
#include <iostream>

// Moje header files
#include "engine/renderer/renderer.hpp"
#include "engine/world/world.hpp"
// #include "engine/particle.hpp"
// #include "engine/camera.hpp"
#include "engine/entity.hpp"
#include "engine/controls.hpp"
#include "others/GLOBALS.hpp"

Player player("MISKO", {100.0f, 100.0f});
Controls controls;
World world(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE);
IRenderer render(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE, &world);

int main()
{
    render.init();
    render.enable_blending();
    render.enable_ortho_projection();
    // render.set_world(&world);
    
    controls.set_player(&player);
    controls.set_window(render.get_window());
    controls.set_world(&world);
    
    world.entities.push_back(player);
    
    render.print_render_info();

    // game loop
    while (!render.should_close())
    {
        // controls
        controls.handle_input();

        // update sveta
        world.update_world();

        // render everything
        render.render_everything();
    }

    render.cleanup();

    std::cout << "END\n";
}
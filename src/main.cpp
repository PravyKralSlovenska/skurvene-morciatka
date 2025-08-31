// Standartne cpp kniznice
#include <iostream>

// Moje header files
#include "engine/renderer/renderer.hpp"
#include "engine/world/world.hpp"
#include "engine/entity.hpp"
#include "engine/controls.hpp"
#include "engine/audio/audio_manager.hpp"
#include "others/GLOBALS.hpp"

Player player("MISKO", {100.0f, 100.0f});
Controls controls;
World world(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE);
IRenderer render(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE, &world);
Audio_Manager audio_manager;

enum GAME_STATES
{
    MENU,
    GAME,
    PAUSE,
    END
    // OPTIONS // ???
};

int main()
{
    render.init();
    render.enable_blending();
    render.enable_ortho_projection();
    // render.set_world(&world);

    audio_manager.init();
    audio_manager.set_listener(&player);
    audio_manager.load_music("mulano stylos", "../music/menu/KONTRAFAKT - Mulano stylos.mp3");
    audio_manager.load_music("nemaj stres", "../music/menu/H16 - Nemaj stres.mp3");
    audio_manager.load_music("era", "../music/menu/KONTRAFAKT - E.R.A.mp3");
    audio_manager.load_music("zme uplne na picu", "../music/end/KONTRAFAKT - Zme uplne na picu.mp3");

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
        world.update_world_loop();

        // render everything
        render.render_everything();
    }

    render.cleanup();

    std::cout << "END\n";
    return 0;
}
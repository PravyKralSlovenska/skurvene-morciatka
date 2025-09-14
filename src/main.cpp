// Standartne cpp kniznice
#include <iostream>

// Moje header files
#include "engine/renderer/renderer.hpp"
#include "engine/world/world.hpp"
#include "engine/entity.hpp"
#include "engine/controls.hpp"
#include "engine/audio/audio_manager.hpp"
#include "engine/time_manager.hpp"

// namespace na globalne premenne
#include "others/GLOBALS.hpp"

Controls controls;
Audio_Manager audio_manager;
Time_Manager time_manager;
Player player("MISKO", {100.0f, 100.0f});
World world(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE);
IRenderer render(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, Globals::PARTICLE_SIZE, &world);

// chcem pouzit na nejake dalsie renderovanie
// napriklad chcem vyrenderovat obrazovku ked je hra zastavena ci som len v menu
enum GAME_STATES
{
    MENU,
    GAME,
    PAUSE,
    OPTIONS, // ???
    END
};

int main()
{
    time_manager.init();
    time_manager.set_target_fps(60);

    render.init();
    render.enable_blending();
    render.enable_ortho_projection();
    render.set_time_manager(&time_manager);
    // render.set_world(&world);

    audio_manager.init();
    audio_manager.set_player(&player);
    audio_manager.load_music("mulano stylos", "../music/menu/KONTRAFAKT - Mulano stylos.mp3");
    // audio_manager.load_music("era", "../music/menu/KONTRAFAKT - E.R.A.mp3");
    // audio_manager.load_music("nemaj stres", "../music/menu/H16 - Nemaj stres.mp3");
    // audio_manager.load_music("zme uplne na picu", "../music/end/KONTRAFAKT - Zme uplne na picu.mp3");

    controls.set_player(&player);
    controls.set_window(render.get_window());
    controls.set_world(&world);
    controls.set_time_manager(&time_manager);

    world.entities.push_back(player);

    render.print_render_info();

    // niekedy vyhodi chybu 40961, 40964
    audio_manager.play("mulano stylos");
    // audio_manager.play("era");
    // audio_manager.play("nemaj stres");
    // audio_manager.play("zme uplne na picu");

    // game loop
    while (!render.should_close())
    {
        // time
        time_manager.time_update();

        // controls
        controls.handle_input();

        // update sveta
        if (!time_manager.is_paused())
        {
            world.update_world_loop();
        }

        // world.update_world_loop();

        // render everything
        render.render_everything();

        // audio`
        // mal by som kontrolovat activne sourcy a mazat ich
    }

    render.cleanup();
    // audio_manager.cleanup(); // volam v destructori

    std::cout << "END\n";
    return 0;
}
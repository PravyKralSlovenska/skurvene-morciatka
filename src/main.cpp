// Standartne cpp kniznice
#include <iostream>

// Moje main header files
#include "engine/renderer/renderer.hpp"
#include "engine/world/world.hpp"
#include "engine/world/herringbone_world_generation.hpp"
#include "engine/entity.hpp"
#include "engine/controls.hpp"
#include "engine/audio/audio_manager.hpp"
#include "engine/time_manager.hpp"

// namespace na globalne premenne
#include "others/GLOBALS.hpp"

Controls controls;
Audio_Manager audio_manager;
Time_Manager time_manager;
// Random random;
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
    Herringbone_World_Generation world_gen(2);
    world_gen.load_tileset_from_image("../tilesets/template_caves_limit_connectivity.png");
    // world_gen.load_tileset_from_image("../tilesets/template_ref2_corner_caves.png");
    // world_gen.load_tileset_from_image("../tilesets/template_sean_dungeon.png");
    world_gen.generate_map("../map.png", 200, 200);

    time_manager.init();
    // time_manager.set_target_fps(5);
    // time_manager.enable_fps_limiting();

    render.init();
    render.enable_blending();
    render.enable_ortho_projection();
    render.set_time_manager(&time_manager);
    // render.set_world(&world);
    render.print_render_info();

    controls.set_player(&player);
    controls.set_window(render.get_window());
    controls.set_world(&world);
    controls.set_audio_manager(&audio_manager);
    controls.set_time_manager(&time_manager);

    world.entities.push_back(player);
    // world.set_time_manager(&time_manager);

    // audio_manager.init();
    // audio_manager.set_player(&player);
    // audio_manager.set_time_manager(&time_manager);
    // audio_manager.send_execute(Pending_Execute::Operations::LOAD, "background music", "../music/menu/KONTRAFAKT - Mulano stylos.mp3");
    // audio_manager.send_execute(Pending_Execute::Operations::PLAY, "background music");
    // audio_manager.send_execute(Pending_Execute::Operations::LOAD, "background music", "../music/menu/KONTRAFAKT - E.R.A.mp3");
    // audio_manager.send_execute(Pending_Execute::Operations::LOAD, "background music", "../music/menu/H16 - Nemaj stres.mp3");
    // audio_manager.send_execute(Pending_Execute::Operations::LOAD, "background music", "../music/end/KONTRAFAKT - Zme uplne na picu.mp3");
    // audio_manager.send_execute(Pending_Execute::Operations::PLAY, "background music");

    // game loop
    // while (!render.should_close())
    {
        // time
        time_manager.time_update();

        // controls
        controls.handle_input();

        // update sveta
        if (!time_manager.paused())
        {
            world.update_world_loop();
        }

        // render everything
        render.render_everything();

        // audio
        // mal by som kontrolovat activne sourcy a mazat ich
    }

    audio_manager.send_execute(Pending_Execute::Operations::STOP);
    // audio_manager.cleanup();
    render.cleanup();

    std::cout << "END\n";
    return 0;
}
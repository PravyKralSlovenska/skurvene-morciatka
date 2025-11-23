// Standartne cpp kniznice
#include <iostream>

// Moje main header files
#include "engine/renderer/renderer.hpp"
#include "engine/world/world.hpp"
#include "engine/world/herringbone_world_generation.hpp"
#include "engine/entity.hpp"
#include "engine/controls.hpp"
#include "engine/camera.hpp"
#include "engine/audio/audio_manager.hpp"
#include "engine/time_manager.hpp"

// namespace na globalne premenne
#include "others/GLOBALS.hpp"

Controls controls;
Audio_Manager audio_manager;
Time_Manager time_manager;
World world;
Player player("MISKO", {500.0f, 400.0f});
Camera camera(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT);
IRenderer render(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT);

// chcem pouzit na nejake dalsie renderovanie
// napriklad chcem vyrenderovat obrazovku ked je hra zastavena ci som len v menu
enum GAME_STATES
{
    MENU,
    GAME,
    PAUSE,
    OPTIONS, // ???
    LOADING, // ???
    END
};

int main()
{
    render.init();
    render.enable_blending();
    render.enable_ortho_projection();
    render.set_world(&world);
    render.set_time_manager(&time_manager);
    render.set_camera(&camera);
    
    time_manager.init();
    // time_manager.set_target_fps(5);
    // time_manager.enable_fps_limiting();

    controls.set_player(&player);
    controls.set_window(render.get_window());
    controls.set_world(&world);
    controls.set_audio_manager(&audio_manager);
    controls.set_time_manager(&time_manager);
    controls.set_camera(&camera);

    // world.entities.push_back(player);
    // world.set_time_manager(&time_manager);
    world.set_player(&player);

    // audio_manager.init();
    // audio_manager.set_player(&player);
    // audio_manager.set_time_manager(&time_manager);
    // audio_manager.send_execute(Pending_Execute::Operations::LOAD, "background music", "../music/menu/Rick Ross - Maybach Music III.mp3");
    // audio_manager.send_execute(Pending_Execute::Operations::PLAY, "background music");

    // game loop
    while (!render.should_close())
    {
        // vsetko by malo dostavat parameter delta_time
        // float delta_time = time_manager.get_delta();

        // time
        time_manager.time_update();

        // controls
        controls.handle_input();

        // update sveta
        if (!time_manager.paused())
        {
            world.update();
        }

        // camera update
        camera.follow_target(player.coords, 1);
        camera.update();
        
        // render everything
        render.render_everything();
    }

    // audio_manager.send_execute(Pending_Execute::Operations::STOP);
    // audio_manager.cleanup();
    render.cleanup();

    std::cout << "END\n";
    return 0;
}

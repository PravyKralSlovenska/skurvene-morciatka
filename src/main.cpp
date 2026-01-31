// Standartne cpp kniznice
#include <iostream>

// Moje main header files
#include "engine/renderer/renderer.hpp"
#include "engine/world/world.hpp"
#include "engine/world/herringbone_world_generation.hpp"
#include "engine/player/entity.hpp"
#include "engine/player/entity_manager.hpp"
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
Entity_Manager entity_manager;
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
    time_manager.init();
    // time_manager.set_target_fps(5);
    // time_manager.enable_fps_limiting();

    render.init();
    render.enable_blending();
    render.enable_ortho_projection();
    render.set_world(&world);
    render.set_time_manager(&time_manager);
    render.set_camera(&camera);
    render.set_entity_manager(&entity_manager);

    int window_width = 0, window_height = 0; // static ???
    GLFWwindow *glfw_window = render.get_window();
    glfwGetWindowSize(glfw_window, &window_width, &window_height);
    camera.set_window_dimensions(static_cast<float>(window_width), static_cast<float>(window_height));

    entity_manager.set_difficulty(1.5f);
    // entity_manager.set_spawn_interval(0.5f);
    // entity_manager.set_spawn_distance();
    // entity_manager.set_max_enemies(50);
    // entity_manager.set_spawn_enabled(false);
    Player *player = entity_manager.get_player();
    entity_manager.set_world(&world);

    controls.set_player(player);
    controls.set_window(render.get_window());
    controls.set_world(&world);
    controls.set_audio_manager(&audio_manager);
    controls.set_time_manager(&time_manager);
    controls.set_camera(&camera);
    controls.set_renderer(&render);

    // world.entities.push_back(player);
    // world.set_time_manager(&time_manager);
    world.set_player(player);

    // audio_manager.init();
    // audio_manager.set_player(&player);
    // audio_manager.set_time_manager(&time_manager);
    // audio_manager.send_execute(Pending_Execute::Operations::LOAD, "background music", "../music/menu/Rick Ross - Maybach Music III.mp3");
    // audio_manager.send_execute(Pending_Execute::Operations::PLAY, "background music");

    // game loop
    while (!render.should_close())
    {
        // vsetko by malo dostavat parameter delta_time
        float delta_time = static_cast<float>(time_manager.get_delta_time());

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
        camera.follow_target(player->coords, 1);
        camera.update();

        // update entities (enemies, NPCs, etc. - not the player)
        if (!time_manager.paused())
        {
            entity_manager.update(delta_time);
        }

        // render everything
        render.render_everything();
    }

    // audio_manager.send_execute(Pending_Execute::Operations::STOP);
    render.cleanup();

    std::cout << "END\n";
    return 0;
}

// Standartne cpp kniznice
#include <iostream>
// #include <functional>a

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
#include "others/utils.hpp"

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
    Log::init("../logs/runtime.log", true, true, false);
    Log::info("game start");

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
    entity_manager.ensure_player_valid_position();
    entity_manager.register_sprite("slime", "../sprites/devushka_slime_enemy1.png");
    // entity_manager.register_sprite("big_boss", "../sprites/boss.png", 256, 64, 64, 64, 4);

    const int devushki_count = 1; // how many devushki to save (change this to set the objective)
    entity_manager.set_devushki_objective_count(devushki_count);
    entity_manager.spawn_devushki_objective(devushki_count, 2000.0f);
    world.set_devushki_column_spawn_count(devushki_count);

    controls.set_player(player);
    controls.set_window(render.get_window());
    controls.set_world(&world);
    controls.set_audio_manager(&audio_manager);
    controls.set_time_manager(&time_manager);
    controls.set_camera(&camera);
    controls.set_renderer(&render);
    controls.set_entity_manager(&entity_manager);

    // world.entities.push_back(player);
    // world.set_time_manager(&time_manager);
    world.set_player(player);

    // Image structures are now loaded inside World::World() before predetermined positions are generated
    // world.load_image_structures("../structure_images");

    // audio_manager.init();
    // audio_manager.set_player(player);
    // audio_manager.set_time_manager(&time_manager);
    // audio_manager.send_execute(Pending_Execute::Operations::LOAD, "background music", "../music/menu/Rick Ross - Maybach Music III.mp3");
    // audio_manager.send_execute(Pending_Execute::Operations::PLAY, "background music");

    // game loop
    GAME_STATES game_state = MENU;
    GAME_STATES options_return_state = MENU;
    bool escape_was_down = false;
    bool enter_was_down = false;

    SpawnConfig spawn_cfg = entity_manager.get_spawn_config();
    float option_enemy_difficulty = spawn_cfg.difficulty_multiplier;
    float option_spawn_interval = spawn_cfg.spawn_interval;
    int option_max_enemies = spawn_cfg.max_enemies;
    bool option_spawn_enabled = spawn_cfg.spawn_enabled;

    time_manager.pause();

    while (!render.should_close())
    {
        // time
        time_manager.time_update();

        // vsetko by malo dostavat parameter delta_time
        float delta_time = static_cast<float>(time_manager.get_delta_time());

        const bool escape_down = glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        const bool enter_down = glfwGetKey(glfw_window, GLFW_KEY_ENTER) == GLFW_PRESS;
        const bool escape_pressed = escape_down && !escape_was_down;
        const bool enter_pressed = enter_down && !enter_was_down;

        bool render_world = true;
        bool render_in_game_ui = false;
        std::function<void()> overlay_ui = nullptr;
        Menu_Actions menu_actions;

        Menu_Screen menu_screen = Menu_Screen::NONE;
        Menu_Options_Model options_model = {
            option_enemy_difficulty,
            option_spawn_interval,
            option_max_enemies,
            option_spawn_enabled,
            render.get_fullscreen_state()};

        switch (game_state)
        {
        case MENU:
            time_manager.pause();
            render_in_game_ui = false;
            menu_screen = Menu_Screen::MENU;
            break;

        case GAME:
            render_in_game_ui = true;

            if (escape_pressed)
            {
                game_state = PAUSE;
                time_manager.pause();
            }
            else
            {
                controls.handle_input();

                // update sveta
                if (!time_manager.paused())
                {
                    world.update(delta_time);
                }

                // update entities (enemies, NPCs, etc. - not the player)
                if (!time_manager.paused())
                {
                    entity_manager.update(delta_time);
                }
            }
            break;

        case PAUSE:
            time_manager.pause();
            render_in_game_ui = false;
            menu_screen = Menu_Screen::PAUSE;
            break;

        case OPTIONS:
            time_manager.pause();
            render_in_game_ui = false;
            menu_screen = Menu_Screen::OPTIONS;
            break;

        case LOADING:
            time_manager.pause();
            menu_screen = Menu_Screen::LOADING;
            break;

        case END:
            menu_actions.quit_game = true;
            break;
        }

        if (menu_screen != Menu_Screen::NONE)
        {
            overlay_ui = [&]()
            {
                menu_actions = render.render_menu_screen(menu_screen, enter_pressed, escape_pressed, options_model);
            };
        }

        // camera update
        camera.follow_target(player->coords, 1);
        camera.update();

        // render frame
        render.render_everything(render_world, render_in_game_ui, overlay_ui);

        option_enemy_difficulty = options_model.enemy_difficulty;
        option_spawn_interval = options_model.spawn_interval;
        option_max_enemies = options_model.max_enemies;
        option_spawn_enabled = options_model.spawn_enabled;

        if (menu_actions.toggle_fullscreen)
        {
            render.toggle_fullscreen();
        }

        if (menu_actions.open_options)
        {
            options_return_state = game_state;
            game_state = OPTIONS;
        }

        if (menu_actions.start_game)
        {
            game_state = GAME;
            time_manager.resume();
        }

        if (menu_actions.resume_game)
        {
            game_state = GAME;
            time_manager.resume();
        }

        if (menu_actions.back_from_options)
        {
            game_state = options_return_state;
            if (game_state == GAME)
                time_manager.resume();
        }

        if (menu_actions.quit_to_menu)
        {
            game_state = MENU;
            time_manager.pause();
        }

        if (menu_actions.quit_game)
        {
            glfwSetWindowShouldClose(glfw_window, true);
        }

        escape_was_down = escape_down;
        enter_was_down = enter_down;
    }

    // audio_manager.send_execute(Pending_Execute::Operations::STOP);
    render.cleanup();

    Log::info("game end");
    Log::shutdown();

    return 0;
}

// Standartne cpp kniznice
#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <functional>

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
    NEW_GAME_SETUP,
    GAME,
    PAUSE,
    OPTIONS, // ???
    LOADING, // ???
    END,
    BOSS_DEFEATED,
    PLAYER_LOST
};

int main()
{
    Log::init("../logs/runtime.log", true, true, false);
    Log::info("game start");

    time_manager.init();
    // time_manager.set_target_fps(5);
    // time_manager.enable_fps_limiting();

    render.init();
    render.maximize_window(); // Auto-apply the same maximize action as F10 at startup

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
    entity_manager.set_world(&world);
    entity_manager.register_sprite("player", "sprites/devushka_player.png");
    entity_manager.register_sprite("slime", "sprites/devushka_slime_enemy1.png");
    entity_manager.register_sprite("big_boss", "sprites/devushka_boss.png");
    entity_manager.register_sprite("devushki_1", "sprites/devushki_devushka1.png");
    entity_manager.register_sprite("devushki_2", "sprites/devushki_devushka2.png");
    entity_manager.register_sprite("devushki_3", "sprites/devushki_devushka3.png");
    entity_manager.apply_sprite(entity_manager.get_player(), "player");

    SpawnConfig spawn_cfg = entity_manager.get_spawn_config();
    float option_enemy_difficulty = spawn_cfg.difficulty_multiplier;
    float option_spawn_interval = spawn_cfg.spawn_interval;
    int option_max_enemies = spawn_cfg.max_enemies;
    int option_column_spawn_radius = world.get_devushki_column_spawn_radius_particles();
    int option_column_spawn_count = 4;
    std::string option_world_seed_input;
    bool option_use_custom_seed = false;
    int option_custom_seed = 0;
    bool option_spawn_enabled = spawn_cfg.spawn_enabled;
    float option_audio_master_volume = 1.0f;
    float option_audio_player_died_volume = 0.90f;
    float option_audio_player_damaged_volume = 0.70f;
    float option_audio_gunshot_volume = 0.58f;
    float option_audio_flamethrower_volume = 0.27f;

    StructureSpawner::StoreSpawnConfig option_store_spawn_config = world.get_store_spawn_config();
    option_store_spawn_config.enabled = true;
    option_store_spawn_config.chunks_per_spawn = 90;
    option_store_spawn_config.min_generated_chunks_before_first_store = 40;
    option_store_spawn_config.min_spawn_radius_particles = 80;
    option_store_spawn_config.max_spawn_radius_particles = 900;
    option_store_spawn_config.min_distance_between_stores_particles = 320;
    option_store_spawn_config.min_distance_from_origin_particles = 150;

    entity_manager.set_devushki_objective_count(option_column_spawn_count);
    entity_manager.spawn_devushki_objective(option_column_spawn_count, 200.0f, "devushki");

    // Startup workflow:
    // 1) seed already chosen in World constructor
    // 2) suitable column targets found and remembered (not placed yet)
    // 3) columns are placed lazily only when nearby chunks load
    // 4) player spawn validated without forcing remote structure placement
    world.set_store_spawn_config(option_store_spawn_config);
    world.set_devushki_column_spawn_radius_particles(option_column_spawn_radius);
    world.set_devushki_column_spawn_count(option_column_spawn_count);

    Player *player = entity_manager.get_player();
    bool player_death_sound_played = false;
    entity_manager.ensure_player_valid_position();

    auto render_loading_frame = [&](float progress, const std::string &status)
    {
        render.set_loading_screen_state(progress, status);
        Menu_Options_Model loading_options;
        render.render_everything(false, false, [&]()
                                { render.render_menu_screen(Menu_Screen::LOADING, false, false, loading_options); });
    };

    auto rebuild_world_with_new_seed = [&](const std::optional<int> &requested_seed)
    {
        render_loading_frame(0.0f, "Preparing world rebuild...");

        if (requested_seed.has_value())
        {
            render_loading_frame(0.02f, "Applying selected seed...");
            world.regenerate_with_seed(*requested_seed);
        }
        else
        {
            render_loading_frame(0.02f, "Generating random seed...");
            world.regenerate_random_seed();
        }

        render_loading_frame(0.04f, "World seed ready.");

        render_loading_frame(0.06f, "Resetting entities...");

        entity_manager.reset_for_new_world();
        entity_manager.set_world(&world);

        render_loading_frame(0.08f, "Rebinding player systems...");

        Player *active_player = entity_manager.get_player();
        player = active_player;
        world.set_player(active_player);
        controls.set_player(active_player);
        audio_manager.set_player(active_player);
        player_death_sound_played = false;

        render_loading_frame(0.10f, "Spawning devushki objective...");

        entity_manager.set_devushki_objective_count(option_column_spawn_count);
        entity_manager.spawn_devushki_objective(option_column_spawn_count, 2000.0f, "devushki");

        render_loading_frame(0.12f, "Objective ready.");

        // Keep the same startup ordering for rebuilt worlds.
        render_loading_frame(0.14f, "Applying column spawn radius...");
        world.set_devushki_column_spawn_radius_particles(option_column_spawn_radius);

        render_loading_frame(0.15f, "Finding coordinates for columns...");
        world.set_devushki_column_spawn_count(
            option_column_spawn_count,
            [&](const std::string &status, float sub_progress)
            {
                const float mapped_progress = 0.15f + std::clamp(sub_progress, 0.0f, 1.0f) * 0.80f;
                render_loading_frame(mapped_progress, status);
            });

        render_loading_frame(0.95f, "Validating player spawn position...");
        entity_manager.ensure_player_valid_position();

        render_loading_frame(1.0f, "World ready. Entering game...");

        Log::info("new world seed: " + std::to_string(world.get_seed()));
    };

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

    audio_manager.init();
    audio_manager.set_player(player);
    audio_manager.set_time_manager(&time_manager);

    audio_manager.send_execute(Pending_Execute::LOAD, "sfx_player_died", resolve_asset_path("music/sounds/devushki_church_bell_player_died.mp3"));
    audio_manager.send_execute(Pending_Execute::LOAD, "sfx_player_damaged", resolve_asset_path("music/sounds/devushki_player_getting_damaged.mp3"));
    audio_manager.send_execute(Pending_Execute::LOAD, "sfx_gunshot", resolve_asset_path("music/sounds/devushki_desert_eagle_gunshot.mp3"));
    audio_manager.send_execute(Pending_Execute::LOAD, "sfx_flamethrower", resolve_asset_path("music/sounds/devushki_flamethrower.mp3"));

    auto apply_audio_volumes = [&]()
    {
        audio_manager.set_sound_gain("sfx_player_died", option_audio_master_volume * option_audio_player_died_volume);
        audio_manager.set_sound_gain("sfx_player_damaged", option_audio_master_volume * option_audio_player_damaged_volume);
        audio_manager.set_sound_gain("sfx_gunshot", option_audio_master_volume * option_audio_gunshot_volume);
        audio_manager.set_sound_gain("sfx_flamethrower", option_audio_master_volume * option_audio_flamethrower_volume);
    };

    apply_audio_volumes();

    // game loop
    GAME_STATES game_state = MENU;
    GAME_STATES options_return_state = MENU;
    bool escape_was_down = false;
    bool enter_was_down = false;
    bool boss_defeat_menu_shown = false;

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
            option_column_spawn_radius,
            option_column_spawn_count,
            option_world_seed_input,
            option_use_custom_seed,
            option_custom_seed,
            option_spawn_enabled,
            render.get_fullscreen_state(),
            option_audio_master_volume,
            option_audio_player_died_volume,
            option_audio_player_damaged_volume,
            option_audio_gunshot_volume,
            option_audio_flamethrower_volume};

        switch (game_state)
        {
        case MENU:
            time_manager.pause();
            render_in_game_ui = false;
            menu_screen = Menu_Screen::MENU;
            break;

        case NEW_GAME_SETUP:
            time_manager.pause();
            render_in_game_ui = false;
            menu_screen = Menu_Screen::NEW_GAME_SETUP;
            break;

        case GAME:
            if (!player || !player->get_is_alive())
            {
                if (player && !player_death_sound_played)
                {
                    audio_manager.send_execute(Pending_Execute::PLAY, "sfx_player_died");
                    player_death_sound_played = true;
                }

                game_state = PLAYER_LOST;
                time_manager.pause();
                render_in_game_ui = false;
                menu_screen = Menu_Screen::PLAYER_LOST;
                break;
            }

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
                    const float health_before_update = player->healthpoints;
                    entity_manager.update(delta_time);
                    const float health_after_update = player->healthpoints;

                    if (health_after_update < health_before_update && player->get_is_alive())
                    {
                        audio_manager.send_execute(Pending_Execute::PLAY, "sfx_player_damaged");
                    }

                    if (!player->get_is_alive())
                    {
                        if (!player_death_sound_played)
                        {
                            audio_manager.send_execute(Pending_Execute::PLAY, "sfx_player_died");
                            player_death_sound_played = true;
                        }

                        game_state = PLAYER_LOST;
                        time_manager.pause();
                        render_in_game_ui = false;
                        menu_screen = Menu_Screen::PLAYER_LOST;
                        break;
                    }

                    DevushkiObjective &objective = entity_manager.get_devushki_objective();
                    const bool boss_defeated = objective.boss_spawned && entity_manager.get_boss_count() == 0;
                    if (!boss_defeat_menu_shown && boss_defeated)
                    {
                        boss_defeat_menu_shown = true;
                        game_state = BOSS_DEFEATED;
                        time_manager.pause();
                    }
                }
            }
            break;

        case PAUSE:
            time_manager.pause();
            render_in_game_ui = false;
            menu_screen = Menu_Screen::PAUSE;
            break;

        case BOSS_DEFEATED:
            time_manager.pause();
            render_in_game_ui = false;
            menu_screen = Menu_Screen::BOSS_DEFEATED;
            break;

        case PLAYER_LOST:
            time_manager.pause();
            render_in_game_ui = false;
            menu_screen = Menu_Screen::PLAYER_LOST;
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
        option_column_spawn_radius = options_model.devushki_column_spawn_radius_particles;
        option_column_spawn_count = options_model.devushki_column_spawn_count;
        option_world_seed_input = options_model.world_seed_input;
        option_use_custom_seed = options_model.use_custom_seed;
        option_custom_seed = options_model.custom_seed;
        option_spawn_enabled = options_model.spawn_enabled;
        option_audio_master_volume = options_model.audio_master_volume;
        option_audio_player_died_volume = options_model.audio_player_died_volume;
        option_audio_player_damaged_volume = options_model.audio_player_damaged_volume;
        option_audio_gunshot_volume = options_model.audio_gunshot_volume;
        option_audio_flamethrower_volume = options_model.audio_flamethrower_volume;

        apply_audio_volumes();

        if (menu_actions.toggle_fullscreen)
        {
            render.toggle_fullscreen();
        }

        if (menu_actions.open_new_game_setup)
        {
            game_state = NEW_GAME_SETUP;
            time_manager.pause();
        }

        if (menu_actions.open_options)
        {
            options_return_state = game_state;
            game_state = OPTIONS;
        }

        if (menu_actions.start_game)
        {
            if (game_state == NEW_GAME_SETUP)
            {
                game_state = LOADING;
                time_manager.pause();
                render.set_loading_screen_state(0.0f, "Preparing world rebuild...");
                rebuild_world_with_new_seed(option_use_custom_seed ? std::optional<int>(option_custom_seed) : std::nullopt);
                boss_defeat_menu_shown = false;
            }
            if (player && player->get_is_alive())
            {
                player_death_sound_played = false;
            }
            game_state = GAME;
            time_manager.resume();
        }
        else if (menu_actions.resume_game)
        {
            if (player && player->get_is_alive())
            {
                player_death_sound_played = false;
            }
            game_state = GAME;
            time_manager.resume();
        }
        else if (menu_actions.create_new_world)
        {
            game_state = LOADING;
            time_manager.pause();
            render.set_loading_screen_state(0.0f, "Preparing world rebuild...");
            rebuild_world_with_new_seed(std::nullopt);
            boss_defeat_menu_shown = false;
            game_state = GAME;
            time_manager.resume();
        }
        else if (menu_actions.back_from_new_game_setup)
        {
            game_state = MENU;
            time_manager.pause();
        }
        else if (menu_actions.back_from_options)
        {
            game_state = options_return_state;
            if (game_state == GAME)
                time_manager.resume();
        }
        else if (menu_actions.quit_to_menu)
        {
            game_state = MENU;
            time_manager.pause();
        }
        else if (menu_actions.quit_game)
        {
            glfwSetWindowShouldClose(glfw_window, true);
        }

        escape_was_down = escape_down;
        enter_was_down = enter_down;
    }

    audio_manager.send_execute(Pending_Execute::STOP);
    render.cleanup();

    Log::info("game end");
    Log::shutdown();

    return 0;
}

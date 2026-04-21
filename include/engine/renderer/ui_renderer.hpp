#pragma once

// File purpose: Defines ImGui-based menus, HUD, and UI rendering logic.
#include "imgui/imgui.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <algorithm>

// forward declarations
class Player;
class Camera;
class World;
class Time_Manager;
class Entity_Manager;

enum class Menu_Screen
{
    NONE,
    MENU,
    NEW_GAME_SETUP,
    PAUSE,
    OPTIONS,
    LOADING,
    BOSS_DEFEATED,
    PLAYER_LOST
};

// Defines the Menu_Options_Model struct.
struct Menu_Options_Model
{
    float enemy_difficulty = 1.0f;
    float spawn_interval = 3.0f;
    int max_enemies = 20;
    int devushki_column_spawn_radius_particles = 500;
    int devushki_column_spawn_count = 4;
    std::string world_seed_input;
    bool use_custom_seed = false;
    int custom_seed = 0;
    bool spawn_enabled = true;
    bool fullscreen_enabled = false;
    float audio_master_volume = 1.0f;
    float audio_player_died_volume = 0.90f;
    float audio_player_damaged_volume = 0.70f;
    float audio_gunshot_volume = 0.58f;
    float audio_flamethrower_volume = 0.27f;
};

// Defines the Menu_Actions struct.
struct Menu_Actions
{
    bool start_game = false;
    bool open_new_game_setup = false;
    bool open_options = false;
    bool resume_game = false;
    bool back_from_options = false;
    bool back_from_new_game_setup = false;
    bool quit_to_menu = false;
    bool create_new_world = false;
    bool quit_game = false;
    bool toggle_fullscreen = false;
};

// Draws in-game and menu UI panels.
class UI_Renderer
{
private:
    Player *player = nullptr;
    Camera *camera = nullptr;
    World *world = nullptr;
    Time_Manager *time_manager = nullptr;
    Entity_Manager *entity_manager = nullptr;

    // UI state
    bool show_debug_info = false;
    bool show_hotbar = true;
    bool show_health_bar = true;
    bool show_minimap = true;
    bool show_fullscreen_map = false;
    double session_play_time_seconds = 0.0;

    float loading_progress = 0.0f;
    std::string loading_status = "Preparing loading...";
    float objective_panel_bottom_y = 0.0f;

    // Store offer icon textures used for world-space UI labels.
    std::unordered_map<std::string, unsigned int> store_offer_textures;
    bool store_offer_textures_loaded = false;

    // Fullscreen map dragging state
    bool map_dragging = false;
    glm::vec2 map_offset = {0.0f, 0.0f};
    glm::vec2 map_drag_start = {0.0f, 0.0f};
    float map_zoom = 3.0f;

    // Map rendering cache/throttling.
    std::unordered_map<std::uint64_t, ImU32> map_chunk_color_cache;
    double map_chunk_color_refresh_timer_seconds = 0.0;
    bool map_chunk_color_refresh_due = true;
    static constexpr double MAP_CHUNK_COLOR_REFRESH_INTERVAL_SECONDS = 1.0 / 8.0;

    // Centers next window.
    void center_next_window(float width, float height);
    // Renders main menu.
    void render_main_menu(Menu_Actions &actions);
    // Renders new game setup menu.
    void render_new_game_setup_menu(Menu_Actions &actions, Menu_Options_Model &options);
    // Renders pause menu.
    void render_pause_menu(Menu_Actions &actions);
    // Renders boss defeated menu.
    void render_boss_defeated_menu(Menu_Actions &actions);
    // Renders player lost menu.
    void render_player_lost_menu(Menu_Actions &actions);
    // Renders options menu.
    void render_options_menu(Menu_Actions &actions, Menu_Options_Model &options);
    // Renders loading screen.
    void render_loading_screen();
    // Ensures store offer textures loaded.
    bool ensure_store_offer_textures_loaded();
    // Loads ui texture.
    unsigned int load_ui_texture(const std::string &path);
    // Creates solid color texture.
    unsigned int create_solid_color_texture(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

public:
    // Constructs UI_Renderer.
    UI_Renderer() = default;
    // Destroys UI_Renderer and releases owned resources.
    ~UI_Renderer();

    // Sets player.
    void set_player(Player *player);
    // Sets camera.
    void set_camera(Camera *camera);
    // Sets world.
    void set_world(World *world);
    // Sets time manager.
    void set_time_manager(Time_Manager *time_manager);
    // Sets entity manager.
    void set_entity_manager(Entity_Manager *entity_manager);

    // main render call - call between imgui_new_frame and imgui_render
    void render_ui();

    // individual UI panels
    void render_debug_overlay();
    // Renders player status panel.
    void render_player_status_panel();
    // Renders health bar.
    void render_health_bar();
    // Renders boss health bar.
    void render_boss_health_bar();
    // Renders hotbar.
    void render_hotbar();
    // Renders minimap.
    void render_minimap();
    // Renders fullscreen map.
    void render_fullscreen_map();
    // Renders devushki objective.
    void render_devushki_objective();
    // Renders store offers.
    void render_store_offers();

    void set_loading_state(float progress, const std::string &status)
    {
        // Clamps.
        loading_progress = std::clamp(progress, 0.0f, 1.0f);
        loading_status = status;
    }

    // Main menu/pause/options overlay renderer used by game state machine.
    Menu_Actions render_menu_screen(Menu_Screen screen,
                                    bool enter_pressed,
                                    bool escape_pressed,
                                    Menu_Options_Model &options);

    // shared minimap drawing logic
    void draw_map_content(ImDrawList *draw_list, ImVec2 pos, float map_w, float map_h,
                          float center_chunk_x, float center_chunk_y, float view_radius, bool show_label);

    // toggles
    void toggle_debug_info() { show_debug_info = !show_debug_info; }
    // Toggles hotbar.
    void toggle_hotbar() { show_hotbar = !show_hotbar; }
    // Toggles health bar.
    void toggle_health_bar() { show_health_bar = !show_health_bar; }
    // Toggles minimap.
    void toggle_minimap() { show_minimap = !show_minimap; }
    // Toggles fullscreen map.
    void toggle_fullscreen_map();
    // Returns true if fullscreen map open.
    bool is_fullscreen_map_open() const { return show_fullscreen_map; }
};

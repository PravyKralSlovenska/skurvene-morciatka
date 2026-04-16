#pragma once

#include "imgui/imgui.h"
#include <glm/glm.hpp>
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

    void center_next_window(float width, float height);
    void render_main_menu(Menu_Actions &actions);
    void render_new_game_setup_menu(Menu_Actions &actions, Menu_Options_Model &options);
    void render_pause_menu(Menu_Actions &actions);
    void render_boss_defeated_menu(Menu_Actions &actions);
    void render_player_lost_menu(Menu_Actions &actions);
    void render_options_menu(Menu_Actions &actions, Menu_Options_Model &options);
    void render_loading_screen();
    bool ensure_store_offer_textures_loaded();
    unsigned int load_ui_texture(const std::string &path);
    unsigned int create_solid_color_texture(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

public:
    UI_Renderer() = default;
    ~UI_Renderer();

    void set_player(Player *player);
    void set_camera(Camera *camera);
    void set_world(World *world);
    void set_time_manager(Time_Manager *time_manager);
    void set_entity_manager(Entity_Manager *entity_manager);

    // main render call - call between imgui_new_frame and imgui_render
    void render_ui();

    // individual UI panels
    void render_debug_overlay();
    void render_player_status_panel();
    void render_health_bar();
    void render_boss_health_bar();
    void render_hotbar();
    void render_minimap();
    void render_fullscreen_map();
    void render_devushki_objective();
    void render_store_offers();

    void set_loading_state(float progress, const std::string &status)
    {
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
    void toggle_hotbar() { show_hotbar = !show_hotbar; }
    void toggle_health_bar() { show_health_bar = !show_health_bar; }
    void toggle_minimap() { show_minimap = !show_minimap; }
    void toggle_fullscreen_map();
    bool is_fullscreen_map_open() const { return show_fullscreen_map; }
};

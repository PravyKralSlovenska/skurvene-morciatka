#pragma once

#include "imgui/imgui.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

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
    PAUSE,
    OPTIONS,
    LOADING
};

struct Menu_Options_Model
{
    float enemy_difficulty = 1.0f;
    float spawn_interval = 3.0f;
    int max_enemies = 20;
    bool spawn_enabled = true;
    bool fullscreen_enabled = false;
};

struct Menu_Actions
{
    bool start_game = false;
    bool open_options = false;
    bool resume_game = false;
    bool back_from_options = false;
    bool quit_to_menu = false;
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
    bool show_debug_info = true;
    bool show_hotbar = true;
    bool show_health_bar = true;
    bool show_minimap = true;
    bool show_fullscreen_map = false;

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
    void render_pause_menu(Menu_Actions &actions);
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
    void render_health_bar();
    void render_hotbar();
    void render_minimap();
    void render_fullscreen_map();
    void render_devushki_objective();
    void render_store_offers();

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

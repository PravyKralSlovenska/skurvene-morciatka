#pragma once

#include "imgui/imgui.h"
#include <glm/glm.hpp>

// forward declarations
class Player;
class Camera;
class World;
class Time_Manager;
class Entity_Manager;

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

    // Fullscreen map dragging state
    bool map_dragging = false;
    glm::vec2 map_offset = {0.0f, 0.0f};
    glm::vec2 map_drag_start = {0.0f, 0.0f};
    float map_zoom = 3.0f;

public:
    UI_Renderer() = default;
    ~UI_Renderer() = default;

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

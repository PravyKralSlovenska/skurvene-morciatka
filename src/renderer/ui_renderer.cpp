#include "engine/renderer/ui_renderer.hpp"

#include "engine/player/entity.hpp"
#include "engine/player/entity_manager.hpp"
#include "engine/camera.hpp"
#include "engine/world/world.hpp"
#include "engine/world/world_chunk.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/time_manager.hpp"
#include "others/GLOBALS.hpp"

void UI_Renderer::set_player(Player *player) { this->player = player; }
void UI_Renderer::set_camera(Camera *camera) { this->camera = camera; }
void UI_Renderer::set_world(World *world) { this->world = world; }
void UI_Renderer::set_time_manager(Time_Manager *time_manager) { this->time_manager = time_manager; }
void UI_Renderer::set_entity_manager(Entity_Manager *entity_manager) { this->entity_manager = entity_manager; }

void UI_Renderer::render_ui()
{
    if (show_debug_info)
        render_debug_overlay();
    if (show_health_bar && player)
        render_health_bar();
    if (show_hotbar && player)
        render_hotbar();
    if (show_fullscreen_map && world && camera)
        render_fullscreen_map();
    else if (show_minimap && world && camera)
        render_minimap();
}

// ============================================================================
// DEBUG OVERLAY - top-left corner
// ============================================================================
void UI_Renderer::render_debug_overlay()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    const float padding = 10.0f;
    ImGui::SetNextWindowPos(ImVec2(padding, padding), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.6f);

    if (ImGui::Begin("##DebugOverlay", nullptr, flags))
    {
        // FPS / UPS
        if (time_manager)
        {
            int fps = time_manager->get_frames_per_second();
            int ups = time_manager->get_updates_per_second();

            // Color FPS based on performance
            ImVec4 fps_color;
            if (fps >= 60)
                fps_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // green
            else if (fps >= 30)
                fps_color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow
            else
                fps_color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // red

            ImGui::TextColored(fps_color, "%d FPS", fps);
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%d UPS", ups);

            if (time_manager->paused())
            {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "PAUSED");
            }
        }

        ImGui::Separator();

        // Camera info
        if (camera)
        {
            auto pos = camera->get_position();
            ImGui::Text("Pos: %.0f, %.0f", pos.x, pos.y);
            ImGui::Text("Zoom: %.2f", camera->get_zoom());
        }

        ImGui::Separator();

        // World info
        if (world)
        {
            ImGui::Text("Chunks: %d", world->get_chunks_size());
        }

        // Entity counts
        if (entity_manager)
        {
            ImGui::Text("Entities: %d", entity_manager->get_entity_count());
            ImGui::Text("  Enemies: %d", entity_manager->get_enemy_count());
            ImGui::Text("  NPCs: %d", entity_manager->get_devushki_count());
            ImGui::Text("  Bosses: %d", entity_manager->get_boss_count());
        }

        // Player info
        if (player)
        {
            ImGui::Separator();
            ImGui::Text("HP: %.0f / %.0f", player->healthpoints, player->max_healthpoints);
            ImGui::Text("Vel: %.1f, %.1f", player->velocity.x, player->velocity.y);
            ImGui::Text("On ground: %s", player->on_ground ? "yes" : "no");
            ImGui::Text("Noclip: %s", player->noclip ? "ON" : "off");
        }
    }
    ImGui::End();
}

// ============================================================================
// HEALTH BAR - bottom-center of screen
// ============================================================================
void UI_Renderer::render_health_bar()
{
    ImGuiIO &io = ImGui::GetIO();
    float screen_w = io.DisplaySize.x;
    float screen_h = io.DisplaySize.y;

    float bar_width = 300.0f;
    float bar_height = 28.0f;
    float bar_x = (screen_w - bar_width) * 0.5f;
    float bar_y = screen_h - 80.0f;

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowPos(ImVec2(bar_x, bar_y), ImGuiCond_Always);

    if (ImGui::Begin("##HealthBar", nullptr, flags))
    {
        float hp_ratio = player->healthpoints / player->max_healthpoints;
        if (hp_ratio < 0.0f)
            hp_ratio = 0.0f;
        if (hp_ratio > 1.0f)
            hp_ratio = 1.0f;

        // Health color: green -> yellow -> red
        ImVec4 hp_color;
        if (hp_ratio > 0.6f)
            hp_color = ImVec4(0.1f, 0.8f, 0.1f, 1.0f);
        else if (hp_ratio > 0.3f)
            hp_color = ImVec4(0.9f, 0.8f, 0.1f, 1.0f);
        else
            hp_color = ImVec4(0.9f, 0.1f, 0.1f, 1.0f);

        // Draw custom health bar
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        // Background
        draw_list->AddRectFilled(
            pos,
            ImVec2(pos.x + bar_width, pos.y + bar_height),
            IM_COL32(30, 30, 30, 200),
            4.0f);

        // Health fill
        draw_list->AddRectFilled(
            ImVec2(pos.x + 2, pos.y + 2),
            ImVec2(pos.x + 2 + (bar_width - 4) * hp_ratio, pos.y + bar_height - 2),
            ImGui::ColorConvertFloat4ToU32(hp_color),
            3.0f);

        // Border
        draw_list->AddRect(
            pos,
            ImVec2(pos.x + bar_width, pos.y + bar_height),
            IM_COL32(180, 180, 180, 200),
            4.0f,
            0,
            2.0f);

        // Text overlay
        char hp_text[64];
        snprintf(hp_text, sizeof(hp_text), "%.0f / %.0f", player->healthpoints, player->max_healthpoints);
        ImVec2 text_size = ImGui::CalcTextSize(hp_text);
        draw_list->AddText(
            ImVec2(pos.x + (bar_width - text_size.x) * 0.5f, pos.y + (bar_height - text_size.y) * 0.5f),
            IM_COL32(255, 255, 255, 255),
            hp_text);

        // Dummy to reserve space
        ImGui::Dummy(ImVec2(bar_width, bar_height));
    }
    ImGui::End();
}

// ============================================================================
// HOTBAR - bottom-center, above health bar
// ============================================================================
void UI_Renderer::render_hotbar()
{
    ImGuiIO &io = ImGui::GetIO();
    float screen_w = io.DisplaySize.x;
    float screen_h = io.DisplaySize.y;

    const float slot_size = 50.0f;
    const float slot_padding = 4.0f;
    const int hotbar_size = Hotbar::size();
    float total_width = hotbar_size * (slot_size + slot_padding) - slot_padding;

    float bar_x = (screen_w - total_width) * 0.5f;
    float bar_y = screen_h - 140.0f; // above health bar

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowPos(ImVec2(bar_x, bar_y), ImGuiCond_Always);

    if (ImGui::Begin("##Hotbar", nullptr, flags))
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        Hotbar &hotbar = player->get_hotbar();
        int selected = hotbar.get_selected_slot();

        for (int i = 0; i < hotbar_size; i++)
        {
            if (i > 0)
                ImGui::SameLine(0.0f, slot_padding);

            ImVec2 pos = ImGui::GetCursorScreenPos();
            Wand &wand = hotbar.get_wand(i);

            // Slot background
            ImU32 bg_color = (i == selected)
                                 ? IM_COL32(80, 80, 120, 220)
                                 : IM_COL32(40, 40, 40, 200);
            draw_list->AddRectFilled(
                pos,
                ImVec2(pos.x + slot_size, pos.y + slot_size),
                bg_color,
                4.0f);

            // Selected slot border highlight
            if (i == selected)
            {
                draw_list->AddRect(
                    pos,
                    ImVec2(pos.x + slot_size, pos.y + slot_size),
                    IM_COL32(255, 255, 100, 255),
                    4.0f,
                    0,
                    2.5f);
            }
            else
            {
                draw_list->AddRect(
                    pos,
                    ImVec2(pos.x + slot_size, pos.y + slot_size),
                    IM_COL32(100, 100, 100, 180),
                    4.0f);
            }

            // Wand color swatch (if not empty)
            if (!wand.is_empty())
            {
                float swatch_margin = 8.0f;
                ImU32 wand_color = IM_COL32(
                    (int)(wand.color.r * 255),
                    (int)(wand.color.g * 255),
                    (int)(wand.color.b * 255),
                    (int)(wand.color.a * 255));

                draw_list->AddRectFilled(
                    ImVec2(pos.x + swatch_margin, pos.y + swatch_margin),
                    ImVec2(pos.x + slot_size - swatch_margin, pos.y + slot_size - swatch_margin - 12.0f),
                    wand_color,
                    3.0f);

                // Wand name (abbreviated)
                const char *name = wand.name.c_str();
                ImVec2 text_size = ImGui::CalcTextSize(name);
                float text_x = pos.x + (slot_size - text_size.x) * 0.5f;
                float text_y = pos.y + slot_size - 14.0f;
                draw_list->AddText(
                    ImVec2(text_x, text_y),
                    IM_COL32(220, 220, 220, 255),
                    name);
            }

            // Slot number
            char slot_num[4];
            snprintf(slot_num, sizeof(slot_num), "%d", i + 1);
            draw_list->AddText(
                ImVec2(pos.x + 3.0f, pos.y + 1.0f),
                IM_COL32(180, 180, 180, 150),
                slot_num);

            // Invisible button for click interaction
            char btn_id[16];
            snprintf(btn_id, sizeof(btn_id), "##slot%d", i);
            ImGui::InvisibleButton(btn_id, ImVec2(slot_size, slot_size));

            if (ImGui::IsItemClicked())
            {
                player->select_hotbar_slot(i);
            }

            // Tooltip on hover
            if (ImGui::IsItemHovered() && !wand.is_empty())
            {
                ImGui::SetTooltip("%s\nBrush: %d", wand.name.c_str(), wand.brush_size);
            }
        }
    }
    ImGui::End();
}

// ============================================================================
// MINIMAP - top-right corner (local area around player)
// ============================================================================
void UI_Renderer::render_minimap()
{
    ImGuiIO &io = ImGui::GetIO();
    float screen_w = io.DisplaySize.x;

    const float map_size = 180.0f;
    const float padding = 10.0f;
    const float map_x = screen_w - map_size - padding;
    const float map_y = padding;

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowPos(ImVec2(map_x, map_y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(map_size + 4, map_size + 20), ImGuiCond_Always);

    if (ImGui::Begin("##Minimap", nullptr, flags))
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        // Background
        draw_list->AddRectFilled(
            pos,
            ImVec2(pos.x + map_size, pos.y + map_size),
            IM_COL32(15, 15, 25, 200),
            4.0f);

        // Border
        draw_list->AddRect(
            pos,
            ImVec2(pos.x + map_size, pos.y + map_size),
            IM_COL32(100, 100, 120, 200),
            4.0f, 0, 1.5f);

        // Center on player/camera, show local area (~view_radius chunks around)
        glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        if (chunk_dims.x == 0 || chunk_dims.y == 0)
        {
            ImGui::End();
            return;
        }

        glm::vec2 cam_pos = camera->get_position();
        float chunk_pixel_w = (float)chunk_dims.x * Globals::PARTICLE_SIZE;
        float chunk_pixel_h = (float)chunk_dims.y * Globals::PARTICLE_SIZE;
        float center_cx = cam_pos.x / chunk_pixel_w;
        float center_cy = cam_pos.y / chunk_pixel_h;
        float view_radius = 15.0f; // chunks around player

        draw_map_content(draw_list, pos, map_size, map_size, center_cx, center_cy, view_radius, true);

        ImGui::Dummy(ImVec2(map_size, map_size + 16));
    }
    ImGui::End();
}

// ============================================================================
// FULLSCREEN MAP - toggle with M, draggable
// ============================================================================
void UI_Renderer::toggle_fullscreen_map()
{
    show_fullscreen_map = !show_fullscreen_map;
    if (show_fullscreen_map)
    {
        // Reset offset to center on player
        map_offset = {0.0f, 0.0f};
        map_zoom = 4.0f;
    }
}

void UI_Renderer::render_fullscreen_map()
{
    ImGuiIO &io = ImGui::GetIO();
    float screen_w = io.DisplaySize.x;
    float screen_h = io.DisplaySize.y;

    float margin = 40.0f;
    float map_w = screen_w - margin * 2;
    float map_h = screen_h - margin * 2;

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(map_w + 4, map_h + 30), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);

    if (ImGui::Begin("##FullscreenMap", nullptr, flags))
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        // Background
        draw_list->AddRectFilled(
            pos,
            ImVec2(pos.x + map_w, pos.y + map_h),
            IM_COL32(10, 10, 20, 240),
            6.0f);

        // Border
        draw_list->AddRect(
            pos,
            ImVec2(pos.x + map_w, pos.y + map_h),
            IM_COL32(120, 120, 140, 220),
            6.0f, 0, 2.0f);

        // Handle mouse dragging inside the map
        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton("##MapDrag", ImVec2(map_w, map_h));

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            if (!map_dragging)
            {
                map_dragging = true;
                map_drag_start = map_offset;
            }
            map_offset.x = map_drag_start.x + delta.x / map_zoom;
            map_offset.y = map_drag_start.y + delta.y / map_zoom;
        }
        else
        {
            map_dragging = false;
        }

        // Mouse wheel zoom on map
        if (ImGui::IsItemHovered())
        {
            float wheel = io.MouseWheel;
            if (wheel != 0.0f)
            {
                map_zoom += wheel * 0.5f;
                if (map_zoom < 1.0f)
                    map_zoom = 1.0f;
                if (map_zoom > 15.0f)
                    map_zoom = 15.0f;
            }
        }

        // Center on player/camera + offset
        glm::ivec2 chunk_dims = world->get_chunk_dimensions();
        if (chunk_dims.x == 0 || chunk_dims.y == 0)
        {
            ImGui::End();
            return;
        }

        glm::vec2 cam_pos = camera->get_position();
        float chunk_pixel_w = (float)chunk_dims.x * Globals::PARTICLE_SIZE;
        float chunk_pixel_h = (float)chunk_dims.y * Globals::PARTICLE_SIZE;
        float center_cx = cam_pos.x / chunk_pixel_w - map_offset.x;
        float center_cy = cam_pos.y / chunk_pixel_h - map_offset.y;

        // View radius depends on map_zoom — smaller zoom = see more
        float base_w = map_w / map_zoom;
        float base_h = map_h / map_zoom;
        float view_radius = (base_w > base_h ? base_w : base_h) * 0.5f;

        draw_map_content(draw_list, pos, map_w, map_h, center_cx, center_cy, view_radius, false);

        // Title and controls hint
        draw_list->AddText(
            ImVec2(pos.x + 8, pos.y + map_h + 4),
            IM_COL32(180, 180, 180, 220),
            "MAP  |  Drag: move  |  Scroll: zoom  |  M: close");

        // Zoom indicator
        char zoom_text[32];
        snprintf(zoom_text, sizeof(zoom_text), "%.1fx", map_zoom);
        ImVec2 zt_size = ImGui::CalcTextSize(zoom_text);
        draw_list->AddText(
            ImVec2(pos.x + map_w - zt_size.x - 8, pos.y + map_h + 4),
            IM_COL32(160, 160, 160, 200),
            zoom_text);
    }
    ImGui::End();
}

// ============================================================================
// SHARED MAP DRAWING - used by both minimap and fullscreen map
// ============================================================================
void UI_Renderer::draw_map_content(ImDrawList *draw_list, ImVec2 pos, float map_w, float map_h,
                                   float center_chunk_x, float center_chunk_y, float view_radius, bool show_label)
{
    auto *chunks_map = world->get_chunks();
    glm::ivec2 chunk_dims = world->get_chunk_dimensions();
    float chunk_pixel_w = (float)chunk_dims.x * Globals::PARTICLE_SIZE;
    float chunk_pixel_h = (float)chunk_dims.y * Globals::PARTICLE_SIZE;
    glm::vec2 cam_pos = camera->get_position();
    ImGuiIO &io = ImGui::GetIO();

    if (!chunks_map || chunks_map->empty())
        return;

    // Visible chunk range
    int min_cx = (int)(center_chunk_x - view_radius) - 1;
    int max_cx = (int)(center_chunk_x + view_radius) + 1;
    int min_cy = (int)(center_chunk_y - view_radius) - 1;
    int max_cy = (int)(center_chunk_y + view_radius) + 1;

    int range_cx = max_cx - min_cx + 1;
    int range_cy = max_cy - min_cy + 1;
    if (range_cx <= 0 || range_cy <= 0)
        return;

    // Scale chunks to fit the map area
    float scale_x = map_w / (float)range_cx;
    float scale_y = map_h / (float)range_cy;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;

    float content_w = range_cx * scale;
    float content_h = range_cy * scale;
    float offset_x = pos.x + (map_w - content_w) * 0.5f;
    float offset_y = pos.y + (map_h - content_h) * 0.5f;

    draw_list->PushClipRect(pos, ImVec2(pos.x + map_w, pos.y + map_h), true);

    // Draw chunks
    for (auto &[coord, chunk] : *chunks_map)
    {
        if (!chunk)
            continue;

        // Skip chunks outside visible range
        if (coord.x < min_cx || coord.x > max_cx || coord.y < min_cy || coord.y > max_cy)
            continue;

        float rx = (float)(coord.x - min_cx) * scale + offset_x;
        float ry = (float)(coord.y - min_cy) * scale + offset_y;

        // Sample particles for average color
        int sample_count = 0;
        float avg_r = 0, avg_g = 0, avg_b = 0;
        int cw = chunk->width;
        int ch = chunk->height;

        int step = 1;
        if (cw > 5)
            step = cw / 5;

        for (int sy = 0; sy < ch; sy += step)
        {
            for (int sx = 0; sx < cw; sx += step)
            {
                WorldCell *cell = chunk->get_worldcell(sx, sy);
                if (cell && cell->particle.type != Particle_Type::EMPTY)
                {
                    avg_r += cell->particle.color.r;
                    avg_g += cell->particle.color.g;
                    avg_b += cell->particle.color.b;
                    sample_count++;
                }
            }
        }

        ImU32 chunk_color;
        if (sample_count > 0)
        {
            avg_r /= sample_count;
            avg_g /= sample_count;
            avg_b /= sample_count;
            chunk_color = IM_COL32(
                (int)(avg_r * 255),
                (int)(avg_g * 255),
                (int)(avg_b * 255),
                220);
        }
        else
        {
            chunk_color = IM_COL32(20, 20, 35, 100);
        }

        float block_size = scale > 1.0f ? scale : 1.0f;
        draw_list->AddRectFilled(
            ImVec2(rx, ry),
            ImVec2(rx + block_size, ry + block_size),
            chunk_color);
    }

    // Player indicator
    if (player)
    {
        float player_cx = (float)player->coords.x / chunk_pixel_w;
        float player_cy = (float)player->coords.y / chunk_pixel_h;

        float px = (player_cx - min_cx) * scale + offset_x;
        float py = (player_cy - min_cy) * scale + offset_y;

        float dot_size = show_label ? 4.0f : 6.0f;
        draw_list->AddCircleFilled(ImVec2(px, py), dot_size, IM_COL32(255, 255, 255, 255));
        draw_list->AddCircle(ImVec2(px, py), dot_size, IM_COL32(0, 0, 0, 200), 0, 1.5f);
    }

    // Camera viewport rectangle
    float zoom = camera->get_zoom();
    float view_w_chunks = (io.DisplaySize.x / zoom) / chunk_pixel_w;
    float view_h_chunks = (io.DisplaySize.y / zoom) / chunk_pixel_h;

    float cam_chunk_x = cam_pos.x / chunk_pixel_w;
    float cam_chunk_y = cam_pos.y / chunk_pixel_h;

    float vx1 = (cam_chunk_x - view_w_chunks * 0.5f - min_cx) * scale + offset_x;
    float vy1 = (cam_chunk_y - view_h_chunks * 0.5f - min_cy) * scale + offset_y;
    float vx2 = vx1 + view_w_chunks * scale;
    float vy2 = vy1 + view_h_chunks * scale;

    draw_list->AddRect(
        ImVec2(vx1, vy1),
        ImVec2(vx2, vy2),
        IM_COL32(255, 255, 100, 180),
        0.0f, 0, 1.5f);

    draw_list->PopClipRect();

    if (show_label)
    {
        draw_list->AddText(
            ImVec2(pos.x + 4, pos.y + map_h + 2),
            IM_COL32(160, 160, 160, 200),
            "MINIMAP");
    }
}

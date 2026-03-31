#include "engine/controls.hpp"

#include <iostream>
#include <algorithm>
#include <array>
#include <cmath>
#include <random>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "engine/world/world.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/time_manager.hpp"
#include "engine/player/entity.hpp"
#include "engine/player/entity_manager.hpp"
#include "engine/player/wand.hpp"
#include "engine/camera.hpp"
#include "engine/audio/audio_manager.hpp"

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    // TODO
    // if (-15.0 < zoom && zoom < 15.0)

    float offset = 0.1;
    if (yoffset < 0)
    {
        zoom = std::abs(zoom -= offset);
    }
    else
    {
        zoom += offset;
    }
}

Controls::Controls() {}

void Controls::set_player(Player *player)
{
    this->player = player;
}

void Controls::set_window(GLFWwindow *window)
{
    this->window = window;

    // pretoze je to set funkcia tak ju dam do konstruktora
    // keby bola niekde inde bolo by to furt ze nastavujem tu funkciu
    // tiez je to tu lebo window je sucast toho
    glfwSetScrollCallback(window, scroll_callback);
}

void Controls::set_world(World *world)
{
    this->world = world;
}

void Controls::set_time_manager(Time_Manager *time_manager)
{
    this->time_manager = time_manager;
}

void Controls::set_audio_manager(Audio_Manager *audio_manager)
{
    this->audio_manager = audio_manager;
}

void Controls::set_camera(Camera *camera)
{
    this->camera = camera;
}

void Controls::set_renderer(class IRenderer *renderer)
{
    this->renderer = renderer;
}

void Controls::set_entity_manager(Entity_Manager *entity_manager)
{
    this->entity_manager = entity_manager;
}

void Controls::handle_input()
{
    if (!window || !player || !camera)
    {
        return;
    }

    camera->set_zoom(zoom);

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    cursor_position = {xpos, ypos};

    // Convert screen cursor to world coordinates
    glm::vec2 cam_pos = camera->get_position();
    float zoom_factor = camera->get_zoom();
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    glm::vec2 cursor_world;
    cursor_world.x = (xpos - window_width / 2.0f) / zoom_factor + cam_pos.x;
    cursor_world.y = (ypos - window_height / 2.0f) / zoom_factor + cam_pos.y;

    // Update player's cursor position for wand aiming
    player->set_cursor_world_pos(cursor_world);

    // Calculate aim direction
    glm::vec2 player_center = player->get_center();
    glm::vec2 aim_dir = cursor_world - player_center;
    if (glm::length(aim_dir) > 0.001f)
    {
        player->set_aim_direction(aim_dir);
    }

    // Handle wand usage with left mouse button
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        Wand &wand = player->get_current_wand();

        if (!wand.is_empty())
        {
            if (wand.type == Wand_Type::GUN_WAND)
            {
                const double now = glfwGetTime();
                if (entity_manager && now - wand.last_use_time >= wand.cooldown)
                {
                    if (entity_manager->try_consume_ammo_for_shot())
                    {
                        glm::vec2 dir = player->get_aim_direction();
                        float len = glm::length(dir);
                        if (len > 0.001f)
                        {
                            dir /= len;
                            const float projectile_speed = 900.0f;
                            const float projectile_damage = 25.0f;
                            glm::vec2 spawn_pos = player->get_center() + dir * 20.0f;
                            entity_manager->create_projectile(spawn_pos, dir * projectile_speed,
                                                              wand.particle_type, projectile_damage,
                                                              Entity_Type::PLAYER);
                            wand.last_use_time = static_cast<float>(now);
                        }
                    }
                }
            }
            else if (wand.type == Wand_Type::COMPASS_WAND)
            {
                const double now = glfwGetTime();
                if (entity_manager && now - wand.last_use_time >= wand.cooldown)
                {
                    glm::vec2 dir = player->get_aim_direction();

                    glm::ivec2 nearest_devushki_pos(0, 0);
                    if (entity_manager->get_nearest_devushki_position(nearest_devushki_pos, nullptr))
                    {
                        const glm::vec2 toward = glm::vec2(nearest_devushki_pos) - player->get_center();
                        const float toward_len = glm::length(toward);
                        if (toward_len > 0.001f)
                        {
                            dir = toward / toward_len;
                        }
                    }

                    const float len = glm::length(dir);
                    if (len > 0.001f)
                    {
                        dir /= len;

                        // "General direction" behavior: accurate guidance with slight random spread.
                        static thread_local std::mt19937 rng(std::random_device{}());
                        std::uniform_real_distribution<float> spread_angle(-0.24f, 0.24f);
                        const float angle = spread_angle(rng);
                        const float cos_a = std::cos(angle);
                        const float sin_a = std::sin(angle);
                        glm::vec2 launch_dir(
                            dir.x * cos_a - dir.y * sin_a,
                            dir.x * sin_a + dir.y * cos_a);

                        const float projectile_speed = 520.0f;
                        glm::vec2 spawn_pos = player->get_center() + launch_dir * 20.0f;
                        Projectile *compass_particle = entity_manager->create_projectile(
                            spawn_pos,
                            launch_dir * projectile_speed,
                            Particle_Type::SAND,
                            0.0f,
                            Entity_Type::PLAYER);

                        if (compass_particle)
                        {
                            compass_particle->set_hitbox_dimensions(44, 22);
                            compass_particle->set_lifetime(1.65f);
                            compass_particle->set_gravity_multiplier(0.0f);
                            compass_particle->set_air_drag(0.998f);
                            compass_particle->set_world_impact_enabled(false);
                        }

                        wand.last_use_time = static_cast<float>(now);
                    }
                }
            }
            else if (wand.type == Wand_Type::FIRE_WAND)
            {
                const double now = glfwGetTime();
                if (entity_manager && now - wand.last_use_time >= wand.cooldown)
                {
                    glm::vec2 dir = player->get_aim_direction();
                    const float len = glm::length(dir);

                    if (len > 0.001f)
                    {
                        dir /= len;
                        const glm::vec2 origin = player->get_center() + dir * 20.0f;

                        static thread_local std::mt19937 rng(std::random_device{}());
                        std::uniform_real_distribution<float> spread_angle(-0.18f, 0.18f);
                        std::uniform_real_distribution<float> speed_jitter(-80.0f, 80.0f);

                        // Gun-inspired firing model: multiple fast pellets from muzzle with spread.
                        static constexpr int FLAME_PELLETS = 6;
                        static constexpr float BASE_SPEED = 820.0f;

                        for (int i = 0; i < FLAME_PELLETS; ++i)
                        {
                            const float angle = spread_angle(rng);
                            const float cos_a = std::cos(angle);
                            const float sin_a = std::sin(angle);
                            glm::vec2 shot_dir(
                                dir.x * cos_a - dir.y * sin_a,
                                dir.x * sin_a + dir.y * cos_a);

                            const float speed = BASE_SPEED + speed_jitter(rng);
                            Projectile *flame_projectile = entity_manager->create_projectile(
                                origin,
                                shot_dir * speed,
                                Particle_Type::FIRE,
                                8.0f,
                                Entity_Type::PLAYER);

                            if (flame_projectile)
                            {
                                flame_projectile->set_lifetime(0.28f);
                                flame_projectile->set_gravity_multiplier(-0.18f);
                                flame_projectile->set_air_drag(0.94f);
                            }
                        }
                    }

                    wand.last_use_time = static_cast<float>(now);
                }
            }
            else
            {
                glm::ivec2 target_pos = glm::ivec2(cursor_world);
                const int brush = std::max(1, wand.brush_size);
                const int particle_size = std::max(1, static_cast<int>(Globals::PARTICLE_SIZE));

                // Place particles in a square brush area
                for (int dx = -brush / 2; dx <= brush / 2; dx++)
                {
                    for (int dy = -brush / 2; dy <= brush / 2; dy++)
                    {
                        glm::ivec2 pos = target_pos + glm::ivec2(dx * particle_size, dy * particle_size);

                        if (wand.type == Wand_Type::DELETE_WAND)
                        {
                            // Delete wand removes any particle
                            world->place_particle(pos, Particle_Type::EMPTY);
                        }
                        else
                        {
                            // Other wands place their particle type
                            world->place_particle(pos, wand.particle_type);
                        }
                    }
                }
            }
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        // Right click could be secondary action (e.g., larger brush, different mode)
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_4) == GLFW_PRESS)
    {
        // int x = (int)xpos / world->scale;
        // int y = (int)ypos / world->scale;

        // world->add_particle({x, y}, Particle_Type::EMPTY, 3);

        // std::cout << "CLICK STREDNE TLACIDLO\n";
    }

    keyboard_input();
}

void Controls::keyboard_input()
{
    auto key_just_pressed = [this](int key) -> bool
    {
        static std::array<bool, GLFW_KEY_LAST + 1> key_was_down = {};

        if (key < 0 || key > GLFW_KEY_LAST)
        {
            return false;
        }

        const bool is_down = glfwGetKey(window, key) == GLFW_PRESS;
        const bool just_pressed = is_down && !key_was_down[key];
        key_was_down[key] = is_down;
        return just_pressed;
    };

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        // std::cout << "W\n";
        player->move_up();
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        player->move_down();
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        player->move_left();
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        player->move_right();
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        // world->clear_world_curr();
    }

    // Hotbar selection (1-9 keys)
    static bool number_key_pressed[9] = {false};

    for (int i = 0; i < 9; i++)
    {
        int key = GLFW_KEY_1 + i; // GLFW_KEY_1, GLFW_KEY_2, ... GLFW_KEY_9
        if (glfwGetKey(window, key) == GLFW_PRESS)
        {
            if (!number_key_pressed[i])
            {
                player->select_hotbar_slot(i);
                Wand &wand = player->get_current_wand();
                // std::cout << "Selected slot " << (i + 1) << ": " << wand.name << std::endl;
                number_key_pressed[i] = true;
            }
        }
        else
        {
            number_key_pressed[i] = false;
        }
    }

    // if pressed the world update loop will stop
    if (key_just_pressed(GLFW_KEY_P))
    {
        time_manager->pause();
    }

    if (key_just_pressed(GLFW_KEY_O))
    {
        time_manager->resume();
    }

    if (key_just_pressed(GLFW_KEY_I))
    {
        audio_manager->send_execute(Pending_Execute::PLAY, "background music");
    }

    if (key_just_pressed(GLFW_KEY_U))
    {
        // audio_manager->send_execute(Pending_Execute::STOP, "background music");
    }

    if (key_just_pressed(GLFW_KEY_F11))
    {
        if (renderer)
        {
            renderer->toggle_fullscreen();
        }
    }

    if (key_just_pressed(GLFW_KEY_F10))
    {
        if (renderer)
        {
            renderer->maximize_window();
        }
    }

    // TAB - Toggle noclip mode (allows player to move through solid terrain)
    if (key_just_pressed(GLFW_KEY_TAB))
    {
        player->toggle_noclip();
        std::cout << "Noclip: " << (player->get_noclip() ? "ON" : "OFF") << '\n';
    }

    // M - Toggle fullscreen map
    if (key_just_pressed(GLFW_KEY_M))
    {
        if (renderer)
            renderer->toggle_fullscreen_map();
    }

    // E - Interact with nearby store (buy healing for coins)
    if (key_just_pressed(GLFW_KEY_E) && entity_manager)
    {
        if (!entity_manager->is_player_near_store())
        {
            std::cout << "No store nearby.\n";
        }
        else if (entity_manager->try_buy_store_item())
        {
            std::cout << "Purchased store item.\n";
        }
        else
        {
            std::cout << "Cannot buy store item: not enough coins or no free wand slot.\n";
        }
    }
}

void Controls::left_mouse_click(double xpos, double ypos)
{
}

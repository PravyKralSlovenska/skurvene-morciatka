#pragma once
// File purpose: Defines camera movement, zoom, and world-to-screen transforms.
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
 * pohybovat budem hraca a kamera ho bude len prensaldevoat
 */
// Handles camera position, zoom, and view/projection matrices.
class Camera
{
private:
    glm::vec2 camera_position;

    float window_width, window_height;

    float old_zoom = 1.0f;
    float zoom = 1.0f;
    float rotate = 0.0f;

    glm::vec2 target_pos;

    /*
     * View Matrix
     * - je to priestor, ktory kamera vnima
     */
    glm::mat4 view_matrix;

    /*
     * Projection Matrix
     * -
     */
    glm::mat4 projection_matrix;

    bool needs_update;

public:
    // Constructs Camera.
    Camera(float window_width, float window_height);
    // Destroys Camera and releases owned resources.
    ~Camera() = default;

    // Sets zoom.
    void set_zoom(const float zoom);
    // Zoom by.
    void zoom_by(const float zoom); // mozes vlozit zaporne aj kladne hodnoty
    // Zoom in.
    void zoom_in(const float zoom);
    // Zoom out.
    void zoom_out(const float zoom);

    // Sets rotate.
    void set_rotate(const float rotate);
    // Rotate camera.
    void rotate_camera(const float degrees);

    // Sets position.
    void set_position(glm::vec2 pos);

    // Moves by.
    void move_by(float dx, float dy);
    // Moves to.
    void move_to(glm::vec2 pos);

    // Follows target.
    void follow_target(const glm::vec2 &target_pos, float speed);

    // Updates state.
    void update();

    // Sets window dimensions.
    void set_window_dimensions(float width, float height);

    // Returns position.
    glm::vec2 get_position();
    // Returns rotate.
    float get_rotate();
    // Returns zoom.
    float get_zoom();

    // Returns view matrix.
    glm::mat4 get_view_matrix();
    // Returns projection matrix.
    glm::mat4 get_projection_matrix();
    // Returns view projection matrix.
    glm::mat4 get_view_projection_matrix();

    // Screen to world.
    glm::ivec2 screen_to_world(float screen_x, float screen_y);
    // Converts world to screen.
    glm::vec2 world_to_screen(float world_x, float world_y);
};

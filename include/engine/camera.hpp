#pragma once
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


/*
 * pohybovat budem hraca a kamera ho bude len prensaldevoat
 */

class Camera
{
private:
    glm::vec2 camera_position;

    float window_width, window_height;

    float zoom = 1.0f;
    float rotate = 0.0f;

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;

    bool needs_update;

public:
    Camera(float window_width, float window_height);
    ~Camera() = default;

    void set_zoom(float zoom);
    void zoom_in(float zoom);
    void zoom_out(float zoom);

    void set_rotate(float rotate);
    void rotate_camera(float degrees);

    void set_position(glm::vec2 pos);

    void move_by(float dx, float dy);
    void move_to(glm::vec2 pos);

    void follow_target(glm::vec2 target_pos, float speed);

    void update();

    glm::vec2 get_position();
    float get_rotate();
    float get_zoom();

    glm::mat4 get_view_matrix();
    glm::mat4 get_projection_matrix();
    glm::mat4 get_view_projection_matrix();

    glm::vec2 screen_to_world(float screen_x, float screen_y);
    glm::vec2 world_to_screen(float world_x, float world_y);
};

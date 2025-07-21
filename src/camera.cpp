#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/camera.hpp"

Camera::Camera(float window_width, float window_height)
    : window_width(window_width), window_height(window_height)
{
    projection_matrix = glm::ortho(0.0f, window_width, window_height, 0.0f);
}

void Camera::set_position(glm::vec2 pos)
{
    camera_position = pos;
    needs_update = true;
}

void Camera::move_to(glm::vec2 pos)
{
    camera_position = pos;
    needs_update = true;
}

void Camera::set_rotate(float degrees)
{
    rotate = degrees;
    needs_update = true;
}

void Camera::rotate_camera(float degrees)
{
    rotate += degrees;
    needs_update = true;
}

void Camera::set_zoom(float zoom)
{
    this->zoom = zoom;
    needs_update = true;
}

void Camera::zoom_in(float zoom)
{

}

void Camera::zoom_out(float zoom)
{

}

glm::vec2 Camera::get_position()
{
    return camera_position;
}

float Camera::get_rotate()
{
    return rotate;
}

float Camera::get_zoom()
{
    return zoom;
}

glm::mat4 Camera::get_view_matrix()
{
    return view_matrix;
}

glm::mat4 Camera::get_projection_matrix()
{
    return projection_matrix;
}

glm::mat4 Camera::get_view_projection_matrix()
{
    return projection_matrix * get_view_matrix();
}

void Camera::move_by(float dx, float dy)
{
    camera_position.x += dx;
    camera_position.y += dy;
    needs_update = true;
}

void Camera::update()
{
    if (!needs_update) return;

    view_matrix = glm::mat4(1.0f);
    
    // 1. Translate to center of screen
    view_matrix = glm::translate(view_matrix, glm::vec3(window_width / 2.0f, window_height / 2.0f, 0.0f));
    
    // 2. Apply zoom
    view_matrix = glm::scale(view_matrix, glm::vec3(zoom, zoom, 1.0f));
    
    // 3. Apply rotation
    view_matrix = glm::rotate(view_matrix, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // 4. Translate by camera position (inverted)
    view_matrix = glm::translate(view_matrix, glm::vec3(-camera_position.x, -camera_position.y, 0.0f));
    
    needs_update = false;
}

glm::vec2 Camera::screen_to_world(float screen_x, float screen_y)
{
    glm::vec4 screen_pos(screen_x, screen_y, 0.0f, 1.0f);
    glm::mat4 inverse_view_proj = glm::inverse(get_view_projection_matrix());
    glm::vec4 world_pos = inverse_view_proj * screen_pos;
    
    return glm::vec2(world_pos.x, world_pos.y);
}

glm::vec2 Camera::world_to_screen(float world_x, float world_y)
{
    glm::vec4 world_pos(world_x, world_y, 0.0f, 1.0f);
    glm::vec4 screen_pos = get_view_projection_matrix() * world_pos;
    
    return glm::vec2(screen_pos.x, screen_pos.y);
}

void Camera::follow_target(glm::vec2 target_pos, float speed)
{
    camera_position = glm::mix(camera_position, target_pos, speed);
    needs_update = true;
}


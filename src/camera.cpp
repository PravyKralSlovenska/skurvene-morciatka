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

void Camera::set_rotate(const float degrees)
{
    rotate = degrees;
    needs_update = true;
}

void Camera::rotate_camera(const float degrees)
{
    rotate += degrees;
    needs_update = true;
}

void Camera::set_zoom(const float zoom)
{
    if (this->old_zoom != zoom)
    {
        this->old_zoom = this->zoom;
        this->zoom = zoom;
        needs_update = true;
    }
}

void Camera::zoom_in(const float zoom)
{
}

void Camera::zoom_out(const float zoom)
{
}

void Camera::zoom_by(const float zoom)
{
    this->zoom += zoom;
    needs_update = true;
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
    if (!needs_update)
        return;

    view_matrix = glm::mat4(1.0f);

    view_matrix = glm::translate(view_matrix, glm::vec3(window_width / 2.0f, window_height / 2.0f, 0.0f));

    view_matrix = glm::scale(view_matrix, glm::vec3(zoom, zoom, 1.0f));

    view_matrix = glm::rotate(view_matrix, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));

    view_matrix = glm::translate(view_matrix, glm::vec3(-camera_position.x, -camera_position.y, 0.0f));

    needs_update = false;
}

glm::ivec2 Camera::screen_to_world(float screen_x, float screen_y)
{
    float ndc_x = (2.0f * screen_x) / window_width - 1.0f;
    float ndc_y = (2.0f * screen_y) / window_height - 1.0f;
    
    glm::vec4 ndc_pos(ndc_x, ndc_y, 0.0f, 1.0f);
    
    glm::mat4 inverse_view_proj = glm::inverse(get_view_projection_matrix());
    glm::vec4 world_pos = inverse_view_proj * ndc_pos;
    
    return glm::ivec2(world_pos.x, world_pos.y);
}

glm::vec2 Camera::world_to_screen(float world_x, float world_y)
{
    glm::vec4 world_pos(world_x, world_y, 0.0f, 1.0f);
    glm::vec4 clip_pos = get_view_projection_matrix() * world_pos;
    
    glm::vec3 ndc = glm::vec3(clip_pos) / clip_pos.w;
    
    float screen_x = (ndc.x + 1.0f) * 0.5f * window_width;
    float screen_y = (ndc.y + 1.0f) * 0.5f * window_height;
    
    return glm::vec2(screen_x, screen_y);
}

void Camera::follow_target(const glm::vec2 &target_pos, float speed)
{
    if (this->target_pos == target_pos)
        return;

    this->target_pos = target_pos;

    camera_position = glm::mix(camera_position, target_pos, speed);
    needs_update = true;
}

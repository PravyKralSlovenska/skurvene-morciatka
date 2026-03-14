#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

// Represents a single frame in a sprite sheet
struct Animation_Frame
{
    glm::ivec2 position; // top-left position in sprite sheet (pixels)
    glm::ivec2 size;     // size of the frame (pixels)

    // UV coordinates for rendering (normalized 0-1)
    glm::vec2 uv_min = {0.0f, 0.0f};
    glm::vec2 uv_max = {1.0f, 1.0f};
};

// Sprite states for entities (maps to sprite sheet columns)
enum class Sprite_State
{
    FACING_LEFT = 0,
    FACING_RIGHT = 1,
    JUMPING = 2, // also used for falling
    HURT = 3     // also used for dying
};

// Manages a sprite sheet with multiple frames/states
class Sprite_Animation
{
private:
    std::vector<Animation_Frame> frames;
    int current_frame = 0;

    // Sprite sheet info
    std::string sprite_path;
    glm::ivec2 sheet_size = {0, 0};   // total size of sprite sheet
    glm::ivec2 frame_size = {32, 32}; // size of each frame
    int frame_count = 1;

    // Animation timing
    float frame_duration = 0.1f; // seconds per frame
    float time_accumulator = 0.0f;

    bool loop = true;
    bool playing = false;

    Sprite_State current_state = Sprite_State::FACING_RIGHT;

public:
    Sprite_Animation() = default;

    // Setup sprite sheet
    void setup_sheet(const std::string &path, int sheet_width, int sheet_height,
                     int frame_width, int frame_height, int num_frames);
    void setup_sheet(const std::string &path, glm::ivec2 sheet_size,
                     glm::ivec2 frame_size, int num_frames);

    // Add individual frame
    void add_frame(const glm::ivec2 &position, const glm::ivec2 &size);
    void add_frame(int x, int y, int width, int height);

    // Animation control
    void update(float delta_time);
    void play();
    void stop();
    void reset();
    void set_looping(bool should_loop);
    void set_frame_duration(float duration);

    // State management
    void set_state(Sprite_State state);
    Sprite_State get_state() const;

    // Get current frame info
    Animation_Frame get_frame() const;
    Animation_Frame get_frame_for_state(Sprite_State state) const;
    int get_current_frame_index() const;

    // Get UV coordinates for current frame (normalized 0-1)
    glm::vec2 get_uv_min() const;
    glm::vec2 get_uv_max() const;

    // Getters
    const std::string &get_sprite_path() const;
    glm::ivec2 get_frame_size() const;
    glm::ivec2 get_sheet_size() const;
    int get_frame_count() const;
    bool is_playing() const;
};

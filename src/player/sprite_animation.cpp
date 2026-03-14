#include "engine/player/sprite_animation.hpp"

void Sprite_Animation::setup_sheet(const std::string &path, int sheet_width, int sheet_height,
                                   int frame_width, int frame_height, int num_frames)
{
    setup_sheet(path, {sheet_width, sheet_height}, {frame_width, frame_height}, num_frames);
}

void Sprite_Animation::setup_sheet(const std::string &path, glm::ivec2 sheet_sz,
                                   glm::ivec2 frame_sz, int num_frames)
{
    sprite_path = path;
    sheet_size = sheet_sz;
    frame_size = frame_sz;
    frame_count = num_frames;

    frames.clear();

    // Calculate frames from left to right
    for (int i = 0; i < num_frames; ++i)
    {
        Animation_Frame frame;
        frame.position = {i * frame_size.x, 0};
        frame.size = frame_size;

        // Calculate normalized UV coordinates
        frame.uv_min.x = static_cast<float>(frame.position.x) / static_cast<float>(sheet_size.x);
        frame.uv_min.y = 0.0f;
        frame.uv_max.x = static_cast<float>(frame.position.x + frame_size.x) / static_cast<float>(sheet_size.x);
        frame.uv_max.y = static_cast<float>(frame_size.y) / static_cast<float>(sheet_size.y);

        frames.push_back(frame);
    }
}

void Sprite_Animation::add_frame(const glm::ivec2 &position, const glm::ivec2 &size)
{
    Animation_Frame frame;
    frame.position = position;
    frame.size = size;

    if (sheet_size.x > 0 && sheet_size.y > 0)
    {
        frame.uv_min.x = static_cast<float>(position.x) / static_cast<float>(sheet_size.x);
        frame.uv_min.y = static_cast<float>(position.y) / static_cast<float>(sheet_size.y);
        frame.uv_max.x = static_cast<float>(position.x + size.x) / static_cast<float>(sheet_size.x);
        frame.uv_max.y = static_cast<float>(position.y + size.y) / static_cast<float>(sheet_size.y);
    }

    frames.push_back(frame);
    frame_count = static_cast<int>(frames.size());
}

void Sprite_Animation::add_frame(int x, int y, int width, int height)
{
    add_frame({x, y}, {width, height});
}

void Sprite_Animation::update(float delta_time)
{
    if (!playing || frames.empty())
        return;

    time_accumulator += delta_time;

    if (time_accumulator >= frame_duration)
    {
        time_accumulator -= frame_duration;
        current_frame++;

        if (current_frame >= static_cast<int>(frames.size()))
        {
            if (loop)
            {
                current_frame = 0;
            }
            else
            {
                current_frame = static_cast<int>(frames.size()) - 1;
                playing = false;
            }
        }
    }
}

void Sprite_Animation::play()
{
    playing = true;
}

void Sprite_Animation::stop()
{
    playing = false;
}

void Sprite_Animation::reset()
{
    current_frame = 0;
    time_accumulator = 0.0f;
}

void Sprite_Animation::set_looping(bool should_loop)
{
    loop = should_loop;
}

void Sprite_Animation::set_frame_duration(float duration)
{
    frame_duration = duration;
}

void Sprite_Animation::set_state(Sprite_State state)
{
    if (current_state != state)
    {
        current_state = state;
        // Set frame to match the state (each state = one frame in the 4-frame sprite)
        int frame_index = static_cast<int>(state);
        if (frame_index >= 0 && frame_index < static_cast<int>(frames.size()))
        {
            current_frame = frame_index;
        }
    }
}

Sprite_State Sprite_Animation::get_state() const
{
    return current_state;
}

Animation_Frame Sprite_Animation::get_frame() const
{
    if (frames.empty())
    {
        return Animation_Frame{{0, 0}, frame_size, {0.0f, 0.0f}, {1.0f, 1.0f}};
    }

    int idx = current_frame;
    if (idx < 0 || idx >= static_cast<int>(frames.size()))
    {
        idx = 0;
    }
    return frames[idx];
}

Animation_Frame Sprite_Animation::get_frame_for_state(Sprite_State state) const
{
    int idx = static_cast<int>(state);
    if (idx >= 0 && idx < static_cast<int>(frames.size()))
    {
        return frames[idx];
    }
    return get_frame();
}

int Sprite_Animation::get_current_frame_index() const
{
    return current_frame;
}

glm::vec2 Sprite_Animation::get_uv_min() const
{
    return get_frame().uv_min;
}

glm::vec2 Sprite_Animation::get_uv_max() const
{
    return get_frame().uv_max;
}

const std::string &Sprite_Animation::get_sprite_path() const
{
    return sprite_path;
}

glm::ivec2 Sprite_Animation::get_frame_size() const
{
    return frame_size;
}

glm::ivec2 Sprite_Animation::get_sheet_size() const
{
    return sheet_size;
}

int Sprite_Animation::get_frame_count() const
{
    return frame_count;
}

bool Sprite_Animation::is_playing() const
{
    return playing;
}
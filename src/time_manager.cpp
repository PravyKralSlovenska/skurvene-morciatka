#include "engine/time_manager.hpp"

Time_Manager::Time_Manager()
{
}

void Time_Manager::init()
{
    running = true;

    START_TIME = std::chrono::steady_clock::now();
    previous_time = std::chrono::steady_clock::now();

    time_scale = 1;

    frames_per_second = 0;
    updates_per_second = 0;

    std::cout << "Time manager init zbehnuty\n";
}

void Time_Manager::time_update()
{
    now = std::chrono::steady_clock::now();

    delta_time = std::chrono::duration<double>(now - previous_time).count();

    frames_count++;
    calculate_FPS();
}

void Time_Manager::calculate_FPS()
{
    if (delta_time >= 1.0)
    {
        frames_per_second = frames_count;
        frames_count = 0;
        previous_time = now;
    }
}

void Time_Manager::set_time_scale(float time_scale)
{
    this->time_scale = time_scale;
}

void Time_Manager::set_target_fps(int FPS)
{
    MAX_FPS = FPS;
}

void Time_Manager::pause()
{
    running = !running;
}

bool Time_Manager::is_paused()
{
    return running;
}

int Time_Manager::get_frames_per_second()
{
    return frames_per_second;
}

int Time_Manager::get_updates_per_second()
{
    return updates_per_second;
}
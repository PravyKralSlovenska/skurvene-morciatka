#include "engine/time_manager.hpp"

Time_Manager::Time_Manager()
{
}

void Time_Manager::init()
{
    is_paused = false;
    fps_limited = false;

    START_TIME = std::chrono::steady_clock::now();
    current_frame_time = START_TIME;

    time_scale = 1;

    frames_per_second = 0;
    updates_per_second = 0;

    frame_time = 1.0 / 60.0;

    std::cout << "Time manager init zbehnuty\n";
}

void Time_Manager::time_update()
{
    previous_frame_time = current_frame_time;
    current_frame_time = std::chrono::steady_clock::now();

    delta_time = std::chrono::duration<double>(current_frame_time - previous_frame_time).count();
    // std::cout << delta_time << "\n";

    calculate_FPS();

    if (fps_limited)
    {
        sleep_until_next_frame();
    }
}

void Time_Manager::calculate_FPS()
{
    frames_count++;
    frame_calculation_time += delta_time;

    if (frame_calculation_time >= 1.0)
    {
        frames_per_second = frames_count;
        frame_calculation_time = 0;
        frames_count = 0;
    }
}

void Time_Manager::set_time_scale(float time_scale)
{
    this->time_scale = time_scale;
}

void Time_Manager::set_target_fps(int FPS)
{
    MAX_FPS = FPS;
    frame_time = 1.0 / MAX_FPS;
}

void Time_Manager::enable_fps_limiting()
{
    fps_limited = true;
}

void Time_Manager::sleep_until_next_frame()
{
    const auto frame_end_time = current_frame_time +
                                std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                                    std::chrono::duration<double>(frame_time));

    const auto now = std::chrono::steady_clock::now();

    if (frame_end_time > now)
    {
        std::this_thread::sleep_until(frame_end_time);
    }
}

void Time_Manager::pause()
{
    if (!is_paused)
    {
        is_paused = true;
    }
}

void Time_Manager::resume()
{
    if (is_paused)
    {
        is_paused = false;
    }
}

bool Time_Manager::paused()
{
    return is_paused;
}

int Time_Manager::get_frames_per_second()
{
    return frames_per_second;
}

int Time_Manager::get_updates_per_second()
{
    // if (is_paused)
    // {
    // return "PAUSED";
    // }

    return updates_per_second;
}

double Time_Manager::get_delta_time()
{
    return delta_time;
}

void Time_Manager::debug()
{
}

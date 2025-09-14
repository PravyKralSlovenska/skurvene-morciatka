#pragma once

#include <iostream>
#include <chrono>

class Time_Manager
{
private:
    std::chrono::steady_clock::time_point START_TIME; // const ?
    std::chrono::steady_clock::time_point previous_time;
    std::chrono::steady_clock::time_point now;

    double FIXED_DELTA_TIME;
    double FIXED_TIME;

    double delta_time;
    double average_delta_time;

    float time_scale;

    bool running;

    int MAX_FPS;
    int frames_count;
    double frame_time; // 1 / MAX_FPS alebo frames_per_second
    // bool fps_limited;

public:
    int frames_per_second;  // FPS
    int updates_per_second; // UPS

public:
    Time_Manager();
    ~Time_Manager() = default;

    void init();
    void pause();

    void time_update();
    void set_time_scale(float time_scale);
    void time_snapshot();

    void calculate_FPS();
    void set_target_fps(int FPS);
    void enable_fps_limiting();

    bool is_paused();

    // getters
    int get_frames_per_second();
    int get_updates_per_second();
};

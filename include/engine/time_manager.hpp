#pragma once

#include <iostream>
#include <chrono>
#include <thread> // pre sleep

class Time_Manager
{
private:
    std::chrono::steady_clock::time_point START_TIME; // const ?
    std::chrono::steady_clock::time_point previous_frame_time;
    std::chrono::steady_clock::time_point current_frame_time;

    const double FIXED_DELTA_TIME = 0.2;
    const double FIXED_TIME = 0.2;

    double delta_time; // cas, za ktory
    double average_delta_time;

    float time_scale;

    bool is_paused;

    int MAX_FPS;
    int frames_count;
    double frame_calculation_time;
    double frame_time; // miliseconds
    bool fps_limited;

public:
    int frames_per_second;  // FPS
    int updates_per_second; // UPS

public:
    Time_Manager();
    ~Time_Manager() = default;

    void init();

    void pause();
    void resume();

    void time_update();
    void set_time_scale(float time_scale);
    void time_snapshot();

    void calculate_UPS();
    void calculate_FPS();

    void set_target_fps(int FPS);
    void enable_fps_limiting();
    void sleep_until_next_frame();

    bool paused();

    // getters
    int get_frames_per_second();
    int get_updates_per_second();
    double get_delta_time();

    void debug();
};

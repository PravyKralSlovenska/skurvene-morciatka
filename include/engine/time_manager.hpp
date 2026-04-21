#pragma once

// File purpose: Tracks frame timing, pause state, and frame pacing.
#include <iostream>
#include <chrono>
#include <thread> // pre sleep

// Tracks frame timing, pause state, and fixed-rate pacing.
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
    // Constructs Time_Manager.
    Time_Manager();
    // Destroys Time_Manager and releases owned resources.
    ~Time_Manager() = default;

    // Initializes state.
    void init();

    // Pauses.
    void pause();
    // Resumes.
    void resume();

    // Time update.
    void time_update();
    // Sets time scale.
    void set_time_scale(float time_scale);
    // Time snapshot.
    void time_snapshot();

    // Calculates UPS.
    void calculate_UPS();
    // Calculates FPS.
    void calculate_FPS();

    // Sets target fps.
    void set_target_fps(int FPS);
    // Enables fps limiting.
    void enable_fps_limiting();
    // Sleeps until next frame.
    void sleep_until_next_frame();

    // Paused.
    bool paused();

    // getters
    int get_frames_per_second();
    // Returns updates per second.
    int get_updates_per_second();
    // Returns delta time.
    double get_delta_time();

    // Debugs this component state.
    void debug();
};

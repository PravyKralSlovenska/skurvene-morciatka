#pragma once

// File purpose: Manages audio playback, music state, and audio thread tasks.
#include <unordered_map>
#include <vector>
#include <memory>
#include <thread>
#include <queue>
#include <chrono>
#include <mutex>
#include <atomic>
#include <string>
#include <glm/glm.hpp>
#include <AL/al.h>
#include <AL/alc.h>

// forward declarations
class Sound_Buffer;
class Audio_Source;
class Listener;
class Player;
class Time_Manager;

// Defines the Pending_Execute struct.
struct Pending_Execute
{
    enum Operations
    {
        PLAY,
        STOP,
        RESUME,
        SKIP,
        FORWARD,
        LOAD
    } operation;
    std::string name = "";
    std::string path = "";

    // Constructs Pending_Execute.
    Pending_Execute();
    // Constructs Pending_Execute.
    Pending_Execute(Operations operation, std::string name = "", std::string path = "");
};

// Coordinates music playback, queueing, and audio thread work.
class Audio_Manager
{
private:
    ALCdevice *device;
    ALCcontext *context;

    Listener *listener;
    Player *player;
    Time_Manager *time_manager;

    std::thread audio_thread;
    std::atomic<bool> audio_thread_running;
    std::queue<Pending_Execute> audio_thread_queue;
    std::mutex queue_mutex;

    std::unordered_map<std::string, Sound_Buffer> sounds;
    std::unordered_map<std::string, float> sound_gains;
    std::vector<Audio_Source> active_sources;

private:
    // Execute stuff from queue.
    void execute_stuff_from_queue();
    // Removes not active sources.
    void remove_not_active_sources();

    // Initializes openal.
    bool init_openal();
    // Audio thread loop.
    void audio_thread_loop();
    // Cleanups this component state.
    void cleanup();

public:
    // Constructs Audio_Manager.
    Audio_Manager();
    // Destroys Audio_Manager and releases owned resources.
    ~Audio_Manager();

    // Initializes state.
    void init(); // init thread
    // Send execute.
    void send_execute(const Pending_Execute::Operations operation, const std::string name = "", const std::string path = "");

    // Sets player.
    void set_player(Player *player);
    // Sets time manager.
    void set_time_manager(Time_Manager *time_manager);

    // Loads music.
    bool load_music(const std::string name, const std::string path_to_sound);
    // Sets sound gain.
    void set_sound_gain(const std::string &name, float gain);
    // Plays.
    bool play(const std::string name);
    // Stops.
    bool stop(const ALuint audio_source_id);
    // Forward.
    bool forward();
    // Backward.
    bool backward();
    // Skip.
    bool skip();
    // Skip backwards.
    bool skip_backwards();
    // Changes volume.
    bool change_volume();
    // Sets volume.
    bool set_volume();
    // Sets loop.
    bool set_loop();
    // Fade.
    bool fade();

    // Cleans up finished sources.
    void cleanup_finished_sources();
    // Stops all.
    void stop_all();
};

#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <thread>
#include <queue>
#include <chrono>
#include <mutex>
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

    Pending_Execute();
    Pending_Execute(Operations operation, std::string name = "", std::string path = "");
};

class Audio_Manager
{
private:
    ALCdevice *device;
    ALCcontext *context;

    Listener *listener;
    Player *player;
    Time_Manager *time_manager;

    std::thread audio_thread;
    // std::atomic<bool> 
    bool audio_thread_running;
    std::queue<Pending_Execute> audio_thread_queue;
    std::mutex queue_mutex;

    std::unordered_map<std::string, Sound_Buffer> sounds;
    std::vector<Audio_Source> active_sources;

private:
    void execute_stuff_from_queue();
    void remove_not_active_sources();

    bool init_openal();
    void audio_thread_loop();
    void cleanup();

public:
    Audio_Manager();
    ~Audio_Manager();

    void init(); // init thread
    void send_execute(const Pending_Execute::Operations operation, const std::string name = "", const std::string path = "");

    void set_player(Player *player);
    void set_time_manager(Time_Manager *time_manager);

    bool load_music(const std::string name, const std::string path_to_sound);
    bool play(const std::string name);
    bool stop(const ALuint audio_source_id);
    bool forward();
    bool backward();
    bool skip();
    bool skip_backwards();
    bool change_volume();
    bool set_volume();
    bool set_loop();
    bool fade();

    void cleanup_finished_sources();
    void stop_all();
};

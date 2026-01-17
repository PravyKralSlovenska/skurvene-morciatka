#include "engine/audio/audio_manager.hpp"
#include "engine/audio/audio_source.hpp"
#include "engine/audio/audio_buffer.hpp"
#include "engine/audio/listener.hpp"
#include "engine/player/entity.hpp"
#include "engine/time_manager.hpp"

Pending_Execute::Pending_Execute(Operations operation, std::string name, std::string path)
    : operation(operation), name(name), path(path) {}

Audio_Manager::Audio_Manager() {}
Audio_Manager::~Audio_Manager()
{
    cleanup();
}

void Audio_Manager::init()
{
    if (!init_openal())
    {
        std::cout << "nepodaril sa init";
        return;
    }

    audio_thread_running = true;
    audio_thread = std::thread(&Audio_Manager::audio_thread_loop, this);
    // std::cout << audio_thread.get_id() << std::endl;
}

bool Audio_Manager::init_openal()
{
    device = alcOpenDevice(nullptr);
    if (!device)
    {
        std::cerr << "device error\n";
        return false;
    }

    context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);

    if (!context)
    {
        std::cerr << "context error\n";
        cleanup();
        return false;
    }

    return true;
}

void Audio_Manager::audio_thread_loop()
{
    // listener = &Listener();

    while (audio_thread_running)
    {
        execute_stuff_from_queue();
        // remove not active sources

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 60 FPS
    }

    // cleanup();
}

void Audio_Manager::send_execute(const Pending_Execute::Operations operation, const std::string name, const std::string path)
{
    audio_thread_queue.push(Pending_Execute(operation, name, path));
}

void Audio_Manager::execute_stuff_from_queue()
{
    std::lock_guard<std::mutex> lock(queue_mutex);

    while (!audio_thread_queue.empty())
    {
        const Pending_Execute &execute = audio_thread_queue.front();

        switch (execute.operation)
        {
        case Pending_Execute::Operations::PLAY:
            // Handle play operation
            play(execute.name);
            break;

        case Pending_Execute::Operations::STOP:
            audio_thread_running = false;
            break;

        case Pending_Execute::Operations::RESUME:
            // Handle resume operation
            break;

        case Pending_Execute::Operations::SKIP:
            // Handle skip operation
            break;

        case Pending_Execute::Operations::FORWARD:
            // Handle forward operation
            break;

        case Pending_Execute::Operations::LOAD:
            // Handle load operation
            load_music(execute.name, execute.path);
            /* code */
            break;

        default:
            break;
        }

        audio_thread_queue.pop();
    }

    // std::cout << "audio thread queue je prazdny\n";
}

void Audio_Manager::cleanup()
{

    // niekde tu by mal byt prikaz aby sa zastail loop v audio_thread_loop

    if (audio_thread.joinable())
    {
        audio_thread.join();
    }

    alcCloseDevice(device);
    alcDestroyContext(context);
}

void Audio_Manager::set_player(Player *player)
{
    this->player = player;
}

void Audio_Manager::set_time_manager(Time_Manager *time_manager)
{
    this->time_manager = time_manager;
}

bool Audio_Manager::load_music(const std::string name, const std::string path_to_sound)
{
    // sounds.insert({name, std::make_unique<Sound_Buffer>(path_to_sound)});
    sounds.insert({name, Sound_Buffer(path_to_sound)});
    // sounds.emplace(name, Sound_Buffer(path_to_sound));
    return true;
}

bool Audio_Manager::play(const std::string name)
{
    auto it = sounds.find(name);
    if (it == sounds.end())
    {
        std::cerr << "takyto track na discu nemam :cool:\n";
        return false;
    }

    const Sound_Buffer buffer = it->second;

    active_sources.emplace_back(glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), 1.0f, 1.0f);
    // active_sources.emplace_back(glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), 2.0f, 2.0f);
    Audio_Source &source = active_sources.back();
    source.play_sound(buffer);

    // check_al_error("nahravam do source cez audio managera");
    return true;
}

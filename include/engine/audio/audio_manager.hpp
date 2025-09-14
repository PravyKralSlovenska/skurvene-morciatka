#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include <AL/al.h>
#include <AL/alc.h>

#include "engine/entity.hpp"

// dobry napad mat toto na inom thread

// konkretna pesnicka/zvuk (cdcko)
class Sound_Buffer
{
private:
    const std::string path_to_sound;

public:
    ALuint id;                     // id k bufferu
    std::vector<int16_t> pcm_data; // Pulse Code Modulation: cisielka reprezentujuce audio vlnu
    ALint frequency;               // Frq == Sample Rate: Ako casto za sekundu je marana vlna. Hz
    ALint bit_depth;               // Kolko bitov je pouzitich na reprezentaciu velkosti amplitudy
    ALint bit_rate;                // typicky sa pozuiva pre MP3 a reprezentuje kolko bitov za sekundu kompresia pouziva? bit_rate = sample_rate * bits * channels
    ALint channels;                // 1 = mono; 2 = stereo; 6 = 8d audio ci take nieco idk
    ALint size;                    // Realna dlzka pcm_data v bytoch. size = samples * channels * (bits/8)
    int duration;                  // kolko sekund trva nahravka :3

public:
    Sound_Buffer(const std::string path_to_sound);
    ~Sound_Buffer();

    void read_data_mp3();
    void read_data_wav();

    void fill_with_data();
};

/*
 * Sources
 * - zdroj, z ktore "vznika" zvuk
 * - "reproduktor"
 */
class Audio_Source
{
public:
    ALuint id;
    glm::vec3 coords; // suradnice x,y,z kedze sme v 2D, suradnica z bude 1 alebo 0
    // std::unique_ptr<Sound_Buffer> sound;

    bool currently_playing;

public:
    Audio_Source(glm::ivec3 coords, glm::ivec3 velocity = {0, 0, 0}, float pitch = 1.0f, float gain = 1.0f);
    Audio_Source();
    ~Audio_Source();

    void play_sound(Sound_Buffer sound);
    void stop_sound();
    void pause_sound();

    void set_pitch(const float pitch);
    void set_gain(const float gain);
    void set_position(const glm::ivec3 position);
    void set_velocity(const glm::ivec3 velocity);
    void set_looping(const ALboolean yes);
};

/*```
 * Listener
 * - clovek/hrac, ku ktoremu zvuk potuje
 */
class Listener
{
public:
    glm::vec2 coords;
    glm::vec2 velocity;

public:
    Listener();
    ~Listener() = default;

    void set_gain(const float gain);
    void set_position(const glm::ivec3 coords);
    void set_velocity(const glm::ivec3 velocity);
    void set_orientation(const glm::ivec3 orientation);
};

class Audio_Manager
{
private:
    ALCdevice *device;
    ALCcontext *context;

    Listener *listener;
    Player *player;

    std::unordered_map<std::string, Sound_Buffer> sounds;
    std::vector<Audio_Source> active_sources;

public:
    Audio_Manager();
    ~Audio_Manager();

    void init();
    void cleanup();

    void set_player(Player *player);

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

// queue
// mixing
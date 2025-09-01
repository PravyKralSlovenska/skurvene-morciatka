#include "engine/audio/audio_manager.hpp"

// kniznice od David Reid
#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#include "engine/audio/dr_mp3.h"
#include "engine/audio/dr_wav.h"

void check_al_error(const std::string &operation)
{
    ALenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        std::cerr << "OpenAL Error in " << operation << ": " << error << std::endl;
    }
}

Sound_Buffer::Sound_Buffer(const std::string path_to_sound)
    : path_to_sound(path_to_sound)
{
    alGenBuffers(1, &id);
    std::cerr << "OpenAL INFO: vytvaram buffer s idckom: " << id << "\n";

    if (!alIsBuffer(id))
    {
        std::cerr << "OpenAL ERROR: vygenerovany buffer nie je ok: " << id << '\n';
        return;
    }

    char koncovka = path_to_sound[path_to_sound.size() - 1];
    if (koncovka == '3')
    {
        read_data_mp3();
    }
    else if (koncovka == 'v')
    {
        read_data_wav();
    }
    else
    {
        std::cerr << "OpenAL ERROR: tuto koncovku nepoznam: " + koncovka;
    }

    fill_with_data();
}

Sound_Buffer::~Sound_Buffer()
{
    // if (alIsBuffer(id))
    // {
    //     std::cout << "mazem kokotka\n";
    //     alDeleteBuffers(1, &id);
    // }
}

void Sound_Buffer::read_data_mp3()
{
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, path_to_sound.c_str(), nullptr))
    {
        std::cerr << "MP3 nepodarilo sa nahrat subor\n"
                  << path_to_sound << '\n';
    }

    drmp3_uint64 frame_count = mp3.totalPCMFrameCount;
    channels = mp3.channels;
    frequency = mp3.sampleRate;

    size_t sample_count = frame_count * channels;

    pcm_data.resize(sample_count);

    drmp3_read_pcm_frames_s16(&mp3, frame_count, pcm_data.data());

    drmp3_uninit(&mp3);

    check_al_error("Nahravanie v read_data_mp3");
}

void Sound_Buffer::read_data_wav()
{
    drwav wav;
    if (!drwav_init_file(&wav, path_to_sound.c_str(), nullptr))
    {
        std::cerr << "WAV nepodarilo sa nahrat subor\n";
    }

    channels = wav.channels;
    frequency = wav.sampleRate;

    size_t sample_count = wav.totalPCMFrameCount * wav.channels;
    pcm_data.resize(sample_count);

    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pcm_data.data());

    drwav_uninit(&wav);

    check_al_error("Nahravanie v read_data_wav");
}

void Sound_Buffer::fill_with_data()
{
    if (pcm_data.empty())
    {
        std::cerr << "OpenAL ERROR: PCM DATA su dopice prazdne\n";
        return;
    }

    // 0 - idcko
    // 1 - format
    // 2 - data v bytoch
    // 3 - velkost dat v bytoch
    // 4 - frekvencia
    ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    alBufferData(id, format, pcm_data.data(), pcm_data.size() * sizeof(int16_t), frequency);

    check_al_error("nahravanie dat do bufferu s id" + id);
    std::cout << "HALO?" << '\n';
}

Audio_Source::Audio_Source(glm::ivec3 coords, glm::ivec3 velocity, float pitch, float gain)
{
    alGenSources(1, &id);

    set_pitch(pitch);
    set_gain(gain);
    set_position(coords);
    set_velocity(velocity);
}

// Audio_Source::Audio_Source()
// {
//     alGenSources(1, &id);
// }

Audio_Source::~Audio_Source()
{
    alDeleteSources(1, &id);
}

void Audio_Source::play_sound(Sound_Buffer sound)
{
    // alSourceQueueBuffers(id, 1, &sound.id);
    alSourcei(id, AL_BUFFER, sound.id);

    alSourcePlay(id);
}

void Audio_Source::set_pitch(const float pitch)
{
    alSourcef(id, AL_PITCH, pitch);
}

void Audio_Source::set_gain(const float gain)
{
    alSourcef(id, AL_GAIN, gain);
}

void Audio_Source::set_position(const glm::ivec3 coords)
{
    alSource3i(id, AL_POSITION, coords.x, coords.y, coords.z);
}

void Audio_Source::set_velocity(const glm::ivec3 velocity)
{
    alSource3i(id, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Audio_Source::set_looping(const ALboolean yes)
{
    alSourcei(id, AL_LOOPING, yes);
}

Listener::Listener()
{
}

void Listener::set_gain(const float gain)
{
    alListenerf(AL_GAIN, gain);
}

void Listener::set_position(const glm::ivec3 coords)
{
    alListener3i(AL_POSITION, coords.x, coords.y, coords.z);
}

void Listener::set_velocity(const glm::ivec3 velocity)
{
    alListener3i(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Listener::set_orientation(const glm::ivec3 orientation)
{
    // default je {0, 0, -1, 0, 1 , 0} neviem co to znamena ale okej
    alListener3f(AL_ORIENTATION, orientation.x, orientation.y, orientation.z);
}

Audio_Manager::Audio_Manager() {}
Audio_Manager::~Audio_Manager()
{
    clean_up();
}

void Audio_Manager::init()
{
    device = alcOpenDevice(nullptr);
    if (!device)
    {
        std::cerr << "device error\n";
        return;
    }

    context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);

    if (!context)
    {
        std::cerr << "context error\n";
        clean_up();
        return;
    }
}

void Audio_Manager::clean_up()
{
    alcCloseDevice(device);
    alcDestroyContext(context);
}

void Audio_Manager::set_player(Player *player)
{
    this->player = player;
}

bool Audio_Manager::load_music(const std::string name, const std::string path_to_sound)
{
    std::cout << "1\n";
    sounds.insert({name, Sound_Buffer(path_to_sound)});
    // sounds.emplace(name, Sound_Buffer(path_to_sound));
    std::cout << "2\n";
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

    check_al_error("nahravam do source cez audio managera");
    return true;
}

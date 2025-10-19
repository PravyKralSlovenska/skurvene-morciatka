#include "engine/audio/audio_buffer.hpp"
#include <iostream>

// kniznice od David Reid
#define DR_MP3_IMPLEMENTATION
#include "engine/audio/dr_mp3.h"

#define DR_WAV_IMPLEMENTATION
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
    // std::cout << "mazem kokotka\n";
    // if (alIsBuffer(id))
    // {
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
}
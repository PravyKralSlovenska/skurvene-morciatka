#include "engine/audio/audio_buffer.hpp"
#include <iostream>
#include <limits>

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
    pcm_data.clear();

    drmp3 mp3;
    if (!drmp3_init_file(&mp3, path_to_sound.c_str(), nullptr))
    {
        std::cerr << "MP3 nepodarilo sa nahrat subor\n"
                  << path_to_sound << '\n';
        return;
    }

    channels = static_cast<ALint>(mp3.channels);
    frequency = static_cast<ALint>(mp3.sampleRate);

    if (channels <= 0 || frequency <= 0)
    {
        std::cerr << "MP3 ma neplatne metadata: " << path_to_sound << '\n';
        drmp3_uninit(&mp3);
        return;
    }

    constexpr size_t kMaxSafeSamples = static_cast<size_t>(48000) * 2 * 60 * 120; // 2h stereo @ 48kHz
    constexpr drmp3_uint64 kChunkFrames = 4096;
    std::vector<drmp3_int16> chunk(static_cast<size_t>(kChunkFrames) * static_cast<size_t>(channels));

    while (true)
    {
        const drmp3_uint64 frames_read = drmp3_read_pcm_frames_s16(&mp3, kChunkFrames, chunk.data());
        if (frames_read == 0)
            break;

        const size_t samples_read = static_cast<size_t>(frames_read) * static_cast<size_t>(channels);
        if (pcm_data.size() + samples_read > kMaxSafeSamples)
        {
            std::cerr << "MP3 ma podozrivu velkost PCM: " << path_to_sound << '\n';
            pcm_data.clear();
            drmp3_uninit(&mp3);
            return;
        }

        pcm_data.insert(pcm_data.end(), chunk.begin(), chunk.begin() + static_cast<std::ptrdiff_t>(samples_read));
    }

    drmp3_uninit(&mp3);

    check_al_error("Nahravanie v read_data_mp3");
}

void Sound_Buffer::read_data_wav()
{
    pcm_data.clear();

    drwav wav;
    if (!drwav_init_file(&wav, path_to_sound.c_str(), nullptr))
    {
        std::cerr << "WAV nepodarilo sa nahrat subor\n"
                  << path_to_sound << '\n';
        return;
    }

    channels = static_cast<ALint>(wav.channels);
    frequency = static_cast<ALint>(wav.sampleRate);

    if (channels <= 0 || frequency <= 0)
    {
        std::cerr << "WAV ma neplatne metadata: " << path_to_sound << '\n';
        drwav_uninit(&wav);
        return;
    }

    constexpr size_t kMaxSafeSamples = static_cast<size_t>(48000) * 2 * 60 * 120; // 2h stereo @ 48kHz
    constexpr drwav_uint64 kChunkFrames = 4096;
    std::vector<drwav_int16> chunk(static_cast<size_t>(kChunkFrames) * static_cast<size_t>(channels));

    while (true)
    {
        const drwav_uint64 frames_read = drwav_read_pcm_frames_s16(&wav, kChunkFrames, chunk.data());
        if (frames_read == 0)
            break;

        const size_t samples_read = static_cast<size_t>(frames_read) * static_cast<size_t>(channels);
        if (pcm_data.size() + samples_read > kMaxSafeSamples)
        {
            std::cerr << "WAV ma podozrivu velkost PCM: " << path_to_sound << '\n';
            pcm_data.clear();
            drwav_uninit(&wav);
            return;
        }

        pcm_data.insert(pcm_data.end(), chunk.begin(), chunk.begin() + static_cast<std::ptrdiff_t>(samples_read));
    }

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
    if (channels != 1 && channels != 2)
    {
        std::cerr << "OpenAL ERROR: nepodporovany pocet kanalov: " << channels << " subor: " << path_to_sound << '\n';
        return;
    }

    if (frequency <= 0)
    {
        std::cerr << "OpenAL ERROR: neplatna frekvencia: " << frequency << " subor: " << path_to_sound << '\n';
        return;
    }

    ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    alBufferData(id, format, pcm_data.data(), pcm_data.size() * sizeof(int16_t), frequency);

    check_al_error("nahravanie dat do bufferu s id" + id);
}
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <AL/al.h>

// konkretna pesnicka/zvuk (cdcko)
class Sound_Buffer
{
private:
    const std::string path_to_sound;

public:
    ALuint id = 0;                 // id k bufferu
    std::vector<int16_t> pcm_data; // Pulse Code Modulation: cisielka reprezentujuce audio vlnu
    ALint frequency = 0;           // Frq == Sample Rate: Ako casto za sekundu je marana vlna. Hz
    ALint bit_depth = 0;           // Kolko bitov je pouzitich na reprezentaciu velkosti amplitudy
    ALint bit_rate = 0;            // typicky sa pozuiva pre MP3 a reprezentuje kolko bitov za sekundu kompresia pouziva? bit_rate = sample_rate * bits * channels
    ALint channels = 0;            // 1 = mono; 2 = stereo; 6 = 8d audio ci take nieco idk
    ALint size = 0;                // Realna dlzka pcm_data v bytoch. size = samples * channels * (bits/8)
    int duration = 0;              // kolko sekund trva nahravka :3

public:
    Sound_Buffer(const std::string path_to_sound);
    ~Sound_Buffer();

    void read_data_mp3();
    void read_data_wav();
    void fill_with_data();
};

#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Animation_Frame
{
    glm::ivec2 position;
    glm::ivec2 size;
};

// 1 animacia = 1 class Sprite_Animation
class Sprite_Animation
{
private:
    std::vector<Animation_Frame> frames;
    int current_frame = 0;

    float time_lenght = 0;
    float delta_time = 0;

    bool loop = true;
    bool playing = false;

public:
    void add_frame();
    void update();

    Animation_Frame get_frame();

    void play();
    void stop();
    void reset();
    void set_looping();
};

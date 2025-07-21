#pragma once

#include <GLFW/glfw3.h>
#include "engine/entity.hpp"

class Controls
{
private:
    GLFWwindow *window;
    Player *player;

public:
    Controls(GLFWwindow *window);
    ~Controls() = default;
};
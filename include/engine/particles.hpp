#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>

class Particle
{
public:
    int x_coord;                // x pozicia
    int y_coord;                // y pozicia
    int velocity_x;             // x rychlost
    int velocity_y;             // y rychlost
    int mass;                   // hmotnost
    int lifetime;               // zivotnost
    bool hurt = false;          // ci je particle zranitelny
    std::array<float, 3> color; // farba v RGB formate (Red, Green, Blue)

    Particle(int x, int y, int vx, int vy, int m, int lt, std::array<float, 3> c) : x_coord(x),
                                                                                    y_coord(y),
                                                                                    velocity_x(vx),
                                                                                    velocity_y(vy),
                                                                                    mass(m),
                                                                                    lifetime(lt),
                                                                                    color(c)
    {
    }
    virtual ~Particle() = default;

    virtual void update() {}
    virtual void render() {}

    int getX()
    {
        return x_coord;
    }

    int getY()
    {
        return y_coord;
    }
};

class Liquid : public Particle
{
};

class Water : public Liquid
{
};

class Solid : public Particle
{
};

class Sand : public Solid
{
};

class Gas : public Particle
{
};

class Smoke : public Gas
{
};
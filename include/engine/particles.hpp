#include <GLFW/glfw3.h>

#include <iostream>

#pragma once

class Particle
{
public:
    int x_coord;       // x pozicia
    int y_coord;       // y pozicia
    int velocity_x;    // x rychlost
    int velocity_y;    // y rychlost
    int mass;          // hmotnost
    int lifetime;      // zivotnost
    float color[3];    // farba v RGB formate (Red, Green, Blue)
    bool hurt = false; // ci je particle zranitelny

    Particle(int x, int y, int vx, int vy, int m, int lt, float c[]) : x_coord(x),
                                                                       y_coord(y),
                                                                       velocity_x(vx),
                                                                       velocity_y(vy),
                                                                       mass(m),
                                                                       lifetime(lt)
    {
        color[0] = c[0];
        color[1] = c[1];
        color[2] = c[2];
    }
    ~Particle() = default;

    virtual void update() {}
    virtual void render() {}
};

class Air : public Particle
{
public:
    Air(int x, int y, int vx, int vy, int m, int lt, float c[3]) : Particle(x, y, vx, vy, m, lt, c)
    {
    }
};

class Sand : public Particle
{
public:
    Sand(int x, int y, int vx, int vy, int m, int lt, float c[3]) : Particle(x, y, vx, vy, m, lt, c)
    {
    }
};

class Water : public Particle
{
public:
    Water(int x, int y, int vx, int vy, int m, int lt, float c[3]) : Particle(x, y, vx, vy, m, lt, c)
    {
    }
};

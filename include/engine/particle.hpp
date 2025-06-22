#include <GLFW/glfw3.h>

#include <iostream>

#pragma once

class Particle
{
public:
    int x_coord;            // x pozicia
    int y_coord;            // y pozicia
    int velocity_x;         // x rychlost
    int velocity_y;         // y rychlost
    int mass;               // hmotnost
    int lifetime;           // zivotnost
    int color = 0xFFFFFFFF; // farba v RGBA formate (Red, Green, Blue, Alpha)
    bool hurt = false;      // ci je particle zranitelny    

    Particle(int x, int y, int vx, int vy, int m, int lt, int c = 0xFFFFFFFF) : x_coord(x),
                                                                                y_coord(y),
                                                                                velocity_x(vx),
                                                                                velocity_y(vy),
                                                                                mass(m),
                                                                                lifetime(lt),
                                                                                color(c)
    { }
    ~Particle() = default;

    virtual void update() { }
    virtual void render() { }
};

class Sand : public Particle
{
public:
    Sand(int x, int y, int vx, int vy, int m, int lt, int c = 0xFFFFD700) : Particle(x, y, vx, vy, m, lt, c)
    {
    }
};

class Water : public Particle
{
public:
    Water(int x, int y, int vx, int vy, int m, int lt, int c = 0xFF0000FF) : Particle(x, y, vx, vy, m, lt, c)
    {
    }
};

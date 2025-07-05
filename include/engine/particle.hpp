#pragma once
#include <iostream>
#include <vector>
#include <array>

class Particle
{
public:
    std::string name;
    int x_coord;                // x pozicia
    int y_coord;                // y pozicia
    int velocity_x;             // x rychlost
    int velocity_y;             // y rychlost
    int mass;                   // hmotnost
    int lifetime;               // zivotnost
    int temperature;            // teplota
    bool hurt = false;          // ci je particle zranitelny
    std::array<float, 3> color; // farba v RGB formate (Red, Green, Blue)

    Particle();
    Particle(int x, int y, int vx, int vy, int m, int lt, std::array<float, 3> c, bool h = false);
    virtual ~Particle() = default;

    /*
     * Update methoda ma fungovat ako iste "pravidla" ako by sa particle mal spravat
     * v danom svete. Napriklad, ako sa ma pohybovat, ako ma reagovat na okolie
     */
    std::pair<int, int> getCoords();
    int get_x();
    int get_y();
    void add_x(int x);
    void set_x(int x);
    void add_y(int y);
    void set_y(int y);
    void set_hurt(bool h);
    void set_coords(int x, int y);
    void set_velocity(int vx = 0, int vy = 0);
    void set_temperature(int t);
    void set_color(std::array<float, 3> c);
};

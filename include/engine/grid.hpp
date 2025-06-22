#include <iostream>
#include <vector>
#include <memory>

#include "particle.hpp"
#pragma once

class World
{
public:
    int width;
    int height;
    int particle_size;
    std::vector<std::unique_ptr<Particle>> grid;
    std::vector<std::unique_ptr<Particle>> grid_next;

    World(int width, int height, int particle_size) : width(width),
                                                      height(height),
                                                      particle_size(particle_size)
    {
        grid.resize(width * height);
        grid_next.resize(width * height);
    }

    void print_grid()
    {
        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                size_t index = y * width + x;
                if (grid[index])
                {
                    std::cout << "P ";
                }
                else
                {
                    std::cout << ". ";
                }
            }
            std::cout << std::endl;
        }
    }
};
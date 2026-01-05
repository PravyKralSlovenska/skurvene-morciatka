#pragma once

// forward declarations
enum class Particle_Type;

enum Biome_Type
{
    SANDY,
    STONE,
    ICY,
    URANIUM
};

struct Biome
{
    const Biome_Type type;

    // nejake definicie biomu
    const float cave_noise; // od -1 do 1
    const float cave_size;
    const float cave_temperature;

    const Particle_Type particle_fill;

    Biome(Biome_Type type, float cave_noise, float cave_size, float cave_temperature, Particle_Type particle_fill)
        : type(type), cave_noise(cave_noise), cave_size(cave_size), cave_temperature(cave_temperature), particle_fill(particle_fill) {}
};

inline Biome get_desert_biome()
{
    return Biome(Biome_Type::SANDY, 0.03f, 0.5f, 40.0f, Particle_Type::SAND);
}

inline Biome get_stone_biome()
{
    return Biome(Biome_Type::STONE, 0.05f, 0.3f, 10.0f, Particle_Type::STONE);
}

inline Biome get_icy_biome()
{
    return Biome(Biome_Type::ICY, -0.04f, 0.7f, -20.0f, Particle_Type::WATER);
}

inline Biome get_uranium_biome()
{
    return Biome(Biome_Type::URANIUM, 0.07f, 0.5f, 30.0f, Particle_Type::STONE);
}
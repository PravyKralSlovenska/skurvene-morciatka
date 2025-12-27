#pragma once

enum Biome_Type
{
    SANDY,
    STONE
};

struct Biome
{
    const float fill;
    const Biome_Type type;

    Biome();
};
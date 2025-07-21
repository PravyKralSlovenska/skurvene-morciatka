#pragma once

#include <iostream>
#include <vector>
#include <tuple>
#include <map>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Offset to advance to next glyph

    Character(unsigned int textureID, glm::ivec2 size, glm::ivec2 bearing, unsigned int advance);
};

// mozny performance upgrade -> bitmapy
class Text_Renderer
{
private:
    unsigned int VAO, VBO;
    FT_Library ft;
    FT_Face face;
    const char *font_path = "";
    std::map<char, Character> characters;
    std::vector<std::tuple<std::string, int, glm::vec2>> text_to_render;


public:
    Text_Renderer();
    ~Text_Renderer() = default;

    void init();
    void load_characters();
    void add_text(std::string text, int size, glm::vec2 coords);
    void render_text();
    void clear_buffers();
};

#pragma once

#include <iostream>
#include <vector>
#include <tuple>
#include <map>

#include <glm/glm.hpp>

#include "engine/renderer/shader.hpp"
#include "others/utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Offset to advance to next glyph

    Character() {}
    Character(unsigned int textureID, glm::ivec2 size, glm::ivec2 bearing, unsigned int advance);
};

// mozny performance upgrade -> bitmapy
class Text_Renderer
{
private:
    FT_Library ft;
    FT_Face face;

    unsigned int VAO, VBO;
    Shader shader;

    const char *font_path = "../fonts/MySims Racing DS Large Font.ttf";
    std::map<char, Character> characters;
    std::vector<std::tuple<std::string, glm::vec2, int, Color>> text_to_render;

public:
    Text_Renderer();
    ~Text_Renderer();

    void init();
    void load_characters();
    void add_text(std::string text, glm::vec2 coords, int scale, Color color);
    void render_text();
    void clear_buffers();
};

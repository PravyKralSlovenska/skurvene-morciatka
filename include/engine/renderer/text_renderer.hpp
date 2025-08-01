#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <tuple>
#include <map>

#include <glm/glm.hpp>

#include "engine/renderer/shader.hpp"
#include "engine/renderer/buffer.hpp"
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

    std::unique_ptr<VERTEX_ARRAY_OBJECT> VAO;
    std::unique_ptr<VERTEX_BUFFER_OBJECT> VBO;

    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> shader2;

    const char *font_path;
    std::map<char, Character> characters;
    std::vector<std::tuple<std::string, glm::vec2, float, Color>> text_to_render;

public:
    Text_Renderer();
    // Text_Renderer(const char *font_path);
    ~Text_Renderer();

    void init();
    void load_characters();
    void add_text(std::string text, glm::vec2 coords, int scale, Color color);
    void render_text();
    void clear_buffers();

    // test
    void render_triangle();
};

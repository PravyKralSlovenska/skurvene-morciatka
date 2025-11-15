#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <tuple>
#include <map>

#include "glad/gl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/text_renderer.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/renderer/buffers/vertex_buffer_object.hpp"
#include "engine/renderer/buffers/vertex_array_object.hpp"

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

    glm::mat4 projection;

public:
    Text_Renderer();
    // Text_Renderer(const char *font_path);
    ~Text_Renderer();

    void init();
    void load_characters();
    void render_text(std::string text, glm::vec2 coords, float scale, Color color);
    void clear_buffers();
    void set_projection(glm::mat4 projection);

    // test
    void render_triangle();
};

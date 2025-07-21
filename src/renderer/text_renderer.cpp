#include <iostream>

#include "glad/gl.h"

#include "engine/text_renderer.hpp"

Character::Character(unsigned int textureID, glm::ivec2 size, glm::ivec2 bearing, unsigned int advance)
    : TextureID(textureID), Size(size), Bearing(bearing), Advance(advance) {}

Text_Renderer::Text_Renderer() {}

void Text_Renderer::init()
{
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Libraryn\n";
        return;
    }

    if (FT_New_Face(ft, font_path, 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load a font\n";
        return;
    }

    load_characters();

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void Text_Renderer::load_characters()
{
    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // snazim sa nahrat prvych 128 znakov (ASCII)
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load a glyph\n";
            continue; // mozno by mal rovno skoncit lebo co ak nebudem vediet vykreslit napriklad 'S'
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character(
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x);

        characters.insert(std::pair<char, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Text_Renderer::add_text(std::string text, int size, glm::vec2 coords)
{
    text_to_render.push_back(std::tuple(text, size, coords));
}

void Text_Renderer::clear_buffers()
{

}

void Text_Renderer::render_text()
{
    for (auto text : text_to_render)
    {
        // add to buffer
    }

    clear_buffers();
}
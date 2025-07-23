#include <iostream>
#include <vector>
#include <tuple>
#include <map>

#include "glad/gl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/text_renderer.hpp"
#include "engine/renderer/shader.hpp"
#include "others/utils.hpp"

Character::Character(unsigned int textureID, glm::ivec2 size, glm::ivec2 bearing, unsigned int advance)
    : TextureID(textureID), Size(size), Bearing(bearing), Advance(advance) {}

Text_Renderer::Text_Renderer() 
    : shader("../shaders/text_vertex.glsl", "../shaders/text_texture.glsl")
{
    // toto je na test ci to realne funguje 
    text_to_render = {
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZ", {0.0f, 350.0f}, 1, Color(255, 255, 255, 1.0f)},
        {"abcdefghijklmnopqrstuvwxyz", {0.0f, 400.0f}, 1, Color(255, 255, 255, 1.0f)},
        {"0123456789"                , {0.0f, 450.0f}, 2, Color(255,   0,   0, 1.0f)},
        {"!@#$%^&*()_+-="            , {0.0f, 500.0f}, 3, Color(  0, 255,   0, 1.0f)},
        {"[]{}|\\:;\"'<>?,./~`"      , {0.0f, 550.0f}, 4, Color(  0,   0, 255, 0.5f)},
    };
}

Text_Renderer::~Text_Renderer()
{
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

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

    shader.create_shader();

    VBO = create_vertex_buffer_object();
    VAO = create_vertex_array_buffer();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    load_characters();
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

    std::cout << "INFO::FREETYPE: Loaded " << characters.size() << " characters\n";

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Text_Renderer::add_text(std::string text, glm::vec2 coords, int scale, Color color)
{
    text_to_render.push_back(std::tuple(text, coords, scale, color));
}

void Text_Renderer::clear_buffers()
{
}

void Text_Renderer::render_text()
{
    if (text_to_render.empty())
    {
        return;
    }

    shader.use();

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    // glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    shader.set_mat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (size_t i = 0; i < text_to_render.size(); i++)
    {
        auto [text, coords, scale, color] = text_to_render[i];

        // glUniform3f(glGetUniformLocation(shader.ID, "text_color"), color.r, color.g, color.b);
        shader.set_vec4("text_color", glm::vec4(color.r, color.g, color.b, color.a));

        for (char c : text)
        {
            Character ch = characters[c];

            float xpos = coords[0] + ch.Bearing.x * scale;
            float ypos = coords[1] - (ch.Size.y - ch.Bearing.y) * scale;
            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            float vertices[6][4] = {
                {xpos, ypos + h, 0.0f, 0.0f},
                {xpos, ypos, 0.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 1.0f},
                {xpos, ypos + h, 0.0f, 0.0f},
                {xpos + w, ypos, 1.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 0.0f}};

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            coords[0] += (ch.Advance >> 6) * scale;
        }
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Clear for next frame
    // text_to_render.clear();
}
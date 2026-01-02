#include "engine/renderer/text_renderer.hpp"

Character::Character(unsigned int textureID, glm::ivec2 size, glm::ivec2 bearing, unsigned int advance)
    : TextureID(textureID), Size(size), Bearing(bearing), Advance(advance) {}

Text_Renderer::Text_Renderer()
{
    // toto je na test ci to realne funguje
    text_to_render = {
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZ", {0.0f, 350.0f}, 1, Color(255, 255, 255, 1.0f)},
        {"abcdefghijklmnopqrstuvwxyz", {0.0f, 400.0f}, 1, Color(255, 255, 255, 1.0f)},
        {"0123456789", {0.0f, 450.0f}, 1, Color(255, 0, 0, 1.0f)},
        {"!@#$%^&*()_+-=", {0.0f, 500.0f}, 1, Color(0, 255, 0, 1.0f)},
        {"[]{}|\\:;\"'<>?,./~`", {0.0f, 550.0f}, 1, Color(0, 0, 255, 0.5f)},
        {"MISKO POZOR ZITRA :-)", {0.0f, 600.0f}, 0.3, Color(0, 255, 125, 1.0f)}};
}

Text_Renderer::~Text_Renderer()
{
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void Text_Renderer::init()
{
    font_path = "../fonts/Minecraft.ttf";
    // font_path = "../fonts/EmojiFont.ttf";
    // font_path = "../fonts/MySims.ttf";
    // font_path = "../fonts/Coolvetica.otf";

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

    std::cout << "Font loaded successfully!" << std::endl;

    shader = std::make_unique<Shader>("../shaders/texture/text_vertex.glsl", "../shaders/texture/text_texture.glsl");
    // shader->create_shader();

    // shader2 = std::make_unique<Shader>("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    // shader2->create_shader();

    VAO = std::make_unique<VERTEX_ARRAY_OBJECT>();
    VAO->bind();

    VBO = std::make_unique<VERTEX_BUFFER_OBJECT>();
    VBO->bind();

    VAO->setup_vertex_attribute_pointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    VBO->fill_with_data_raw(sizeof(float) * 6 * 4, NULL, GL_DYNAMIC);

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
            return; // mozno by mal rovno skoncit lebo co ak nebudem vediet vykreslit napriklad 'S'
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

void Text_Renderer::render_text(std::string text, glm::vec2 coords, float scale, Color color)
{
    if (text.empty())
    {
        return;
    }

    shader->use();
    shader->set_mat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    VAO->bind();
    VBO->bind();

    // auto [text, coords, scale, color] = text_to_render[i];

    shader->set_vec4("text_color", glm::vec4(color.r, color.g, color.b, color.a));

    float x = coords[0];
    float y = coords[1];

    for (char c : text)
    {
        Character ch = characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y + (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            {xpos, ypos - h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos, ypos - h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos - h, 1.0f, 0.0f}};

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }

    // glBindVertexArray(0);
    // glBindTexture(GL_TEXTURE_2D, 0);
}

void Text_Renderer::clear_buffers()
{
}

void Text_Renderer::set_projection(glm::mat4 projection)
{
    this->projection = projection;
}
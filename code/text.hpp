#pragma once

#include <string_view>

#include <ft2build.h>
#include <freetype/freetype.h>

#include "core/vec2.hpp"

struct text_vertex
{
    core::vec2 position;
    core::vec2 texcoord;
};

struct text_character
{
    graphics::opengl::Texture texture;

    glm::ivec2 size;       // size of glyph
    glm::ivec2 bearing;    // offset from baseline to left/top of glyph
    unsigned int advance;  // horizontal offset to advance to next glyph
};

static std::map<GLchar, text_character> font_characters;

static auto render_text(const graphics::opengl::Buffer& vbo, std::string text, float x, float y, float scale, glm::vec3 color) -> void
{
    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c)
    {
        auto [texture, size, bearing, advance] = font_characters[*c];

        const auto xpos = x + bearing.x * scale;
        const auto ypos = y - bearing.y * scale;

        const auto w = size.x * scale;
        const auto h = size.y * scale;

        const float vertices[6][4]
        {
            { xpos,     ypos,      0.0f, 0.0f },
            { xpos,     ypos + h,  0.0f, 1.0f },
            { xpos + w, ypos + h,  1.0f, 1.0f },

            { xpos,     ypos,      0.0f, 0.0f },
            { xpos + w, ypos + h,  1.0f, 1.0f },
            { xpos + w, ypos,      1.0f, 0.0f }
        };

        texture.bind(0);

        vbo.subdata(0, sizeof(vertices), vertices);

        graphics::opengl::Commands::draw_vertices(GL_TRIANGLES, 0, 6);

        x += (advance >> 6) * scale;
    }
}

static auto load_font(std::string_view font_name) -> int32_t
{
    FT_Library ft_library;
    if (FT_Init_FreeType(&ft_library))
    {
        return  -1;
    }

    FT_Face ft_face;
    if (FT_New_Face(ft_library, font_name.data(), 0, &ft_face))
    {
        return  -1;
    }

    FT_Set_Pixel_Sizes(ft_face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER))
        {
            std::println("Failed to load Glyph");

            continue;
        }

        graphics::opengl::Texture texture;
        texture.type = GL_TEXTURE_2D;
        texture.create();
        texture.storage(ft_face->glyph->bitmap.width,ft_face->glyph->bitmap.rows, GL_R8, 1);
        texture.subdata(ft_face->glyph->bitmap.width,ft_face->glyph->bitmap.rows, GL_RED, 0, ft_face->glyph->bitmap.buffer);

        text_character character;
        character.texture = texture;
        character.size = glm::ivec2(ft_face->glyph->bitmap.width,ft_face->glyph->bitmap.rows);
        character.bearing = glm::ivec2(ft_face->glyph->bitmap_left,ft_face->glyph->bitmap_top);
        character.advance = static_cast<unsigned int>(ft_face->glyph->advance.x);

        font_characters.insert(std::make_pair(c, character));
    }

    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);

    return 0;
}
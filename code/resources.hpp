#pragma once

#include <cstdio>
#include <string_view>

#include <stb_image.h>

struct texture_data
{
    int32_t width;
    int32_t height;
    int32_t channels;

    uint8_t* pixels;
};

inline auto load_texture(std::string_view filename) -> texture_data
{
    texture_data data{};

    data.pixels = stbi_load(filename.data(), &data.width, &data.height, &data.channels, 0);

    if (data.pixels == nullptr)
    {
        printf("Failed to load texture '%s': %s\n", filename.data(), stbi_failure_reason());
    }

    return data;
}

inline auto load_textures(const std::vector<std::string_view>& filenames) -> std::vector<texture_data>
{
    std::vector<texture_data> data;

    for (const auto& filename : filenames)
    {
        data.emplace_back(load_texture(filename));
    }

    return data;
}

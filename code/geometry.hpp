#pragma once

#include <string_view>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "core/rect.hpp"

struct model_vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct sprite_vertex
{
    core::vec2 position;
    core::vec2 texcoord;
};

struct editor_vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

template <typename vertex_, typename element_> // TODO maybe use the C++ way with V, E?
struct geometry
{
    std::vector<vertex_>  vertices;
    std::vector<element_> elements;

    // TODO add some functions to be able to expend the geometry easy, something like an append

    auto append(const geometry& other) -> void
    {
        const auto offset = vertices.size();

        vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());

        for (const auto element : other.elements)
        {
            elements.emplace_back(element + offset);
        }
    }
};

// не загружает несколько мешей, просто берет первый.
inline auto load_geometry(std::string_view filename) -> geometry<model_vertex, uint32_t>
{
    Assimp::Importer importer;
    if (const auto scene = importer.ReadFile(filename.data(), 0); scene != nullptr)
    {
        geometry<model_vertex, uint32_t> geometry;

        const auto mesh = scene->mMeshes[0];

        for (auto i = 0; i < mesh->mNumVertices; i++)
        {
            const auto& position = mesh->mVertices[i];
            const auto& normal = mesh->mNormals[i];
            const auto& texcoord = mesh->mTextureCoords[0][i];

            geometry.vertices.emplace_back(model_vertex {{ position.x, position.y, position.z }, { normal.x, normal.y, normal.z }, { texcoord.x, texcoord.y } });
        }

        for (auto i = 0; i < mesh->mNumFaces; i++)
        {
            const auto& face = mesh->mFaces[i];
            for (auto j = 0; j < face.mNumIndices; j++)
            {
                geometry.elements.emplace_back(face.mIndices[j]);
            }
        }

        return geometry;
    }

    return {};
}

// TODO add a new version of create sprite which takes an x, y, w, h and some size fore the texture?

// TODO check if this flip_vertical parameter it makes what stbi_set_flip_vertically_on_load does for stb image on load
// flip_vertical = true  -> for textures stored bottom-up (e.g. framebuffer / render target textures)
// flip_vertical = false -> for textures loaded with stb_image (stored top-down)
inline auto create_sprite(const core::rect& position, const bool flip_vertical) -> geometry<sprite_vertex, uint32_t>
{
    const auto v_top    = flip_vertical ? 1.0f : 0.0f;
    const auto v_bottom = flip_vertical ? 0.0f : 1.0f;

    geometry<sprite_vertex, uint32_t> geometry
    {
        {
            { position.min, { 0.0f, v_top } },
            { { position.max.x, position.min.y }, { 1.0f, v_top } },
            {  position.max, { 1.0f, v_bottom } },
            { { position.min.x, position.max.y }, { 0.0f, v_bottom } }
        },
        {
            0, 2, 1,
            0, 3, 2
        }
    };

    return geometry;
}

static auto create_rectangle(const core::rect& position, const glm::vec3& color) -> geometry<editor_vertex, uint32_t>
{
    geometry<editor_vertex, uint32_t> geometry
    {
        // TODO adding the 0.5f padding in here? makes sense? check with the claude about this 0.5f to be sure
        {
            { { position.min.x, position.min.y, 0.0f }, color },
            { { position.max.x, position.min.y, 0.0f }, color },
            { { position.max.x, position.max.y, 0.0f }, color },
            { { position.min.x, position.max.y, 0.0f }, color }

        },
        {
            0, 1,
            1, 2,
            2, 3,
            3, 0
        }
    };

    return geometry;
}
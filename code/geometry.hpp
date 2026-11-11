#pragma once

#include <string_view>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "core/rect.hpp"

struct model_vertex
{
    glm::vec3 position;
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
            const auto& texcoord = mesh->mTextureCoords[0][i];

            geometry.vertices.emplace_back(model_vertex {glm::vec3{ position.x, position.y, position.z }, { texcoord.x, texcoord.y } });
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

inline auto create_sprite(const core::rect& position) -> geometry<sprite_vertex, uint32_t>
{
    geometry<sprite_vertex, uint32_t> geometry
    {
        {
            { position.min, { 0.0f, 1.0f } },
            { { position.max.x, position.min.y }, { 1.0f, 1.0f  } },
            { position.max, { 1.0f, 0.0f } },
            { { position.min.x, position.max.y }, { 0.0f, 0.0f } },
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
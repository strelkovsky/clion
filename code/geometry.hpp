#pragma once

#include <string_view>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

struct model_vertex
{
    glm::vec3 position;
    glm::vec2 texcoord;
};

struct sprite_vertex
{
    glm::vec2 position;
    glm::vec2 texcoord;
};

template <typename vertex>
struct geometry
{
    std::vector<vertex> vertices;
    std::vector<uint32_t> elements;
};

// не загружает несколько мешей, просто берет первый.
inline auto load_geometry(std::string_view filename) -> geometry<model_vertex>
{
    Assimp::Importer importer;
    if (const auto scene = importer.ReadFile(filename.data(), 0); scene != nullptr)
    {
        geometry<model_vertex> geometry;

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
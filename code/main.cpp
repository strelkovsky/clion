#include <glad/glad.h>

#include <stb_image.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <btBulletCollisionCommon.h>

#include <print>

#include "backends/glfw/window_factory.hpp"

int main()
{
    const auto window_factory = std::make_unique<backends::glfw::WindowFactory>();

    const auto window = window_factory->create_window();
    window->create("Hello World", 640, 480);

    gladLoadGL();

    Assimp::Importer importer;

    if (const aiScene* scene = importer.ReadFile("test_scene.obj", 0); scene != nullptr)
    {
        std::println("Scene loading...");
    }

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    while (!window->is_closing)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        window->update();
    }

    window->destroy();

    return 0;
}

#include <glad/glad.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <print>

#include "backends/glfw/window_factory.hpp"

#include "core/window_manager.hpp"

auto main() -> int32_t
{
    const std::unique_ptr<core::base::WindowFactory> window_factory = std::make_unique<backends::glfw::WindowFactory>();

    //const auto window = window_factory->create_window();
    //window->create({.name = "Hello World", .width = 640, .height = 480});

    core::WindowManager window_manager;
    window_manager.init(window_factory);

    const auto window = window_manager.window();

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
        window_manager.update();
    }

    window->destroy();

    return 0;
}

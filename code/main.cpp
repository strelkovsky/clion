#include <glad/glad.h>
#include <glm/glm.hpp>
#include <print>

#include "backends/glfw/window_factory.hpp"

#include "core/window_manager.hpp"
#include "core/file.hpp"

#include "graphics/opengl/buffer.hpp"
#include "graphics/opengl/vertex_array.hpp"
#include "graphics/opengl/commands.hpp"
#include "graphics/opengl/pipeline.hpp"
#include "graphics/opengl/shader_stage.hpp"
#include "graphics/opengl/shader_program.hpp"

namespace
{
    struct vertex
    {
        glm::vec2 pos;
        glm::vec3 col;
    };
}

static constexpr vertex geometry_vertices[3] =
{
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
};

static constexpr uint32_t geometry_elements[3] =
{
    0, 1, 2
};

auto main() -> int32_t
{
    const std::unique_ptr<core::base::WindowFactory> window_factory = std::make_unique<backends::glfw::WindowFactory>();

    core::WindowManager window_manager;
    window_manager.init(window_factory);

    const auto window = window_manager.window();
    const auto window_events = window_manager.window_events();

    bool is_closing = false;

    window_events->callbacks.on_close = [&]
    {
        is_closing = true;
    };

    window_events->callbacks.on_resize = [](uint32_t width, uint32_t height)
    {
        std::println("resize {} {}", width, height);
    };

    gladLoadGL();

    graphics::opengl::ShaderStage vertex_shader;
    vertex_shader.type = GL_VERTEX_SHADER;
    vertex_shader.create();
    vertex_shader.source(core::File::read("base_vert_shader.glsl"));

    graphics::opengl::ShaderStage fragment_shader;
    fragment_shader.type = GL_FRAGMENT_SHADER;
    fragment_shader.create();
    fragment_shader.source(core::File::read("base_frag_shader.glsl"));

    graphics::opengl::ShaderProgram program_shader;
    program_shader.create();
    program_shader.attach(vertex_shader);
    program_shader.attach(fragment_shader);
    program_shader.link();

    graphics::opengl::Buffer geometry_vbo;
    geometry_vbo.create();
    geometry_vbo.data(sizeof(geometry_vertices), geometry_vertices);

    graphics::opengl::Buffer geometry_ebo;
    geometry_ebo.create();
    geometry_ebo.data(sizeof(geometry_elements), geometry_elements);

    graphics::opengl::VertexArray geometry_vao;
    geometry_vao.create();

    geometry_vao.attach_vertices(geometry_vbo, sizeof(vertex));
    geometry_vao.attach_elements(geometry_ebo);

    geometry_vao.attach_attribute({0, offsetof(vertex, pos), 2, GL_FLOAT, 0});
    geometry_vao.attach_attribute({1, offsetof(vertex, col), 3, GL_FLOAT,  0});

    graphics::opengl::Commands::clear(0.5f, 0.5f, 0.5f, 1.0f);

    graphics::opengl::Pipeline::enable(GL_MULTISAMPLE);

    while (!is_closing)
    {
        graphics::opengl::Commands::clear(GL_COLOR_BUFFER_BIT);

        program_shader.bind();

        geometry_vao.bind();

        //graphics::opengl::Commands::draw_vertices(GL_TRIANGLES, 0, 3);
        graphics::opengl::Commands::draw_elements(GL_TRIANGLES, 0, 3);

        window_manager.update();
    }

    geometry_vbo.destroy();
    geometry_ebo.destroy();
    geometry_vao.destroy();

    vertex_shader.destroy();
    fragment_shader.destroy();

    program_shader.destroy();

    window->destroy();

    return 0;
}

#include <print>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glfw/glfw3.h>

#include <stb_image.h>

#include "backends/glfw/window_factory.hpp"

#include "core/window_manager.hpp"
#include "core/file.hpp"

#include "graphics/opengl/buffer.hpp"
#include "graphics/opengl/vertex_array.hpp"
#include "graphics/opengl/commands.hpp"
#include "graphics/opengl/pipeline.hpp"
#include "graphics/opengl/shader_stage.hpp"
#include "graphics/opengl/shader_program.hpp"
#include "graphics/opengl/texture_sampler.hpp"
#include "graphics/opengl/texture.hpp"

#include "geometry.hpp"

static geometry<sprite_vertex> sprite_geometry
{
    {
        sprite_vertex { { -100.0f, -100.0f }, { 0.0f, 1.0f } },
        sprite_vertex { {  100.0f, -100.0f }, { 1.0f, 1.0f  } },
        sprite_vertex { {  100.0f,  100.0f }, { 1.0f, 0.0f, } },
        sprite_vertex { { -100.0f,  100.0f }, { 0.0f, 0.0f } },
    },
    {
        0, 2, 1,
        0, 3, 2
    }
};

auto main() -> int32_t
{
    constexpr core::window::configuration window_configuration
    {
        "Hello World",1024, 768, 4
    };

    const std::unique_ptr<core::base::WindowFactory> window_factory = std::make_unique<backends::glfw::WindowFactory>();

    core::WindowManager window_manager;
    window_manager.init(window_factory, window_configuration);

    const auto window = window_manager.window();
    const auto window_events = window_manager.window_events();

    auto window_width = window_configuration.width;
    auto window_height = window_configuration.height;

    auto window_is_running = true;

    window_events->callbacks.on_close = [&]
    {
        window_is_running = false;
    };

    gladLoadGL();

    graphics::opengl::ShaderStage model_vertex_stage;
    model_vertex_stage.create(GL_VERTEX_SHADER);
    model_vertex_stage.source(core::File::read("model_vert_shader.glsl"));

    graphics::opengl::ShaderStage model_fragment_stage;
    model_fragment_stage.create(GL_FRAGMENT_SHADER);
    model_fragment_stage.source(core::File::read("model_frag_shader.glsl"));

    graphics::opengl::ShaderStage sprite_vertex_stage;
    sprite_vertex_stage.create(GL_VERTEX_SHADER);
    sprite_vertex_stage.source(core::File::read("sprite_vert_shader.glsl"));

    graphics::opengl::ShaderStage sprite_fragment_stage;
    sprite_fragment_stage.create(GL_FRAGMENT_SHADER);
    sprite_fragment_stage.source(core::File::read("sprite_frag_shader.glsl"));

    graphics::opengl::ShaderProgram model_program_shader;
    model_program_shader.create();
    model_program_shader.attach(model_vertex_stage);
    model_program_shader.attach(model_fragment_stage);
    model_program_shader.link();

    graphics::opengl::ShaderProgram sprite_program_shader;
    sprite_program_shader.create();
    sprite_program_shader.attach(sprite_vertex_stage);
    sprite_program_shader.attach(sprite_fragment_stage);
    sprite_program_shader.link();

    model_vertex_stage.destroy();
    model_fragment_stage.destroy();

    sprite_vertex_stage.destroy();
    sprite_fragment_stage.destroy();

    const auto [geometry_vertices, geometry_elements] = load_geometry("cube.obj");

    graphics::opengl::Buffer cube_geometry_vbo;
    cube_geometry_vbo.create();
    cube_geometry_vbo.data(sizeof(model_vertex) * geometry_vertices.size(), geometry_vertices.data());

    graphics::opengl::Buffer cube_geometry_ebo;
    cube_geometry_ebo.create();
    cube_geometry_ebo.data(sizeof(uint32_t) * geometry_elements.size(), geometry_elements.data());

    graphics::opengl::VertexArray cube_geometry_vao;
    cube_geometry_vao.create();

    cube_geometry_vao.attach_vertices(cube_geometry_vbo, sizeof(model_vertex));
    cube_geometry_vao.attach_elements(cube_geometry_ebo);

    cube_geometry_vao.attach_attribute({0, offsetof(model_vertex, position), 3, GL_FLOAT, 0});
    cube_geometry_vao.attach_attribute({1, offsetof(model_vertex, texcoord), 2, GL_FLOAT,  0});

    graphics::opengl::Buffer sprite_geometry_vbo;
    sprite_geometry_vbo.create();
    sprite_geometry_vbo.data(sizeof(sprite_vertex) * sprite_geometry.vertices.size(), sprite_geometry.vertices.data());

    graphics::opengl::Buffer sprite_geometry_ebo;
    sprite_geometry_ebo.create();
    sprite_geometry_ebo.data(sizeof(uint32_t) * sprite_geometry.elements.size(), sprite_geometry.elements.data());

    graphics::opengl::VertexArray sprite_geometry_vao;
    sprite_geometry_vao.create();
    sprite_geometry_vao.attach_vertices(sprite_geometry_vbo, sizeof(sprite_vertex));
    sprite_geometry_vao.attach_elements(sprite_geometry_ebo);

    sprite_geometry_vao.attach_attribute({0, offsetof(sprite_vertex, position), 2, GL_FLOAT, 0});
    sprite_geometry_vao.attach_attribute({1, offsetof(sprite_vertex, texcoord), 2, GL_FLOAT,  0});

    glm::mat4 model_camera_matrices[2];
    model_camera_matrices[0] = glm::lookAt(glm::vec3{ 0.0f, 0.0f, 5.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.f, 0.0f });
    model_camera_matrices[1] = glm::perspective(glm::radians(45.0f), static_cast<float>(window_width) / static_cast<float>(window_height), 0.3f, 100.0f);

    glm::mat4 sprite_camera_matrices[2];
    sprite_camera_matrices[0] = glm::mat4(1.0f);
    sprite_camera_matrices[1] = glm::ortho(0.0f, static_cast<float>(window_width), static_cast<float>(window_height), 0.0f, 0.0f, 1.0f);

    graphics::opengl::Buffer camera_ubo;
    camera_ubo.create();
    camera_ubo.bind(GL_UNIFORM_BUFFER, 0);
    camera_ubo.data(sizeof(model_camera_matrices), model_camera_matrices);

    glm::mat4 model_matrix {1.0f};
    glm::mat4 sprite_matrix {1.0f};

    graphics::opengl::Buffer transforms_ubo;
    transforms_ubo.create();
    transforms_ubo.bind(GL_UNIFORM_BUFFER, 1);
    transforms_ubo.data(sizeof(model_matrix), &model_matrix);

    // идея семплера - вам не нужно указывать для каждой текстуры какой фильтр использовать.
    graphics::opengl::TextureSampler texture_sampler;
    texture_sampler.create();
    texture_sampler.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture_sampler.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture_sampler.bind(0);

    int32_t image_width;
    int32_t image_height;
    int32_t image_channels;
    const auto image_data = stbi_load("pavement_albedo.png", &image_width, &image_height, &image_channels, 0);

    graphics::opengl::Texture brick_texture;
    brick_texture.type = GL_TEXTURE_2D;
    brick_texture.create();

    brick_texture.storage(image_width, image_height, GL_RGB8, 1);
    brick_texture.subdata(image_width, image_height, GL_RGB, 0, image_data);

    brick_texture.bind(0);

    window_events->callbacks.on_resize = [&](uint32_t width, uint32_t height)
    {
        window_width = width;
        window_height = height;
    };

    graphics::opengl::Commands::clear(0.5f, 0.5f, 0.5f, 1.0f);

    graphics::opengl::Pipeline::enable(GL_MULTISAMPLE);
    graphics::opengl::Pipeline::enable(GL_DEPTH_TEST);

    while (window_is_running)
    {
        // update

        model_matrix = glm::rotate(glm::mat4(1.0f), static_cast<float>(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f } );
        sprite_matrix = glm::translate(glm::mat4(1.0f), { 110.0f, 110.0f, 0.0f } );

        // draw

        graphics::opengl::Commands::viewport(0, 0, window_width, window_height);
        graphics::opengl::Commands::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // models

        model_camera_matrices[1] = glm::perspective(glm::radians(45.0f), static_cast<float>(window_width) / static_cast<float>(window_height), 0.3f, 100.0f);
        camera_ubo.subdata(0, sizeof(model_camera_matrices), model_camera_matrices);

        model_program_shader.bind();

        cube_geometry_vao.bind();

        transforms_ubo.subdata(0, sizeof(model_matrix), &model_matrix);

        graphics::opengl::Commands::draw_elements(GL_TRIANGLES, 0, geometry_elements.size());

        // sprites

        sprite_camera_matrices[1] = glm::ortho(0.0f, static_cast<float>(window_width), static_cast<float>(window_height), 0.0f, 0.0f, 1.0f);
        camera_ubo.subdata(0, sizeof(sprite_camera_matrices), sprite_camera_matrices);

        sprite_program_shader.bind();

        sprite_geometry_vao.bind();

        transforms_ubo.subdata(0, sizeof(sprite_matrix), &sprite_matrix);

        graphics::opengl::Commands::draw_elements(GL_TRIANGLES, 0, sprite_geometry.elements.size());

        window_manager.update();
    }

    brick_texture.destroy();
    texture_sampler.destroy();
    cube_geometry_vbo.destroy();
    cube_geometry_ebo.destroy();
    cube_geometry_vao.destroy();

    model_program_shader.destroy();

    window->destroy();

    return 0;
}

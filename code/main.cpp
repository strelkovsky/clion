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

    graphics::opengl::ShaderStage vertex_shader;
    vertex_shader.create(GL_VERTEX_SHADER);
    vertex_shader.source(core::File::read("model_vert_shader.glsl"));

    graphics::opengl::ShaderStage fragment_shader;
    fragment_shader.create(GL_FRAGMENT_SHADER);
    fragment_shader.source(core::File::read("model_frag_shader.glsl"));

    graphics::opengl::ShaderProgram program_shader;
    program_shader.create();
    program_shader.attach(vertex_shader);
    program_shader.attach(fragment_shader);
    program_shader.link();

    const auto [geometry_vertices, geometry_elements] = load_geometry("cube.obj");

    graphics::opengl::Buffer geometry_vbo;
    geometry_vbo.create();
    geometry_vbo.data(sizeof(vertex) * geometry_vertices.size(), geometry_vertices.data());

    graphics::opengl::Buffer geometry_ebo;
    geometry_ebo.create();
    geometry_ebo.data(sizeof(uint32_t) * geometry_elements.size(), geometry_elements.data());

    graphics::opengl::VertexArray geometry_vao;
    geometry_vao.create();

    geometry_vao.attach_vertices(geometry_vbo, sizeof(vertex));
    geometry_vao.attach_elements(geometry_ebo);

    geometry_vao.attach_attribute({0, offsetof(vertex, position), 3, GL_FLOAT, 0});
    geometry_vao.attach_attribute({1, offsetof(vertex, texcoord), 2, GL_FLOAT,  0});

    glm::mat4 camera_matrices[2];
    camera_matrices[0] = glm::lookAt(glm::vec3{ 0.0f, 0.0f, 5.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.f, 0.0f });
    camera_matrices[1] = glm::perspective(glm::radians(45.0f), static_cast<float>(window_width) / static_cast<float>(window_height), 0.3f, 100.0f);

    graphics::opengl::Buffer camera_ubo;
    camera_ubo.create();
    camera_ubo.bind(GL_UNIFORM_BUFFER, 0);
    camera_ubo.data(sizeof(camera_matrices), camera_matrices);

    glm::mat4 model_matrix {1.0f};

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

        const auto projection = glm::perspective(glm::radians(45.0f), static_cast<float>(window_width) / static_cast<float>(window_height), 0.3f, 100.0f);
        camera_ubo.subdata(sizeof(glm::mat4), sizeof(glm::mat4), &projection);
    };

    graphics::opengl::Commands::clear(0.5f, 0.5f, 0.5f, 1.0f);

    graphics::opengl::Pipeline::enable(GL_MULTISAMPLE);
    graphics::opengl::Pipeline::enable(GL_DEPTH_TEST);

    while (window_is_running)
    {
        // update

        model_matrix = glm::rotate(glm::mat4(1.0f), static_cast<float>(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f } );

        // draw

        graphics::opengl::Commands::viewport(0, 0, window_width, window_height);
        graphics::opengl::Commands::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program_shader.bind();

        geometry_vao.bind();

        transforms_ubo.subdata(0, sizeof(model_matrix), &model_matrix);
        //glNamedBufferSubData(transforms_ubo.handle, 0, sizeof(model_matrix), &model_matrix);

        //graphics::opengl::Commands::draw_vertices(GL_TRIANGLES, 0, 3);
        graphics::opengl::Commands::draw_elements(GL_TRIANGLES, 0, geometry_elements.size());

        window_manager.update();
    }

    brick_texture.destroy();
    texture_sampler.destroy();
    geometry_vbo.destroy();
    geometry_ebo.destroy();
    geometry_vao.destroy();

    vertex_shader.destroy();
    fragment_shader.destroy();

    program_shader.destroy();

    window->destroy();

    return 0;
}

#include <print>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glfw/glfw3.h>

#include <stb_image_write.h>
#include <stb_rect_pack.h>

#include "backends/glfw/window_factory.hpp"

#include "core/window_manager.hpp"
#include "core/file.hpp"
#include "core/rgba.hpp"

#include "graphics/opengl/buffer.hpp"
#include "graphics/opengl/commands.hpp"
#include "graphics/opengl/framebuffer.hpp"
#include "graphics/opengl/pipeline.hpp"
#include "graphics/opengl/renderbuffer.hpp"
#include "graphics/opengl/shader_stage.hpp"
#include "graphics/opengl/shader_program.hpp"
#include "graphics/opengl/texture_sampler.hpp"
#include "graphics/opengl/texture.hpp"
#include "graphics/opengl/vertex_array.hpp"

#include "geometry.hpp"
#include "resources.hpp"

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

    graphics::opengl::ShaderStage basic_vertex_stage;
    basic_vertex_stage.create(GL_VERTEX_SHADER);
    basic_vertex_stage.source(core::File::read("basic_vert_shader.glsl"));

    graphics::opengl::ShaderStage basic_fragment_stage;
    basic_fragment_stage.create(GL_FRAGMENT_SHADER);
    basic_fragment_stage.source(core::File::read("basic_frag_shader.glsl"));

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

    graphics::opengl::ShaderProgram basic_program_shader;
    basic_program_shader.create();
    basic_program_shader.attach(basic_vertex_stage);
    basic_program_shader.attach(basic_fragment_stage);
    basic_program_shader.link();

    model_vertex_stage.destroy();
    model_fragment_stage.destroy();

    sprite_vertex_stage.destroy();
    sprite_fragment_stage.destroy();

    basic_vertex_stage.destroy();
    basic_fragment_stage.destroy();

    const auto [geometry_vertices, geometry_elements] = load_geometry("cube.obj");

    graphics::opengl::Buffer cube_geometry_vbo;
    cube_geometry_vbo.create();
    cube_geometry_vbo.storage(sizeof(model_vertex) * geometry_vertices.size(), geometry_vertices.data(), 0);

    graphics::opengl::Buffer cube_geometry_ebo;
    cube_geometry_ebo.create();
    cube_geometry_ebo.storage(sizeof(uint32_t) * geometry_elements.size(), geometry_elements.data(), 0);

    graphics::opengl::VertexArray cube_geometry_vao;
    cube_geometry_vao.create();

    cube_geometry_vao.attach_vertices(cube_geometry_vbo, sizeof(model_vertex));
    cube_geometry_vao.attach_elements(cube_geometry_ebo);

    cube_geometry_vao.attach_attribute({0, offsetof(model_vertex, position), 3, GL_FLOAT, 0});
    cube_geometry_vao.attach_attribute({1, offsetof(model_vertex, texcoord), 2, GL_FLOAT,  0});

    const auto [sprite_vertices, sprite_elements] = create_sprite({0, 0, 256.0f, 256.0f}, true);

    graphics::opengl::Buffer sprite_geometry_vbo;
    sprite_geometry_vbo.create();
    sprite_geometry_vbo.storage(sizeof(sprite_vertex) * sprite_vertices.size(), sprite_vertices.data(), 0);

    graphics::opengl::Buffer sprite_geometry_ebo;
    sprite_geometry_ebo.create();
    sprite_geometry_ebo.storage(sizeof(uint32_t) * sprite_elements.size(), sprite_elements.data(), 0);

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
    camera_ubo.storage(sizeof(model_camera_matrices), model_camera_matrices, GL_DYNAMIC_STORAGE_BIT);

    glm::mat4 model_matrix {1.0f};
    glm::mat4 sprite_matrix {1.0f};

    graphics::opengl::Buffer transform_ubo;
    transform_ubo.create();
    transform_ubo.bind(GL_UNIFORM_BUFFER, 1);
    transform_ubo.storage(sizeof(model_matrix), &model_matrix, GL_DYNAMIC_STORAGE_BIT);

    // идея семплера - вам не нужно указывать для каждой текстуры какой фильтр использовать.
    graphics::opengl::TextureSampler texture_sampler;
    texture_sampler.create();
    texture_sampler.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture_sampler.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture_sampler.bind(0);

    texture_data brick_image = load_texture("textures/pavement_albedo.png");

    graphics::opengl::Texture brick_texture;
    brick_texture.type = GL_TEXTURE_2D;
    brick_texture.create();

    brick_texture.storage(brick_image.width, brick_image.height, GL_RGB8, 1);
    brick_texture.subdata(brick_image.width, brick_image.height, GL_RGB, 0, brick_image.pixels);

    brick_texture.bind(0);

    // rectangles packing - begin

    auto images_data = load_textures({ "textures/body/blue_circle.png", "textures/body/blue_square.png", "textures/body/blue_rhombus.png", "textures/body/blue_squircle.png" });

    stbrp_context context;

    auto atlas_width = 1024;
    auto atlas_height = 1024;

    constexpr auto atlas_nodes_count = 1024;
    stbrp_node atlas_nodes[atlas_nodes_count];

    stbrp_init_target(&context, atlas_width, atlas_height, atlas_nodes, atlas_nodes_count);

    constexpr auto rectangles_count = 4;
    stbrp_rect rectangles[rectangles_count] {};

    for (auto i = 0; i < images_data.size(); ++i)
    {
        const auto& image_data = images_data[i];

        rectangles[i].w = image_data.width;
        rectangles[i].h = image_data.height;
    }

    auto all_packed = stbrp_pack_rects(&context, rectangles, images_data.size());

    printf("Packing status: %s\n\n", all_packed ? "All packed successfully!" : "Some rectangles did not fit.");

    geometry<sprite_vertex, uint32_t> rectangles_geometry;

    for (auto i = 0; i < images_data.size(); ++i)
    {
        if (rectangles[i].was_packed)
        {
            printf("Rect : Positioned at (%d, %d) [Size: %dx%d]\n", rectangles[i].x, rectangles[i].y, rectangles[i].w, rectangles[i].h);

            rectangles_geometry.append(create_sprite({ 0, 0,  static_cast<float>(rectangles[i].w), static_cast<float>(rectangles[i].h) }, false));
        }
        else
        {
            printf("Rect : FAILED to pack [Size: %dx%d]\n", rectangles[i].w, rectangles[i].h);
        }
    }

    // rectangles packing - end

    // framebuffer - begin

    graphics::opengl::framebuffer framebuffer;
    framebuffer.create();
    framebuffer.bind(); // TODO в DSA вроде не нужно

    graphics::opengl::Texture framebuffer_texture;
    framebuffer_texture.type = GL_TEXTURE_2D;
    framebuffer_texture.create();
    framebuffer_texture.storage(atlas_width, atlas_height, GL_RGBA8, 1);

    //graphics::opengl::Renderbuffer renderbuffer;
    //renderbuffer.create();
    //renderbuffer.storage(atlas_width, atlas_height, GL_DEPTH_COMPONENT24);

    //glNamedFramebufferTexture(framebuffer, framebuffer_texture.handle, framebuffer_texture.handle, 0);
    // сюда отрисуется только ColorBit
    framebuffer.attach(framebuffer_texture,GL_COLOR_ATTACHMENT0, 0);
    // а сюда только DepthBit
    //framebuffer.attach(renderbuffer,GL_DEPTH_ATTACHMENT);

    // для работы буфера кадров нужно что-то к нему прикрепить,
    // он должен куда-то выводить изображение (какой-то контейнер, учакток памяти, чаще всего - текстуры).
    // а еще есть т.н. буфер рендеринга. Разница между ним и текстурой в том, что текстуру можно увидеть,
    // прикрепить к другому объекту, или сделать с ней что угодно. А буфер рендеринга - это то, чего вы не
    // видите, это просто часть буфера кадров. Допустим, буфер глубины может быть чем-то вроде буфера рендеринга.


    // framebuffer - end

    // pixel buffer object

    graphics::opengl::Buffer framebuffer_pbo;
    framebuffer_pbo.create();
    //glNamedBufferStorage(framebuffer_pbo.handle, atlas_width * atlas_height * 4, nullptr, GL_MAP_READ_BIT);
    framebuffer_pbo.storage(atlas_width * atlas_height * 4, nullptr, GL_MAP_READ_BIT);
    framebuffer_pbo.bind(GL_PIXEL_PACK_BUFFER);

    // rectangles

    graphics::opengl::Buffer rectangles_vbo;
    rectangles_vbo.create();
    rectangles_vbo.storage(sizeof(sprite_vertex) * rectangles_geometry.vertices.size(), rectangles_geometry.vertices.data(), 0);

    graphics::opengl::Buffer rectangles_ebo;
    rectangles_ebo.create();
    rectangles_ebo.storage(sizeof(uint32_t) * rectangles_geometry.elements.size(), rectangles_geometry.elements.data(), 0);

    graphics::opengl::VertexArray rectangles_vao;
    rectangles_vao.create();

    rectangles_vao.attach_vertices(rectangles_vbo, sizeof(sprite_vertex));
    rectangles_vao.attach_elements(rectangles_ebo);

    rectangles_vao.attach_attribute({ 0, offsetof(sprite_vertex, position), 2, GL_FLOAT, 0 });
    rectangles_vao.attach_attribute({ 1, offsetof(sprite_vertex, texcoord), 2, GL_FLOAT, 0 });

    window_events->callbacks.on_resize = [&](uint32_t width, uint32_t height)
    {
        window_width = width;
        window_height = height;
    };

    core::rgba framebuffer_color { 0.0f, 0.0f, 0.0f, 0.0f };

    graphics::opengl::Commands::clear(0.5f, 0.5f, 0.5f, 1.0f);

    graphics::opengl::Pipeline::enable(GL_MULTISAMPLE);
    //graphics::opengl::Pipeline::enable(GL_DEPTH_TEST);

    // atlas output begin

    framebuffer.clear(&framebuffer_color.r, 0);
    //framebuffer.clear(1.0f);
    framebuffer.bind();

    graphics::opengl::Commands::viewport(0, 0, atlas_width, atlas_height);
    // тут вызов не нужен, тк мы здесь рендерим в забинженный фреймбуффер, в котором уже установлены цвет и дептх.
    //graphics::opengl::Commands::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    graphics::opengl::Pipeline::enable(GL_BLEND);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //лучше использовать отдельную функцию:
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    // TODO use the glBlendFuncSeparatei seems to work directly with the framebuffer?
    // TODO check the glBlendFunc if is still usable or not

    // rectangles

    sprite_camera_matrices[1] = glm::ortho(0.0f, static_cast<float>(atlas_width), static_cast<float>(atlas_height), 0.0f, 0.0f, 1.0f);
    camera_ubo.subdata(0, sizeof(sprite_camera_matrices), sprite_camera_matrices);

    sprite_program_shader.bind();

    rectangles_vao.bind();

    for (auto i = 0; i < rectangles_count; ++i)
    {
        auto& rectangle = rectangles[i];

        // TODO remove it if needed later on
        auto parent_matrix = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 0.0f });

        auto rectangle_matrix = glm::translate(parent_matrix, { rectangle.x, rectangle.y, 0.0f });
        transform_ubo.subdata(0, sizeof(glm::mat4), &rectangle_matrix);

        graphics::opengl::Texture rectangle_texture;
        rectangle_texture.type = GL_TEXTURE_2D;
        rectangle_texture.create();
        rectangle_texture.storage(rectangle.w, rectangle.h, GL_RGBA8, 1);
        rectangle_texture.subdata(rectangle.w, rectangle.h, GL_RGBA, 0, images_data[i].pixels);
        rectangle_texture.bind(0);

        graphics::opengl::Commands::draw_elements(GL_TRIANGLES, i * 6, 6);

        rectangle_texture.destroy();
    }

    auto channels = 4;
    auto buffer_size = atlas_width * atlas_height * channels;

    graphics::opengl::Commands::read_piexels(0, 0, atlas_width, atlas_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer_size);

    if (auto* pixels = static_cast<uint8_t*>(framebuffer_pbo.map(0, buffer_size, GL_MAP_READ_BIT)))
    {
        stbi_write_png("sprite_atlas.png",
                   atlas_width, atlas_height, channels,
                   pixels + atlas_width * channels * (atlas_height - 1),
                   -atlas_width * channels);

        framebuffer_pbo.unmap();
    }

    // atlas output end

    // у этого фреймбуффера будет handel = 0, что будет эквивалентно bind(0)
    graphics::opengl::framebuffer default_fb;
    default_fb.bind();

    while (window_is_running)
    {
        // update

        model_matrix = glm::rotate(glm::mat4(1.0f), static_cast<float>(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f } );
        sprite_matrix = glm::translate(glm::mat4(1.0f), { 10.0f, 10.0f, 0.0f } );

        // clearing

        graphics::opengl::Commands::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        graphics::opengl::Commands::viewport(0, 0, window_width, window_height);

        // 3D drawing

        graphics::opengl::Pipeline::enable(GL_DEPTH_TEST);

        // models

        model_camera_matrices[1] = glm::perspective(glm::radians(45.0f), static_cast<float>(window_width) / static_cast<float>(window_height), 0.3f, 100.0f);
        camera_ubo.subdata(0, sizeof(model_camera_matrices), model_camera_matrices);

        model_program_shader.bind();

        brick_texture.bind(0);

        cube_geometry_vao.bind();

        transform_ubo.subdata(0, sizeof(model_matrix), &model_matrix);

        graphics::opengl::Commands::draw_elements(GL_TRIANGLES, 0, geometry_elements.size());

        // 2D drawing

        graphics::opengl::Pipeline::disable(GL_DEPTH_TEST);

        // sprites

        sprite_camera_matrices[1] = glm::ortho(0.0f, static_cast<float>(window_width), static_cast<float>(window_height), 0.0f, 0.0f, 1.0f);
        camera_ubo.subdata(0, sizeof(sprite_camera_matrices), sprite_camera_matrices);

        sprite_program_shader.bind();

        framebuffer_texture.bind(0);

        sprite_geometry_vao.bind();

        transform_ubo.subdata(0, sizeof(sprite_matrix), &sprite_matrix);

        graphics::opengl::Commands::draw_elements(GL_TRIANGLES, 0, sprite_elements.size());

        window_manager.update();
    }

    brick_texture.destroy();
    texture_sampler.destroy();

    cube_geometry_vao.destroy();
    sprite_geometry_vao.destroy();

    cube_geometry_vbo.destroy();
    cube_geometry_ebo.destroy();
    sprite_geometry_vbo.destroy();
    sprite_geometry_ebo.destroy();

    model_program_shader.destroy();
    sprite_program_shader.destroy();

    window->destroy();

    return 0;
}

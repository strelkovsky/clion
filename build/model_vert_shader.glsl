#version 430

uniform mat4 MVP;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texcoord;

layout (location = 0) out vec2 out_texcoord;

layout (binding = 0, std140) uniform camera_ubo
{
    mat4 view;
    mat4 projection;
};

layout (binding = 1, std140) uniform transform_ubo
{
    mat4 model;
    // TODO mat4 normal - матрица нормалей для освещения
};

void main()
{
    gl_Position = projection * view * model * vec4(in_position, 1.0);
    out_texcoord = in_texcoord;
};
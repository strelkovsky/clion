#version 430

layout (location = 0) in vec2 in_texcoord;

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform sampler2D u_albedo_texture;

void main()
{
    float red = texture(u_albedo_texture, in_texcoord).r; // TODO maybe call it alpha?

    out_color = vec4(1.0, 1.0, 1.0, red);
}
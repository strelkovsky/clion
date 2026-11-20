#version 430

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec3 in_position;

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform sampler2D u_albedo_texture;

layout (binding = 3, std140) uniform light_ubo
{
    vec3 color;
    vec3 position;
} u_light;

void main()
{
    // это важно. Например, если вы находитесь в темной комнате, и этот коэффициент равен нулю, то все объекты,
    // на которые не падает прямой свет будут черными. При 0.1 даже если на поверхность не падает свет, она будет видна.
    float ambient_factor = 0.1;
    vec3 ambient_color = ambient_factor * u_light.color;

    vec3 normal = normalize(in_normal);
    vec3 light_direction = normalize(u_light.position - in_position); // откуда исходит свет

    // max нужен, потому что скалярное произведение может вернуть от -1 до 1.
    float diffuse_factor = max(dot(normal, light_direction), 0.0);
    vec3 diffuse_color = diffuse_factor * u_light.color;

    out_color = vec4(ambient_color + diffuse_color, 1.0) * texture(u_albedo_texture, in_texcoord);
}
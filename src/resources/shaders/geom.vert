#version 460

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec4 color;

layout(push_constant) uniform Globals {
    mat4 model;
    mat4 view;
    mat4 projection;
};

layout(set = 0, binding = 0) buffer Debug {
    vec4 positions[];
} debug;


layout(location = 0) out struct {
    vec3 normal;
} vs_out;

void main() {
    vs_out.normal = normal;
    debug.positions[gl_VertexIndex] = position;

    gl_Position = projection * view * model * position;
}

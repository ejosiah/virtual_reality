#version 460

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec4 color;

layout(set = 0, binding = 0) uniform Globals {
    mat4 view;
    mat4 projection;
};

layout(set = 0, binding = 1) buffer GEOM_XFORMS {
    mat4 transform[];
};

layout(location = 0) out struct {
    vec3 normal;
} vs_out;

void main() {
    mat4 model = transform[gl_InstanceIndex];
    vs_out.normal = mat3(model) * normal;

    gl_Position = projection * view * model * position;
}

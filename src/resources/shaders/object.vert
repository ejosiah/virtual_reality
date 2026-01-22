#version 460

#extension GL_EXT_multiview : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec4 color;
layout(location = 5) in vec2 uv;

struct Camera {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(set = 0, binding = 0) uniform CameraSSBO {
    Camera cameras[2];
};

layout(location = 0) out struct {
    vec4 color;
    vec3 worldPos;
    vec3 normal;
    vec2 uv;
} vs_out;

void main() {
    Camera cam = cameras[gl_ViewIndex];

    vec4 worldPos = cam.model * position;

    vs_out.color = color;
    vs_out.normal = normal;
    vs_out.uv = uv;
    vs_out.worldPos = worldPos.xyz;

    gl_Position = cam.proj * cam.view * worldPos;
}
#version 460

#extension GL_EXT_multiview : enable

#include "common.glsl"

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec4 color;
layout(location = 5) in vec2 uv;
layout(location = 6) in int matId;


layout(set = 0, binding = 0, scalar) uniform CameraSSBO {
    Camera cameras[2];
};

layout(location = 0) out struct {
    vec3 worldPos;
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
} vs_out;

void main() {
    Camera cam = cameras[gl_ViewIndex];

    vs_out.worldPos = position.xyz;
    vs_out.viewPos = (cam.view * position).xyz;
    vs_out.normal = normal;
    vs_out.uv = uv;

    gl_Position = cam.proj * cam.view * position;
}
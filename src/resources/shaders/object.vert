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

layout(push_constant) uniform Constants {
    mat4 modelTransform;
    mat4 normalTransform;
};

layout(location = 0) out struct {
    vec4 color;
    vec3 worldPos;
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
} vs_out;

layout(location = 5) flat out int materialID;

void main() {
    Camera cam = cameras[gl_ViewIndex];

    vec4 worldPos = modelTransform * position;

    materialID = matId;
    vs_out.worldPos = worldPos.xyz;
    vs_out.viewPos = cam.position;
    vs_out.color = color;
    vs_out.normal = mat3(normalTransform) * normal;
    vs_out.uv = uv;

    gl_Position = cam.proj * cam.view * worldPos;
}
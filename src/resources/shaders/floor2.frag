#version 460

#include "functions.glsl"

layout(location = 0) in struct {
    vec3 worldPos;
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
} fs_in;

layout(location = 0) out vec4 fragColor;

void main() {
    vec3 albedo =  GetColorFromPositionAndNormal(fs_in.worldPos, fs_in.normal);

    float z = length(fs_in.viewPos);
    float sd = 1 - smoothstep(0, 10, z);
    fragColor.rgb = mix(vec3(0.2), albedo, sd);
}
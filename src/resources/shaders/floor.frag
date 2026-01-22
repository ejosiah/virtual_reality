#version 460

#include "common.glsl"
#include "functions.glsl"

layout(set = 0, binding = 0, scalar) uniform CameraSSBO {
    Camera camera[2];
};

layout(location = 0) in struct {
    vec3 direction;
} fs_in;

layout(location = 1) in flat int view_id;

layout(location = 0) out vec4 fragColor;

void main() {
    mat4 view = camera[view_id].view;
    mat4 proj = camera[view_id].proj;
    vec3 o = camera[view_id].position;

    gl_FragDepth = 1;
    fragColor = vec4(0.2, 0.2, 0.2, 1);

    vec3 n = vec3(0, 1, 0);
    vec3 rd = normalize(fs_in.direction);
    float t = -dot(n, o) / dot(n, rd);

    if(t > 0){
        vec3 p = o + rd * t;
        vec4 depth = proj * view * vec4(p, 1);
        depth /= depth.w;
        gl_FragDepth = depth.z;
        float sd = length(p.xz) - 10;
        sd = 1 - smoothstep(0, 10, sd);
        vec3 albedo =  GetColorFromPositionAndNormal(p, n);
//        vec3 albedo =  mod(floor(p.x) + floor(p.z), 2) == 0 ? vec3(1) : vec3(0.01);

        fragColor.rgb = mix(vec3(0.2), albedo, sd);
    }

}
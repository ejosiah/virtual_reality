#version 460

#extension GL_EXT_multiview : enable

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

struct Camera {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(set = 0, binding = 0) uniform CameraSSBO {
    Camera camera[2];
};


layout(location = 0) out struct {
    vec3 direction;
} vs_out;

layout(location = 1) out flat int view_id;

void main() {
    view_id = gl_ViewIndex;

    vec4 rd = inverse(camera[view_id].proj) * vec4(pos.xy, 1, 1);
    rd /= rd.w;
    rd = inverse(camera[view_id].view) * rd;

    vs_out.direction = normalize(rd.xyz);
    gl_Position = vec4(pos, 0, 1);
}
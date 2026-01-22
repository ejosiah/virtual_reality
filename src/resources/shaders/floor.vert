#version 460

#extension GL_EXT_multiview : enable

#include "common.glsl"

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;


layout(set = 0, binding = 0, scalar) uniform CameraSSBO {
    Camera camera[2];
};


layout(location = 0) out struct {
    vec3 direction;
} vs_out;

layout(location = 1) out flat int view_id;

void main() {
    view_id = gl_ViewIndex;

    mat4 invProj = camera[view_id].inverseProj;
    mat4 invView = camera[view_id].inverseView;

    vec4 v = invProj * vec4(pos.xy, 1.0, 1.0);
    vec3 rd_view = v.xyz / v.w;
    vs_out.direction = (invView * vec4(rd_view, 0.0)).xyz;

    gl_Position = vec4(pos, 0, 1);
}
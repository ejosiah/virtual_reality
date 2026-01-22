#version 460

#include "common.glsl"
layout(set = 0, binding = 0, scalar) uniform CameraSSBO {
    Camera camera[2];
};

layout(location = 0) in struct {
    vec3 direction;
} fs_in;

layout(location = 1) in flat int view_id;

layout(location = 0) out vec4 fragColor;

vec3 GetColorFromPositionAndNormal( in vec3 worldPosition, in vec3 normal );


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

vec3 GetColorFromPositionAndNormal( in vec3 worldPosition, in vec3 normal ) {
    const float pi = 3.141519;
    float scale = 4;
    vec3 scaledPos = scale * worldPosition.xyz * pi * 2.0;
    vec3 scaledPos2 = scale * worldPosition.xyz * pi * 2.0 / 10.0 + vec3( pi / 4.0 );
    float s = cos( scaledPos2.x ) * cos( scaledPos2.y ) * cos( scaledPos2.z );  // [-1,1] range
    float t = cos( scaledPos.x ) * cos( scaledPos.y ) * cos( scaledPos.z );     // [-1,1] range


    t = ceil( t * 0.9 );
    s = ( ceil( s * 0.9 ) + 3.0 ) * 0.25;
    vec3 colorB = vec3( 0.85, 0.85, 0.85 );
    vec3 colorA = vec3( 1, 1, 1 );
    vec3 finalColor = mix( colorA, colorB, t ) * s;

    return vec3(0.8) * finalColor;
}

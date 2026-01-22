#ifndef COMMONG_GLSL
#define COMMONG_GLSL

#extension GL_EXT_scalar_block_layout : enable

struct Camera {
    mat4 view;
    mat4 proj;
    vec3 position;
};

#endif // COMMONG_GLSL
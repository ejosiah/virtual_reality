#ifndef COMMONG_GLSL
#define COMMONG_GLSL

#extension GL_EXT_scalar_block_layout : enable

struct Camera {
    mat4 view;
    mat4 proj;
    mat4 inverseView;
    mat4 inverseProj;
    vec3 position;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    vec3 shininess;
    float ior;
    float opacity;

    int ambientTexId;
    int diffuseTexId;
    int specularTexId;
    int shininessTexId;
    int normalTexId;
};

#endif // COMMONG_GLSL
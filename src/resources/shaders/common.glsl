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
    float shininess;
    float ior;
    float opacity;

    int ambientTexId;
    int diffuseTexId;
    int specularTexId;
    int shininessTexId;
    int normalTexId;
};

struct Surface {
    vec3 point;
    vec3 normal;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    float shininess;
    float ior;
    float opacity;
};


Surface defaultSurface(vec3 point, vec3 normal) {
    return Surface(point, normal, vec3(0.6), vec3(0.6), vec3(1.0), vec3(0.0), vec3(0.0), 50, 1.0, 1.0);
}


#endif // COMMONG_GLSL
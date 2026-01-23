#version 460

#extension GL_EXT_nonuniform_qualifier : enable

#include "common.glsl"

layout(set = 1, binding = 0, scalar) readonly buffer MaterialSSBO {
    Material materials[];
};

layout(set = 1, binding = 1) uniform sampler2D textures[];

Surface getSurface();

layout(location = 0) in struct {
    vec4 color;
    vec3 worldPos;
    vec3 normal;
    vec2 uv;
} fs_in;

layout(location = 4) flat in int matID;

layout(location = 0) out vec4 fragColor;

void main() {
    Surface surface = getSurface();
    vec3 color = surface.diffuse;
    fragColor = vec4(color, 1);
}

Surface getSurface() {
    Surface surface = defaultSurface();
    surface.ambient = surface.diffuse = fs_in.color.rgb;

    if(matID < 0) return surface;

    Material material = materials[matID];
    surface.ambient = material.ambient;
    surface.diffuse = material.diffuse;
    surface.specular = material.specular;
    surface.transmittance = material.transmittance;
    surface.emission = material.emission;
    surface.shininess = material.shininess;
    surface.ior = material.ior;
    surface.opacity = material.opacity;

    vec2 uv = fs_in.uv;
    if(material.ambientTexId != -1) {
        surface.ambient = texture(textures[nonuniformEXT(material.ambientTexId)], uv).rgb;
    }

    if(material.diffuseTexId != -1) {
        surface.diffuse = texture(textures[nonuniformEXT(material.diffuseTexId)], uv).rgb;
    }

    if(material.specularTexId != -1) {
        surface.specular = texture(textures[nonuniformEXT(material.specularTexId)], uv).rgb;
    }

    if(material.shininessTexId != -1) {
        surface.shininess = texture(textures[nonuniformEXT(material.shininessTexId)], uv).rgb;
    }

    // TODO normals

    return surface;
}
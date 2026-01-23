#version 460

#extension GL_EXT_nonuniform_qualifier : enable

#include "common.glsl"
#include "light.glsl"
#include "functions.glsl"

layout(set = 1, binding = 0, scalar) readonly buffer MaterialSSBO {
    Material materials[];
};

layout(set = 1, binding = 1) uniform sampler2D textures[];

layout(set = 2, binding = 0, scalar) uniform LightSSBO {
    Light lights[2];
};

Surface getSurface();

layout(location = 0) in struct {
    vec4 color;
    vec3 worldPos;
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
} fs_in;

layout(location = 5) flat in int matID;

layout(location = 0) out vec4 fragColor;

void main() {
    Surface surface = getSurface();
    vec3 radiance = computeRadiance(lights[0], surface, fs_in.viewPos);

    radiance /= (radiance + 1);

    fragColor = vec4(radiance, 1);
}

Surface getSurface() {
    Surface surface = defaultSurface(fs_in.worldPos, fs_in.normal);
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
        surface.ambient = srgbToLinear(surface.ambient);
    }

    if(material.diffuseTexId != -1) {
        surface.diffuse = texture(textures[nonuniformEXT(material.diffuseTexId)], uv).rgb;
        surface.diffuse = srgbToLinear(surface.diffuse);
    }

    if(material.specularTexId != -1) {
        surface.specular = texture(textures[nonuniformEXT(material.specularTexId)], uv).rgb;
    }

    if(material.shininessTexId != -1) {
        surface.shininess = texture(textures[nonuniformEXT(material.shininessTexId)], uv).r;
    }

    // TODO normals

    return surface;
}
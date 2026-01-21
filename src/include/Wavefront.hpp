#pragma once

#include "vr/graphics/vulkan/VulkanGraphicsService.hpp"
#include "vr/graphics/vulkan/Memory.hpp"
#include <glm/glm.hpp>

#include <filesystem>
#include <string>
#include <vector>
#include <cinttypes>

namespace wavefront {

    struct Material {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        glm::vec3 transmittance;
        glm::vec3 emission;
        glm::vec3 shininess;
        float ior;
        float opacity;

        int ambientTexId{-1};
        int diffuseTexId{-1};
        int specularTexId{-1};
        int shininessTexId{-1};
        int normalTexId{-1};

    };

    struct Mesh {
        std::string name;
        uint32_t firstIndex{};
        uint32_t indexCount{};
        uint32_t firstVertex{};
        uint32_t vertexCount{};
        uint32_t vertexOffset{};
    };

    struct Instance {
        int meshId{};
        int materialId{};
        int transformId{};
        float opacity{1};
    };


    struct Object {
        std::vector<Instance> instances;
        std::vector<glm::mat4> transforms;
        std::vector<vr::Texture> textures;
        vr::Sampler sampler;
        std::vector<Mesh> meshes;
        vr::Buffer vertices;
        vr::Buffer indexes;
        vr::Buffer materials;
        VkDescriptorSetLayout descriptorSetLayout{};
        VkDescriptorSet descriptorSet{};
    };
}
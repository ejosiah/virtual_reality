#pragma once

#include "vr/graphics/vulkan/VulkanGraphicsService.hpp"
#include "vr/graphics/vulkan/Memory.hpp"
#include <glm/glm.hpp>

#include <filesystem>
#include <string>
#include <vector>
#include <cinttypes>
#include <numbers>

namespace wavefront {

    struct Bounds {
        glm::vec3 min{std::numeric_limits<float>::max()};
        glm::vec3 max{std::numeric_limits<float>::lowest()};
    };

    struct Material {
        glm::vec3 ambient{0.6};
        glm::vec3 diffuse{0.6};
        glm::vec3 specular{1};
        glm::vec3 transmittance{0};
        glm::vec3 emission{0};
        float shininess{50};
        float ior{1};
        float opacity{1};

        int ambientTexId{-1};
        int diffuseTexId{-1};
        int specularTexId{-1};
        int shininessTexId{-1};
        int normalTexId{-1};

    };

    struct Mesh {
        std::string name;
        Bounds bounds;
        uint32_t firstIndex{};
        uint32_t indexCount{};
        uint32_t firstVertex{};
        uint32_t vertexCount{};
        int vertexOffset{};
    };

    struct Instance {
        int meshId{};
        int materialId{};
        int transformId{};
        float opacity{1};
    };


    struct Scene {
        std::vector<Instance> instances;
        glm::mat4 transform{1};
        std::vector<vr::Texture> textures;
        std::vector<std::string> materialNames;
        std::vector<std::string> meshNames;
        Bounds bounds;
        vr::Sampler sampler;
        std::vector<Mesh> meshes;
        vr::Buffer vertices;
        vr::Buffer indexes;
        vr::Buffer materials;
        vr::Buffer drawCommands;
        uint32_t drawCommandCount{};
        VkDescriptorSetLayout descriptorSetLayout{};
        VkDescriptorSet descriptorSet{};
        uint32_t indexCount{};
    };
}
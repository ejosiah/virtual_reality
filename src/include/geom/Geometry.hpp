#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <string>

namespace geom {

    static const glm::vec4 GRAY{0.6, 0.6,0.6, 1};
    constexpr uint32_t RESTART_PRIMITIVE = UINT32_MAX;

    enum class Topology {
        POINTS,
        LINES,
        LINE_STRIPS,
        TRIANGLES,
        TRIANGLE_FAN,
        TRIANGLE_STRIPS,
        LISTS_WITH_ADJACENCY,
        LINE_STRIPS_WITH_ADJACENCY,
        TRIANGLES_WITH_ADJACENCY,
        TRIANGLE_STRIP_WITH_ADJACENCY,
        PATCHES
    };
    
    struct Vertex {
        glm::vec4 position;
        glm::vec4 color;
        alignas(16) glm::vec3 normal;
        alignas(16) glm::vec3 tangent;
        alignas(16) glm::vec3 bitangent;
        glm::vec2 uv;
        int material;

        static std::vector<VkVertexInputBindingDescription> bindingDisc(){
            return {
                    {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
            };
        }

        static std::vector<VkVertexInputAttributeDescription> attributeDisc(){
            return {
                    {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, (uint32_t)offsetof(Vertex, position)},
                    {1, 0, VK_FORMAT_R32G32B32_SFLOAT, (uint32_t)offsetof(Vertex, normal)},
                    {2, 0, VK_FORMAT_R32G32B32_SFLOAT, (uint32_t)offsetof(Vertex, tangent)},
                    {3, 0, VK_FORMAT_R32G32B32_SFLOAT, (uint32_t)offsetof(Vertex, bitangent)},
                    {4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, (uint32_t)offsetof(Vertex, color)},
                    {5, 0, VK_FORMAT_R32G32_SFLOAT, (uint32_t)offsetof(Vertex, uv)},
                    {6, 0, VK_FORMAT_R8_SINT, (uint32_t)offsetof(Vertex, material)}
            };
        }
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        Topology topology;
    };

    struct ClipSpace{

        struct Quad{
            static constexpr  std::array<glm::vec2, 8> positions{
                    glm::vec2{-1, -1},glm::vec2{0, 0},
                    glm::vec2{-1, 1}, glm::vec2{0, 1},
                    glm::vec2{1, -1}, glm::vec2{1, 0},
                    glm::vec2{1, 1}, glm::vec2{1, 1}
            };
            static constexpr VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

            static constexpr VkFrontFace  frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        };

        struct Triangle{
            static constexpr  std::array<glm::vec2, 6> positions{
                    glm::vec2{-1, 1},glm::vec2{0, 1},
                    glm::vec2{1, 1}, glm::vec2{1, 1},
                    glm::vec2{0, -1}, glm::vec2{0.5, 0.5},
            };
            static constexpr VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            static constexpr VkFrontFace  frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        };


        static std::vector<VkVertexInputBindingDescription> bindingDescription(){
            return {
                    {0, 2 * sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX}
            };
        }

        static std::vector<VkVertexInputAttributeDescription> attributeDescriptions(){
            return {
                    {0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
                    {1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2)}
            };
        }
    };
    
    /**
     * Generates vertices for cube
     * @param color cube color
     * @return vertices defining a cube
     */
    [[maybe_unused]]
    Mesh cube(const glm::vec4& color = GRAY);

    [[maybe_unused]]
    Mesh teapot(glm::mat4 xform = glm::mat4{1}, glm::mat4 lidXform = glm::mat4{1}, const glm::vec4& color = GRAY);

    /**
     * Generates Mesh for a sphere
     * @param rows number of rows on the sphere
     * @param columns number of columns on the sphere
     * @param radius radius of sphere
     * @param color color of sphere
     * @return vertices defining a sphere
     */
    [[maybe_unused]]
    Mesh sphere(int rows, int columns, float radius = 1.0f, glm::mat4 xform = glm::mat4{1}, const glm::vec4& color = GRAY, Topology topology = Topology::TRIANGLE_STRIPS);

    /**
     *
     * Generates Mesh for a hemisphere
     * @param rows number of rows on the hemisphere
     * @param columns number of columns on the hemisphere
     * @param radius radius of hemisphere
     * @param color color of hemisphere
     * @return vertices defining a hemisphere
     */
    [[maybe_unused]]
    Mesh hemisphere(int rows, int columns, float radius = 1.0f, const glm::vec4& color = GRAY, Topology topology = Topology::TRIANGLE_STRIPS);

    /**
     * Generates Mesh for a cone
     * @param rows number of rows on the cone
     * @param columns number of columns on the cone
     * @param radius radius of the cone
     * @param height height of the cone
     * @param color color of the cone
     * @return vertices defining a cone
     */
    [[maybe_unused]]
    Mesh cone(int rows, int columns, float radius = 1.0f, float height = 1.0f, const glm::vec4& color = GRAY, Topology topology = Topology::TRIANGLE_STRIPS);

    /**
     * @brief Generates Mesh for a cylinder
     * @param rows number of rows on the cylinder
     * @param columns number of columns on the cylinder
     * @param radius radius of the cylinder
     * @param height height of the cylinder
     * @param color color of the cylinder
     * @return vertices defining a cylinder
     */
    [[maybe_unused]]
    Mesh cylinder(int rows, int columns, float radius = 1.0f, float height = 1.0f,  const glm::vec4& color = GRAY, Topology topology = Topology::TRIANGLE_STRIPS);

    [[maybe_unused]]
    Mesh plane(int rows, int columns, float width, float height, const glm::mat4& xform = glm::mat4(1), const glm::vec4& color = GRAY, Topology topology = Topology::TRIANGLE_STRIPS);

    /**
     * @brief Generates Mesh for a torus
     * @param rows number or rows on the torus
     * @param columns number of columns on the torus
     * @param innerRadius inner radius of the torus
     * @param outerRadius outer radius of the torus
     * @param color color of the torus
     * @return vertices defining a torus
     */
    [[maybe_unused]]
    Mesh torus(int rows, int columns, float innerRadius = 0.5f, float outerRadius = 1.0f, glm::mat4 xform = glm::mat4{1}, const glm::vec4& color = GRAY, Topology topology = Topology::TRIANGLE_STRIPS);

    /**
     * Generates a parametric surface
     * @tparam SurfaceFunction giving an i,j pair generates a position and normal vector i.e f(i, j) -> std::tuple<glm::vec3, glm::vec3>
     * @param p horizontal sample points
     * @param q vertical sample points
     * @param f giving an i,j pair generates a position and normal vector i.e f(i, j) -> std::tuple<glm::vec3, glm::vec3>
     * @param color surface color
     * @return  returns a surface defined by the parametric function f
     */
    template<typename SurfaceFunction>
    Mesh surface(int p,
                     int q,
                     SurfaceFunction&& f,
                     const glm::vec4& color,
                     const glm::mat4& xform = glm::mat4(1),
                     Topology topology = Topology::TRIANGLE_STRIPS);
    
}
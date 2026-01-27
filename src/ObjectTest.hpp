#pragma once

#include "vr/graphics/vulkan/VulkanRenderer.hpp"
#include "vr/graphics/vulkan/Rendering.hpp"
#include "vr/SessionConfig.hpp"
#include "geom/Geometry.hpp"
#include "vr/Models.hpp"

#include <vulkanizer/builders.hpp>

struct Camera {
    glm::mat4 view{};
    glm::mat4 projection{};
    glm::mat4 inverseView{};
    glm::mat4 inverseProjection{};
    glm::vec3 position{};
};

namespace LightType {
    static constexpr int Directional = 0;
    static constexpr int Point = 1;
}

struct Light {
    glm::vec3 color{1};
    glm::vec3 position{};
    glm::vec3 direction{1};
    float cutOff{};
    float outerCutOff{};
    int type{LightType::Directional};
};

class ObjectTest : public vr::VulkanRenderer {
public:

    void init() final {
        setupLayers();
        initCamera();
        initLights();
        createDepthBuffer();
        initMsaa();
        initFrameBuffer();
        createDescriptorPool();
        loadObject();
        createDescriptorSetLayouts();
        updateDescriptorSets();
        initFloor();
        createPipelines();
    }

    void setupLayers() {
        auto m_depthView = makeStruct<XrCompositionLayerDepthInfoKHR>();
        m_depthView.minDepth = 0.0f;
        m_depthView.maxDepth = 1.0f;
        m_depthView.nearZ = 0.1f;
        m_depthView.farZ = 100.0f;

        m_depthViews = std::vector(2, m_depthView);

        m_layerViews = std::vector(2, makeStruct<XrCompositionLayerProjectionView>());
        m_layerViews[0].next = &m_depthViews[0];
        m_layerViews[1].next = &m_depthViews[1];

        m_layer.views = m_layerViews.data();
        m_layer.viewCount = m_layerViews.size();
    }

    void initCamera() {
        m_camera = graphicsService().link<Camera>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 2);
    }

    void initLights() {
        m_lights = graphicsService().link<Light>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 2);
        m_lights[0] = Light{ };

    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{{
              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100u},
              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100u}
        }};
        auto createInfo = makeStruct<VkDescriptorPoolCreateInfo>();
        createInfo.maxSets = 100u;
        createInfo.poolSizeCount = poolSizes.size();
        createInfo.pPoolSizes = poolSizes.data();

        m_descriptorPool = graphicsService().createDescriptorPool(createInfo);
    }

    void createDescriptorSetLayouts() {
        m_cameraDescriptorSetLayout =
            graphicsService().descriptorSetLayoutBuilder()
                .name("camera_descriptor_set_layout")
                .binding(0)
                    .descriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                    .descriptorCount(1)
                    .shaderStages(VK_SHADER_STAGE_ALL_GRAPHICS)
                .createLayout();

        m_lightDescriptorSetLayout =
            graphicsService().descriptorSetLayoutBuilder()
                .name("lights_descriptor_set_layout")
                .binding(0)
                    .descriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                    .descriptorCount(1)
                    .shaderStages(VK_SHADER_STAGE_ALL_GRAPHICS)
                .createLayout();
    }

    void updateDescriptorSets() {
        m_cameraDescriptorSet = graphicsService().allocate(m_descriptorPool, m_cameraDescriptorSetLayout).front();
        m_lightDescriptorSet = graphicsService().allocate(m_descriptorPool, m_lightDescriptorSetLayout).front();

        std::vector writes(2, makeStruct<VkWriteDescriptorSet>());

        writes[0].dstSet = m_cameraDescriptorSet;
        writes[0].dstBinding = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].descriptorCount = 1;
        VkDescriptorBufferInfo cameraInfo{ m_camera, 0, VK_WHOLE_SIZE};
        writes[0].pBufferInfo = &cameraInfo;

        writes[1].dstSet = m_lightDescriptorSet;
        writes[1].dstBinding = 0;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[1].descriptorCount = 1;
        VkDescriptorBufferInfo lightInfo{ m_lights, 0, VK_WHOLE_SIZE};
        writes[1].pBufferInfo = &lightInfo;

        graphicsService().update(writes);
    }

    void initMsaa() {
        auto samples = graphicsService().getClosestSupportedSampleCount(VK_SAMPLE_COUNT_64_BIT);
        spdlog::info("{} max supported MSAA samples", samples);

        auto swapchain = graphicsService().getSwapChain("color");
        auto imageInfo = makeStruct<VkImageCreateInfo>();
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = swapchain.format;
        imageInfo.extent = {(swapchain.width), (swapchain.height), 1};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 2;
        imageInfo.samples = m_msaa.samples;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImageViewCreateInfo createViewInfo = makeStruct<VkImageViewCreateInfo>();
        createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        createViewInfo.format = swapchain.format;
        createViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2 };

        for (auto i = 0; i < swapchain.images.size(); i++) {
            auto image = graphicsService().createImage(imageInfo);
            m_msaa.images.push_back(image);

            createViewInfo.image = image;
            auto imageView = graphicsService().createImageView(createViewInfo);
            m_msaa.views.push_back(imageView);

        }
    }

    void initFrameBuffer() {
        auto colorSC = graphicsService().getSwapChain("color");
        auto numImages = colorSC.images.size();

        VkImageViewCreateInfo createViewInfo = makeStruct<VkImageViewCreateInfo>();
        createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        createViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2 };

        for(auto i = 0; i < numImages; ++i) {
            auto& frameBuffer = m_frameBuffers.emplace_back();
            createViewInfo.image = colorSC.images[i].image;
            createViewInfo.format = colorSC.format;
            createViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            auto imageView = graphicsService().createImageView(createViewInfo);
            frameBuffer.colorAttachments.emplace_back(m_msaa.views[i], colorSC.format,
                                    glm::vec4{0.2, 0.2, 0.2, 1.0}, imageView);

            frameBuffer.depthAttachment = { m_depthBuffer.views[i], VK_FORMAT_D16_UNORM };
            frameBuffer.renderArea = { colorSC.width, colorSC.height };
            frameBuffer.numLayers = 2;
            frameBuffer.viewMask = viewMask;
        }
    }

    void createDepthBuffer() {
        const auto depthSC = graphicsService().getSwapChain("depth");
        const auto depthCount = depthSC.images.size();

        VkImageViewCreateInfo viewInfo = makeStruct<VkImageViewCreateInfo>();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.format = depthSC.format;
        viewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 2 };

        m_depthBuffer.views.resize(depthCount);
        for (auto i = 0; i < depthCount; ++i) {
            const auto& image = depthSC.images[i];
            viewInfo.image = image.image;
            m_depthBuffer.views[i] = graphicsService().createImageView(viewInfo);
        }
    }

    void initFloor() {
        auto vertices = geom::ClipSpace::Quad::positions;
        auto staging = graphicsService().createStagingBuffer(BYTE_SIZE(vertices));
        auto mapping = graphicsService().map(staging);
        std::memcpy(mapping, vertices.data(), BYTE_SIZE(vertices));

        m_floor.vertices = graphicsService().createDeviceLocalBuffer(BYTE_SIZE(vertices),
                                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        graphicsService().copy(staging, m_floor.vertices, BYTE_SIZE(vertices));

        auto& swapchain = graphicsService().getSwapChain("color");
        m_floor.pipeline.handle =
            graphicsService().graphicsPipelineBuilder()
                .shaderStage()
                    .vertexShader(R"(..\..\..\src\resources\shaders\floor.vert.spv)")
                    .fragmentShader(R"(..\..\..\src\resources\shaders\floor.frag.spv)")
                .vertexInputState()
                    .addVertexBindingDescriptions(geom::ClipSpace::bindingDescription())
                    .addVertexAttributeDescriptions(geom::ClipSpace::attributeDescriptions())
                .inputAssemblyState()
                    .triangleStrip()
                .viewportState()
                    .viewport()
                        .origin(0, 0)
                        .dimension(swapchain.width, swapchain.height)
                        .minDepth(0)
                        .maxDepth(1)
                    .scissor()
                        .offset(0, 0)
                        .extent(static_cast<int>(swapchain.width), static_cast<int>(swapchain.height))
                    .add()
                    .rasterizationState()
                        .cullNone()
                        .frontFaceCounterClockwise()
                        .polygonModeFill()
                    .multisampleState()
                        .rasterizationSamples(VK_SAMPLE_COUNT_8_BIT)
                    .depthStencilState()
                        .enableDepthWrite()
                        .enableDepthTest()
                        .compareOpLess()
                        .minDepthBounds(0)
                        .maxDepthBounds(1)
                    .colorBlendState()
                        .attachment()
                        .add()
                    .layout()
                        .addDescriptorSetLayout(m_cameraDescriptorSetLayout)
                        .addDescriptorSetLayout(m_lightDescriptorSetLayout)
                    .dynamicRenderPass()
                        .addColorAttachment(swapchain.format)
                        .depthAttachment(VK_FORMAT_D16_UNORM)
                        .viewMask(viewMask)
                    .subpass(0)
                    .name("floor_render")
                .build(m_floor.pipeline.layout);
    }

    void createPipelines() {
        auto& swapchain = graphicsService().getSwapChain("color");
        m_modelPipeline.handle =
            graphicsService().graphicsPipelineBuilder()
                .shaderStage()
                    .vertexShader(R"(..\..\..\src\resources\shaders\object.vert.spv)")
                    .fragmentShader(R"(..\..\..\src\resources\shaders\object.frag.spv)")
                .vertexInputState()
                    .addVertexBindingDescriptions(geom::Vertex::bindingDisc())
                    .addVertexAttributeDescriptions(geom::Vertex::attributeDisc())
                .inputAssemblyState()
                    .triangles()
                .viewportState()
                    .viewport()
                        .origin(0, 0)
                        .dimension(swapchain.width, swapchain.height)
                        .minDepth(0)
                        .maxDepth(1)
                    .scissor()
                        .offset(0, 0)
                        .extent(static_cast<int>(swapchain.width), static_cast<int>(swapchain.height))
                    .add()
                .rasterizationState()
                    .cullBackFace()
                    .frontFaceCounterClockwise()
                    .polygonModeFill()
                .multisampleState()
                    .rasterizationSamples(VK_SAMPLE_COUNT_8_BIT)
                .depthStencilState()
                    .enableDepthWrite()
                    .enableDepthTest()
                    .compareOpLess()
                    .minDepthBounds(0)
                    .maxDepthBounds(1)
                .colorBlendState()
                    .attachment()
                    .add()
                .layout()
                    .addDescriptorSetLayout(m_cameraDescriptorSetLayout)
                    .addDescriptorSetLayout(m_scene.descriptorSetLayout)
                    .addDescriptorSetLayout(m_lightDescriptorSetLayout)
                    .addPushConstantRange(VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(instanceData))
                .dynamicRenderPass()
                    .addColorAttachment(swapchain.format)
                    .depthAttachment(VK_FORMAT_D16_UNORM)
                    .viewMask(viewMask)
                .subpass(0)
                .name("object_render")
            .build(m_modelPipeline.layout);
    }

    void loadObject() {
        m_scene = graphicsService().loadObject(m_descriptorPool, m_path);
    }

    vr::cstring name() override {
        return "Object Test";
    }

    static vr::SessionConfig session() {
        return
            vr::SessionConfig()
                .baseSpaceType().localFloor()
                .addSpace(
                    ReferenceSpaceSpecification()
                        .name("object")
                        .localFloor()
                        .pose()
                            .translate(0, 0, -5))
                .addSwapChain(
                    vr::SwapchainSpecification()
                        .name("color")
                        .usage().colorAttachment()
                        .format(VK_FORMAT_R8G8B8A8_SRGB)
                        .width(2064)
                        .height(2096)
                        .arraySize(2)
                        .depthBuffer("depth"))
                .addSwapChain(
                    vr::SwapchainSpecification()
                        .name("depth")
                        .usage().depthStencilAttachment()
                        .format(VK_FORMAT_D16_UNORM)
                        .sampleCount(VK_SAMPLE_COUNT_8_BIT)
                        .width(2064)
                        .height(2096)
                        .arraySize(2));

    }

    static std::shared_ptr<vr::Renderer> shared() {
        return std::make_shared<ObjectTest>();
    }

    void set(const std::vector<vr::SpaceLocation> &spaceLocations) final {
        auto location = find(spaceLocations, [](const auto& sloc){ return sloc.name == "object"; });
        auto pose =  vr::toMatrix(location->pose);
        m_scene.transform = pose;
    }

    void render(const vr::FrameInfo &frameInfo, vr::Layers &layers) override {

        for(auto i = 0; i < frameInfo.viewInfo.views.size(); ++i) {
            const auto& view = frameInfo.viewInfo.views[i];
            const auto pose = vr::toMatrix(view.pose);
            m_camera[i].view = glm::inverse(pose);
            m_camera[i].projection = graphicsService().projection(view.fov, 0.1, 100);
            m_camera[i].inverseView = pose;
            m_camera[i].inverseProjection = glm::inverse(m_camera[i].projection);
            m_camera[i].position = vr::convert(view.pose).position;
        }

        graphicsService().scoped([&](auto commandBuffer){
           auto& frameBuffer = m_frameBuffers[frameInfo.imageId.imageIndex];
           vr::render(commandBuffer, frameBuffer, [&]{
               renderFloor(commandBuffer);
               renderObject(commandBuffer);
           });
        });

        const auto swapchain = graphicsService().getSwapChain(frameInfo.imageId.swapChain);
        for(auto i = 0; i < 2; ++i) {
            m_layerViews[i].pose = frameInfo.viewInfo.views[i].pose;
            m_layerViews[i].fov = frameInfo.viewInfo.views[i].fov;
            m_layerViews[i].subImage.swapchain = swapchain;
            m_layerViews[i].subImage.imageRect = {{0, 0}, static_cast<int32_t>(swapchain.width), static_cast<int32_t>(swapchain.height)};
            m_layerViews[i].subImage.imageArrayIndex = i;

            if (frameInfo.depthImageId.has_value()) {
                m_depthViews[i].subImage.swapchain = frameInfo.depthImageId->swapChain;
                m_depthViews[i].subImage.imageArrayIndex = i;
            }
        }
        m_layer.space = frameInfo.space;
        layers.push_back({ &m_layer });
    }

    void renderFloor(VkCommandBuffer commandBuffer) {
        VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_floor.vertices._, &offset);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_floor.pipeline.handle);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_floor.pipeline.layout, 0, 1, &m_cameraDescriptorSet, 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_floor.pipeline.layout, 1, 1, &m_lightDescriptorSet, 0, nullptr);
        vkCmdDraw(commandBuffer, 4, 1, 0, 0);
    }

    void renderObject(VkCommandBuffer commandBuffer) {

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_scene.vertices._, &offset);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_modelPipeline.handle);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_modelPipeline.layout, 0, 1, &m_cameraDescriptorSet, 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_modelPipeline.layout, 1, 1, &m_scene.descriptorSet, 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_modelPipeline.layout, 2, 1, &m_lightDescriptorSet, 0, nullptr);
        vkCmdBindIndexBuffer(commandBuffer, m_scene.indexes, 0, VK_INDEX_TYPE_UINT32);

        instanceData.modelTransform = m_scene.transform;
        instanceData.normalTransform = glm::inverse(glm::transpose(instanceData.modelTransform ));

        vkCmdPushConstants(commandBuffer, m_modelPipeline.layout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(instanceData), &instanceData);
        vkCmdDrawIndexedIndirect(commandBuffer, m_scene.drawCommands, 0, m_scene.drawCommandCount, sizeof(VkDrawIndexedIndirectCommand));
    }

private:

    // std::filesystem::path m_path{R"(C:\Users\joebh\OneDrive\media\models\bigship1.obj)"};
    // std::filesystem::path m_path{R"(C:\Users\joebh\OneDrive\media\models\CornellBox\CornellBox-Original.obj)"};
    std::filesystem::path m_path{R"(C:\Users\joebh\OneDrive\media\models\leaving_room\living_room.obj)"};
    // std::filesystem::path m_path{R"(C:\Users\joebh\OneDrive\media\models\Office Interior Reception 2\office.obj)"};
    // std::filesystem::path m_path{R"(C:\Users\joebh\OneDrive\media\models\Sports_car\highpolyGHS.obj)"};
    wavefront::Scene m_scene;
    VkDescriptorPool m_descriptorPool{};

    VkDescriptorSetLayout m_cameraDescriptorSetLayout{};
    VkDescriptorSet m_cameraDescriptorSet{};

    VkDescriptorSetLayout m_lightDescriptorSetLayout{};
    VkDescriptorSet m_lightDescriptorSet{};

    vr::Pipeline m_modelPipeline;

    struct {
        vr::Pipeline pipeline;
        vr::Buffer vertices;
    } m_floor;

    std::vector<vr::RenderInfo> m_frameBuffers;
    XrCompositionLayerProjection m_layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    std::vector<XrCompositionLayerProjectionView> m_layerViews;
    std::vector<XrCompositionLayerDepthInfoKHR> m_depthViews;
    struct {
        std::vector<vr::ImageView> views;
    } m_depthBuffer;

    struct {
        std::vector<vr::Image> images;
        std::vector<vr::ImageView> views;
        VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_8_BIT};
    } m_msaa;

    struct {
        glm::mat4 modelTransform{1};
        glm::mat4 normalTransform{1};
    } instanceData;

    vr::Link<Camera> m_camera{};
    vr::Link<Light> m_lights;
    uint32_t viewMask = 0b11;
};
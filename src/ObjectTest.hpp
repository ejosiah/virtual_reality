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

class ObjectTest : public vr::VulkanRenderer {
public:

    void init() final {
        setupLayers();
        initCamera();
        createDepthBuffer();
        initFrameBuffer();
        createDescriptorPool();
        loadObject();
        createDescriptorSetLayouts();
        updateDescriptorSets();
        initFloor();
        createPipelines();
    }

    void setupLayers() {
        m_layerViews = std::vector<XrCompositionLayerProjectionView>(2, makeStruct<XrCompositionLayerProjectionView>());
        m_layer.views = m_layerViews.data();
        m_layer.viewCount = 2;
    }

    void initCamera() {
        m_camera = graphicsService().link<Camera>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 2);
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{{
              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10u},
              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10u}
        }};
        auto createInfo = makeStruct<VkDescriptorPoolCreateInfo>();
        createInfo.maxSets = 10u;
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
    }

    void updateDescriptorSets() {
        m_cameraDescriptorSet = graphicsService().allocate(m_descriptorPool, m_cameraDescriptorSetLayout).front();

        std::vector<VkWriteDescriptorSet> writes(1, makeStruct<VkWriteDescriptorSet>());

        writes[0].dstSet = m_cameraDescriptorSet;
        writes[0].dstBinding = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].descriptorCount = 1;
        VkDescriptorBufferInfo cameraInfo{ m_camera, 0, VK_WHOLE_SIZE};
        writes[0].pBufferInfo = &cameraInfo;

        graphicsService().update(writes);
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
            frameBuffer.colorAttachments.emplace_back(imageView, colorSC.format, glm::vec4{0.2, 0.2, 0.2, 1.0});

            frameBuffer.depthAttachment = { m_depthBuffer.view, VK_FORMAT_D16_UNORM };
            frameBuffer.renderArea = { colorSC.width, colorSC.height };
            frameBuffer.numLayers = 2;
            frameBuffer.viewMask = viewMask;
        }
    }

    void createDepthBuffer() {
        auto swapchain = graphicsService().getSwapChain("color");
        auto imageInfo = makeStruct<VkImageCreateInfo>();
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_D16_UNORM;
        imageInfo.extent = {(swapchain.width), (swapchain.height), 1};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 2;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        m_depthBuffer.image = graphicsService().createImage(imageInfo);

        VkImageViewCreateInfo viewInfo = makeStruct<VkImageViewCreateInfo>();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.image = m_depthBuffer.image;
        viewInfo.format = m_depthBuffer.image.info.format;
        viewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 2 };

        m_depthBuffer.view = graphicsService().createImageView(viewInfo);
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
                        .rasterizationSamples(VK_SAMPLE_COUNT_1_BIT)
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
                    .rasterizationSamples(VK_SAMPLE_COUNT_1_BIT)
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
                    .addDescriptorSetLayout(m_object.descriptorSetLayout)
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
        m_object = graphicsService().loadObject(m_descriptorPool, m_path);
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
                        .name("floor")
                        .localFloor())
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
                        .depthBuffer("depth"));

    }

    static std::shared_ptr<vr::Renderer> shared() {
        return std::make_shared<ObjectTest>();
    }

    void set(const std::vector<vr::SpaceLocation> &spaceLocations) final {
        const auto& location = spaceLocations[1];
        auto pose =  vr::toMatrix(location.pose);
        for (auto& transform : m_object.transforms) {
            transform = pose;
        }
    }

    void render(const vr::FrameInfo &frameInfo, vr::Layers &layers) override {

        for(auto i = 0; i < frameInfo.viewInfo.views.size(); ++i) {
            const auto& view = frameInfo.viewInfo.views[i];
            const auto pose = vr::toMatrix(view.pose);
            m_camera.cpu[i].view = glm::inverse(pose);
            m_camera.cpu[i].projection = graphicsService().projection(view.fov, 0.1, 100);
            m_camera.cpu[i].inverseView = pose;
            m_camera.cpu[i].inverseProjection = glm::inverse(m_camera.cpu[i].projection);
            m_camera.cpu[i].position = vr::convert(view.pose).position;
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
        }
        m_layer.space = frameInfo.space;
        layers.push_back({ &m_layer });
    }

    void renderFloor(VkCommandBuffer commandBuffer) {
        VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_floor.vertices._, &offset);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_floor.pipeline.handle);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_floor.pipeline.layout, 0, 1, &m_cameraDescriptorSet, 0, nullptr);
        vkCmdDraw(commandBuffer, 4, 1, 0, 0);
    }

    void renderObject(VkCommandBuffer commandBuffer) {

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_object.vertices._, &offset);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_modelPipeline.handle);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_modelPipeline.layout, 0, 1, &m_cameraDescriptorSet, 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_modelPipeline.layout, 1, 1, &m_object.descriptorSet, 0, nullptr);
        vkCmdBindIndexBuffer(commandBuffer, m_object.indexes, 0, VK_INDEX_TYPE_UINT32);

        for(auto& instance : m_object.instances) {
            const auto& mesh = m_object.meshes[instance.meshId];

            instanceData.modelTransform = m_object.transforms[instance.transformId];
            instanceData.normalTransform = glm::inverse(glm::transpose(instanceData.modelTransform ));

            vkCmdPushConstants(commandBuffer, m_modelPipeline.layout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(instanceData), &instanceData);
            vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, mesh.firstIndex, mesh.vertexOffset, 0);
        }
    }

private:

    // std::filesystem::path m_path{R"(C:\Users\joebh\OneDrive\media\models\bigship1.obj)"};
    // std::filesystem::path m_path{R"(C:\Users\joebh\OneDrive\media\models\CornellBox\CornellBox-Original.obj)"};
    std::filesystem::path m_path{R"(C:\Users\joebh\OneDrive\media\models\leaving_room\living_room.obj)"};
    wavefront::Object m_object;
    VkDescriptorPool m_descriptorPool{};

    VkDescriptorSetLayout m_cameraDescriptorSetLayout{};
    VkDescriptorSet m_cameraDescriptorSet{};

    vr::Pipeline m_modelPipeline;

    struct {
        vr::Pipeline pipeline;
        vr::Buffer vertices;
    } m_floor;

    std::vector<vr::RenderInfo> m_frameBuffers;
    XrCompositionLayerProjection m_layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    std::vector<XrCompositionLayerProjectionView> m_layerViews;
    struct {
        vr::Image image;
        vr::ImageView view;
    } m_depthBuffer;

    struct {
        glm::mat4 modelTransform{1};
        glm::mat4 normalTransform{1};
    } instanceData;

    vr::Link<Camera> m_camera{};
    uint32_t viewMask = 0b11;
};
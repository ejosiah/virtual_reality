#pragma once

#include "util/util.hpp"
#include "util/Units.hpp"
#include "vr/graphics/vulkan/VulkanRenderer.hpp"
#include "vr/SessionConfig.hpp"
#include "geom/Geometry.hpp"
#include "xform/xforms.hpp"
#include "vr/Models.hpp"

#include <vulkanizer/builders.hpp>

#include <algorithm>
#include <array>

enum Hand : uint32_t { LEFT = 0, RIGHT };

struct FrameBufferAttachment {
    vr::Image image;
    VkImageView imageView{};
    VkFormat format{VK_FORMAT_UNDEFINED};
};

using ColorBuffer = FrameBufferAttachment;
using DepthBuffer = FrameBufferAttachment;

struct FrameBuffer {
    ColorBuffer color;
    VkFramebuffer _{};
};

struct CameraType {
    glm::mat4 view;
    glm::mat4 projection;
};

struct Mvp{
    glm::mat4 model{1};
    glm::mat4 view{1};
    glm::mat4 projection{1};
};

struct Cube {
    vr::Transform transform;
};

struct SpaceVisualization : public vr::VulkanRenderer {
public:
    ~SpaceVisualization() override = default;

    void init() override {
        createCubes();
        createFrameBufferAttachments();
        createRenderPass();
        createFrameBuffer();
        createDescriptorPool();
        createDescriptorSetLayout();
        updateDescriptorSet();
        createPipeline();
        createCommandBuffer();
        setupViews();
    }

    void createCubes() {
        using namespace units;
        auto cube = geom::cube();
        auto staging = graphicsService().createStagingBuffer(1_mb);

        debugBuffer = graphicsService().createMappableBuffer(1_kb, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        auto size = BYTE_SIZE(cube.vertices);
        auto mapping = graphicsService().map(staging);
        std::memcpy(mapping._, cube.vertices.data(), size);
        m_cube.vertex = graphicsService().createDeviceLocalBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        graphicsService().copy(staging, m_cube.vertex, size);

        size = BYTE_SIZE(cube.indices);
        m_cube.index = graphicsService().createDeviceLocalBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        std::memcpy(mapping._, cube.indices.data(), size);
        graphicsService().copy(staging, m_cube.index, size);
        graphicsService().release(staging);
    }

    void createRenderPass() {
        std::vector<VkAttachmentDescription> attachments {
                {   0,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                    VK_ATTACHMENT_STORE_OP_STORE,
                    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                },
                {
                    0,
                    depthFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                    VK_ATTACHMENT_STORE_OP_STORE,
                    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                }
        };

        VkAttachmentReference colorAttachment{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthAttachment{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subPassDescription{ 0, VK_PIPELINE_BIND_POINT_GRAPHICS};
        subPassDescription.colorAttachmentCount = 1;
        subPassDescription.pColorAttachments = &colorAttachment;
        subPassDescription.pDepthStencilAttachment = &depthAttachment;

        VkSubpassDependency dependency{};

        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        auto createInfo = makeStruct<VkRenderPassCreateInfo>();
        createInfo.attachmentCount = attachments.size();
        createInfo.pAttachments = attachments.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subPassDescription;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;

        m_renderPass = graphicsService().createRenderPass(createInfo);
    }

    void createFrameBufferAttachments() {
        const auto& swapChain = graphicsService().getSwapChain("main");
        const auto numImages = swapChain.images.size();

        VkImageViewCreateInfo createViewInfo = makeStruct<VkImageViewCreateInfo>();
        createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createViewInfo.format = swapChain.format;
        createViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createViewInfo.subresourceRange.levelCount = 1;
        createViewInfo.subresourceRange.baseMipLevel = 0;
        createViewInfo.subresourceRange.layerCount = 1;
        createViewInfo.subresourceRange.baseArrayLayer = 0;

        for (int vi = 0; vi < 2; vi++) {
            m_frameBuffers[vi].resize(numImages);

            for (int i = 0; i < numImages; i++) {
                m_frameBuffers[vi][i].color.image.handle = swapChain.images[i].image;

                createViewInfo.image = swapChain.images[i].image;
                createViewInfo.format = swapChain.format;
                createViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                createViewInfo.subresourceRange.baseArrayLayer = vi;
                m_frameBuffers[vi][i].color.imageView = graphicsService().createImageView(createViewInfo);
            }


            m_depthBuffers[vi].format = depthFormat;
            auto createImageInfo = makeStruct<VkImageCreateInfo>();
            createImageInfo.imageType = VK_IMAGE_TYPE_2D;
            createImageInfo.format = depthFormat;
            createImageInfo.extent = {swapChain.width, swapChain.height, 1};
            createImageInfo.mipLevels = 1;
            createImageInfo.arrayLayers = 1;
            createImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            createImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            createImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            createImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            m_depthBuffers[vi].image = graphicsService().createImage(createImageInfo);

            createViewInfo.format = m_depthBuffers[vi].format;
            createViewInfo.image = m_depthBuffers[vi].image.handle;
            createViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            createViewInfo.subresourceRange.baseArrayLayer = 0;
            m_depthBuffers[vi].imageView = graphicsService().createImageView(createViewInfo);
        }
    }

    void createFrameBuffer() {
        const auto& swapChain = graphicsService().getSwapChain("main");
        const auto numImages = swapChain.images.size();

        for (auto vi = 0; vi < 2; vi++) {
            auto depthAttachment = m_depthBuffers[vi].imageView;
            for (int i = 0; i < numImages; i++) {
                std::array<VkImageView, 2> attachments{};
                attachments[0] = m_frameBuffers[vi][i].color.imageView;
                attachments[1] = depthAttachment;

                auto createInfo = makeStruct<VkFramebufferCreateInfo>();
                createInfo.renderPass = m_renderPass;
                createInfo.attachmentCount = attachments.size();
                createInfo.pAttachments = attachments.data();
                createInfo.width = swapChain.width;
                createInfo.height = swapChain.height;
                createInfo.layers = 1;

                m_frameBuffers[vi][i]._ = graphicsService().createFrameBuffer(createInfo);
            }
        }
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{{
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1U},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1U}
        }};
        auto createInfo = makeStruct<VkDescriptorPoolCreateInfo>();
        createInfo.maxSets = 1;
        createInfo.poolSizeCount = poolSizes.size();
        createInfo.pPoolSizes = poolSizes.data();

        m_pool = graphicsService().createDescriptorPool(createInfo);
    }

    void createDescriptorSetLayout() {
        m_descriptorSetLayout =
            graphicsService().descriptorSetLayoutBuilder()
                .binding(0)
                    .descriptorType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
                    .descriptorCount(1)
                    .shaderStages(VK_SHADER_STAGE_VERTEX_BIT)
                .createLayout();

    }

    void updateDescriptorSet() {
        m_descriptorSet = graphicsService().allocate(m_pool, m_descriptorSetLayout).front();
        std::array<VkWriteDescriptorSet, 1 > writes {
                makeStruct<VkWriteDescriptorSet>()
        };
        
//        writes[0].dstSet = m_descriptorSet;
//        writes[0].dstBinding = 0;
//        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//        writes[0].descriptorCount = 1;
//        VkDescriptorBufferInfo cameraInfo{ m_camera.gpu.handle, 0, VK_WHOLE_SIZE};
//        writes[0].pBufferInfo = &cameraInfo;

        writes[0].dstSet = m_descriptorSet;
        writes[0].dstBinding = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[0].descriptorCount = 1;
        VkDescriptorBufferInfo xformInfo{ debugBuffer._, 0, VK_WHOLE_SIZE};
        writes[0].pBufferInfo = &xformInfo;

        graphicsService().update(writes);

    }

    void createPipeline() {
        const auto& swapChain = graphicsService().getSwapChain("main");
        m_pipeline._ =
            graphicsService().graphicsPipelineBuilder()
                .shaderStage()
                    .vertexShader(R"(..\..\..\src\resources\shaders\geom.vert.spv)")
                    .fragmentShader(R"(..\..\..\src\resources\shaders\geom.frag.spv)")
                .vertexInputState()
                    .addVertexBindingDescription(0, sizeof(geom::Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
                    .addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetOf(geom::Vertex, position))
                    .addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetOf(geom::Vertex, normal))
                    .addVertexAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetOf(geom::Vertex, tangent))
                    .addVertexAttributeDescription(3, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetOf(geom::Vertex, bitangent))
                    .addVertexAttributeDescription(4, 0, VK_FORMAT_R32G32_SFLOAT,       offsetOf(geom::Vertex, uv))
                    .addVertexAttributeDescription(5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetOf(geom::Vertex, color))
                .inputAssemblyState()
                    .triangles()
                .viewportState()
                    .viewport()
                        .origin(0, 0)
                        .dimension(swapChain.width, swapChain.height)
                        .minDepth(0)
                        .maxDepth(1)
                    .scissor()
                        .offset(0, 0)
                        .extent({swapChain.width, swapChain.height})
                    .add()
                .rasterizationState()
                    .polygonModeFill()
                    .cullBackFace()
                    .frontFaceCounterClockwise()
                    .lineWidth(1)
                .multisampleState()
                    .rasterizationSamples(VK_SAMPLE_COUNT_1_BIT)
                .depthStencilState()
                    .enableDepthTest()
                    .enableDepthWrite()
                    .compareOpLess()
                    .minDepthBounds(0)
                    .maxDepthBounds(1)
                .colorBlendState()
                    .attachment()
                    .add()
                .layout()
                    .addDescriptorSetLayout(m_descriptorSetLayout)
                    .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mvp))
                .renderPass(m_renderPass)
                .subpass(0)
                .name("space_visualization")
            .build(m_pipeline.layout);
    }

    void createCommandBuffer() {
        auto cmdBuffers = graphicsService().allocateCommandBuffers(2);
        m_commandBuffers = std::vector<VkCommandBuffer>(cmdBuffers.begin(), cmdBuffers.end());
    }

    void setupViews() {
        const auto swapChain = graphicsService().getSwapChain("main");
        XrSwapchainSubImage subImage{
            swapChain._,
            {{0, 0}, {static_cast<int32_t>(swapChain.width), static_cast<int32_t>(swapChain.height)}},
            0
        };
        m_views[0].subImage = subImage;
        m_views[1].subImage = subImage;
        m_views[1].subImage.imageArrayIndex = 1;


        m_projectionLayer.viewCount = 2;
        m_projectionLayer.views = m_views.data();
    }

    void beginFrame() override {
        m_cubes.clear();
    }

    void set(const std::vector<vr::SpaceLocation> &spaceLocations) final {
        Cube cube{};
        for(const auto & spaceLocation : spaceLocations) {
            cube.transform.pose = spaceLocation.pose;
            cube.transform.scale = glm::vec3(0.25);
            m_cubes.push_back(cube);
        }
    }

    std::vector<vr::Vibrate> set(const vr::ActionSet &actionSet) final {
        std::vector<vr::Vibrate> vibrations{};
        if(actionSet.get("left_hand_squeeze").isActive) {
            auto value = actionSet.get("left_hand_squeeze").value<float>();
            handScale[Hand::LEFT] = 1.0f - 0.5f * value;

            if(value > 0.9) {
                vibrations.push_back({"vibrate_left"});
            }
        }
        if(actionSet.get("right_hand_squeeze").isActive) {
            auto value = actionSet.get("right_hand_squeeze").value<float>();
            handScale[Hand::RIGHT] = 1.0f - 0.5f * value;
            if(value > 0.9) {
                vibrations.push_back({"vibrate_right"});
            }
        }

        Cube cube{};
        if(actionSet.get("left_hand_pose").isActive) {
            cube.transform.pose = actionSet.get("left_hand_pose").value<vr::Pose>();
            cube.transform.scale = glm::vec3(0.1) * handScale[Hand::LEFT];
            m_cubes.push_back(cube);
        }
        if(actionSet.get("right_hand_pose").isActive) {
            cube.transform.pose = actionSet.get("right_hand_pose").value<vr::Pose>();
            cube.transform.scale = glm::vec3(0.1) * handScale[Hand::RIGHT];
            m_cubes.push_back(cube);
        }

        return vibrations;
    }

    void paused(const vr::FrameInfo &frameInfo, vr::Layers& layers) override {
        return render(frameInfo, layers);
    }

    void render(const vr::FrameInfo &frameInfo, vr::Layers& layers) override {
        renderCubes(frameInfo);

        m_views[0].pose = frameInfo.viewInfo.views[0].pose;
        m_views[1].pose = frameInfo.viewInfo.views[1].pose;
        m_views[0].fov = frameInfo.viewInfo.views[0].fov;
        m_views[1].fov = frameInfo.viewInfo.views[1].fov;
        m_projectionLayer.space = frameInfo.space;

        return layers.push_back( { &m_projectionLayer } );
    }

    void renderCubes(const vr::FrameInfo &frameInfo) {
        const auto& views = frameInfo.viewInfo.views;

        for (auto vi = 0; vi < views.size(); ++vi) {
            auto& view = views[vi];
            const auto &swapChain = graphicsService().getSwapChain("main");
            mvp.view = glm::inverse(vr::toMatrix(view.pose));
            mvp.projection = graphicsService().projection(view.fov, 0.05, 100);

            auto commandBuffer = m_commandBuffers[vi];
            auto beginInfo = makeStruct<VkCommandBufferBeginInfo>();
            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.184313729f, 0.309803933f, 0.309803933f, 1.f};
            clearValues[1].depthStencil = {1.0, 0u};
            auto renderPassInfo = makeStruct<VkRenderPassBeginInfo>();
            renderPassInfo.renderPass = m_renderPass;
            renderPassInfo.framebuffer = m_frameBuffers[vi][frameInfo.imageId.imageIndex]._;
            renderPassInfo.renderArea = {{0,               0},
                                         {swapChain.width, swapChain.height}};
            renderPassInfo.clearValueCount = 2;
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline._);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout, 0, 1,
                                    &m_descriptorSet, 0, VK_NULL_HANDLE);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_cube.vertex._, &offset);
            vkCmdBindIndexBuffer(commandBuffer, m_cube.index._, 0, VK_INDEX_TYPE_UINT32);
            uint32_t indexCount = m_cube.index.info.size / sizeof(uint32_t);

            for (const auto& cube : m_cubes) {
                mvp.model = static_cast<glm::mat4>(cube.transform);
                vkCmdPushConstants(commandBuffer, m_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp),
                                   &mvp.model);
                vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
            }
            vkCmdEndRenderPass(commandBuffer);
            vkEndCommandBuffer(commandBuffer);
        }
        auto submitInfo = makeStruct<VkSubmitInfo>();
        submitInfo.commandBufferCount = m_commandBuffers.size();
        submitInfo.pCommandBuffers = m_commandBuffers.data();
        graphicsService().submitToGraphicsQueue(submitInfo);

    }

    static vr::SessionConfig session() {
        return
        vr::SessionConfig()
            .addSpace(
                    ReferenceSpaceSpecification()
                            .name("ViewFront")
                            .view()
                            .pose()
                            .translate(0, 0, -2)
                            .orientation(1, 0, 0, 0)
            )
            .addSpace(ReferenceSpaceSpecification().name("Local").local())
            .addSpace(ReferenceSpaceSpecification().name("Stage").stage())
            .addSpace(
                ReferenceSpaceSpecification()
                    .name("StageLeft")
                    .stage()
                    .pose()
                        .translate(-2, 0, -2)
                        .rotateY(0)
            )
            .addSpace(
                ReferenceSpaceSpecification()
                    .name("StageRight")
                    .stage()
                    .pose()
                        .translate(2, 0, -2)
                        .rotateY(0)
            )
            .addSpace(
                ReferenceSpaceSpecification()
                    .name("StageLeftRotated")
                    .stage()
                    .pose()
                        .translate(-2, 0.5, -2)
                        .rotateY(60)
            )
            .addSpace(
                ReferenceSpaceSpecification()
                    .name("StageRightRotated")
                    .stage()
                    .pose()
                        .translate(2, 0.5, -2)
                        .rotateInverseY(60)
            )
            .addActionSet(
                vr::ActionSetSpecification()
                    .name("main_action_set")
                    .description("main action set")
                    .addAction("left_hand_pose", vr::Location::LEFT_HAND, vr::Identifier::GRIP, vr::Component::POSE, "left hand grip")
                    .addAction("right_hand_pose", vr::Location::RIGHT_HAND, vr::Identifier::GRIP, vr::Component::POSE, "right hand grip")
                    .addAction("left_hand_squeeze", vr::Location::LEFT_HAND, vr::Identifier::SQUEEZE, vr::Component::VALUE, "left hand squeeze")
                    .addAction("right_hand_squeeze", vr::Location::RIGHT_HAND, vr::Identifier::SQUEEZE, vr::Component::VALUE, "right hand squeeze")
                    .addAction("vibrate_left", vr::Location::LEFT_HAND, vr::Identifier::HAPTIC, vr::Component::VIBRATE, "vibrate left hand")
                    .addAction("vibrate_right", vr::Location::RIGHT_HAND, vr::Identifier::HAPTIC, vr::Component::VIBRATE, "vibrate right hand")
             )
            .addSwapChain(
                vr::SwapchainSpecification()
                .name("main")
                .usage()
                    .colorAttachment()
                .format(VK_FORMAT_R8G8B8A8_SRGB)
                .arraySize(4)
                .width(2064)
                .height(2096));
    }

    static std::shared_ptr<Renderer> shared() {
        return std::make_shared<SpaceVisualization>();
    }

    vr::cstring name() override {
        return "Space Visualization";
    }

private:
    struct {
        vr::Buffer vertex;
        vr::Buffer index;
    } m_cube;

    vr::Buffer debugBuffer;

    struct {
        VkPipelineLayout layout;
        VkPipeline _;
    } m_pipeline{};

    VkDescriptorPool m_pool{};
    VkDescriptorSetLayout m_descriptorSetLayout{};
    VkDescriptorSet m_descriptorSet{};
    VkRenderPass m_renderPass{};
    std::array<std::vector<FrameBuffer>, 2> m_frameBuffers;
    std::array<DepthBuffer, 2> m_depthBuffers;
    static constexpr VkFormat depthFormat{ VK_FORMAT_D32_SFLOAT_S8_UINT };
    std::vector<VkCommandBuffer> m_commandBuffers;
    mutable XrCompositionLayerProjection m_projectionLayer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
    std::array<XrCompositionLayerProjectionView, 2> m_views {{
         {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW},
         {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}
     }};
    std::vector<Cube> m_cubes;
    Mvp mvp{};
    std::array<float, 2> handScale{1, 1};
};
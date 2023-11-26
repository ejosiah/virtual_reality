#pragma once

#include "util/util.hpp"
#include "util/Units.hpp"
#include "vr/graphics/vulkan/VulkanRenderer.hpp"
#include "vr/SessionConfig.hpp"
#include "geom/Geometry.hpp"
#include "xform/xforms.hpp"

#include <algorithm>
#include <array>

struct FrameBufferAttachment {
    vr::Image image;
    VkImageView imageView{};
    VkFormat format{VK_FORMAT_UNDEFINED};
};

using ColorBuffer = FrameBufferAttachment;
using DepthBuffer = FrameBufferAttachment;

struct FrameBuffer {
    ColorBuffer color;
    DepthBuffer  depth;
    VkFramebuffer _{};
};

struct CameraType {
    glm::mat4 view;
    glm::mat4 projection;
};

using Camera = vr::Link<CameraType>;
using Transforms = vr::Link<glm::mat4>;

struct SpaceVisualization : public vr::VulkanRenderer {
public:
    ~SpaceVisualization() override = default;

    void init() override {
        createCubes();
        initCamera();
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

        auto size = BYTE_SIZE(cube.vertices);
        graphicsService().map(staging);
        std::memcpy(staging.mapping, cube.vertices.data(), size);
        m_cube.vertex = graphicsService().createDeviceLocalBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        graphicsService().copy(staging, m_cube.vertex, size);

        size = BYTE_SIZE(cube.indices);
        m_cube.index = graphicsService().createDeviceLocalBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        graphicsService().copy(staging, m_cube.index, size);
        graphicsService().release(staging);

        m_instances.transforms = graphicsService().link<glm::mat4>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, numInstances);

    }

    void initCamera() {
        m_camera = graphicsService().link<CameraType>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    }

    void createRenderPass() {
        std::vector<VkAttachmentDescription> attachments {
                {   0,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                    VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                },
                {
                    0,
                    m_frameBuffers.front().depth.format,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                    VK_ATTACHMENT_STORE_OP_DONT_CARE,
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

        m_frameBuffers.resize( numImages );

        for(int i = 0; i < numImages; i++){
            m_frameBuffers[i].color.image.handle = swapChain.images[i].image;

            createViewInfo.image = swapChain.images[i].image;
            m_frameBuffers[i].color.imageView = graphicsService().createImageView(createViewInfo);
        }


        for(int i = 0; i < numImages; i++) {
            m_frameBuffers[i].depth.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
            auto createImageInfo = makeStruct<VkImageCreateInfo>();
            createImageInfo.imageType = VK_IMAGE_TYPE_2D;
            createImageInfo.format = m_frameBuffers[i].depth.format;
            createImageInfo.extent = { swapChain.width, swapChain.height, 1};
            createImageInfo.mipLevels = 1;
            createImageInfo.arrayLayers = 1;
            createImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            createImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            createImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            createImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            m_frameBuffers[i].depth.image = graphicsService().creatImage(createImageInfo);

            createViewInfo.format =  m_frameBuffers[i].depth.format;
            createViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            createViewInfo.image = m_frameBuffers[i].depth.image.handle;
            m_frameBuffers[i].depth.imageView = graphicsService().createImageView(createViewInfo);
        }
    }

    void createFrameBuffer() {
        const auto& swapChain = graphicsService().getSwapChain("main");
        const auto numImages = swapChain.images.size();

        for(int i = 0; i < numImages; i++){
            std::array<VkImageView, 2> attachments{};
            attachments[0] = m_frameBuffers[i].color.imageView;
            attachments[1] = m_frameBuffers[i].depth.imageView;

            auto createInfo = makeStruct<VkFramebufferCreateInfo>();
            createInfo.renderPass = m_renderPass;
            createInfo.attachmentCount = 2;
            createInfo.pAttachments = attachments.data();
            createInfo.width = swapChain.width;
            createInfo.height = swapChain.height;
            createInfo.layers = 1;

            m_frameBuffers[i]._ = graphicsService().createFrameBuffer(createInfo);
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
        std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        
        auto createInfo = makeStruct<VkDescriptorSetLayoutCreateInfo>();
        createInfo.flags = 0;
        createInfo.bindingCount = bindings.size();
        createInfo.pBindings = bindings.data();
        
        
        m_descriptorSetLayout = graphicsService().createDescriptorSetLayout(createInfo);
        
    }

    void updateDescriptorSet() {
        m_descriptorSet = graphicsService().allocate(m_pool, m_descriptorSetLayout).front();
        std::array<VkWriteDescriptorSet, 2 > writes {
                makeStruct<VkWriteDescriptorSet>(),
                makeStruct<VkWriteDescriptorSet>()
        };
        
        writes[0].dstSet = m_descriptorSet;
        writes[0].dstBinding = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].descriptorCount = 1;
        VkDescriptorBufferInfo cameraInfo{ m_camera.gpu.handle, 0, VK_WHOLE_SIZE};
        writes[0].pBufferInfo = &cameraInfo;

        writes[1].dstSet = m_descriptorSet;
        writes[1].dstBinding = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[1].descriptorCount = 1;
        VkDescriptorBufferInfo xformInfo{ m_instances.transforms.gpu.handle, 0, VK_WHOLE_SIZE};
        writes[1].pBufferInfo = &xformInfo;

        graphicsService().update(writes);

    }

    void createPipeline() {
        const auto& swapChain = graphicsService().getSwapChain("main");
        const auto fWidth = static_cast<float>(swapChain.width);
        const auto fHeight = static_cast<float>(swapChain.height);

        // ShaderStage
        auto vModule = graphicsService().createShaderModule(R"(..\..\..\src\resources\shaders\geom.vert.spv)");
        auto vertexShaderStage = makeStruct<VkPipelineShaderStageCreateInfo>();
        vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStage.module = vModule;
        vertexShaderStage.pName = "main";


        auto fModule = graphicsService().createShaderModule(R"(..\..\..\src\resources\shaders\geom.frag.spv)");
        auto fragmentShaderModule = makeStruct<VkPipelineShaderStageCreateInfo>();
        fragmentShaderModule.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderModule.module = fModule;
        fragmentShaderModule.pName = "main";

        std::array<VkPipelineShaderStageCreateInfo, 2> stages{ vertexShaderStage, fragmentShaderModule };

        // Vertex Input State
        VkVertexInputBindingDescription vertexBinding{ 0, sizeof(geom::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };
        std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions {{
                {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetOf(geom::Vertex, position)},
                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetOf(geom::Vertex, normal)},
                {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetOf(geom::Vertex, tangent)},
                {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetOf(geom::Vertex, bitangent)},
                {4, 0, VK_FORMAT_R32G32_SFLOAT, offsetOf(geom::Vertex, uv)},
                {5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetOf(geom::Vertex, color)},
        }};
        auto vertexInputState = makeStruct<VkPipelineVertexInputStateCreateInfo>();
        vertexInputState.vertexBindingDescriptionCount = 1;
        vertexInputState.pVertexBindingDescriptions = &vertexBinding;
        vertexInputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
        vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();


        // Input Assembly
        auto inputAssemblyState = makeStruct<VkPipelineInputAssemblyStateCreateInfo>();
        inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyState.primitiveRestartEnable = VK_FALSE;

        // Tessellation State
        auto tessellationState = makeStruct<VkPipelineTessellationStateCreateInfo>();

        // View port
        VkViewport viewport{ 0, 0, fWidth, fHeight, 0, 1};
        VkRect2D scissor{{0, 0}, {swapChain.width, swapChain.height}};
        auto viewportState = makeStruct<VkPipelineViewportStateCreateInfo>();
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // Rasterization State
        auto rasterState = makeStruct<VkPipelineRasterizationStateCreateInfo>();
        rasterState.depthClampEnable = VK_FALSE;
        rasterState.polygonMode = VK_POLYGON_MODE_FILL;
        rasterState.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterState.lineWidth = 1.0f;

        // Multisample state
        auto multisampleState = makeStruct<VkPipelineMultisampleStateCreateInfo>();
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Depth and Stencil state
        auto depthStencilState = makeStruct<VkPipelineDepthStencilStateCreateInfo>();
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.minDepthBounds = 0;
        depthStencilState.maxDepthBounds = 1;
        
        // color blend state
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        auto colorBlendState = makeStruct<VkPipelineColorBlendStateCreateInfo>();
        colorBlendState.attachmentCount = 1;
        colorBlendState.pAttachments = &colorBlendAttachment;

        // Dynamic state
        auto dynamicState = makeStruct<VkPipelineDynamicStateCreateInfo>();

        auto pipelineLayoutCreateInfo = makeStruct<VkPipelineLayoutCreateInfo>();
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;


        // pipeline layout
        m_pipeline.layout = graphicsService().createPipelineLayout(pipelineLayoutCreateInfo);


        auto createInfo = makeStruct<VkGraphicsPipelineCreateInfo>();
        createInfo.stageCount = stages.size();
        createInfo.pStages = stages.data();
        createInfo.pVertexInputState = &vertexInputState;
        createInfo.pInputAssemblyState = &inputAssemblyState;
        createInfo.pTessellationState = &tessellationState;
        createInfo.pViewportState = &viewportState;
        createInfo.pRasterizationState = &rasterState;
        createInfo.pMultisampleState = &multisampleState;
        createInfo.pDepthStencilState = &depthStencilState;
        createInfo.pColorBlendState = &colorBlendState;
        createInfo.pDynamicState = &dynamicState;
        createInfo.layout = m_pipeline.layout;
        createInfo.renderPass = m_renderPass;
        createInfo.subpass = 0;

        m_pipeline._ = graphicsService().createGraphicsPipeline(createInfo);

    }

    void createCommandBuffer() {
        const auto& swapChain = graphicsService().getSwapChain("main");
        const auto numImages = swapChain.images.size();
        auto commandBuffers = graphicsService().allocateCommandBuffers(numImages);

        for(auto i = 0; i < numImages; ++i) {
            auto commandBuffer = commandBuffers[i];
            auto beginInfo = makeStruct<VkCommandBufferBeginInfo>();
            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.184313729f, 0.309803933f, 0.309803933f, 1.f};
            clearValues[1].depthStencil = {1.0, 0u};
            auto renderPassInfo = makeStruct<VkRenderPassBeginInfo>();
            renderPassInfo.renderPass = m_renderPass;
            renderPassInfo.framebuffer = m_frameBuffers[i]._;
            renderPassInfo.renderArea = {{0, 0}, {swapChain.width, swapChain.height}};
            renderPassInfo.clearValueCount = 2;
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

//            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline._);
//            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout
//                                    , 0, 1, &m_descriptorSet, 0, VK_NULL_HANDLE);
//
//            VkDeviceSize offset = 0;
//            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_cube.vertex.handle, &offset);
//            vkCmdBindIndexBuffer(commandBuffer, m_cube.index.handle, 0, VK_INDEX_TYPE_UINT32);
//            uint32_t indexCount = m_cube.index.info.size / sizeof(uint32_t);
//            vkCmdDrawIndexed(commandBuffer, indexCount, numInstances, 0, 0, 0);
//
            vkCmdEndRenderPass(commandBuffer);
            vkEndCommandBuffer(commandBuffer);

            m_commandBuffers.push_back(commandBuffer);
        }
    }

    void setupViews() {
        const auto swapChain = graphicsService().getSwapChain("main");
        XrSwapchainSubImage subImage{
            swapChain.swapchain,
            {{0, 0}, {static_cast<int32_t>(swapChain.width), static_cast<int32_t>(swapChain.height)}},
            0
        };
        m_views[0].subImage = subImage;
        m_views[1].subImage = subImage;

        m_projectionLayer.viewCount = 2;
        m_projectionLayer.views = m_views.data();
    }


    void set(const std::vector<vr::SpaceLocation> &spaceLocations) override {
        for(auto i = 0; i < spaceLocations.size(); ++i) {
            auto spaceLocation = spaceLocations[i];
            spaceLocation.pose.scale = glm::vec3(0.25);
            m_instances.transforms.cpu[i] = vr::toMatrix(spaceLocation.pose);
        }
    }

    vr::FrameEnd paused(const vr::FrameInfo &frameInfo) override {
        return render(frameInfo);
    }

    vr::FrameEnd render(const vr::FrameInfo &frameInfo) override {
        renderCubes(frameInfo);

        m_views[0].pose = frameInfo.viewInfo.views[0].pose;
        m_views[1].pose = frameInfo.viewInfo.views[1].pose;
        m_views[0].fov = frameInfo.viewInfo.views[0].fov;
        m_views[1].fov = frameInfo.viewInfo.views[1].fov;
        m_projectionLayer.space = frameInfo.space;
        vr::FrameEnd frameEnd{};
        frameEnd.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&m_projectionLayer));

        return frameEnd;
    }

    void renderCubes(const vr::FrameInfo &frameInfo) {
        const auto& view = frameInfo.viewInfo.views.front();
        auto fov = view.fov.angleRight - view.fov.angleLeft;
        m_camera.cpu->view = vr::toMatrix(view.pose);
        m_camera.cpu->projection = xform::perspectiveHFov(fov, frameInfo.aspectRatio, 0.1, 5.0);
        auto commandBuffer = m_commandBuffers[frameInfo.imageId.imageIndex];

        auto submitInfo = makeStruct<VkSubmitInfo>();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        graphicsService().submitToGraphicsQueue(submitInfo);
    }

    static vr::SessionConfig session() {
        return
        vr::SessionConfig()
            .addSpace(ReferenceSpaceSpecification().name("Local").local())
            .addSpace(ReferenceSpaceSpecification().name("Stage").stage())
            .addSpace(
                ReferenceSpaceSpecification()
                    .name("ViewFront")
                    .view()
                    .pose()
                        .translate(0, 0, -2)
                        .orientation(1, 0, 0, 0)
            )
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
            .addSwapChain(
                vr::SwapchainSpecification()
                .name("main")
                .usage()
                    .colorAttachment()
                .format(VK_FORMAT_R8G8B8A8_SRGB)
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

    struct {
        Transforms transforms;
    } m_instances;

    struct {
        VkPipelineLayout layout;
        VkPipeline _;
    } m_pipeline;

    Camera m_camera;
    VkDescriptorPool m_pool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorSet m_descriptorSet;
    VkRenderPass m_renderPass;
    std::vector<FrameBuffer> m_frameBuffers;
    std::vector<VkCommandBuffer> m_commandBuffers;
    mutable XrCompositionLayerProjection m_projectionLayer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
    std::array<XrCompositionLayerProjectionView, 2> m_views {{
         {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW},
         {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}
     }};
    static constexpr uint32_t numInstances{7};
};
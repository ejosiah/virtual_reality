#include "check.hpp"
#include "vr/graphics/vulkan/VulkanGraphicsService.hpp"
#include "io/FileReader.hpp"

namespace vr {

    VulkanGraphicsService::VulkanGraphicsService(const vr::Context &context) : GraphicsService(context) {
        if (!(reinterpret_cast<VulkanContext *>(context.graphicsContext.get()))) {
            throw std::runtime_error{"invalid context, Vulkan context expected"};
        }
    }

    void VulkanGraphicsService::init() {
        pickDevice();
        setupQueues();
        createDevice();
        createInternalCommandPool();
        initMemoryAllocator();
        initializeGraphicsBinding();
        logDevice();
    }

    void VulkanGraphicsService::pickDevice() {
        auto xrInstance = context().instance;
        auto getInfo = makeStruct<XrVulkanGraphicsDeviceGetInfoKHR>();
        getInfo.systemId = context().systemId;
        getInfo.vulkanInstance = vulkanContext().instance;
        LOG_ERROR(xrInstance, xrGetVulkanGraphicsDevice2KHR(xrInstance, &getInfo, &m_physicalDevice))
    }

    void VulkanGraphicsService::setupQueues() {
        assert(m_physicalDevice != VK_NULL_HANDLE);
        auto queueFamilies = get<VkQueueFamilyProperties>(m_physicalDevice, vkGetPhysicalDeviceQueueFamilyProperties);

        for(auto i = 0; i < queueFamilies.size(); i++) {
            const auto& queueFamily = queueFamilies[i];
            if((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
                m_graphicsFamilyIndex = i;
                break;
            }
        }
    }

    void VulkanGraphicsService::createDevice() {
        assert(m_physicalDevice != VK_NULL_HANDLE);
        auto xrInstance = context().instance;
        auto priority = 1.f;
        VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        queueCreateInfo.queueFamilyIndex = m_graphicsFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &priority;

        std::vector<const char*> extensions{
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        auto dynamicRenderingFeatures = makeStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkDeviceCreateInfo createDeviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        createDeviceInfo.pNext = &dynamicRenderingFeatures;
        createDeviceInfo.queueCreateInfoCount = 1;
        createDeviceInfo.pQueueCreateInfos = &queueCreateInfo;

        auto createInfo = makeStruct<XrVulkanDeviceCreateInfoKHR>();
        createInfo.systemId = context().systemId;
        createInfo.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        createInfo.vulkanPhysicalDevice = m_physicalDevice;
        createInfo.vulkanCreateInfo = &createDeviceInfo;
        VkPhysicalDeviceFeatures features{};
        features.shaderStorageImageMultisample = VK_TRUE;
        createDeviceInfo.pEnabledFeatures = &features;

        VkResult result;
        LOG_ERROR(xrInstance, xrCreateVulkanDeviceKHR(xrInstance, &createInfo, &m_device, &result));

        vkGetDeviceQueue(m_device, m_graphicsFamilyIndex, 0, &m_graphicsQueue);
    }

    void VulkanGraphicsService::createInternalCommandPool() {

        VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        createInfo.queueFamilyIndex = m_graphicsFamilyIndex;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        CHECK_VULKAN(vkCreateCommandPool(m_device, &createInfo, VK_NULL_HANDLE, &m_commandPool));

        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        CHECK_VULKAN(vkCreateCommandPool(m_device, &createInfo, VK_NULL_HANDLE, &m_scopedCommandPool));
    }

    void VulkanGraphicsService::initMemoryAllocator() {
        allocator = VmaMemoryAllocator{ vulkanContext().instance, m_physicalDevice, m_device};
        allocator.init();
    }

    void VulkanGraphicsService::initializeGraphicsBinding() {
        m_bindingInfo.instance = vulkanContext().instance;
        m_bindingInfo.physicalDevice = m_physicalDevice;
        m_bindingInfo.device = m_device;
        m_bindingInfo.queueFamilyIndex = m_graphicsFamilyIndex;
        m_bindingInfo.queueIndex = 0;
        initialized = true;
    }

    void VulkanGraphicsService::logDevice() {
        auto apiVersion = vulkanContext().apiVersion;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
        std::stringstream  ss;

        ss << std::format("Graphics provider: Vulkan {}.{}.{}, device: {}"
                , VK_VERSION_MAJOR(apiVersion)
                , VK_VERSION_MINOR(apiVersion)
                , VK_VERSION_PATCH(apiVersion)
                , properties.deviceName );
        spdlog::info("{}", ss.str());
    }

    const VulkanContext &VulkanGraphicsService::vulkanContext() const {
        return *reinterpret_cast<const VulkanContext*>(context().graphicsContext.get());
    }

    const XrBaseInStructure &VulkanGraphicsService::graphicsBinding() const {
        initGuard();
        return *reinterpret_cast<const XrBaseInStructure*>(&m_bindingInfo);
    }

    int64_t VulkanGraphicsService::swapChainFormat() const {
        return vulkanContext().swapChainImageFormat;
    }

    VkQueue VulkanGraphicsService::queue() const {
        return m_graphicsQueue;
    }

    Buffer VulkanGraphicsService::createStagingBuffer(VkDeviceSize size) {
        auto createInfo = makeStruct<VkBufferCreateInfo>();
        createInfo.size = size;
        createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto buffer = allocator.allocate(createInfo, VMA_MEMORY_USAGE_CPU_ONLY);
        m_buffers.push_back(buffer);
        return buffer;
    }

    Buffer VulkanGraphicsService::createDeviceLocalBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
        auto createInfo = makeStruct<VkBufferCreateInfo>();
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto buffer =  allocator.allocate(createInfo, VMA_MEMORY_USAGE_GPU_ONLY);
        m_buffers.push_back(buffer);
        return buffer;
    }

    Buffer VulkanGraphicsService::createMappableBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
        auto createInfo = makeStruct<VkBufferCreateInfo>();
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto buffer = allocator.allocate(createInfo, VMA_MEMORY_USAGE_CPU_TO_GPU);
        m_buffers.push_back(buffer);
        return buffer;
    }

    void VulkanGraphicsService::release(Buffer buffer) {
        if(buffer.mapping != nullptr) {
            unmap(buffer);
        }
        auto itr = std::find_if(m_buffers.begin(), m_buffers.end(), [&buffer](const auto& buf) {
            return buf.handle == buffer.handle;
        });
        assert(itr != m_buffers.end());
        m_buffers.erase(itr);
        allocator.deallocate(buffer);
    }

    VkCommandBuffer VulkanGraphicsService::commandBuffer(uint32_t imageIndex) {
        assert(!m_commandBuffers.empty() && imageIndex < m_commandBuffers.size());
        return m_commandBuffers[imageIndex];
    }

    std::shared_ptr<GraphicsService> VulkanGraphicsService::shared(const Context &context) {
        return std::make_shared<VulkanGraphicsService>(context);
    }

    void VulkanGraphicsService::setSwapChains(std::vector<SwapChain> swapchains) {
        XrResult result;
        for(auto swapchain : swapchains) {
            const auto spec = swapchain.spec;
            VulkanSwapChain vulkanSwapChain{spec._name, swapchain.handle, spec._width
                                            , spec._height, static_cast<VkFormat>(spec._format)};
            std::tie(result, vulkanSwapChain.images) =
                enumerate<XrSwapchainImageVulkanKHR>([&](auto sizePtr, auto structPtr) {
                    return xrEnumerateSwapchainImages(swapchain.handle, *sizePtr, sizePtr, reinterpret_cast<XrSwapchainImageBaseHeader *>(structPtr));
                });
            LOG_ERROR(m_context.instance, result);
            m_swapChains.push_back(vulkanSwapChain);
        }
    }

    const VulkanSwapChain &VulkanGraphicsService::getSwapChain(const std::string &name) {
        auto itr = std::find_if(m_swapChains.begin(), m_swapChains.end(), [&name](const auto& sc){ return name == sc.name; });
        if(itr == m_swapChains.end()){
            THROW(std::format("swapChain[{}] not found", name));
        }
        return *itr;
    }

    VkImageView VulkanGraphicsService::createImageView(VkImageViewCreateInfo createInfo) {
        VkImageView view;
        vkCreateImageView(m_device, &createInfo, nullptr, &view);
        
        return view;
    }

    void VulkanGraphicsService::copyToImage(const CopyRequest &request) {
        const auto& swapChain = getSwapChain(request.imageId.swapChain);
        auto image = swapChain.images[request.imageId.imageIndex].image;

        scoped([&](VkCommandBuffer commandBuffer) {
            auto barrier = makeStruct<VkImageMemoryBarrier>();
            barrier.srcAccessMask = VK_ACCESS_NONE;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = request.mipLevel;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = request.arrayLayer;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &barrier);

            VkBufferImageCopy region{0, 0, 0};
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {swapChain.width, swapChain.height, 1};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = request.mipLevel;
            region.imageSubresource.baseArrayLayer = request.arrayLayer;
            region.imageSubresource.layerCount = 1;
            vkCmdCopyBufferToImage(commandBuffer, request.source.handle, image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_NONE;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &barrier);
        });
    }

    VkDescriptorPool VulkanGraphicsService::createDescriptorPool(const VkDescriptorPoolCreateInfo &createInfo) {
        VkDescriptorPool pool;
        CHECK_VULKAN(vkCreateDescriptorPool(m_device, &createInfo, nullptr, &pool));
        return pool;
    }

    VkDescriptorSetLayout VulkanGraphicsService::createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo createInfo) {
        VkDescriptorSetLayout setLayout;
        CHECK_VULKAN(vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &setLayout));
        return setLayout;
    }

    void VulkanGraphicsService::copy(const Buffer &src, const Buffer &dst, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
        scoped([&](auto commandBuffer){
           VkBufferCopy region{srcOffset, dstOffset, size};
            vkCmdCopyBuffer(commandBuffer, src.handle, dst.handle, 1, &region);
        });
    }

    std::vector<VkDescriptorSet> VulkanGraphicsService::allocate(VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t numSets) {
        std::vector<VkDescriptorSet> sets(numSets);
        std::vector<VkDescriptorSetLayout> layouts(numSets, layout);
        auto allocInfo = makeStruct<VkDescriptorSetAllocateInfo>();
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = numSets;
        allocInfo.pSetLayouts = layouts.data();

        CHECK_VULKAN(vkAllocateDescriptorSets(m_device, &allocInfo, sets.data()));
        return sets;
    }

    void VulkanGraphicsService::update(std::span<VkWriteDescriptorSet> writes) {
        vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
    }

    void VulkanGraphicsService::shutdown() {
        for(const auto& buffer : m_buffers) {
            allocator.deallocate(buffer);
        }
        for(auto pipeline : m_pipelines){
            vkDestroyPipeline(m_device, pipeline, nullptr);
        }
        for(auto layout : m_pipelineLayouts) {
            vkDestroyPipelineLayout(m_device, layout, nullptr);
        }
        for(auto pool : m_descriptorPools) {
            vkDestroyDescriptorPool(m_device, pool, nullptr);
        }
        for(auto setLayout : m_descriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(m_device, setLayout, nullptr);
        }

        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        vkDestroyCommandPool(m_device, m_scopedCommandPool, nullptr);
        for(auto commandPool : m_commandPools) {
            vkDestroyCommandPool(m_device, commandPool, nullptr);
        }
        allocator.destroy();
    }


    void VulkanGraphicsService::initGuard() const {
        if(!initialized){
            throw std::runtime_error{"init has not yet been called on GraphicsService"};
        }
    }

    std::span<VkCommandBuffer> VulkanGraphicsService::allocateCommandBuffers(uint32_t size) {
        VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = size;

        m_commandBuffers.resize(allocateInfo.commandBufferCount);

        std::span<VkCommandBuffer> commandBuffers = { m_commandBuffers.data()  +  numCommandBuffers, size };
        CHECK_VULKAN(vkAllocateCommandBuffers(m_device, &allocateInfo, m_commandBuffers.data()));
        numCommandBuffers += size;
        return commandBuffers;
    }

    VkPipelineLayout VulkanGraphicsService::createPipelineLayout(const VkPipelineLayoutCreateInfo &createInfo) {
        VkPipelineLayout layout;
        CHECK_VULKAN(vkCreatePipelineLayout(m_device, &createInfo, nullptr, &layout));
        m_pipelineLayouts.push_back(layout);
        return layout;
    }

    VkPipeline VulkanGraphicsService::createGraphicsPipeline(const VkGraphicsPipelineCreateInfo &createInfo) {
        VkPipeline pipeline;
        CHECK_VULKAN(vkCreateGraphicsPipelines(m_device, nullptr, 1, &createInfo, nullptr, &pipeline));
        m_pipelines.push_back(pipeline);
        return pipeline;
    }

    VkRenderPass VulkanGraphicsService::createRenderPass(const VkRenderPassCreateInfo &createInfo) {
        VkRenderPass renderPass;
        CHECK_VULKAN(vkCreateRenderPass(m_device, &createInfo, nullptr, &renderPass));
        m_renderPasses.push_back(renderPass);
        return renderPass;
    }

    std::vector<VkFramebuffer>
    VulkanGraphicsService::createFrameBuffers(const std::vector<VkFramebufferCreateInfo> &createInfos) {
        const auto size = createInfos.size();
        std::vector<VkFramebuffer> frameBuffers(size);
        for(auto i = 0u; i < size; ++i){
            CHECK_VULKAN(vkCreateFramebuffer(m_device, &createInfos[i], nullptr, &frameBuffers[i]));
            m_frameBuffers.push_back(frameBuffers[i]);
        }
        return frameBuffers;
    }

    Image VulkanGraphicsService::creatImage(const VkImageCreateInfo &createInfo) {
        auto image = allocator.allocate(createInfo);
        m_images.push_back(image);
        return image;
    }

    VkShaderModule VulkanGraphicsService::createShaderModule(const std::filesystem::path &path) {
        io::FileReader fileReader{path};
        auto code = fileReader.readFully<uint32_t>();

        auto createInfo = makeStruct<VkShaderModuleCreateInfo>();
        createInfo.codeSize = fileReader.size();
        createInfo.pCode = code.data();

        VkShaderModule shaderModule;
        CHECK_VULKAN(vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }

    void VulkanGraphicsService::submitToGraphicsQueue(const VkSubmitInfo &submitInfo) {
        CHECK_VULKAN(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, nullptr));
        vkQueueWaitIdle(m_graphicsQueue);
    }

    VkFramebuffer VulkanGraphicsService::createFrameBuffer(const VkFramebufferCreateInfo &createInfo) {
        VkFramebuffer framebuffer;
        CHECK_VULKAN(vkCreateFramebuffer(m_device, &createInfo, nullptr, &framebuffer));
        return framebuffer;
    }

}



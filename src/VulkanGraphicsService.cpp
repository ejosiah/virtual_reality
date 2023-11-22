#include "check.hpp"
#include "vr/graphics/vulkan/VulkanGraphicsService.hpp"

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
        createCommandPool();
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

    void VulkanGraphicsService::createCommandPool() {
        VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        createInfo.queueFamilyIndex = m_graphicsFamilyIndex;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        CHECK_VULKAN(vkCreateCommandPool(m_device, &createInfo, VK_NULL_HANDLE, &m_commandPool));

        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        CHECK_VULKAN(vkCreateCommandPool(m_device, &createInfo, VK_NULL_HANDLE, &m_scopedCommandPool));
    }

    void VulkanGraphicsService::createCommandBuffers() {
        assert(!m_swapChains.empty() && !m_swapChains.front().images.empty());
        VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = m_swapChains.front().images.size();

        m_commandBuffers.resize(allocateInfo.commandBufferCount);
        CHECK_VULKAN(vkAllocateCommandBuffers(m_device, &allocateInfo, m_commandBuffers.data()));
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

       return allocator.allocate(createInfo, VMA_MEMORY_USAGE_CPU_ONLY);
    }

    Buffer VulkanGraphicsService::createDeviceLocalBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
        auto createInfo = makeStruct<VkBufferCreateInfo>();
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        return allocator.allocate(createInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    }

    Buffer VulkanGraphicsService::createMappableBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
        auto createInfo = makeStruct<VkBufferCreateInfo>();
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        return allocator.allocate(createInfo, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }

    void VulkanGraphicsService::release(Buffer buffer) {
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
        createCommandBuffers();
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

}



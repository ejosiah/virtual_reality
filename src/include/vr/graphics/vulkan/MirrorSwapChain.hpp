#pragma once

#include "check.hpp"
#include "vr/WindowingSystem.hpp"
#include "vr/graphics/vulkan/Memory.hpp"
#include "xr_struct_mapping.hpp"
#include "XrVulkanSwapChain.hpp"
#include <vulkan/vulkan.h>

#include <cassert>
#include <vector>
#include <set>

namespace vr {

    struct MirrorSwapChain {
        VkSwapchainKHR swapchain{VK_NULL_HANDLE};
        VkSurfaceKHR  surface{VK_NULL_HANDLE};
        VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        VkQueue presentQueue{VK_NULL_HANDLE};
        VkPhysicalDevice pDevice{VK_NULL_HANDLE};
        VkDevice device{VK_NULL_HANDLE};
        uint32_t presentQueueFamilyIndex{~0u};
        std::vector<VkImage> images;
        std::vector<uint32_t> queueFamilies;

        void createSurface(const Window& window, VkInstance instance) {
            surface = WindowingSystem::surface(window, instance);
            info.surface = surface;
        }
        
        void setPresentQueueFamilyIndex(uint32_t graphicsQueueFamilyIndex) {
            assert(surface != VK_NULL_HANDLE && pDevice != VK_NULL_HANDLE);
            VkBool32 presentSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, graphicsQueueFamilyIndex, surface, &presentSupported);
            if(presentSupported) {
                presentQueueFamilyIndex = graphicsQueueFamilyIndex;
                spdlog::info("Graphics queue Family supports presentation");
            }else {
                uint32_t count;
                vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, nullptr);

                for(auto queueFamilyIndex = 0u; queueFamilyIndex < count; queueFamilyIndex++) {
                    vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, queueFamilyIndex, surface, &presentSupported);
                    if(presentSupported){
                        presentQueueFamilyIndex = queueFamilyIndex;
                        break;
                    }
                }

                if(presentQueueFamilyIndex == ~0u){
                    THROW("present queue not found");
                }

                queueFamilies.push_back(presentQueueFamilyIndex);
                queueFamilies.push_back(graphicsQueueFamilyIndex);
                spdlog::warn("concurrent sharing mode will be enabled for Presentation and Graphics queues");
            }
        }

        void createSwapChain(const XrVulkanSwapChain &source) {
            assert(device != VK_NULL_HANDLE && pDevice != VK_NULL_HANDLE && surface != VK_NULL_HANDLE);

            info.minImageCount = source.images.size();
            info.imageFormat = source.format;
            info.imageColorSpace = findColorSpaceFor(source.format);
            info.imageExtent = {source.width / 2, source.height / 2}; // TODO make it close to 1024
            info.imageArrayLayers = 1;
            info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if(!queueFamilies.empty()) {
                info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                info.queueFamilyIndexCount = queueFamilies.size();
                info.pQueueFamilyIndices = queueFamilies.data();
            }
            info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.presentMode = getMode({VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR});
            info.clipped = VK_FALSE;
            info.oldSwapchain = VK_NULL_HANDLE;

            CHECK_VULKAN(vkCreateSwapchainKHR(device, &info, nullptr, &swapchain));
            vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);

            uint32_t imageCount = 0;
            vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
            assert(imageCount != 0);
            images.resize(imageCount);
            vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
        }

        [[nodiscard]]
        VkColorSpaceKHR findColorSpaceFor(VkFormat format) const {
            const auto& surfaceFormats = supportedFormats();
            auto itr = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), [format](auto sFormat){
                return sFormat.format == format;
            });

            if(itr == surfaceFormats.end()) {
                THROW("Cant find suitable color space");
            }
            return itr->colorSpace;
        }

        [[nodiscard]]
        VkPresentModeKHR getMode(const std::vector<VkPresentModeKHR>& preferredModes) const {
            const auto supportedModes = supportedPresentModes();
            for(auto preferredMode : preferredModes) {
                if(supportedModes.contains(preferredMode)){
                    return preferredMode;
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        [[nodiscard]]
        VkSurfaceCapabilitiesKHR surfaceCapabilities() const {
            assert(surface != VK_NULL_HANDLE && pDevice != VK_NULL_HANDLE);
            VkSurfaceCapabilitiesKHR capabilities{};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &capabilities);
            return capabilities;
        }

        [[nodiscard]]
        std::vector<VkSurfaceFormatKHR> supportedFormats() const {
            assert(pDevice != VK_NULL_HANDLE);
            uint32_t count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &count, nullptr);
            std::vector<VkSurfaceFormatKHR> formats(count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &count, formats.data());

            return formats;
        }

        [[nodiscard]]
        std::set<VkPresentModeKHR> supportedPresentModes() const {
            assert(pDevice != VK_NULL_HANDLE);
            uint32_t count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &count, nullptr);
            std::vector<VkPresentModeKHR> modes(count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &count, modes.data());
            std::set<VkPresentModeKHR> sModes{ modes.begin(), modes.end() };
            return sModes;
        }

        void present(uint32_t imageIndex, const std::vector<VkSemaphore>& waits) {
            assert(swapchain != VK_NULL_HANDLE);
            auto presentInfo = makeStruct<VkPresentInfoKHR>();
            presentInfo.waitSemaphoreCount = waits.size();
            presentInfo.pWaitSemaphores = waits.data();
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapchain;
            presentInfo.pImageIndices = &imageIndex;

            auto result = vkQueuePresentKHR(presentQueue, &presentInfo);
            CHECK_VULKAN(result);
        }

    };
}
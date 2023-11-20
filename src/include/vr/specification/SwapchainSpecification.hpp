#pragma once

#include <openxr/openxr.h>
#include <map>
#include <string>

namespace vr {

    struct SwapchainSpecification {
        std::string _name;
        XrSwapchainCreateFlags      _createFlags{0};
        XrSwapchainUsageFlags       _usageFlags{0};
        int64_t                     _format{0};
        uint32_t                    _sampleCount{1};
        uint32_t                    _width{0};
        uint32_t                    _height{0};
        uint32_t                    _faceCount{1};
        uint32_t                    _arraySize{1};
        uint32_t                    _mipCount{1};

        SwapchainSpecification& name(std::string str) {
            _name = std::move(str);
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& protectedContent() {
            _createFlags |= XR_SWAPCHAIN_CREATE_PROTECTED_CONTENT_BIT;
            return *this;
        }

        SwapchainSpecification& usage() {
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& staticImage() {
            _createFlags |= XR_SWAPCHAIN_CREATE_STATIC_IMAGE_BIT;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& colorAttachment() {
            _usageFlags |= XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& depthStencilAttachment() {
            _usageFlags |= XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& transferSource() {
            _usageFlags |= XR_SWAPCHAIN_USAGE_TRANSFER_SRC_BIT;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& transferDestination() {
            _usageFlags |= XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& format(int64_t format) {
            _format = format;
            return *this;
        }

        SwapchainSpecification& width(uint32_t width) {
            _width = width;
            return *this;
        }

        SwapchainSpecification& height(uint32_t height) {
            _height = height;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& sampleCount(uint32_t count) {
            _sampleCount = count;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& faceCount(uint32_t count) {
            _faceCount = count;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& arraySize(uint32_t size) {
            _arraySize = size;
            return *this;
        }

        [[maybe_unused]]
        SwapchainSpecification& mipCount(uint32_t count) {
            _mipCount = count;
            return *this;
        }

    };

}
#pragma once

#include "Types.hpp"
#include "forwards.hpp"

#include <vulkan/vulkan.h>
#include <openxr/openxr_platform.h>
#include <vector>
#include <memory>
#include <optional>
#include <string>

namespace vr {

    struct GraphicsContextCreation {

        virtual ~GraphicsContextCreation() = default;

        [[nodiscard]] virtual cstring extension() const = 0;

        [[nodiscard]]
        virtual std::shared_ptr<GraphicsContext> create(const Context& context) const = 0;
    };

    struct VulkanContextCreation : GraphicsContextCreation {
        std::vector<cstring> layers{};
        std::string _appName{};
        uint32_t _appVersion{0};
        std::string _engineName{};
        uint32_t _engineVersion{0};
        uint32_t _apiVersion{VK_API_VERSION_1_3};
        mutable std::vector<cstring> extensions {
#ifndef NDEBUG
#ifdef VK_DEBUG
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
#endif
        };

        ~VulkanContextCreation() override = default;

        [[nodiscard]]
        cstring extension() const final  {
            return XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME;
        }

        [[maybe_unused]]
        VulkanContextCreation& addExtension(cstring ext) {
            extensions.push_back(ext);
            return *this;
        }

        [[maybe_unused]]
        VulkanContextCreation& addLayer(cstring layer) {
            layers.push_back(layer);
            return *this;
        }

        VulkanContextCreation& appName(cstring  name) {
            _appName = name;
            return *this;
        }

        VulkanContextCreation& version(uint32_t version) {
            _appVersion = version;
            return *this;
        }

        VulkanContextCreation& engine(cstring  name) {
            _engineName = name;
            return *this;
        }

        [[maybe_unused]]
        VulkanContextCreation& engineVersion(uint32_t version) {
            _engineVersion = version;
            return *this;
        }

        VulkanContextCreation& apiVersion(uint32_t version) {
            _apiVersion = version;
            return *this;
        }

        void validate(const Context &ctx) const;

        [[nodiscard]]
        std::shared_ptr<GraphicsContext> create(const Context &context) const override;

    };

    struct ContextCreation {
        std::string _appName{"OPenXR Applicaiton"};
        uint32_t _appVersion{1};
        std::string _engineName{"OpenXR Engine"};
        uint32_t _engineVersion{1};
        XrVersion _apiVersion{XR_MAKE_VERSION(1, 0, 34)};
        mutable std::unique_ptr<GraphicsContextCreation> graphicsContextCreation;
        XrFormFactor _formFactor{XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY};
        std::vector<cstring> extensions{
            XR_KHR_COMPOSITION_LAYER_CUBE_EXTENSION_NAME,
            XR_KHR_COMPOSITION_LAYER_EQUIRECT_EXTENSION_NAME,
            XR_EXT_PALM_POSE_EXTENSION_NAME
#ifndef NDEBUG
#ifdef XR_DEBUG
        XR_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
#endif
        };

        [[maybe_unused]]
        ContextCreation& addExtension(cstring  extension) {
            extensions.push_back(extension);
            return *this;
        }

        ContextCreation& appName(cstring  name) {
            _appName = name;
            return *this;
        }

        [[maybe_unused]]
        ContextCreation& appVersion(uint32_t version) {
            _appVersion = version;
            return *this;
        }

        ContextCreation& apiVersion(XrVersion version) {
            _apiVersion = version;
            return *this;
        }

        cstring graphicsExtension() const {
            return graphicsContextCreation->extension();
        }

        template <typename T = GraphicsContextCreation>
        T& graphics() { // TODO provided for use and return self
            return dynamic_cast<T&>(*graphicsContextCreation);
        }

        GraphicsContextCreation& graphics() const {
            return *graphicsContextCreation;
        }


        ContextCreation& formFactor(XrFormFactor formFactor) {
            _formFactor = formFactor;
            return *this;
        }

        void validate() const;

        std::optional<Context> create() const;

        static ContextCreation vulkan() {
            ContextCreation creation{};
            creation.graphicsContextCreation = std::make_unique<VulkanContextCreation>();
            return std::move(creation);
        }
    };
}
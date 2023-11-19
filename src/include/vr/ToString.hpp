#pragma once

#include <openxr/openxr.h>

#include <string>
#include <format>

namespace vr {

    [[maybe_unused]]
    inline std::string toString(XrViewConfigurationType type) {
        switch(type) {
            case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO:
                return "Mono";
            case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO:
                return "Stereo";
            case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO:
                return "Quad";
            case XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT:
                return "Mono First Person Observer";
            default:
                return "Unknown view configuration type";
        }
    }

    [[maybe_unused]]
    inline std::string toString(XrEnvironmentBlendMode mode) {
        switch (mode) {
            case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:
                return "Opaque";
            case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:
                return "Additive";
            case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND:
                return "Alpha blend";
            default:
                return "Unknown blend mode";
        }
    }

    [[maybe_unused]]
    inline std::string toString(XrReferenceSpaceType type) {
        switch (type) {
            case XR_REFERENCE_SPACE_TYPE_VIEW:
                return "View";
            case XR_REFERENCE_SPACE_TYPE_LOCAL:
                return "Local";
            case XR_REFERENCE_SPACE_TYPE_STAGE:
                return "Stage";
            case XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT:
                return "Unbounded MSFT";
            case XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO:
                return "Combined eye varjo";
            case XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT:
                return "Local floor";
            default:
                return "Unknown space";
        }
    }

    [[maybe_unused]]
    inline std::string toString(XrSessionState state) {
        switch (state) {
            case XR_SESSION_STATE_IDLE:
                return "Idle";
            case XR_SESSION_STATE_READY:
                return "Ready";
            case XR_SESSION_STATE_SYNCHRONIZED:
                return "Synchronized";
            case XR_SESSION_STATE_VISIBLE:
                return "Visible";
            case XR_SESSION_STATE_FOCUSED:
                return "Focused";
            case XR_SESSION_STATE_STOPPING:
                return "Stopping";
            case XR_SESSION_STATE_LOSS_PENDING:
                return "Loss Pending";
            case XR_SESSION_STATE_EXITING:
                return "Exiting";
            default:
                return "Unknown";
        }
    }

    inline std::string toString(XrInstance instance, XrStructureType structureType) {
        char buffer[XR_MAX_STRUCTURE_NAME_SIZE];
        xrStructureTypeToString(instance, structureType, buffer);
        return std::string{buffer};
    }

    [[maybe_unused]]
    inline std::string versionToString(uint32_t version) {
        return std::format("{}.{}.{}", XR_VERSION_MAJOR(version), XR_VERSION_MINOR(version), XR_VERSION_PATCH(version));
    }
}
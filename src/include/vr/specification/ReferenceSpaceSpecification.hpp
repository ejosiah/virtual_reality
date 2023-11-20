#pragma once

#include "vr/Models.hpp"

struct ReferenceSpaceSpecification {
    std::string _name;
    XrReferenceSpaceType _referenceSpaceType;
    XrPosef _poseInReferenceSpace;


    ReferenceSpaceSpecification& name(std::string str) {
        _name = std::move(str);
        return *this;
    }

    ReferenceSpaceSpecification& type() {
        return *this;
    }

    ReferenceSpaceSpecification& view() {
        _referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
        return *this;
    }

    [[maybe_unused]]
    ReferenceSpaceSpecification& local() {
        _referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
        return *this;
    }

    [[maybe_unused]]
    ReferenceSpaceSpecification& stage() {
        _referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
        return *this;
    }

    ReferenceSpaceSpecification& pose() {
        return *this;
    }

    ReferenceSpaceSpecification& position(float x, float y, float z) {
        _poseInReferenceSpace.position = {x, y, z};
        return *this;
    }

    ReferenceSpaceSpecification& orientation(float w, float x, float y, float z) {
        _poseInReferenceSpace.orientation = {x, y, z, w};
        return *this;
    }

};
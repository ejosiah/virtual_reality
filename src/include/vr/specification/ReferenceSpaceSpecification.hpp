#pragma once

#include <glm/glm.hpp>
#include "vr/Models.hpp"

struct ReferenceSpaceSpecification {
    std::string _name;
    XrReferenceSpaceType _referenceSpaceType;
    vr::Pose _poseInReferenceSpace{};


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

    ReferenceSpaceSpecification& translate(float x, float y, float z) {
        _poseInReferenceSpace.position = {x, y, z};
        return *this;
    }

    ReferenceSpaceSpecification& orientation(float w, float x, float y, float z) {
        _poseInReferenceSpace.orientation = {w, x, y, z};
        return *this;
    }

    ReferenceSpaceSpecification& identity(float w, float x, float y, float z) {
        _poseInReferenceSpace.orientation = {w, x, y, z};
        _poseInReferenceSpace.position = glm::vec3(0);
        return *this;
    }

    [[maybe_unused]]
    ReferenceSpaceSpecification& rotateX(float angle) {
        _poseInReferenceSpace.orientation = glm::angleAxis(glm::radians(angle), glm::vec3{1, 0, 0});
        return *this;
    }

    [[maybe_unused]]
    ReferenceSpaceSpecification& rotateInverseX(float angle) {
        _poseInReferenceSpace.orientation = glm::angleAxis(glm::radians(angle), glm::vec3{-1, 0, 0});
        return *this;
    }

    [[maybe_unused]]
    ReferenceSpaceSpecification& rotateY(float angle) {
        _poseInReferenceSpace.orientation = glm::angleAxis(glm::radians(angle), glm::vec3{0, 1, 0});
        return *this;
    }

    [[maybe_unused]]
    ReferenceSpaceSpecification& rotateInverseY(float angle) {
        _poseInReferenceSpace.orientation = glm::angleAxis(glm::radians(angle), glm::vec3{0, -1, 0});
        return *this;
    }

    [[maybe_unused]]
    ReferenceSpaceSpecification& rotateZ(float angle) {
        _poseInReferenceSpace.orientation = glm::angleAxis(glm::radians(angle), glm::vec3{0, 0, 1});
        return *this;
    }

    [[maybe_unused]]
    ReferenceSpaceSpecification& rotateInverseZ(float angle) {
        _poseInReferenceSpace.orientation = glm::angleAxis(glm::radians(angle), glm::vec3{0, 0, -1});
        return *this;
    }

};
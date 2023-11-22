file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

find_package(OPENXR REQUIRED)
find_package(Vulkan REQUIRED)

set(LIB_DEPENDENCIES
        openxr_ext_loader
        spdlog
        OpenXR::openxr_loader
        Vulkan::Vulkan
        VulkanMemoryAllocator)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_definitions(-DXR_USE_GRAPHICS_API_VULKAN -DVK_DEBUG)

add_executable(vr_app main.cpp ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(vr_app ${LIB_DEPENDENCIES})
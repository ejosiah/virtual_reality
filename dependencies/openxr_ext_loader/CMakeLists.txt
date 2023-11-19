find_package(OPENXR REQUIRED)
find_package(Vulkan REQUIRED)

add_definitions(-DXR_USE_GRAPHICS_API_VULKAN)

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/src/include/*.H)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

add_library(openxr_ext_loader ${GLOB_RECURSE} ${SOURCE_FILES})
target_link_libraries(openxr_ext_loader PRIVATE OpenXR::openxr_loader Vulkan::Vulkan)
target_include_directories(openxr_ext_loader PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/include)
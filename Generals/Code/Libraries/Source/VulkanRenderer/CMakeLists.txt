find_package(Vulkan QUIET)

if(Vulkan_FOUND)
    file(GLOB VULKAN_SOURCES *.cpp)
    add_library(VulkanRenderer STATIC ${VULKAN_SOURCES})
    target_include_directories(VulkanRenderer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
        ${Vulkan_INCLUDE_DIRS}
    )
    target_link_libraries(VulkanRenderer PUBLIC Vulkan::Vulkan)
else()
    # Vulkan SDK not available: expose an empty interface so dependents still link.
    # The game's active renderer is still WW3D2/DirectX; this target is scaffolding
    # for the Vulkan port and is not yet wired into the W3D device layer.
    add_library(VulkanRenderer INTERFACE)
    message(STATUS "Vulkan SDK not found - building VulkanRenderer as empty INTERFACE target")
endif()

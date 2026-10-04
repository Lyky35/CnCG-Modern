file(GLOB VULKAN_SOURCES *.cpp)
add_library(VulkanRenderer STATIC ${VULKAN_SOURCES})

target_include_directories(VulkanRenderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
)

find_package(Vulkan REQUIRED)
target_link_libraries(VulkanRenderer PUBLIC Vulkan::Vulkan)

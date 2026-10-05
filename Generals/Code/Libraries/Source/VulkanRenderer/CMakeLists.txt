file(GLOB VULKAN_SOURCES *.cpp)
add_library(VulkanRenderer STATIC ${VULKAN_SOURCES})

target_include_directories(VulkanRenderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
    /usr/include/vulkan
)

find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    target_link_libraries(VulkanRenderer PUBLIC Vulkan::Vulkan)
else()
    target_compile_definitions(VulkanRenderer PUBLIC VULKAN_NO_IMPL)
endif()

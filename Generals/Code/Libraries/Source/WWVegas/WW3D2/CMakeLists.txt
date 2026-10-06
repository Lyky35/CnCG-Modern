file(GLOB SOURCES *.cpp)
list(FILTER SOURCES EXCLUDE REGEX ".*dx8webbrowser\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*soundrobj\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*sr_util\\.cpp$")
add_library(WW3D2 STATIC ${SOURCES})
target_link_libraries(WW3D2 PUBLIC DX8Compat)
target_link_libraries(WW3D2 PRIVATE WWMath WWDebug WWLib)

# Native Vulkan backend (VulkanRenderer) is the only renderer.
target_link_libraries(WW3D2 PUBLIC VulkanRenderer)

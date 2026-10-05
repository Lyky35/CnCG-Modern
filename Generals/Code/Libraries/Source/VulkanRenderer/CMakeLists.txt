find_package(Vulkan QUIET)

# Fallback for cross-builds (MinGW) where no Vulkan SDK is installed: use the
# Khronos headers vendored under Libraries/Include/Vulkan plus a locally
# generated vulkan-1.dll import library.
set(VULKAN_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Vulkan")
if(NOT Vulkan_FOUND AND EXISTS "${VULKAN_LOCAL_DIR}/vulkan/vulkan.h")
    if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/cmake/mingw-import-libs/libvulkan-1.a")
        set(Vulkan_FOUND TRUE)
        set(Vulkan_INCLUDE_DIRS "${VULKAN_LOCAL_DIR}")
        set(Vulkan_LIBRARIES "${CMAKE_SOURCE_DIR}/cmake/mingw-import-libs/libvulkan-1.a")
        message(STATUS "Vulkan SDK not installed - using vendored Khronos headers + generated vulkan-1 import library")
    endif()
endif()

if(Vulkan_FOUND)
    file(GLOB VULKAN_SOURCES *.cpp DX8Bridge/*.cpp)
    add_library(VulkanRenderer STATIC ${VULKAN_SOURCES})
    target_include_directories(VulkanRenderer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
        ${CMAKE_CURRENT_SOURCE_DIR}/DX8Bridge
        ${Vulkan_INCLUDE_DIRS}
    )
    target_compile_definitions(VulkanRenderer PUBLIC VK_USE_PLATFORM_WIN32_KHR)
    if(TARGET Vulkan::Vulkan)
        target_link_libraries(VulkanRenderer PUBLIC Vulkan::Vulkan)
    else()
        target_link_libraries(VulkanRenderer PUBLIC ${Vulkan_LIBRARIES})
    endif()

    # ---- SPIR-V shader build step ----
    # Each shader source is compiled with glslc and converted to a C++ header with
    # a uint32_t SPIR-V array; the renderers include these instead of their raw
    # GLSL strings when VULKAN_GENERATED_SHADERS is defined.
    find_program(GLSLC_EXECUTABLE NAMES glslc glslangValidator
        DOC "Vulkan GLSL-to-SPIRV compiler")
    # Older distro glslangValidator lacks --target-env; use -V there.
    if(GLSLC_EXECUTABLE)
        get_filename_component(_glslc_name "${GLSLC_EXECUTABLE}" NAME)
        if(_glslc_name STREQUAL "glslangValidator")
            set(GLSLC_FLAGS -V)
        else()
            set(GLSLC_FLAGS --target-env=vulkan1.0)
        endif()
    endif()
    set(_shader_map
        ui_vert|ui.vert
        ui_frag|ui.frag
        terrain_vert|terrain.vert
        terrain_frag|terrain.frag
        water_vert|water.vert
        water_frag|water.frag
        particle_vert|particle.vert
        particle_frag|particle.frag
        shadow_vert|shadow.vert
        object_vert|object.vert
        object_frag|object.frag
        sprite_vert|sprite.vert
        sprite_frag|sprite.frag
        dx8bridge_vert|dx8bridge.vert
        dx8bridge_frag|dx8bridge.frag
    )
    set(_shader_headers "")
    if(GLSLC_EXECUTABLE)
        set(_gen_dir ${CMAKE_CURRENT_BINARY_DIR}/GeneratedShaders)
        file(MAKE_DIRECTORY ${_gen_dir})
        foreach(_entry IN LISTS _shader_map)
            string(REPLACE "|" ";" _e ${_entry})
            list(GET _e 0 _var)
            list(GET _e 1 _file)
            set(_glsl ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${_file})
            set(_spv ${_gen_dir}/${_var}.spv)
            set(_hdr ${_gen_dir}/${_var}_spv.h)
            add_custom_command(OUTPUT ${_hdr}
                COMMAND ${GLSLC_EXECUTABLE} ${GLSLC_FLAGS} -o ${_spv} ${_glsl}
                COMMAND ${CMAKE_COMMAND}
                    -DSRC=${_spv} -DSRC_NAME=${_file} -DVAR=${_var}_spv -DOUT=${_hdr}
                    -P ${CMAKE_SOURCE_DIR}/cmake/SpvToHeader.cmake
                DEPENDS ${_glsl} ${CMAKE_SOURCE_DIR}/cmake/SpvToHeader.cmake
                COMMENT "Compiling SPIR-V: ${_file}"
                VERBATIM)
            list(APPEND _shader_headers ${_hdr})
        endforeach()
        add_custom_target(VulkanShaders DEPENDS ${_shader_headers})
        add_dependencies(VulkanRenderer VulkanShaders)
        target_include_directories(VulkanRenderer PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_GENERATED_SHADERS=1)
        set(DX8VK_BRIDGE ON CACHE INTERNAL "DX8->Vulkan bridge available")
    else()
        message(WARNING "glslc not found - VulkanRenderer will embed raw GLSL strings (non-functional shaders)")
    endif()
else()
    # Vulkan headers unavailable: expose an empty interface so dependents still link.
    add_library(VulkanRenderer INTERFACE)
    message(STATUS "No Vulkan headers found - building VulkanRenderer as empty INTERFACE target")
endif()

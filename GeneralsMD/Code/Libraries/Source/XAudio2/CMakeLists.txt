file(GLOB XAUDIO2_SOURCES *.cpp)
add_library(XAudio2 STATIC ${XAUDIO2_SOURCES})

target_include_directories(XAudio2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GAME_CODE_DIR}/GameEngine/Include
)

if(WIN32)
    target_link_libraries(XAudio2 PUBLIC xaudio2)
endif()

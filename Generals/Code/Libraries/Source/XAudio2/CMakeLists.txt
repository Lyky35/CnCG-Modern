file(GLOB XAUDIO2_SOURCES *.cpp)
add_library(XAudio2 STATIC ${XAUDIO2_SOURCES})

target_include_directories(XAudio2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GAME_CODE_DIR}/GameEngine/Include
)

# XAudio2 is loaded dynamically at runtime (XAudio2Engine.cpp) so the exe does not
# depend on any particular xaudio2[_N].dll being present at link time.

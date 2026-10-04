file(GLOB_RECURSE GAMEENGINEDEVICE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp")
list(FILTER GAMEENGINEDEVICE_SOURCES EXCLUDE REGEX ".*W3DWebBrowser\\.cpp$")
list(FILTER GAMEENGINEDEVICE_SOURCES EXCLUDE REGEX ".*Win32CDManager\\.cpp$")
list(FILTER GAMEENGINEDEVICE_SOURCES EXCLUDE REGEX ".*MilesAudioManager\\.cpp$")

add_library(GameEngineDevice STATIC ${GAMEENGINEDEVICE_SOURCES})

target_include_directories(GameEngineDevice PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")

target_link_libraries(GameEngineDevice
    GameEngine
    WW3D2
    WWMath
    WWDebug
    WWLib
    WWSaveLoad
    WWAudio
    Compression
    WPAudio
    GameSpy
    XAudio2
)

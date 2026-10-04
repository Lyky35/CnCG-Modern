file(GLOB SOURCES *.cpp)

add_library(FFmpegVideo STATIC ${SOURCES})

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(FFMPEG IMPORTED_TARGET libavcodec libavformat libavutil libswscale)
endif()

if(TARGET PkgConfig::FFMPEG)
    target_link_libraries(FFmpegVideo PUBLIC PkgConfig::FFMPEG)
else()
    target_link_libraries(FFmpegVideo PUBLIC avcodec avformat avutil swscale)
endif()

target_include_directories(FFmpegVideo PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${GAME_CODE_DIR}/GameEngine/Include"
)

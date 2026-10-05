# FFmpegVideo is currently unreferenced by the engine (the active video path is
# GameEngineDevice/Source/VideoDevice/Bink). Only build the real player when FFmpeg
# development files for the *target* platform are available; otherwise expose an
# empty INTERFACE target so link references resolve. (Using host pkg-config for a
# Windows cross-build would pull in Linux headers, so it is disabled when
# cross-compiling.)

set(TARGET_FFMPEG_FOUND FALSE)

if(MSVC)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFMPEG IMPORTED_TARGET libavcodec libavformat libavutil libswscale)
        if(TARGET PkgConfig::FFMPEG)
            set(TARGET_FFMPEG_FOUND TRUE)
        endif()
    endif()
elseif(CMAKE_CROSSCOMPILING)
    if(EXISTS "/usr/${CMAKE_SYSTEM_PROCESSOR}-w64-mingw32/lib/pkgconfig/libavcodec.pc")
        set(TARGET_FFMPEG_FOUND TRUE)
    endif()
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFMPEG IMPORTED_TARGET libavcodec libavformat libavutil libswscale)
        if(TARGET PkgConfig::FFMPEG)
            set(TARGET_FFMPEG_FOUND TRUE)
        endif()
    endif()
endif()

if(TARGET_FFMPEG_FOUND)
    file(GLOB SOURCES *.cpp)
    add_library(FFmpegVideo STATIC ${SOURCES})
    target_include_directories(FFmpegVideo PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${GAME_CODE_DIR}/GameEngine/Include"
        "${GAME_CODE_DIR}/GameEngine/Include/Precompiled"
    )
    if(TARGET PkgConfig::FFMPEG)
        target_link_libraries(FFmpegVideo PUBLIC PkgConfig::FFMPEG)
    else()
        target_link_libraries(FFmpegVideo PUBLIC avcodec avformat avutil swscale)
    endif()
else()
    message(STATUS "No target-platform FFmpeg found - building FFmpegVideo stub (movies disabled)")
    add_library(FFmpegVideo STATIC "${CMAKE_CURRENT_SOURCE_DIR}/stub/FFmpegVideoPlayerStub.cpp")
    target_include_directories(FFmpegVideo BEFORE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/stub")
    target_include_directories(FFmpegVideo PUBLIC
        "${GAME_CODE_DIR}/GameEngine/Include"
        "${GAME_CODE_DIR}/GameEngine/Include/Precompiled"
    )
endif()

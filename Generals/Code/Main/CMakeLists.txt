add_executable(RTS WIN32
    "${CMAKE_CURRENT_SOURCE_DIR}/WinMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RTS.manifest"
)

set_target_properties(RTS PROPERTIES OUTPUT_NAME "RTS")

target_link_libraries(RTS
    GameEngine
    GameEngineDevice
    WW3D2
    WWMath
    WWDebug
    WWLib
    WWSaveLoad
    WWAudio
    Compression
    WWDownload
    WPAudio
    GameSpy
    VulkanRenderer
    FFmpegVideo
)

if(MINGW)
    target_link_libraries(RTS
    )
endif()

if(WIN32)
    target_link_libraries(RTS
        kernel32
        user32
        gdi32
        winspool
        comdlg32
        advapi32
        shell32
        shcore
        ole32
        oleaut32
        uuid
        odbc32
        odbccp32
        $<$<NOT:$<BOOL:${DX8VK_BRIDGE}>>:d3d8>
        dinput8
        dxguid
        dsound
        winmm
        wsock32
        wininet
        imm32
        comctl32
    )
endif()

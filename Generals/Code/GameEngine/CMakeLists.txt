file(GLOB_RECURSE GAMEENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp")
list(FILTER GAMEENGINE_SOURCES EXCLUDE REGEX ".*WOLBrowser/WebBrowser\\.cpp$")
list(FILTER GAMEENGINE_SOURCES EXCLUDE REGEX ".*GameSpeech\\.cpp$")
list(FILTER GAMEENGINE_SOURCES EXCLUDE REGEX ".*simpleplayer\\.cpp$")
list(FILTER GAMEENGINE_SOURCES EXCLUDE REGEX ".*urllaunch\\.cpp$")
list(FILTER GAMEENGINE_SOURCES EXCLUDE REGEX ".*GameSpyGameInfo\\.cpp$")
list(APPEND GAMEENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Source/GameNetwork/WOLBrowser/WebBrowserStub.cpp")
list(APPEND GAMEENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Source/Common/System/CDManagerStub.cpp")

add_library(GameEngine STATIC ${GAMEENGINE_SOURCES})

target_include_directories(GameEngine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/Precompiled"
)

target_link_libraries(GameEngine PUBLIC Compression)

file(GLOB SOURCES *.cpp)
add_library(WWAudio STATIC ${SOURCES})
target_link_libraries(WWAudio PRIVATE WWDebug WWLib)

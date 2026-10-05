enable_language(C)

file(GLOB ZLIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ZLib/*.c")
add_library(zlib_static STATIC ${ZLIB_SOURCES})

file(GLOB LZH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/LZHCompress/*.cpp")
file(GLOB EAC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EAC/*.cpp")
file(GLOB SOURCES *.cpp)

add_library(Compression STATIC ${SOURCES} ${EAC_SOURCES} ${LZH_SOURCES})
target_include_directories(Compression PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Compression PUBLIC zlib_static)

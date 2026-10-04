file(GLOB SOURCES *.cpp)
list(FILTER SOURCES EXCLUDE REGEX ".*dx8webbrowser\\.cpp$")
add_library(WW3D2 STATIC ${SOURCES})
target_link_libraries(WW3D2 PRIVATE WWMath WWDebug WWLib)

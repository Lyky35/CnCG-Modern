file(GLOB SOURCES *.cpp)
add_library(WWMath STATIC ${SOURCES})
target_compile_options(WWMath PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/fp:fast>)

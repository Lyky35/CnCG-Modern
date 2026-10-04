file(GLOB SOURCES *.cpp)
add_library(WWDebug STATIC ${SOURCES})
target_compile_options(WWDebug PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/fp:fast>)

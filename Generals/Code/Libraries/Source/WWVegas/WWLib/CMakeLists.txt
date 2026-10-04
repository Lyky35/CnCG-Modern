file(GLOB SOURCES *.cpp)
add_library(WWLib STATIC ${SOURCES})
target_compile_options(WWLib PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/fp:fast>)

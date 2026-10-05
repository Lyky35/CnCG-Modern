add_library(DX8Compat STATIC "${CMAKE_CURRENT_SOURCE_DIR}/dx8compat.cpp")
target_include_directories(DX8Compat PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../Include/DX8Compat")

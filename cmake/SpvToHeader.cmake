# Convert a SPIR-V binary into a C++ header exposing a uint32_t array.
# Usage: cmake -DSRC=x.spv -DSRC_NAME=x.vert -DVAR=name_spv -DOUT=x.h -P SpvToHeader.cmake
if(NOT DEFINED SRC OR NOT DEFINED VAR OR NOT DEFINED OUT)
    message(FATAL_ERROR "usage: -DSRC=... -DSRC_NAME=... -DVAR=... -DOUT=... -P SpvToHeader.cmake")
endif()

file(READ "${SRC}" hex HEX)   # two hex chars per byte, in file order
string(LENGTH "${hex}" hlen)
math(EXPR nwords "${hlen} / 8")

set(body "")
set(p 0)
math(EXPR last "${nwords} - 1")
foreach(i RANGE 0 ${last})
    # bytes b0 b1 b2 b3 (little-endian) -> 0xb3b2b1b0
    string(SUBSTRING "${hex}" "${p}" 2 b0)
    math(EXPR p2 "${p} + 2")
    string(SUBSTRING "${hex}" "${p2}" 2 b1)
    math(EXPR p4 "${p} + 4")
    string(SUBSTRING "${hex}" "${p4}" 2 b2)
    math(EXPR p6 "${p} + 6")
    string(SUBSTRING "${hex}" "${p6}" 2 b3)
    math(EXPR p "${p} + 8")
    string(APPEND body "    0x${b3}${b2}${b1}${b0}U,\n")
endforeach()

file(WRITE "${OUT}"
"// Generated from ${SRC_NAME} by cmake/SpvToHeader.cmake - do not edit.
#pragma once
#include <cstddef>
#include <cstdint>

static const uint32_t ${VAR}[] = {
${body}};
static const size_t ${VAR}_len = sizeof(${VAR});
")

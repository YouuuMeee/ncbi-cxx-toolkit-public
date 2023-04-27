#############################################################################
# $
#############################################################################
#############################################################################
##
##  NCBI CMake wrapper
##  GCC 7.3.0 toolchain

set(NCBI_PTBCFG_FLAGS_DEFINED YES)
include_guard(GLOBAL)

set(CMAKE_C_COMPILER "/opt/ncbi/gcc/7.3.0/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/ncbi/gcc/7.3.0/bin/g++")

set(CMAKE_C_FLAGS              "-gdwarf-4 -Wall -Wno-format-y2k")
set(CMAKE_C_FLAGS_DEBUG        "-ggdb3 -O0")
set(CMAKE_C_FLAGS_RELEASE      "-ggdb1 -O3")

set(CMAKE_CXX_FLAGS            "-gdwarf-4 -Wall -Wno-format-y2k")
set(CMAKE_CXX_FLAGS_DEBUG      "-ggdb3 -O0")
set(CMAKE_CXX_FLAGS_RELEASE    "-ggdb1 -O3")

set(CMAKE_EXE_LINKER_FLAGS     "-Wl,--enable-new-dtags  -Wl,--as-needed")
set(CMAKE_SHARED_LINKER_FLAGS  "-Wl,--no-undefined  -Wl,--as-needed")

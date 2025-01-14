#############################################################################
# $Id$
#############################################################################
#############################################################################
##
##  NCBI CMake wrapper
##  GCC toolchain

set(NCBI_PTBCFG_FLAGS_DEFINED YES)
include_guard(GLOBAL)

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_C_FLAGS              "-gdwarf-4 -Wall -Wno-format-y2k")
set(CMAKE_C_FLAGS_DEBUG        "-ggdb3 -O0")
set(CMAKE_C_FLAGS_RELEASE      "-ggdb1 -O3")
set(CMAKE_C_FLAGS_RELWITHDEBINFO  "-ggdb3 -O3")

set(CMAKE_CXX_FLAGS            "-gdwarf-4 -Wall -Wno-format-y2k")
set(CMAKE_CXX_FLAGS_DEBUG      "-ggdb3 -O0")
set(CMAKE_CXX_FLAGS_RELEASE    "-ggdb1 -O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-ggdb3 -O3")

set(CMAKE_EXE_LINKER_FLAGS     "-Wl,--enable-new-dtags  -Wl,--as-needed")
set(CMAKE_SHARED_LINKER_FLAGS  "-Wl,--no-undefined  -Wl,--as-needed")

#----------------------------------------------------------------------------
set(NCBI_COMPILER_FLAGS_SSE       "")

set(NCBI_COMPILER_FLAGS_COVERAGE  "--coverage")
set(NCBI_LINKER_FLAGS_COVERAGE     "--coverage")

set(NCBI_COMPILER_FLAGS_MAXDEBUG  "-fsanitize=address")
set(NCBI_LINKER_FLAGS_MAXDEBUG   "-fsanitize=address")

set(NCBI_LINKER_FLAGS_STATICCOMPONENTS "-static-libgcc -static-libstdc++")

#############################################################################
# $Id$
#############################################################################
#############################################################################
##
##  NCBI C++ Toolkit CMake wrapper
##    Author: Andrei Gourianov, gouriano@ncbi
##

if(NOT DEFINED NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED)
set( NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED ON)

###############################################################################
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

set(NCBI_VERBOSE_ALLPROJECTS           OFF)
if(NCBI_PTBCFG_SKIP_ANALYSIS)
    set(NCBI_PTBCFG_ENABLE_COLLECTOR       OFF)
else()
    set(NCBI_PTBCFG_ENABLE_COLLECTOR       ON)
endif()

if(BUILD_SHARED_LIBS)
    if(WIN32 OR XCODE)
        set(NCBI_PTBCFG_ALLOW_COMPOSITE ON)
    endif()
endif()

if(NCBI_PTBCFG_PACKAGING)
    set(NCBI_PTBCFG_ADDCHECK          OFF)
    set(NCBI_PTBCFG_DOINSTALL         ON)
else()
    set(NCBI_PTBCFG_ADDCHECK              ON)
    if (NOT "${NCBI_PTBCFG_INSTALL_PATH}" STREQUAL "")
        set(NCBI_PTBCFG_DOINSTALL         ON)
    endif()
endif()
#set(NCBI_PTBCFG_COLLECT_REQUIRES      ON)
#set(NCBI_PTBCFG_COLLECT_REQUIRES_FILE ncbi-cpp_requires)

###############################################################################
set(_listdir "${CMAKE_CURRENT_LIST_DIR}")

include(${_listdir}/CMake.NCBIptb.definitions.cmake)
include(${_listdir}/CMakeMacros.cmake)
include(${_listdir}/CMake.NCBIptb.cmake)
if(NCBI_PTBCFG_PACKAGING OR NCBI_PTBCFG_USECONAN)
    include(${_listdir}/CMake.NCBIpkg.setup.cmake)
endif()
include(${_listdir}/CMakeChecks.cmake)
if(NOT NCBI_PTBCFG_COLLECT_REQUIRES)
    include(${_listdir}/CMake.NCBIptb.ncbi.cmake)
    include(${_listdir}/CMake.NCBIptb.datatool.cmake)
    include(${_listdir}/CMake.NCBIpkg.codegen.cmake)
    include(${_listdir}/CMake.NCBIptb.grpc.cmake)
    if(NOT NCBI_PTBCFG_PACKAGING)
        include(${_listdir}/CMake.NCBIptb.ctest.cmake)
    endif()
    if(NCBI_PTBCFG_ADDCHECK)
        include(${_listdir}/CMake.NCBIptb.ntest.cmake)
    endif()
    if(NCBI_PTBCFG_DOINSTALL)
        include(${_listdir}/CMake.NCBIptb.install.cmake)
    endif()
    if(NOT NCBI_PTBCFG_PACKAGING)
        include(${_listdir}/CMake.NCBIptb.legacy.cmake)
    endif()

    if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
        if (EXISTS ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.cmake)
            include(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.cmake)
        else()
            message(FATAL_ERROR "${NCBI_PTBCFG_INSTALL_EXPORT} was not found in ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}")
        endif()
        NCBI_import_hostinfo(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.hostinfo)
        NCBI_process_imports(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.imports)
    endif()

    include(${_listdir}/CMakeChecks.final-message.cmake)
endif(NOT NCBI_PTBCFG_COLLECT_REQUIRES)
endif(NOT DEFINED NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED)

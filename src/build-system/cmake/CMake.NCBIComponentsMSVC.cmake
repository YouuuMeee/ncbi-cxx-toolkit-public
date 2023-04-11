#############################################################################
# $Id$
#############################################################################

##
## NCBI CMake components description - MSVC
##
##
## As a result, the following variables should be defined for component XXX
##  NCBI_COMPONENT_XXX_FOUND
##  NCBI_COMPONENT_XXX_INCLUDE
##  NCBI_COMPONENT_XXX_DEFINES
##  NCBI_COMPONENT_XXX_LIBS
##  HAVE_LIBXXX
##  HAVE_XXX


#to debug
#set(NCBI_TRACE_COMPONENT_GRPC ON)
#############################################################################
if("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 17 2022")
    set(NCBI_ThirdPartyCompiler vs2019.64)
elseif("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 16 2019")
    set(NCBI_ThirdPartyCompiler vs2019.64)
elseif("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 15 2017 Win64")
    set(NCBI_ThirdPartyCompiler vs2017.64)
elseif("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 15 2017")
    if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64" OR "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win64")
        set(NCBI_ThirdPartyCompiler vs2017.64)
    else()
        set(NCBI_ThirdPartyCompiler vs2017)
        set(NCBI_PlatformBits 32)
        message(FATAL_ERROR "32 bit configurations not supported")
    endif()
else()
#    message(WARNING "Generator ${CMAKE_GENERATOR} not tested")
    set(NCBI_ThirdPartyCompiler vs2019.64)
endif()

#############################################################################
# prebuilt libraries
set(NCBI_ThirdPartyBasePath ${NCBI_TOOLS_ROOT}/Lib/ThirdParty)
set(NCBI_ThirdPartyAppsPath ${NCBI_TOOLS_ROOT}/App/ThirdParty)
set(NCBI_ThirdParty_NCBI_C  ${NCBI_TOOLS_ROOT}/Lib/Ncbi/C/${NCBI_ThirdPartyCompiler}/c.current)
set(NCBI_ThirdParty_VDBROOT //snowman/trace_software/vdb)

set(NCBI_ThirdParty_GNUTLS     ${NCBI_ThirdPartyBasePath}/gnutls/${NCBI_ThirdPartyCompiler}/3.4.9 CACHE PATH "GNUTLS root")
set(NCBI_ThirdParty_FASTCGI    ${NCBI_ThirdPartyBasePath}/fastcgi/${NCBI_ThirdPartyCompiler}/2.4.1 CACHE PATH "FASTCGI root")
set(NCBI_ThirdParty_Boost_VERSION "1.76.0")
set(NCBI_ThirdParty_Boost      ${NCBI_ThirdPartyBasePath}/boost/${NCBI_ThirdPartyCompiler}/${NCBI_ThirdParty_Boost_VERSION}-ncbi1 CACHE PATH "Boost root")
set(NCBI_ThirdParty_PCRE       ${NCBI_ThirdPartyBasePath}/pcre/${NCBI_ThirdPartyCompiler}/8.42 CACHE PATH "PCRE root")
set(NCBI_ThirdParty_Z          ${NCBI_ThirdPartyBasePath}/z/${NCBI_ThirdPartyCompiler}/1.2.11 CACHE PATH "Z root")
set(NCBI_ThirdParty_BZ2        ${NCBI_ThirdPartyBasePath}/bzip2/${NCBI_ThirdPartyCompiler}/1.0.6 CACHE PATH "BZ2 root")
set(NCBI_ThirdParty_LZO        ${NCBI_ThirdPartyBasePath}/lzo/${NCBI_ThirdPartyCompiler}/2.10 CACHE PATH "LZO root")
set(NCBI_ThirdParty_ZSTD       ${NCBI_ThirdPartyBasePath}/zstd/${NCBI_ThirdPartyCompiler}/1.5.2 CACHE PATH "ZSTD root")
set(NCBI_ThirdParty_BerkeleyDB ${NCBI_ThirdPartyBasePath}/berkeleydb/${NCBI_ThirdPartyCompiler}/4.6.21.NC CACHE PATH "BerkeleyDB root")
set(NCBI_ThirdParty_LMDB       ${NCBI_ThirdPartyBasePath}/lmdb/${NCBI_ThirdPartyCompiler}/0.9.24 CACHE PATH "LMDB root")
set(NCBI_ThirdParty_JPEG       ${NCBI_ThirdPartyBasePath}/jpeg/${NCBI_ThirdPartyCompiler}/9c CACHE PATH "JPEG root")
set(NCBI_ThirdParty_PNG        ${NCBI_ThirdPartyBasePath}/png/${NCBI_ThirdPartyCompiler}/1.6.34 CACHE PATH "PNG root")
set(NCBI_ThirdParty_GIF        ${NCBI_ThirdPartyBasePath}/gif/${NCBI_ThirdPartyCompiler}/4.1.3 CACHE PATH "GIF root")
set(NCBI_ThirdParty_TIFF       ${NCBI_ThirdPartyBasePath}/tiff/${NCBI_ThirdPartyCompiler}/3.6.1 CACHE PATH "TIFF root")
set(NCBI_ThirdParty_XML        ${NCBI_ThirdPartyBasePath}/xml/${NCBI_ThirdPartyCompiler}/2.7.8 CACHE PATH "XML root")
set(NCBI_ThirdParty_XSLT       ${NCBI_ThirdPartyBasePath}/xslt/${NCBI_ThirdPartyCompiler}/1.1.26 CACHE PATH "XSLT root")
set(NCBI_ThirdParty_EXSLT      ${NCBI_ThirdParty_XSLT})
set(NCBI_ThirdParty_SQLITE3    ${NCBI_ThirdPartyBasePath}/sqlite/${NCBI_ThirdPartyCompiler}/3.26.0 CACHE PATH "SQLITE3 root")
set(NCBI_ThirdParty_PYTHON     ${NCBI_ThirdPartyAppsPath}/Python38 CACHE PATH "PYTHON root")
set(NCBI_ThirdParty_PROTOBUF   ${NCBI_ThirdPartyBasePath}/grpc/${NCBI_ThirdPartyCompiler}/1.36.4-ncbi2 CACHE PATH "PROTOBUF root")
set(NCBI_ThirdParty_GRPC       ${NCBI_ThirdPartyBasePath}/grpc/${NCBI_ThirdPartyCompiler}/1.36.4-ncbi2 CACHE PATH "GRPC root")
set(NCBI_ThirdParty_FTGL       ${NCBI_ThirdPartyBasePath}/ftgl/${NCBI_ThirdPartyCompiler}/2.1.3-rc5 CACHE PATH "FTGL root")
set(NCBI_ThirdParty_GLEW       ${NCBI_ThirdPartyBasePath}/glew/${NCBI_ThirdPartyCompiler}/1.5.8 CACHE PATH "GLEW root")
set(NCBI_ThirdParty_FreeType   ${NCBI_ThirdPartyBasePath}/freetype/${NCBI_ThirdPartyCompiler}/2.4.10 CACHE PATH "FreeType root")
if(NOT noUNICODE IN_LIST NCBI_PTBCFG_PROJECT_FEATURES AND NOT -UNICODE IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)
    set(NCBI_ThirdParty_wxWidgets  ${NCBI_ThirdPartyBasePath}/wxwidgets/${NCBI_ThirdPartyCompiler}/3.1.3-ncbi1u CACHE PATH "wxWidgets root")
else()
    set(NCBI_ThirdParty_wxWidgets  ${NCBI_ThirdPartyBasePath}/wxwidgets/${NCBI_ThirdPartyCompiler}/3.1.3-ncbi1 CACHE PATH "wxWidgets root")
endif()
set(NCBI_ThirdParty_NGHTTP2    ${NCBI_ThirdPartyBasePath}/nghttp2/${NCBI_ThirdPartyCompiler}/1.40.0 CACHE PATH "NGHTTP2 root")
set(NCBI_ThirdParty_GL2PS      ${NCBI_ThirdPartyBasePath}/gl2ps/${NCBI_ThirdPartyCompiler}/1.4.0 CACHE PATH "GL2PS root")

set(NCBI_ThirdParty_Sybase     ${NCBI_ThirdPartyBasePath}/sybase/${NCBI_ThirdPartyCompiler}/15.5 CACHE PATH "Sybase root")
set(NCBI_ThirdParty_SybaseLocalPath "" CACHE PATH "SybaseLocalPath")

set(NCBI_ThirdParty_VDB        ${NCBI_ThirdParty_VDBROOT}/vdb-versions/3.0.0 CACHE PATH "VDB root")
set(NCBI_ThirdParty_VDB_ARCH_INC x86_64)
if("${NCBI_ThirdPartyCompiler}" STREQUAL "vs2019.64")
    set(NCBI_ThirdParty_VDB_ARCH x86_64/v142)
elseif("${NCBI_ThirdPartyCompiler}" STREQUAL "vs2017.64")
    set(NCBI_ThirdParty_VDB_ARCH x86_64/v141)
endif()

if("${NCBI_ThirdPartyCompiler}" STREQUAL "vs2019.64")
    set(NCBI_ThirdParty_XALAN  ${NCBI_ThirdPartyBasePath}/xalan/${NCBI_ThirdPartyCompiler}/1.12 CACHE PATH "XALAN root")
    set(NCBI_ThirdParty_XERCES ${NCBI_ThirdPartyBasePath}/xerces/${NCBI_ThirdPartyCompiler}/3.2.3 CACHE PATH "XERCES root")
    set(NCBI_ThirdParty_UV     ${NCBI_ThirdPartyBasePath}/uv/${NCBI_ThirdPartyCompiler}/1.35.0.ncbi1 CACHE PATH "UV root")
elseif("${NCBI_ThirdPartyCompiler}" STREQUAL "vs2017.64")
    set(NCBI_ThirdParty_XALAN  ${NCBI_ThirdPartyBasePath}/xalan/${NCBI_ThirdPartyCompiler}/1.10.0-20080814 CACHE PATH "XALAN root")
    set(NCBI_ThirdParty_XERCES ${NCBI_ThirdPartyBasePath}/xerces/${NCBI_ThirdPartyCompiler}/2.8.0 CACHE PATH "XERCES root")
    set(NCBI_ThirdParty_UV     ${NCBI_ThirdPartyBasePath}/uv/${NCBI_ThirdPartyCompiler}/1.35.0 CACHE PATH "UV root")
endif()
set(NCBI_ThirdParty_SQLServer   "C:/Program Files/Microsoft SQL Server/Client SDK/ODBC/170/SDK")


#############################################################################
#############################################################################


#############################################################################
# in-house-resources
if (NOT NCBI_COMPONENT_in-house-resources_DISABLED
        AND EXISTS "${NCBI_TOOLS_ROOT}/ncbi.ini"
        AND EXISTS "${NCBI_TOOLS_ROOT}/Scripts/test_data")
    set(NCBITEST_TESTDATA_PATH "${NCBI_TOOLS_ROOT}/Scripts/test_data")
    set(NCBI_REQUIRE_in-house-resources_FOUND YES)
endif()
NCBIcomponent_report(in-house-resources)

#############################################################################
# NCBI_C
if(OFF)
  set(_c_libs  blast ddvlib medarch ncbi ncbiacc ncbicdr
               ncbicn3d ncbicn3d_ogl ncbidesk ncbiid1
               ncbimain ncbimmdb ncbinacc ncbiobj ncbispel
               ncbitool ncbitxc2 netblast netcli netentr
               regexp smartnet vibgif vibnet vibrant
               vibrant_ogl)
endif()
if(NOT NCBI_COMPONENT_NCBI_C_DISABLED)
    NCBI_define_Wcomponent(NCBI_C ncbiobj.lib ncbimmdb.lib ncbi.lib)
    if(NCBI_COMPONENT_NCBI_C_FOUND)
        set(NCBI_COMPONENT_NCBI_C_LIBPATH ${NCBI_ThirdParty_NCBI_C})
    endif()
endif()
NCBIcomponent_report(NCBI_C)

##############################################################################
# UUID
set(NCBI_COMPONENT_UUID_FOUND YES)
set(NCBI_COMPONENT_UUID_LIBS uuid.lib rpcrt4.lib)

#############################################################################
# MySQL
set(NCBI_COMPONENT_MySQL_FOUND NO)

#############################################################################
# ODBC
if(NOT NCBI_COMPONENT_ODBC_DISABLED)
    set(NCBI_COMPONENT_ODBC_FOUND YES)
    set(NCBI_COMPONENT_ODBC_LIBS odbc32.lib odbccp32.lib
        # odbcbcp.lib
	)
    set(HAVE_ODBC 1)
    set(HAVE_ODBCSS_H 1)
endif()
NCBIcomponent_report(ODBC)

if(NOT NCBI_COMPONENT_SQLServer_DISABLED)
    NCBI_define_Wcomponent(SQLServer "x64/msodbcsql17.lib")
    if(NCBI_COMPONENT_SQLServer_FOUND)
        set(NCBI_COMPONENT_SQLServer_VERSION 170)
    endif()
endif()
NCBIcomponent_report(SQLServer)

##############################################################################
# OpenGL
if(NOT NCBI_COMPONENT_OpenGL_DISABLED)
    set(NCBI_COMPONENT_OpenGL_FOUND YES)
    set(NCBI_COMPONENT_OpenGL_LIBS opengl32.lib glu32.lib)
    set(HAVE_OPENGL 1)
endif()
NCBIcomponent_report(OpenGL)

#############################################################################
# LMDB
NCBI_define_Wcomponent(LMDB liblmdb.lib)
NCBIcomponent_report(LMDB)
if(NOT NCBI_COMPONENT_LMDB_FOUND)
    set(NCBI_COMPONENT_LMDB_FOUND ${NCBI_COMPONENT_LocalLMDB_FOUND})
    set(NCBI_COMPONENT_LMDB_INCLUDE ${NCBI_COMPONENT_LocalLMDB_INCLUDE})
    set(NCBI_COMPONENT_LMDB_NCBILIB ${NCBI_COMPONENT_LocalLMDB_NCBILIB})
endif()
set(HAVE_LIBLMDB ${NCBI_COMPONENT_LMDB_FOUND})

#############################################################################
# PCRE
NCBI_define_Wcomponent(PCRE libpcre.lib)
NCBIcomponent_report(PCRE)
if(NCBI_COMPONENT_PCRE_FOUND)
    set(NCBI_COMPONENT_PCRE_DEFINES PCRE_STATIC NOPOSIX)
endif()
if(NOT NCBI_COMPONENT_PCRE_FOUND)
    set(NCBI_COMPONENT_PCRE_FOUND ${NCBI_COMPONENT_LocalPCRE_FOUND})
    set(NCBI_COMPONENT_PCRE_INCLUDE ${NCBI_COMPONENT_LocalPCRE_INCLUDE})
    set(NCBI_COMPONENT_PCRE_NCBILIB ${NCBI_COMPONENT_LocalPCRE_NCBILIB})
endif()
set(HAVE_LIBPCRE ${NCBI_COMPONENT_PCRE_FOUND})

#############################################################################
# Z
NCBI_define_Wcomponent(Z libz.lib)
NCBIcomponent_report(Z)
if(NOT NCBI_COMPONENT_Z_FOUND)
    set(NCBI_COMPONENT_Z_FOUND ${NCBI_COMPONENT_LocalZ_FOUND})
    set(NCBI_COMPONENT_Z_INCLUDE ${NCBI_COMPONENT_LocalZ_INCLUDE})
    set(NCBI_COMPONENT_Z_NCBILIB ${NCBI_COMPONENT_LocalZ_NCBILIB})
endif()
set(HAVE_LIBZ ${NCBI_COMPONENT_Z_FOUND})

#############################################################################
# BZ2
NCBI_define_Wcomponent(BZ2 libbzip2.lib)
NCBIcomponent_report(BZ2)
if(NOT NCBI_COMPONENT_BZ2_FOUND)
    set(NCBI_COMPONENT_BZ2_FOUND ${NCBI_COMPONENT_LocalBZ2_FOUND})
    set(NCBI_COMPONENT_BZ2_INCLUDE ${NCBI_COMPONENT_LocalBZ2_INCLUDE})
    set(NCBI_COMPONENT_BZ2_NCBILIB ${NCBI_COMPONENT_LocalBZ2_NCBILIB})
endif()
set(HAVE_LIBBZ2 ${NCBI_COMPONENT_BZ2_FOUND})

#############################################################################
# LZO
NCBI_define_Wcomponent(LZO liblzo.lib)
NCBIcomponent_report(LZO)

#############################################################################
# ZSTD
NCBI_define_Wcomponent(ZSTD libzstd_static.lib)
if(NCBI_COMPONENT_ZSTD_FOUND AND
    (DEFINED NCBI_COMPONENT_ZSTD_VERSION AND "${NCBI_COMPONENT_ZSTD_VERSION}" VERSION_LESS "1.4"))
    message("ZSTD: Version requirement not met (required at least v1.4)")
    set(NCBI_COMPONENT_ZSTD_FOUND NO)
    set(HAVE_LIBZSTD 0)
endif()
NCBIcomponent_report(ZSTD)

if(NOT NCBI_COMPONENT_Boost_DISABLED AND NOT NCBI_COMPONENT_Boost_FOUND)
#############################################################################
# Boost.Test.Included
NCBI_define_Wcomponent(Boost.Test.Included)
NCBIcomponent_report(Boost.Test.Included)
if(NCBI_COMPONENT_Boost.Test.Included_FOUND)
    set(NCBI_COMPONENT_Boost.Test.Included_DEFINES BOOST_TEST_NO_LIB)
endif()

#############################################################################
# Boost.Test
NCBI_define_Wcomponent(Boost.Test libboost_unit_test_framework.lib)
NCBIcomponent_report(Boost.Test)
if(NCBI_COMPONENT_Boost.Test_FOUND)
    set(NCBI_COMPONENT_Boost.Test_DEFINES BOOST_AUTO_LINK_NOMANGLE)
endif()

#############################################################################
# Boost.Spirit
NCBI_define_Wcomponent(Boost.Spirit libboost_thread.lib boost_thread.lib boost_system.lib boost_date_time.lib boost_chrono.lib)
NCBIcomponent_report(Boost.Spirit)
if(NCBI_COMPONENT_Boost.Spirit_FOUND)
    set(NCBI_COMPONENT_Boost.Spirit_DEFINES BOOST_AUTO_LINK_NOMANGLE)
endif()

#############################################################################
# Boost.Thread
NCBI_define_Wcomponent(Boost.Thread libboost_thread.lib boost_thread.lib boost_system.lib boost_date_time.lib boost_chrono.lib)
NCBIcomponent_report(Boost.Thread)
if(NCBI_COMPONENT_Boost.Thread_FOUND)
    set(NCBI_COMPONENT_Boost.Thread_DEFINES BOOST_AUTO_LINK_NOMANGLE)
endif()

#############################################################################
# Boost
NCBI_define_Wcomponent(Boost boost_filesystem.lib boost_iostreams.lib boost_date_time.lib boost_regex.lib  boost_system.lib)

endif(NOT NCBI_COMPONENT_Boost_DISABLED AND NOT NCBI_COMPONENT_Boost_FOUND)
NCBIcomponent_report(Boost)

#############################################################################
# JPEG
NCBI_define_Wcomponent(JPEG libjpeg.lib)
NCBIcomponent_report(JPEG)

#############################################################################
# PNG
NCBI_define_Wcomponent(PNG libpng.lib)
NCBIcomponent_report(PNG)

#############################################################################
# GIF
NCBI_define_Wcomponent(GIF libgif.lib)
NCBIcomponent_report(GIF)

#############################################################################
# TIFF
NCBI_define_Wcomponent(TIFF libtiff.lib)
NCBIcomponent_report(TIFF)

#############################################################################
# GNUTLS
if(NOT NCBI_COMPONENT_GNUTLS_DISABLED)
    NCBI_define_Wcomponent(GNUTLS libgnutls-30.lib)
endif()
NCBIcomponent_report(GNUTLS)

#############################################################################
# FASTCGI
NCBI_define_Wcomponent(FASTCGI libfcgi.lib)
NCBIcomponent_report(FASTCGI)

#############################################################################
# BerkeleyDB
NCBI_define_Wcomponent(BerkeleyDB libdb.lib)
NCBIcomponent_report(BerkeleyDB)
if(NCBI_COMPONENT_BerkeleyDB_FOUND)
    set(HAVE_BERKELEY_DB 1)
    set(HAVE_BDB         1)
    set(HAVE_BDB_CACHE   1)
endif()

#############################################################################
# XML
NCBI_define_Wcomponent(XML libxml2.lib)
NCBIcomponent_report(XML)
if (NCBI_COMPONENT_XML_FOUND)
    if(NOT BUILD_SHARED_LIBS)
        set (NCBI_COMPONENT_XML_DEFINES LIBXML_STATIC)
    endif()
endif()

#############################################################################
# XSLT
NCBI_define_Wcomponent(XSLT libexslt.lib libxslt.lib)
NCBIcomponent_report(XSLT)

#############################################################################
# EXSLT
NCBI_define_Wcomponent(EXSLT libexslt.lib)
NCBIcomponent_report(EXSLT)

#############################################################################
# SQLITE3
NCBI_define_Wcomponent(SQLITE3 sqlite3.lib)
NCBIcomponent_report(SQLITE3)

#############################################################################
# LAPACK
set(NCBI_COMPONENT_LAPACK_FOUND NO)

#############################################################################
# Sybase
NCBI_define_Wcomponent(Sybase # libsybdb.lib
                       libsybct.lib libsybblk.lib libsybcs.lib)
NCBIcomponent_report(Sybase)
if (NCBI_COMPONENT_Sybase_FOUND)
    set(SYBASE_PATH ${NCBI_ThirdParty_Sybase}/Sybase)
    set(SYBASE_LCL_PATH "${NCBI_ThirdParty_SybaseLocalPath}")
endif()

#############################################################################
# VDB
if(NOT NCBI_COMPONENT_VDB_DISABLED AND NOT NCBI_COMPONENT_VDB_FOUND)
    set(NCBI_COMPONENT_VDB_INCLUDE
        ${NCBI_ThirdParty_VDB}/interfaces
        ${NCBI_ThirdParty_VDB}/interfaces/cc/vc++/${NCBI_ThirdParty_VDB_ARCH_INC}
        ${NCBI_ThirdParty_VDB}/interfaces/cc/vc++
        ${NCBI_ThirdParty_VDB}/interfaces/os/win)

    if("${NCBI_CONFIGURATION_TYPES_COUNT}" EQUAL 1)
        NCBI_util_Cfg_ToStd(${NCBI_CONFIGURATION_TYPES} _std_cfg)
        set(NCBI_COMPONENT_VDB_BINPATH
            ${NCBI_ThirdParty_VDB}/win/${_std_cfg}/${NCBI_ThirdParty_VDB_ARCH}/bin)
    else()
        set(NCBI_COMPONENT_VDB_BINPATH
            ${NCBI_ThirdParty_VDB}/win/$<IF:$<OR:$<CONFIG:DebugDLL>,$<CONFIG:DebugMT>,$<CONFIG:Debug>>,debug,release>/${NCBI_ThirdParty_VDB_ARCH}/bin)
        foreach(_cfg IN LISTS NCBI_CONFIGURATION_TYPES)
            if("${_cfg}" MATCHES "Debug")
                set(NCBI_COMPONENT_VDB_BINPATH_${_cfg}
                    ${NCBI_ThirdParty_VDB}/win/debug/${NCBI_ThirdParty_VDB_ARCH}/bin)
            else()
                set(NCBI_COMPONENT_VDB_BINPATH_${_cfg}
                    ${NCBI_ThirdParty_VDB}/win/release/${NCBI_ThirdParty_VDB_ARCH}/bin)
            endif()
        endforeach()
    endif()
    set(NCBI_COMPONENT_VDB_LIBS
        ${NCBI_COMPONENT_VDB_BINPATH}/ncbi-vdb-md.lib)

    set(_found YES)
    foreach(_inc IN LISTS NCBI_COMPONENT_VDB_INCLUDE)
        if(NOT EXISTS ${_inc})
            message("NOT FOUND VDB: ${_inc} not found")
            set(_found NO)
        endif()
    endforeach()
    if(_found)
        message(STATUS "Found VDB: ${NCBI_ThirdParty_VDB}")
        set(NCBI_COMPONENT_VDB_FOUND YES)
        set(HAVE_NCBI_VDB 1)
    else()
        set(NCBI_COMPONENT_VDB_FOUND NO)
        unset(NCBI_COMPONENT_VDB_INCLUDE)
        unset(NCBI_COMPONENT_VDB_LIBS)
    endif()
endif()
NCBIcomponent_report(VDB)

#############################################################################
# PYTHON
NCBI_define_Wcomponent(PYTHON python38.lib python3.lib)
NCBIcomponent_report(PYTHON)
if(NCBI_COMPONENT_PYTHON_FOUND)
    set(NCBI_COMPONENT_PYTHON_BINPATH ${NCBI_ThirdParty_PYTHON})
    set(NCBI_COMPONENT_PYTHON_VERSION 38)
endif()

##############################################################################
# GRPC/PROTOBUF
if(NOT DEFINED NCBI_PROTOC_APP)
    set(NCBI_PROTOC_APP "${NCBI_ThirdParty_GRPC}/bin/ReleaseDLL/protoc.exe")
endif()
if(NOT DEFINED NCBI_GRPC_PLUGIN)
    set(NCBI_GRPC_PLUGIN "${NCBI_ThirdParty_GRPC}/bin/ReleaseDLL/grpc_cpp_plugin.exe")
endif()
if(NOT EXISTS "${NCBI_PROTOC_APP}")
    message("NOT FOUND: ${NCBI_PROTOC_APP}")
else()
NCBI_define_Wcomponent(PROTOBUF libprotobuf.lib)
NCBIcomponent_report(PROTOBUF)
NCBI_define_Wcomponent(GRPC
    grpc++.lib grpc.lib gpr.lib address_sorting.lib cares.lib libprotobuf.lib  libprotoc.lib  upb.lib boringcrypto.lib
    boringssl.lib re2.lib absl_bad_any_cast_impl.lib absl_bad_optional_access.lib absl_bad_variant_access.lib absl_base.lib
    absl_city.lib absl_civil_time.lib absl_cord.lib absl_debugging_internal.lib absl_demangle_internal.lib absl_examine_stack.lib
    absl_exponential_biased.lib absl_failure_signal_handler.lib absl_flags.lib absl_flags_commandlineflag.lib
    absl_flags_commandlineflag_internal.lib absl_flags_config.lib absl_flags_internal.lib absl_flags_marshalling.lib
    absl_flags_parse.lib absl_flags_private_handle_accessor.lib absl_flags_program_name.lib absl_flags_reflection.lib
    absl_flags_usage.lib absl_flags_usage_internal.lib absl_graphcycles_internal.lib absl_hash.lib absl_hashtablez_sampler.lib
    absl_int128.lib absl_leak_check.lib absl_leak_check_disable.lib absl_log_severity.lib absl_malloc_internal.lib
    absl_periodic_sampler.lib absl_random_distributions.lib absl_random_internal_distribution_test_util.lib
    absl_random_internal_platform.lib absl_random_internal_pool_urbg.lib absl_random_internal_randen.lib
    absl_random_internal_randen_hwaes.lib absl_random_internal_randen_hwaes_impl.lib absl_random_internal_randen_slow.lib
    absl_random_internal_seed_material.lib absl_random_seed_gen_exception.lib absl_random_seed_sequences.lib absl_raw_hash_set.lib
    absl_raw_logging_internal.lib absl_scoped_set_env.lib absl_spinlock_wait.lib absl_stacktrace.lib absl_status.lib
    absl_statusor.lib absl_str_format_internal.lib absl_strerror.lib absl_strings.lib absl_strings_internal.lib absl_symbolize.lib
    absl_synchronization.lib absl_throw_delegate.lib absl_time.lib absl_time_zone.lib
)
NCBIcomponent_report(GRPC)
if(NCBI_COMPONENT_GRPC_FOUND)
    set(NCBI_COMPONENT_GRPC_DEFINES _WIN32_WINNT=0x0600)
endif()
endif()

##############################################################################
# XALAN
NCBI_define_Wcomponent(XALAN xalan-c.lib XalanMessages.lib)
NCBIcomponent_report(XALAN)

##############################################################################
# XERCES
NCBI_define_Wcomponent(XERCES xerces-c.lib)
NCBIcomponent_report(XERCES)
if(NCBI_COMPONENT_XERCES_FOUND)
    if(BUILD_SHARED_LIBS)
        set(NCBI_COMPONENT_XERCES_DEFINES XERCES_DLL)
    else()
        set(NCBI_COMPONENT_XERCES_DEFINES XML_LIBRARY)
    endif()
endif()

##############################################################################
# FTGL
NCBI_define_Wcomponent(FTGL ftgl.lib)
NCBIcomponent_report(FTGL)
if(NCBI_COMPONENT_FTGL_FOUND)
    set(NCBI_COMPONENT_FTGL_DEFINES FTGL_LIBRARY_STATIC)
endif()

##############################################################################
# FreeType
NCBI_define_Wcomponent(FreeType freetype.lib)
NCBIcomponent_report(FreeType)

##############################################################################
# GLEW
NCBI_define_Wcomponent(GLEW glew32mx.lib)
NCBIcomponent_report(GLEW)
if(NCBI_COMPONENT_GLEW_FOUND)
    if(BUILD_SHARED_LIBS)
        set(NCBI_COMPONENT_GLEW_DEFINES GLEW_MX)
    else()
        set(NCBI_COMPONENT_GLEW_DEFINES GLEW_MX GLEW_STATIC)
    endif()
endif()

##############################################################################
# wxWidgets
NCBI_define_Wcomponent( wxWidgets
        wxbase.lib wxbase_net.lib wxbase_xml.lib wxmsw_core.lib wxmsw_gl.lib
        wxmsw_html.lib wxmsw_aui.lib wxmsw_adv.lib wxmsw_richtext.lib wxmsw_propgrid.lib
        wxmsw_xrc.lib wxexpat.lib wxjpeg.lib wxpng.lib wxregex.lib wxtiff.lib wxzlib.lib)
NCBIcomponent_report( wxWidgets)
if(NCBI_COMPONENT_wxWidgets_FOUND)
    set(NCBI_COMPONENT_wxWidgets_INCLUDE ${NCBI_COMPONENT_wxWidgets_INCLUDE} ${NCBI_COMPONENT_wxWidgets_INCLUDE}/msvc)
    if(BUILD_SHARED_LIBS)
        set(NCBI_COMPONENT_wxWidgets_DEFINES __WXMSW__ NCBI_WXWIN_USE_PCH WXUSINGDLL=1)
    else()
        set(NCBI_COMPONENT_wxWidgets_DEFINES __WXMSW__ NCBI_WXWIN_USE_PCH)
    endif()
endif()

##############################################################################
# UV
NCBI_define_Wcomponent(UV libuv.lib)
NCBIcomponent_report( UV)
if(NCBI_COMPONENT_UV_FOUND)
    set(NCBI_COMPONENT_UV_LIBS ${NCBI_COMPONENT_UV_LIBS} psapi.lib Iphlpapi.lib userenv.lib)
endif()

##############################################################################
# NGHTTP2
NCBI_define_Wcomponent(NGHTTP2 nghttp2.lib)
NCBIcomponent_report( NGHTTP2)

##############################################################################
# GL2PS
NCBI_define_Wcomponent(GL2PS gl2ps.lib)
NCBIcomponent_report( GL2PS)

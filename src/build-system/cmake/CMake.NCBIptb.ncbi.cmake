#############################################################################
# $Id$
#############################################################################
#############################################################################
##
##  NCBI CMake wrapper extension
##  In NCBI CMake wrapper, adds NCBI-specific build parameters
##    Author: Andrei Gourianov, gouriano@ncbi
##


##############################################################################
function(NCBI_internal_add_NCBI_definitions)
    set(_defs "")
    if (${NCBI_${NCBI_PROJECT}_TYPE} STREQUAL "STATIC")
        if (WIN32)
            set(_defs "_LIB")
        endif()
    elseif (${NCBI_${NCBI_PROJECT}_TYPE} STREQUAL "SHARED")
        if (WIN32)
            set(_defs "_USRDLL")
        endif()
    elseif (${NCBI_${NCBI_PROJECT}_TYPE} STREQUAL "CONSOLEAPP")
        if (WIN32)
            set(_defs "_CONSOLE")
        endif()
        if (DEFINED NCBI_${NCBI_PROJECT}_OUTPUT)
            set(_defs  ${_defs} NCBI_APP_BUILT_AS=${NCBI_${NCBI_PROJECT}_OUTPUT})
        else()
            set(_defs  ${_defs} NCBI_APP_BUILT_AS=${NCBI_PROJECT})
        endif()
    endif()
    if(NOT "${_defs}" STREQUAL "")
        target_compile_definitions(${NCBI_PROJECT} PRIVATE ${_defs})
    endif()

endfunction()

##############################################################################
function(NCBI_internal_postproc_NCBI_tree)
    if("${NCBI_TOOLS_ROOT}" STREQUAL "")
        return()
    endif()
    get_property(_alltargets  GLOBAL PROPERTY NCBI_PTBPROP_ALLTARGETS)
    foreach( _target IN LISTS _alltargets)
        get_target_property(_deps ${_target} INTERFACE_LINK_LIBRARIES)
        if(NOT "${_deps}" STREQUAL "")
            if (NCBI_PTBCFG_ENABLE_COLLECTOR)
                get_property(_todo GLOBAL PROPERTY NCBI_PTBPROP_DEPS_${_target})
                foreach( _d IN LISTS _todo)
                    if(TARGET ${_d})
                        get_target_property(_d_deps ${_d} INTERFACE_LINK_LIBRARIES)
                        list(APPEND _deps ${_d_deps})
                    endif()
                endforeach()
            else()
                set(_todo ${_deps})
                set(_done "")
                foreach(_i RANGE 1000)
                    set(_next "")
                    foreach( _d IN LISTS _todo)
                        if(TARGET ${_d} AND NOT ${_d} IN_LIST _done)
                            list(APPEND _done ${_d})
                            get_target_property(_d_deps ${_d} INTERFACE_LINK_LIBRARIES)
                            list(APPEND _next ${_d_deps})
                            list(APPEND _deps ${_d_deps})
                        endif()
                    endforeach()
                    if("${_next}" STREQUAL "")
                        break()
                    endif()
                    set(_todo ${_next})
                endforeach()
            endif()

            list(REMOVE_DUPLICATES _deps)
            set(_rpathdirs "")
            list(APPEND _rpathdirs ${CMAKE_INSTALL_RPATH})
            foreach( _lib IN LISTS _deps)
                get_filename_component(_dir ${_lib} DIRECTORY)
                string(FIND "${_dir}" "${NCBI_TOOLS_ROOT}" _pos)
                if(${_pos} EQUAL 0)
                    string(REPLACE "${NCBI_TOOLS_ROOT}" "${NCBI_OPT_ROOT}" _dir0 "${_dir}")
                    list(APPEND _rpathdirs ${_dir0} ${_dir})
                endif()
            endforeach()
            list(REMOVE_DUPLICATES _rpathdirs)
            if(NOT "${_rpathdirs}" STREQUAL "")
#                set_target_properties(${_target} PROPERTIES BUILD_RPATH "${_rpathdirs}")
                set_target_properties(${_target} PROPERTIES INSTALL_RPATH "${_rpathdirs}")
            endif()
        endif()
    endforeach()

endfunction()

#############################################################################
function(NCBI_internal_handle_VDB_rpath)
    get_property(_req GLOBAL PROPERTY NCBI_PTBPROP_REQUIRES_${NCBI_PROJECT})
    if(VDB IN_LIST _req AND NOT "${NCBI_${NCBI_PROJECT}_TYPE}" STREQUAL "STATIC")
        get_filename_component(_fullver ${NCBI_ThirdParty_VDB} NAME)
        string(REPLACE "." ";" _ver ${_fullver})
        list(GET _ver 0 _major)
        add_custom_command(TARGET ${NCBI_PROJECT} POST_BUILD COMMAND 
#            install_name_tool -change @executable_path/../lib/libncbi-vdb.${_fullver}.dylib @rpath/libncbi-vdb.${_major}.dylib
            install_name_tool -change @executable_path/../lib/libncbi-vdb.${_fullver}.dylib ${NCBI_COMPONENT_VDB_LIBPATH}/libncbi-vdb.${_major}.dylib
            $<TARGET_FILE:${NCBI_PROJECT}>)
    endif()
endfunction()

#############################################################################
function(NCBI_internal_rank_projects)
    set_property(GLOBAL PROPERTY NCBI_PTBPROP_RANK_DONE "")
    set_property(GLOBAL PROPERTY NCBI_PTBPROP_RANK_MAX "0")
    foreach(_prj IN LISTS NCBI_PTB_ALLOWED_PROJECTS)
        get_property(_host GLOBAL PROPERTY NCBI_PTBPROP_HOST_${_prj})
        if ("${_host}" STREQUAL "" OR "${_host}" STREQUAL "${_prj}")
            NCBI_internal_rank_add_project( ${_prj})
        endif()
    endforeach()

    get_property(_count GLOBAL PROPERTY NCBI_PTBPROP_RANK_MAX)
    set(_contents "")
    foreach(_r RANGE ${_count} 0 -1)
        string(APPEND _contents "#${_r}--------------------------------------------------------------------------\n")
        get_property(_lst GLOBAL PROPERTY NCBI_PTBPROP_RANK_${_r})
        list(SORT _lst)
        foreach(_project IN LISTS _lst)
            get_property(_deps GLOBAL PROPERTY NCBI_PTBPROP_DIRECT_DEPS_${_project})
            get_property(_req1 GLOBAL PROPERTY NCBI_PTBPROP_REQUIRES_${_project})
            get_property(_req2 GLOBAL PROPERTY NCBI_PTBPROP_COMPONENTS_${_project})

            set(_pkg_skip FALSE)
            set(_pkg_libs "${_project}")
            set(_pkg_comp "${_project}")
            if("${_project}" STREQUAL "general")
                set(_pkg_libs "\$<1:general>")
                set(_pkg_comp "libgeneral")
            elseif("${_project}" STREQUAL "datatool")
                set(_pkg_skip TRUE)
            endif()
            set(_pkg_reqs)
            foreach(_d IN LISTS _deps)
                if("${_d}" STREQUAL "general")
                    list(APPEND _pkg_reqs "libgeneral")
                else()
                    list(APPEND _pkg_reqs "${_d}")
                endif()
            endforeach()
            foreach(_d IN LISTS _req1 _req2)
                if("${_d}" STREQUAL "ODBC")
                    list(APPEND _pkg_reqs "ODBC")
                    continue()
                endif()
                if("${_d}" STREQUAL "NCBI_C" AND "${_d}" IN_LIST _req2)
                    continue()
                endif()
                if(NCBI_COMPONENT_${_d}_FOUND)
                    if("${_d}" STREQUAL "XODBC")
                        continue()
                    elseif("${_d}" STREQUAL "UNWIND")
                        continue()
                    elseif("${_d}" STREQUAL "FreeTDS")
                        continue()
                    elseif("${_d}" STREQUAL "EXSLT")
                        continue()
                    elseif("${_d}" MATCHES "^Boost")
                        list(APPEND _pkg_reqs "Boost")
                    elseif("${_d}" MATCHES "^Local")
                        set(_pkg_skip TRUE)
                    else()
                        list(APPEND _pkg_reqs "${_d}")
                    endif()
                endif()
            endforeach()
            if("${_deps}" STREQUAL "")
                if("${_project}" STREQUAL "connect")
                    list(APPEND _pkg_reqs "NETWORKLIBS")
                endif()
                list(APPEND _pkg_reqs "ORIGLIBS")
            endif()
            if(_pkg_skip)
                continue()
            endif()

            string(REPLACE ";" "\", \"" _pkg_reqs "${_pkg_reqs}")
            string(APPEND _contents "            if \"${_project}\" in allexports:\n")
            string(APPEND _contents "                self.cpp_info.components[\"${_pkg_comp}\"].libs = [\"${_pkg_libs}\"]\n")
            string(APPEND _contents "                self.cpp_info.components[\"${_pkg_comp}\"].requires = [\"${_pkg_reqs}\"]\n")
        endforeach()
    endforeach()
    if(NOT IS_ABSOLUTE ${NCBI_PTBCFG_PACKAGE_DEPS})
        set(NCBI_PTBCFG_PACKAGE_DEPS ${NCBI_TREE_ROOT}/${NCBI_PTBCFG_PACKAGE_DEPS})
    endif()
    file(WRITE ${NCBI_PTBCFG_PACKAGE_DEPS} ${_contents})
endfunction()

#############################################################################
function(NCBI_internal_rank_add_project _prj)
    get_property(_done GLOBAL PROPERTY NCBI_PTBPROP_RANK_DONE)
    if(${_prj} IN_LIST _done)
        return()
    endif()
    get_property(_deps GLOBAL PROPERTY NCBI_PTBPROP_DIRECT_DEPS_${_prj})
    if("${_deps}" STREQUAL "")
        set_property(GLOBAL APPEND PROPERTY NCBI_PTBPROP_RANK_0 ${_prj})
        set_property(GLOBAL APPEND PROPERTY NCBI_PTBPROP_RANK_DONE ${_prj})
        return()
    endif()
    foreach(_dep IN LISTS _deps)
        NCBI_internal_rank_add_project(${_dep})
    endforeach()
    set(_thisrank 0)
    get_property(_count GLOBAL PROPERTY NCBI_PTBPROP_RANK_MAX)
    foreach(_dep IN LISTS _deps)
        foreach(_r RANGE ${_count})
            get_property(_lst GLOBAL PROPERTY NCBI_PTBPROP_RANK_${_r})
            if(${_dep} IN_LIST _lst)
                if(${_r} GREATER ${_thisrank})
                    set(_thisrank ${_r})
                endif()
            endif()
        endforeach()
    endforeach()
    math(EXPR _thisrank "${_thisrank} + 1")
    if(${_thisrank} GREATER ${_count})
        set_property(GLOBAL PROPERTY NCBI_PTBPROP_RANK_MAX ${_thisrank})
    endif()
    set_property(GLOBAL APPEND PROPERTY NCBI_PTBPROP_RANK_${_thisrank} ${_prj})
    set_property(GLOBAL APPEND PROPERTY NCBI_PTBPROP_RANK_DONE ${_prj})
endfunction()

#############################################################################
# run_with_cd_reporter

function(NCBI_internal_configure_cd_reporter)
    if (NOT NCBI_PYTHON_EXECUTABLE)
        message(STATUS "Could not find Python3. Disabling cd_reporter.")
        return()
    endif()
    set(CD_REPORTER "/am/ncbiapdata/bin/cd_reporter")
    if (NOT EXISTS ${CD_REPORTER})
        message(STATUS "Could not find cd_reporter. Disabling cd_reporter.")
        return()
    endif()
    if ("$ENV{TEAMCITY_VERSION}" STREQUAL "")
        message(STATUS "Detected development build. Disabling cd_reporter.")
        return()
    endif()
    message(STATUS "Generating ${build_root}/run_with_cd_reporter.py...")
    message(STATUS "Python3 path: ${NCBI_PYTHON_EXECUTABLE}")

    set(PYTHON3 ${NCBI_PYTHON_EXECUTABLE})
    if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
        set(abs_top_srcdir ${NCBI_EXTERNAL_TREE_ROOT})
    else()
        set(abs_top_srcdir ${NCBITK_TREE_ROOT})
    endif()
    set(top_src_dir ${abs_top_srcdir})
    set(status_dir ${NCBI_BUILD_ROOT}/status)
    configure_file(${NCBI_TREE_BUILDCFG}/run_with_cd_reporter.py.in ${build_root}/build-system/run_with_cd_reporter.py)
    # copy to build_root and set executable permissions (configure_file doesn't set permissions)
    file(COPY ${build_root}/build-system/run_with_cd_reporter.py DESTINATION ${build_root}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${build_root}/run_with_cd_reporter.py)
endfunction()

if(UNIX AND NOT APPLE)
    NCBI_internal_configure_cd_reporter()
endif()

#############################################################################
NCBI_register_hook(TARGET_ADDED NCBI_internal_add_NCBI_definitions)
if(NOT NCBI_PTBCFG_PACKAGING AND NOT NCBI_PTBCFG_PACKAGED AND NOT NCBI_PTBCFG_COMPONENT_StaticComponents)
    if(APPLE)
        NCBI_register_hook(TARGET_ADDED NCBI_internal_handle_VDB_rpath)
    endif()
    if(UNIX AND NOT APPLE)
        NCBI_register_hook(ALL_ADDED    NCBI_internal_postproc_NCBI_tree)
    endif()
endif()
if(DEFINED NCBI_PTBCFG_PACKAGE_DEPS)
    NCBI_register_hook(COLLECTED    NCBI_internal_rank_projects)
endif()

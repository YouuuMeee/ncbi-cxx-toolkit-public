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
NCBI_register_hook(TARGET_ADDED NCBI_internal_add_NCBI_definitions)
if(APPLE)
    NCBI_register_hook(TARGET_ADDED NCBI_internal_handle_VDB_rpath)
endif()
if(UNIX AND NOT APPLE)
    NCBI_register_hook(ALL_ADDED    NCBI_internal_postproc_NCBI_tree)
endif()

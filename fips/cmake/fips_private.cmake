#-------------------------------------------------------------------------------
#   fips_private.cmake
#   Private cmake macros (all the public stuff is in fips.cmake)
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#   fisit_reset(target)
#   Reset the global tracker variables.
#
macro(fips_reset target)
    set(CurDir)
    set(CurGroup "")
    set(CurSources)
    set(CurDependencies)
    set(CurLinkLibs)
    set(CurLinkLibsDebug)
    set(CurLinkLibsRelease)
    set(CurFrameworks)
    set(CurAppType)
    set(CurImgFiles)
    set(CurCompileFlags "")
    set(FipsAddFilesEnabled 1)
    set(CurTargetName ${target})
endmacro()

#-------------------------------------------------------------------------------
#   fips_apply_target_group(target)
#   Apply IDE group name to target.
#
macro(fips_apply_target_group target)
    if (NOT ${FIPS_TARGET_GROUP} STREQUAL "")
        set_target_properties(${target} PROPERTIES FOLDER ${FIPS_TARGET_GROUP})
    endif()
endmacro()

#-------------------------------------------------------------------------------
#   fips_setup_link_directories()
#   Setup the link library search paths.
#
macro(fips_setup_link_directories)
    if (EXISTS ${FIPS_PROJECT_DIR}/lib/${FIPS_PLATFORM_NAME})
        link_directories(${FIPS_PROJECT_DIR}/lib/${FIPS_PLATFORM_NAME})
    endif()
endmacro()

#-------------------------------------------------------------------------------
#   fips_resolve_dependencies(target)
#   Add all required dependencies to a target
#
macro(fips_resolve_dependencies target)
    if (CurDependencies)
        target_link_libraries(${CurTargetName} ${CurDependencies})
    endif()
    if (CurLinkLibs)
        target_link_libraries(${CurTargetName} ${CurLinkLibs})
    endif()
    foreach (lib_debug ${CurLinkLibsDebug})
        target_link_libraries(${target} debug ${lib_debug})
    endforeach()
    foreach (lib_release ${CurLinkLibsRelease})
        target_link_libraries(${target} optimized ${lib_release})
    endforeach()
    if (FIPS_IOS OR FIPS_OSX)
        foreach (fw ${CurFrameworks})
            unset(found_framework CACHE)
            find_library(found_framework ${fw})
            target_link_libraries(${target} ${found_framework})
            unset(found_framework CACHE)
        endforeach()
    endif()
endmacro()

#-------------------------------------------------------------------------------
#   fips_config_postfixes_for_exe(target)
#   Internal macro to set the executable extensions
#
macro(fips_config_postfixes_for_exe target)
    if (FIPS_EMSCRIPTEN)
        set_target_properties(${target} PROPERTIES RELEASE_POSTFIX .html)
        set_target_properties(${target} PROPERTIES DEBUG_POSTFIX .html)
        set_target_properties(${target} PROPERTIES PROFILING_POSTFIX .html)
    elseif (FIPS_PNACL)
        set_target_properties(${target} PROPERTIES RELEASE_POSTFIX .pexe)
        set_target_properties(${target} PROPERTIES DEBUG_POSTFIX .pexe)
        set_target_properties(${target} PROPERTIES PROFILING_POSTFIX .pexe)
    endif()
endmacro(fips_config_postfixes_for_exe)

#-------------------------------------------------------------------------------
#   fips_output_directory(target)
#   Internal macro to set the output directory for exes and sharedlibs
#
function(fips_config_output_directory target)
    if (NOT (FIPS_IOS OR FIPS_ANDROID))
        set(dir ${FIPS_DEPLOY_DIR}/${CMAKE_PROJECT_NAME}/${FIPS_CONFIG})

        # exes
        set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${dir})
        set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${dir})
        set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${dir})
        set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_PROFILING ${dir})

        # ...and DLLs
        set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${dir})
        set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${dir})
        set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${dir})
        set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_PROFILING ${dir})
    endif()
endfunction()

#-------------------------------------------------------------------------------
#   fips_reset_targets_list()
#   Clears a .yml file which keeps track of all targets.
#
function(fips_reset_targets_list)
    file(WRITE "${CMAKE_BINARY_DIR}/fips_targets.yml" "---\n")
endfunction()

#-------------------------------------------------------------------------------
#   fips_addto_targets_list(target type)
#   Adds a new entry to the targets type list, this is called from 
#   the fips_end_xxx() functions.
#
function(fips_addto_targets_list target type)
    file(APPEND "${CMAKE_BINARY_DIR}/fips_targets.yml" "${target} : ${type}\n")
endfunction()

#-------------------------------------------------------------------------------
#   fips_choose_config()
#   Sets the FIPS_CONFIG variable to a sensible value, call this if
#   FIPS_CONFIG hasn't been provided by the command line when using
#   some IDEs which are not directly supported by cmake, like QtCreator
#   or CLion.
#
macro(fips_choose_config)
    if (FIPS_HOST_WINDOWS)
        if (CMAKE_CL_64)
            if (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
                set(FIPS_CONFIG "win64-vs2013-release")
            elseif (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Profiling")
                set (FIPS_CONFIG "win64-vs2013-profiling")
            else()
                set (FIPS_CONFIG "win64-vs2013-debug")
            endif()
        else()
            if (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
                set (FIPS_CONFIG "win32-vs2013-release")
            elseif (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Profiling")
                set (FIPS_CONFIG "win32-vs2013-profiling")
            else()
                set (FIPS_CONFIG "win32-vs2013-debug")
            endif()
        endif()
    elseif (FIPS_HOST_OSX)
        if (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
            set (FIPS_CONFIG "osx-make-release")
        elseif (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Profiling")
            set (FIPS_CONFIG "osx-make-profiling")
        else()
            set (FIPS_CONFIG "osx-make-debug")
        endif()
    elseif (FIPS_HOST_LINUX)
        if (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
            set (FIPS_CONFIG "linux-make-release")
        elseif (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Profiling")
            set (FIPS_CONFIG "linux-make-profiling")
        else()
            set (FIPS_CONFIG "linux-make-debug")
        endif()
    endif()
endmacro()

#-------------------------------------------------------------------------------
#   fips_add_file()
#   Private helper function to add a single file to the project, with
#   additional handling for code generation files.
#
macro(fips_add_file new_file)

    if (FipsAddFilesEnabled)
        # handle subdirectory
        if (CurDir)
            set(cur_file "${CurDir}${new_file}")
        else()
            set(cur_file ${new_file})
        endif()
        get_filename_component(f_ext ${cur_file} EXT)
        
        # determine source group name and
        # add to current source group
        source_group("${CurGroup}" FILES ${cur_file})

        # mark .m as .c file for older cmake versions (bug is fixed in cmake 3.1+)
        if (FIPS_OSX)
            if (${f_ext} STREQUAL ".m")
                set_source_files_properties(${cur_file} PROPERTIES LANGUAGE C)
            endif()
        endif()

        # add to global tracker variables
        list(APPEND CurSources ${cur_file})

        # remove dups
        if (CurSources)
            list(REMOVE_DUPLICATES CurSources)
        endif()
    endif()
endmacro()

#-------------------------------------------------------------------------------
#   fips_add_target_dependency(target...)
#   Add one or more dependencies to the current target. The dependencies
#   must be cmake build targets defined with fips_begin*/fips_end*().
#   Used to define a build order required when, for example, building tools to
#   use during compilation of the current target.
#
macro(fips_add_target_dependency targets)
    foreach(target ${ARGV})
        if (TARGET ${target})
            list(APPEND CurTargetDependencies ${target})
        endif()
    endforeach()
    if (CurTargetDependencies)
        list(REMOVE_DUPLICATES CurTargetDependencies)
    endif()
endmacro()

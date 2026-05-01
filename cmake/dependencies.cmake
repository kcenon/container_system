# dependencies.cmake - third-party / sibling-system dependency resolution
#
# This module resolves all external dependencies for container_system. It is
# included AFTER cmake/options.cmake so it can read the option values, but
# BEFORE cmake/targets.cmake because target wiring needs the imported targets
# created here (notably kcenon::common_system).

# Find required packages
find_package(Threads REQUIRED)

# Note: Uses C++20 std::format when available, with ostringstream fallback
message(STATUS "Using C++20 std::format if available, with ostringstream fallback")

# Include unified dependency resolution module
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UnifiedDependencies.cmake)

#[[
configure_container_common_system_dep(<target>)

Wires the common_system dependency onto the named target. This includes
calling unified_find_dependency(), exposing the include directories
PUBLICly, defining KCENON_HAS_COMMON_SYSTEM, and creating the
kcenon::common_system imported INTERFACE target if it does not already
exist. The behaviour is identical to the inline block that previously
lived in the top-level CMakeLists.txt.
]]
function(configure_container_common_system_dep target_name)
    if(NOT BUILD_WITH_COMMON_SYSTEM)
        return()
    endif()

    message(STATUS "Container System: Enabling common_system integration")

    # Use unified dependency resolution (replaces ~40 lines of hardcoded path logic)
    unified_find_dependency(common_system REQUIRED)

    # Apply include directories
    target_include_directories(${target_name} PUBLIC
        $<BUILD_INTERFACE:${common_system_INCLUDE_DIR}>
    )
    target_compile_definitions(${target_name} PUBLIC KCENON_HAS_COMMON_SYSTEM=1)

    if(NOT TARGET kcenon::common_system)
        add_library(kcenon::common_system INTERFACE IMPORTED GLOBAL)

        if(TARGET common_system)
            get_target_property(_container_common_include_dirs common_system INTERFACE_INCLUDE_DIRECTORIES)
            get_target_property(_container_common_compile_defs common_system INTERFACE_COMPILE_DEFINITIONS)
            get_target_property(_container_common_compile_features common_system INTERFACE_COMPILE_FEATURES)
            get_target_property(_container_common_link_libraries common_system INTERFACE_LINK_LIBRARIES)

            if(_container_common_include_dirs)
                set_property(TARGET kcenon::common_system PROPERTY
                    INTERFACE_INCLUDE_DIRECTORIES "${_container_common_include_dirs}")
            endif()
            if(_container_common_compile_defs)
                set_property(TARGET kcenon::common_system PROPERTY
                    INTERFACE_COMPILE_DEFINITIONS "${_container_common_compile_defs}")
            endif()
            if(_container_common_compile_features)
                set_property(TARGET kcenon::common_system PROPERTY
                    INTERFACE_COMPILE_FEATURES "${_container_common_compile_features}")
            endif()
            if(_container_common_link_libraries)
                set_property(TARGET kcenon::common_system PROPERTY
                    INTERFACE_LINK_LIBRARIES "${_container_common_link_libraries}")
            endif()
        else()
            set_property(TARGET kcenon::common_system PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES "${common_system_INCLUDE_DIR}")
            set_property(TARGET kcenon::common_system PROPERTY
                INTERFACE_COMPILE_DEFINITIONS "KCENON_WITH_COMMON_SYSTEM=1")
            set_property(TARGET kcenon::common_system PROPERTY
                INTERFACE_COMPILE_FEATURES "cxx_std_20")
        endif()
    endif()
endfunction()

# PluginSystem - Main CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(PluginSystem VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_SHARED_LIBS "Build libraries as shared libraries" ON)
option(BUILD_TESTS "Build test programs" ON)
option(BUILD_ALL_PLUGINS "Build all plugins" ON)
option(BUILD_PLUGIN_TESTS "Build plugin tests" ON)

# Compiler cache optimization (Method 1)
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found")
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define platform-specific settings
if(WIN32)
    # Windows-specific settings
    add_definitions(-DWIN32)
    add_definitions(-D_WINDOWS)
    if(BUILD_SHARED_LIBS)
        add_definitions(-DPLUGIN_SHARED)
    endif()
    set(PLUGIN_EXTENSION ".dll")
    # Enable incremental linking for faster builds
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL")
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    add_definitions(-DLINUX)
    if(BUILD_SHARED_LIBS)
        add_definitions(-DPLUGIN_SHARED)
    endif()
    set(PLUGIN_EXTENSION ".so")
elseif(APPLE)
    # macOS-specific settings
    add_definitions(-DMACOS)
    if(BUILD_SHARED_LIBS)
        add_definitions(-DPLUGIN_SHARED)
    endif()
    set(PLUGIN_EXTENSION ".dylib")
endif()

# Plugin discovery function
function(discover_plugins PLUGIN_DIR)
    file(GLOB PLUGIN_SUBDIRS RELATIVE ${PLUGIN_DIR} ${PLUGIN_DIR}/*/)
    set(DISCOVERED_PLUGINS "")
    
    foreach(SUBDIR ${PLUGIN_SUBDIRS})
        if(IS_DIRECTORY ${PLUGIN_DIR}/${SUBDIR})
            if(EXISTS ${PLUGIN_DIR}/${SUBDIR}/CMakeLists.txt)
                list(APPEND DISCOVERED_PLUGINS ${SUBDIR})
                message(STATUS "发现插件: ${SUBDIR}")
            endif()
        endif()
    endforeach()
    
    set(PLUGIN_LIST ${DISCOVERED_PLUGINS} PARENT_SCOPE)
endfunction()

# Auto-discover plugins
discover_plugins(${CMAKE_SOURCE_DIR}/src/Plugins)

# Create build options for each plugin
foreach(PLUGIN ${PLUGIN_LIST})
    string(TOUPPER ${PLUGIN} PLUGIN_UPPER)
    option(BUILD_${PLUGIN_UPPER} "Build ${PLUGIN}" ${BUILD_ALL_PLUGINS})
endforeach()

# Pass plugin list to subdirectories
set(PLUGIN_LIST ${PLUGIN_LIST} CACHE INTERNAL "List of discovered plugins")

add_subdirectory(src)
if(BUILD_TESTS)
    # Enable testing
    enable_testing()
    add_subdirectory(tests)
endif()

# Add examples
add_subdirectory(examples)

# Installation rules
install(DIRECTORY ${CMAKE_SOURCE_DIR}/scripts/ DESTINATION scripts)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/plugins/ DESTINATION plugins)
install(FILES README.md DESTINATION .)
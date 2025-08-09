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
# src/Plugins/ScriptPlugin/CMakeLists.txt

# Support standalone build
if(NOT TARGET PluginCore)
    cmake_minimum_required(VERSION 3.16)
    project(ScriptPlugin VERSION 1.0.0 LANGUAGES CXX)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Find PluginCore
    find_package(PluginCore REQUIRED)
endif()

# Define header files for interface library
set(SCRIPT_PLUGIN_HEADERS
    include/ScriptPlugin.h
    include/ScriptPluginExport.h
)

# Create INTERFACE library target (no source files, only headers)
add_library(ScriptPlugin INTERFACE)

# Set include directories for interface library
target_include_directories(ScriptPlugin
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies - interface library only provides headers
target_link_libraries(ScriptPlugin INTERFACE PluginCore)

# Installation rules
install(TARGETS ScriptPlugin
    EXPORT ScriptPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${SCRIPT_PLUGIN_HEADERS} DESTINATION include/ScriptPlugin)

# Add subdirectories for specific script language implementations
add_subdirectory(PythonPlugin)
add_subdirectory(LuaPlugin)
# src/Plugins/ScriptPlugin/PythonPlugin/CMakeLists.txt

# Include Python build configuration file
include(${CMAKE_CURRENT_SOURCE_DIR}/PythonBuild.cmake)

# Define source files
set(PYTHON_PLUGIN_SOURCES
    src/PythonPlugin.cpp
)

# Define header files
set(PYTHON_PLUGIN_HEADERS
    include/PythonPlugin.h
)

# Create library target
if(BUILD_SHARED_LIBS)
    add_library(PythonPlugin SHARED ${PYTHON_PLUGIN_SOURCES} ${PYTHON_PLUGIN_HEADERS})
    target_compile_definitions(PythonPlugin PRIVATE PYTHON_PLUGIN_EXPORTS)
    target_compile_definitions(PythonPlugin PUBLIC PYTHON_PLUGIN_SHARED)
else()
    add_library(PythonPlugin STATIC ${PYTHON_PLUGIN_SOURCES} ${PYTHON_PLUGIN_HEADERS})
endif()

# Set include directories
target_include_directories(PythonPlugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${PYTHON_INCLUDE_DIRS}
        ${PYBIND11_SOURCE_DIR}/include
        ${rtm_SOURCE_DIR}/includes
)

# Link dependencies - PythonPlugin is an implementation of ScriptPlugin interface
target_link_libraries(PythonPlugin PRIVATE 
    PluginCore
    MathPlugin
    ${PYTHON_LIBRARIES}
    PUBLIC ScriptPlugin  # Link to ScriptPlugin interface
)

# Add dependency to ensure python_lib is built before PythonPlugin
add_dependencies(PythonPlugin python_lib)

# Installation rules
install(TARGETS PythonPlugin python_lib
    EXPORT PythonPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${PYTHON_PLUGIN_HEADERS} DESTINATION include/PythonPlugin)
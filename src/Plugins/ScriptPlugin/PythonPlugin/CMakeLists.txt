# src/Plugins/ScriptPlugin/PythonPlugin/CMakeLists.txt

# Include FetchContent for pybind11
include(FetchContent)

# Configure pybind11 build
set(PYBIND11_TEST OFF CACHE BOOL "Build pybind11 tests" FORCE)

# Download and build pybind11
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
    GIT_SHALLOW TRUE
)

# Make pybind11 available
FetchContent_MakeAvailable(pybind11)

# Find Python components
find_package(Python COMPONENTS Interpreter Development REQUIRED)

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
        ${Python_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(PythonPlugin PRIVATE 
    PluginCore
    ScriptPlugin
    MathPlugin
    pybind11::pybind11
    ${Python_LIBRARIES}
)

# Installation rules
install(TARGETS PythonPlugin
    EXPORT PythonPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${PYTHON_PLUGIN_HEADERS} DESTINATION include/PythonPlugin)
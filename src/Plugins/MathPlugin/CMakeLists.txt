# src/Plugins/MathPlugin/CMakeLists.txt

# Download and build Realtime Math (RTM) from GitHub
include(FetchContent)
FetchContent_Declare(
    rtm
    GIT_REPOSITORY https://github.com/nfrechette/rtm.git
    GIT_TAG v2.1.5
    GIT_SHALLOW TRUE
)

# Configure RTM build
set(RTM_INSTALL OFF CACHE BOOL "Install RTM" FORCE)
set(RTM_BUILD_TESTS OFF CACHE BOOL "Build RTM tests" FORCE)
set(RTM_BUILD_BENCHMARK OFF CACHE BOOL "Build RTM benchmarks" FORCE)

# Make RTM available
FetchContent_MakeAvailable(rtm)

# Define source files
set(MATH_PLUGIN_SOURCES
    src/MathPlugin.cpp
)

# Define header files
set(MATH_PLUGIN_HEADERS
    include/MathPlugin.h
    include/MathPluginExport.h
)

# Create library target
if(BUILD_SHARED_LIBS)
    add_library(MathPlugin SHARED ${MATH_PLUGIN_SOURCES} ${MATH_PLUGIN_HEADERS})
    target_compile_definitions(MathPlugin PRIVATE MATH_PLUGIN_EXPORTS)
    target_compile_definitions(MathPlugin PUBLIC MATH_PLUGIN_SHARED)
else()
    add_library(MathPlugin STATIC ${MATH_PLUGIN_SOURCES} ${MATH_PLUGIN_HEADERS})
endif()

# Set include directories
target_include_directories(MathPlugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${rtm_SOURCE_DIR}/includes
)

# Link dependencies
target_link_libraries(MathPlugin PRIVATE 
    PluginCore
)

# Installation rules
install(TARGETS MathPlugin
    EXPORT MathPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${MATH_PLUGIN_HEADERS} DESTINATION include/MathPlugin)
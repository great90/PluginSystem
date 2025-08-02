# src/Plugins/LogPlugin/CMakeLists.txt

# Download and build spdlog from GitHub
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.11.0
    GIT_SHALLOW TRUE
)

# Configure spdlog build
set(SPDLOG_INSTALL OFF CACHE BOOL "Install spdlog" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "Build spdlog examples" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "Build spdlog tests" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "Build spdlog benchmarks" FORCE)

# Make spdlog available
FetchContent_MakeAvailable(spdlog)

# Define source files
set(LOG_PLUGIN_SOURCES
    src/LogPlugin.cpp
)

# Define header files
set(LOG_PLUGIN_HEADERS
    include/LogPlugin.h
)

# Create library target
if(BUILD_SHARED_LIBS)
    add_library(LogPlugin SHARED ${LOG_PLUGIN_SOURCES} ${LOG_PLUGIN_HEADERS})
    target_compile_definitions(LogPlugin PRIVATE LOG_PLUGIN_EXPORTS)
    target_compile_definitions(LogPlugin PUBLIC LOG_PLUGIN_SHARED)
else()
    add_library(LogPlugin STATIC ${LOG_PLUGIN_SOURCES} ${LOG_PLUGIN_HEADERS})
endif()

# Set include directories
target_include_directories(LogPlugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(LogPlugin PRIVATE 
    PluginCore
    spdlog::spdlog
)

# Installation rules
install(TARGETS LogPlugin
    EXPORT LogPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${LOG_PLUGIN_HEADERS} DESTINATION include/LogPlugin)
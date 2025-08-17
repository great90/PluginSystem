# LLGLRenderingPlugin CMakeLists.txt
message(STATUS "Processing LLGLRenderingPlugin CMakeLists.txt")

# Include the build configuration
include(Build.cmake)
message(STATUS "LLGL_RENDERING_PLUGIN_SOURCES: ${LLGL_RENDERING_PLUGIN_SOURCES}")

# Set the target name
set(TARGET_NAME LLGLRenderingPlugin)

# Create the plugin library
add_library(${TARGET_NAME} SHARED
    ${LLGL_RENDERING_PLUGIN_SOURCES}
)

# Set target properties
set_target_properties(${TARGET_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(${TARGET_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include  # For RenderingPluginBase.h
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${TARGET_NAME}
    PRIVATE
        PluginCore
        LLGL
        RenderingPluginComponents
)

# Compiler definitions
target_compile_definitions(${TARGET_NAME}
    PRIVATE
        LLGL_RENDERING_PLUGIN_EXPORTS
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
elseif(APPLE)
    target_compile_definitions(${TARGET_NAME} PRIVATE APPLE_PLATFORM)
elseif(UNIX)
    target_compile_definitions(${TARGET_NAME} PRIVATE LINUX_PLATFORM)
endif()
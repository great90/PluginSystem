# Examples CMakeLists.txt

# Rendering Example
add_executable(rendering_example rendering_example.cpp)

# Include directories
target_include_directories(rendering_example PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/Plugins/RenderingPlugin/include
)

# Link libraries
target_link_libraries(rendering_example PRIVATE
    PluginCore
    RenderingPlugin
)

# Set output directory
set_target_properties(rendering_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy plugins to output directory after build
add_custom_command(TARGET rendering_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/plugins
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:RenderingPlugin>
        ${CMAKE_BINARY_DIR}/bin/plugins/
    COMMENT "Copying RenderingPlugin to examples output directory"
)

# Platform-specific settings
if(WIN32)
    # Windows specific settings
    set_target_properties(rendering_example PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
elseif(APPLE)
    # macOS specific settings
    set_target_properties(rendering_example PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(rendering_example PRIVATE /W4)
else()
    target_compile_options(rendering_example PRIVATE -Wall -Wextra -Wpedantic)
endif()
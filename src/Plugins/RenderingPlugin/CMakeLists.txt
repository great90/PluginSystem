# src/Plugins/RenderingPlugin/CMakeLists.txt

# Download and build LLGL from GitHub
include(FetchContent)
FetchContent_Declare(
    LLGL
    GIT_REPOSITORY https://github.com/LukasBanana/LLGL.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# Configure LLGL build
set(LLGL_BUILD_EXAMPLES OFF CACHE BOOL "Build LLGL examples" FORCE)
set(LLGL_BUILD_TESTS OFF CACHE BOOL "Build LLGL tests" FORCE)
set(LLGL_BUILD_TUTORIALS OFF CACHE BOOL "Build LLGL tutorials" FORCE)
set(LLGL_BUILD_WRAPPER_C99 OFF CACHE BOOL "Build LLGL C99 wrapper" FORCE)
set(LLGL_BUILD_WRAPPER_CSHARP OFF CACHE BOOL "Build LLGL C# wrapper" FORCE)
set(LLGL_BUILD_WRAPPER_GO OFF CACHE BOOL "Build LLGL Go wrapper" FORCE)
# Check for OpenGL SDK
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    set(LLGL_BUILD_RENDERER_OPENGL ON CACHE BOOL "Build OpenGL renderer" FORCE)
    message(STATUS "OpenGL SDK found - OpenGL renderer enabled")
else()
    set(LLGL_BUILD_RENDERER_OPENGL OFF CACHE BOOL "Build OpenGL renderer" FORCE)
    message(WARNING "OpenGL SDK not found - OpenGL renderer disabled")
endif()

# Check for Vulkan SDK
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    set(LLGL_BUILD_RENDERER_VULKAN ON CACHE BOOL "Build Vulkan renderer" FORCE)
    message(STATUS "Vulkan SDK found - Vulkan renderer enabled")
else()
    set(LLGL_BUILD_RENDERER_VULKAN OFF CACHE BOOL "Build Vulkan renderer" FORCE)
    message(WARNING "Vulkan SDK not found - Vulkan renderer disabled")
endif()

if(WIN32)
    # Check for DirectX SDK on Windows
    find_path(DIRECTX_INCLUDE_DIR d3d11.h
        PATHS
            "$ENV{DXSDK_DIR}/Include"
            "$ENV{ProgramFiles}/Microsoft DirectX SDK*/Include"
            "$ENV{ProgramFiles(x86)}/Microsoft DirectX SDK*/Include"
            "$ENV{ProgramFiles}/Windows Kits/*/Include/*/um"
            "$ENV{ProgramFiles(x86)}/Windows Kits/*/Include/*/um"
    )
    
    if(DIRECTX_INCLUDE_DIR)
        set(LLGL_BUILD_RENDERER_DIRECT3D11 ON CACHE BOOL "Build Direct3D 11 renderer" FORCE)
        set(LLGL_BUILD_RENDERER_DIRECT3D12 ON CACHE BOOL "Build Direct3D 12 renderer" FORCE)
        message(STATUS "DirectX SDK found - Direct3D renderers enabled")
    else()
        set(LLGL_BUILD_RENDERER_DIRECT3D11 OFF CACHE BOOL "Build Direct3D 11 renderer" FORCE)
        set(LLGL_BUILD_RENDERER_DIRECT3D12 OFF CACHE BOOL "Build Direct3D 12 renderer" FORCE)
        message(WARNING "DirectX SDK not found - Direct3D renderers disabled")
    endif()
    
    set(LLGL_BUILD_RENDERER_METAL OFF CACHE BOOL "Build Metal renderer" FORCE)
else()
    set(LLGL_BUILD_RENDERER_DIRECT3D11 OFF CACHE BOOL "Build Direct3D 11 renderer" FORCE)
    set(LLGL_BUILD_RENDERER_DIRECT3D12 OFF CACHE BOOL "Build Direct3D 12 renderer" FORCE)
    
    if(APPLE)
        # Check for Metal framework on macOS
        find_library(METAL_FRAMEWORK Metal)
        if(METAL_FRAMEWORK)
            set(LLGL_BUILD_RENDERER_METAL ON CACHE BOOL "Build Metal renderer" FORCE)
            message(STATUS "Metal framework found - Metal renderer enabled")
        else()
            set(LLGL_BUILD_RENDERER_METAL OFF CACHE BOOL "Build Metal renderer" FORCE)
            message(WARNING "Metal framework not found - Metal renderer disabled")
        endif()
    else()
        set(LLGL_BUILD_RENDERER_METAL OFF CACHE BOOL "Build Metal renderer" FORCE)
    endif()
endif()

# Make LLGL available
FetchContent_MakeAvailable(LLGL)

# Define source files
set(RENDERING_PLUGIN_SOURCES
    src/RenderingPlugin.cpp
)

# Define header files
set(RENDERING_PLUGIN_HEADERS
    include/RenderingPlugin.h
    include/RenderingPluginExport.h
)

# Create library target
if(BUILD_SHARED_LIBS)
    add_library(RenderingPlugin SHARED ${RENDERING_PLUGIN_SOURCES} ${RENDERING_PLUGIN_HEADERS})
    target_compile_definitions(RenderingPlugin PRIVATE RENDERING_PLUGIN_EXPORTS)
    target_compile_definitions(RenderingPlugin PUBLIC RENDERING_PLUGIN_SHARED)
else()
    add_library(RenderingPlugin STATIC ${RENDERING_PLUGIN_SOURCES} ${RENDERING_PLUGIN_HEADERS})
endif()

# 在Debug模式下添加调试符号
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(RenderingPlugin PRIVATE -g -O0)
endif()

# Set include directories
target_include_directories(RenderingPlugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(RenderingPlugin PRIVATE 
    PluginCore
    LLGL
)

# Set output name to remove lib prefix
set_target_properties(RenderingPlugin PROPERTIES
    OUTPUT_NAME "RenderingPlugin"
    PREFIX ""
)

# Installation rules
install(TARGETS RenderingPlugin
    EXPORT RenderingPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${RENDERING_PLUGIN_HEADERS} DESTINATION include/RenderingPlugin)
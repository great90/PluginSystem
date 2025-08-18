# src/Plugins/RenderingPlugin/CMakeLists.txt

# Download and build LLGL from GitHub
include(FetchContent)
FetchContent_Declare(
    LLGL
    GIT_REPOSITORY https://github.com/LukasBanana/LLGL.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# Download and build Gauss Math library
FetchContent_Declare(
    gauss
    GIT_REPOSITORY https://github.com/LukasBanana/GaussianLib.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# Configure LLGL build
set(LLGL_BUILD_EXAMPLES ON CACHE BOOL "Build LLGL examples" FORCE)
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

# Make LLGL and Gauss available
FetchContent_MakeAvailable(LLGL gauss)

# Define abstract base class headers
set(RENDERING_PLUGIN_BASE_HEADERS
    include/RenderingPluginBase.h
    include/RenderingPlugin.h
    include/RenderingPluginExport.h
)

# Define modular component sources and headers
set(RENDERING_PLUGIN_COMPONENT_SOURCES
    src/RenderingSystem.cpp
    src/ResourceManager.cpp
    src/RenderCommands.cpp
    src/GeometryGenerator.cpp
    src/ShaderManager.cpp
)

set(RENDERING_PLUGIN_COMPONENT_HEADERS
    include/RenderingSystem.h
    include/ResourceManager.h
    include/RenderCommands.h
    include/GeometryGenerator.h
    include/ShaderManager.h
)

# Create a static library for shared components
add_library(RenderingPluginComponents STATIC 
    ${RENDERING_PLUGIN_COMPONENT_SOURCES} 
    ${RENDERING_PLUGIN_COMPONENT_HEADERS}
)

target_include_directories(RenderingPluginComponents
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<TARGET_PROPERTY:LLGL,INTERFACE_INCLUDE_DIRECTORIES>
        ${gauss_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(RenderingPluginComponents 
    PRIVATE PluginCore
    PUBLIC LLGL gausslib
)

# Set C++ standard for RenderingPluginComponents
target_compile_features(RenderingPluginComponents PRIVATE cxx_std_17)
set_target_properties(RenderingPluginComponents PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Add the LLGLRenderingPlugin subdirectory
message(STATUS "About to add LLGLRenderingPlugin subdirectory")
add_subdirectory(LLGLRenderingPlugin)
message(STATUS "Finished adding LLGLRenderingPlugin subdirectory")

# Create an interface library for the abstract base
add_library(RenderingPluginBase INTERFACE)
target_include_directories(RenderingPluginBase INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# For backward compatibility, create an alias that points to LLGLRenderingPlugin
add_library(RenderingPlugin ALIAS LLGLRenderingPlugin)

# 在Debug模式下添加调试符号
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(RenderingPluginComponents PRIVATE -g -O0)
endif()

# Installation rules
install(TARGETS LLGLRenderingPlugin
    EXPORT RenderingPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${RENDERING_PLUGIN_BASE_HEADERS} DESTINATION include/RenderingPlugin)
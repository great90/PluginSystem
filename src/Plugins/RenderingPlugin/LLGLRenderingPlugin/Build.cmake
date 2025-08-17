# LLGLRenderingPlugin Build Configuration

# Define source files
set(LLGL_RENDERING_PLUGIN_SOURCES
    # Header files
    include/Plugin.h
    include/PluginExport.h
    
    # Source files
    src/LLGLRenderingPlugin.cpp
)

# Group source files for IDE
source_group("Header Files" FILES
    include/Plugin.h
    include/PluginExport.h
)

source_group("Source Files" FILES
    src/LLGLRenderingPlugin.cpp
)
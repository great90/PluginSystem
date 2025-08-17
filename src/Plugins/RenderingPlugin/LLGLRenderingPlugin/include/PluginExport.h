#pragma once

// Platform-specific export/import macros for LLGLRenderingPlugin

#ifdef _WIN32
    #ifdef LLGL_RENDERING_PLUGIN_EXPORTS
        #define LLGL_RENDERING_PLUGIN_API __declspec(dllexport)
    #else
        #define LLGL_RENDERING_PLUGIN_API __declspec(dllimport)
    #endif
#else
    #ifdef LLGL_RENDERING_PLUGIN_EXPORTS
        #define LLGL_RENDERING_PLUGIN_API __attribute__((visibility("default")))
    #else
        #define LLGL_RENDERING_PLUGIN_API
    #endif
#endif

// C linkage for plugin entry points
#ifdef __cplusplus
extern "C" {
#endif

// Plugin factory function
LLGL_RENDERING_PLUGIN_API class IPlugin* CreatePlugin();

// Plugin information function
LLGL_RENDERING_PLUGIN_API const char* GetPluginInfo();

#ifdef __cplusplus
}
#endif
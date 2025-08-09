/**
 * @file RenderingPluginExport.h
 * @brief Export macros for the RenderingPlugin
 */

#pragma once

// Define export/import macros for Windows DLL
#if defined(_WIN32) || defined(_WIN64)
    #ifdef RENDERING_PLUGIN_SHARED
        #ifdef RENDERING_PLUGIN_EXPORTS
            #define RENDERING_PLUGIN_API __declspec(dllexport)
        #else
            #define RENDERING_PLUGIN_API __declspec(dllimport)
        #endif
    #else
        #define RENDERING_PLUGIN_API
    #endif
#else
    // For non-Windows platforms
    #if defined(RENDERING_PLUGIN_SHARED) && defined(__GNUC__) && __GNUC__ >= 4
        #define RENDERING_PLUGIN_API __attribute__((visibility("default")))
    #else
        #define RENDERING_PLUGIN_API
    #endif
#endif
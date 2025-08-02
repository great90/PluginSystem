/**
 * @file PluginExport.h
 * @brief Defines macros for exporting/importing symbols in shared libraries
 */

#pragma once

// Platform-specific export/import macros
#if defined(_WIN32) || defined(__CYGWIN__)
    #define PLUGIN_HELPER_EXPORT __declspec(dllexport)
    #define PLUGIN_HELPER_IMPORT __declspec(dllimport)
#else
    #define PLUGIN_HELPER_EXPORT __attribute__((visibility("default")))
    #define PLUGIN_HELPER_IMPORT __attribute__((visibility("default")))
#endif

// Plugin Core library export/import
#if defined(PLUGIN_CORE_SHARED)
    #if defined(PLUGIN_CORE_EXPORTS)
        #define PLUGIN_CORE_API PLUGIN_HELPER_EXPORT
    #else
        #define PLUGIN_CORE_API PLUGIN_HELPER_IMPORT
    #endif
#else
    #define PLUGIN_CORE_API
#endif

// Generic plugin API for plugins to implement
#if defined(PLUGIN_SHARED)
    #define PLUGIN_API PLUGIN_HELPER_EXPORT
#else
    #define PLUGIN_API
#endif

// Macro for registering plugins
#define REGISTER_PLUGIN(PluginClass) \
extern "C" { \
    PLUGIN_API IPlugin* CreatePlugin() { \
        return new PluginClass(); \
    } \
    PLUGIN_API const PluginInfo& GetPluginInfo() { \
        static const PluginInfo info = PluginClass::GetPluginStaticInfo(); \
        return info; \
    } \
}
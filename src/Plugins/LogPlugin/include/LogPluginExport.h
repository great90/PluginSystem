/**
 * @file LogPluginExport.h
 * @brief Defines macros for exporting/importing symbols in LogPlugin shared library
 */

#pragma once

// Include platform-specific export/import macros from PluginCore
#include "PluginExport.h"

// Log Plugin library export/import
#if defined(LOG_PLUGIN_SHARED)
    #if defined(LOG_PLUGIN_EXPORTS)
        #define LOG_PLUGIN_API PLUGIN_HELPER_EXPORT
    #else
        #define LOG_PLUGIN_API PLUGIN_HELPER_IMPORT
    #endif
#else
    #define LOG_PLUGIN_API
#endif
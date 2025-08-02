/**
 * @file ScriptPluginExport.h
 * @brief Defines macros for exporting/importing symbols in ScriptPlugin shared library
 */

#pragma once

// Include platform-specific export/import macros from PluginCore
#include "PluginExport.h"

// Script Plugin library export/import
#if defined(SCRIPT_PLUGIN_SHARED)
    #if defined(SCRIPT_PLUGIN_EXPORTS)
        #define SCRIPT_PLUGIN_API PLUGIN_HELPER_EXPORT
    #else
        #define SCRIPT_PLUGIN_API PLUGIN_HELPER_IMPORT
    #endif
#else
    #define SCRIPT_PLUGIN_API
#endif
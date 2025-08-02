/**
 * @file MathPluginExport.h
 * @brief Defines macros for exporting/importing symbols in MathPlugin shared library
 */

#pragma once

// Include platform-specific export/import macros from PluginCore
#include "PluginExport.h"

// Math Plugin library export/import
#if defined(MATH_PLUGIN_SHARED)
    #if defined(MATH_PLUGIN_EXPORTS)
        #define MATH_PLUGIN_API PLUGIN_HELPER_EXPORT
    #else
        #define MATH_PLUGIN_API PLUGIN_HELPER_IMPORT
    #endif
#else
    #define MATH_PLUGIN_API
#endif
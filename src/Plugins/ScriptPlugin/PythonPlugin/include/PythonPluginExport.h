/**
 * @file PythonPluginExport.h
 * @brief Defines macros for exporting/importing symbols in PythonPlugin shared library
 */

#pragma once

// Include platform-specific export/import macros from PluginCore
#include "PluginExport.h"

// Python Plugin library export/import
#if defined(PYTHON_PLUGIN_SHARED)
    #if defined(PYTHON_PLUGIN_EXPORTS)
        #define PYTHON_PLUGIN_API PLUGIN_HELPER_EXPORT
    #else
        #define PYTHON_PLUGIN_API PLUGIN_HELPER_IMPORT
    #endif
#else
    #define PYTHON_PLUGIN_API
#endif
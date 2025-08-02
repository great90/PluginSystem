/**
 * @file LuaPluginExport.h
 * @brief Defines macros for exporting/importing symbols in LuaPlugin shared library
 */

#pragma once

// Include platform-specific export/import macros from PluginCore
#include "PluginExport.h"

// Lua Plugin library export/import
#if defined(LUA_PLUGIN_SHARED)
    #if defined(LUA_PLUGIN_EXPORTS)
        #define LUA_PLUGIN_API PLUGIN_HELPER_EXPORT
    #else
        #define LUA_PLUGIN_API PLUGIN_HELPER_IMPORT
    #endif
#else
    #define LUA_PLUGIN_API
#endif
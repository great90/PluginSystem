#pragma once

/**
 * @file RenderingPlugin.h
 * @brief Main header for RenderingPlugin - includes the abstract base class
 * 
 * This file provides the main RenderingPlugin interface. The actual implementation
 * is now provided by specific backend plugins like LLGLRenderingPlugin.
 */

#include "RenderingPluginBase.h"

// For backward compatibility, we can typedef or alias the base class
// This ensures existing code continues to work
using RenderingPlugin = RenderingPluginBase;

// Legacy compatibility - include the LLGL implementation by default
// This can be changed to support multiple backends in the future
#include "../LLGLRenderingPlugin/include/Plugin.h"

// Factory function for creating the default rendering plugin
inline RenderingPluginBase* CreateDefaultRenderingPlugin() {
    return new LLGLRenderingPlugin();
}
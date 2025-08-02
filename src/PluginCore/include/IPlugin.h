/**
 * @file IPlugin.h
 * @brief Defines the interface for all plugins in the system
 */

#pragma once

#include <string>
#include <memory>
#include "PluginInfo.h"
#include "PluginExport.h"

// Define PLUGIN_EXTENSION if not already defined
#ifndef PLUGIN_EXTENSION
    #if defined(_WIN32) || defined(_WIN64)
        #define PLUGIN_EXTENSION ".dll"
    #elif defined(__APPLE__)
        #define PLUGIN_EXTENSION ".dylib"
    #else
        #define PLUGIN_EXTENSION ".so"
    #endif
#endif

/**
 * @class IPlugin
 * @brief Interface that all plugins must implement
 * 
 * This interface defines the lifecycle methods that plugins must implement
 * including initialization, shutdown, serialization, deserialization, and hot-reloading.
 */
class PLUGIN_CORE_API IPlugin {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~IPlugin() = default;
    
    /**
     * @brief Initialize the plugin
     * @return true if initialization was successful, false otherwise
     */
    virtual bool Initialize() = 0;
    
    /**
     * @brief Shutdown the plugin
     */
    virtual void Shutdown() = 0;
    
    /**
     * @brief Get plugin information
     * @return Reference to the plugin's information
     */
    virtual const PluginInfo& GetPluginInfo() const = 0;
    
    /**
     * @brief Serialize plugin state to a string
     * 
     * This method should serialize the plugin's current state to a string
     * representation that can be used to restore the state later.
     * 
     * @return String containing serialized plugin state
     */
    virtual std::string Serialize() = 0;
    
    /**
     * @brief Deserialize plugin state from a string
     * 
     * This method should restore the plugin's state from a previously
     * serialized string representation.
     * 
     * @param data String containing serialized plugin state
     * @return true if deserialization was successful, false otherwise
     */
    virtual bool Deserialize(const std::string& data) = 0;
    
    /**
     * @brief Prepare for hot reload
     * 
     * This method is called before a plugin is about to be hot-reloaded.
     * It should prepare the plugin for being replaced with a new version.
     * 
     * @return true if preparation was successful, false if hot-reload should be aborted
     */
    virtual bool PrepareForHotReload() = 0;
    
    /**
     * @brief Complete hot reload
     * 
     * This method is called after a plugin has been hot-reloaded.
     * It should finalize any state transitions needed after the reload.
     * 
     * @return true if completion was successful, false if there were issues
     */
    virtual bool CompleteHotReload() = 0;
};

/**
 * @typedef CreatePluginFunc
 * @brief Function pointer type for plugin creation function
 * 
 * This function is exported by plugin libraries and used to create
 * an instance of the plugin.
 */
typedef IPlugin* (*CreatePluginFunc)();

/**
 * @typedef GetPluginInfoFunc
 * @brief Function pointer type for getting plugin information
 * 
 * This function is exported by plugin libraries and used to retrieve
 * the plugin's metadata without creating an instance.
 */
typedef const PluginInfo& (*GetPluginInfoFunc)();
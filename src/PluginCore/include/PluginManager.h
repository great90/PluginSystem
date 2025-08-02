/**
 * @file PluginManager.h
 * @brief Defines the PluginManager class for managing plugin lifecycle
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "IPlugin.h"
#include "DependencyResolver.h"
#include "PluginExport.h"

/**
 * @class PluginManager
 * @brief Manages the lifecycle of plugins including loading, unloading, and hot-reloading
 * 
 * The PluginManager is responsible for loading plugin libraries, resolving dependencies,
 * initializing plugins in the correct order, and managing plugin lifecycle operations
 * such as hot-reloading.
 */
class PLUGIN_CORE_API PluginManager {
public:
    /**
     * @brief Default constructor
     */
    PluginManager();
    
    /**
     * @brief Destructor
     * 
     * Ensures all plugins are properly unloaded
     */
    ~PluginManager();
    
    /**
     * @brief Set the directory where plugins are located
     * 
     * @param directory Path to the plugins directory
     */
    void SetPluginDirectory(const std::string& directory);
    
    /**
     * @brief Get the current plugin directory
     * 
     * @return Path to the plugins directory
     */
    std::string GetPluginDirectory() const;
    
    /**
     * @brief Load a plugin from a file
     * 
     * @param pluginPath Path to the plugin file
     * @return true if the plugin was loaded successfully, false otherwise
     */
    bool LoadPlugin(const std::string& pluginPath);
    
    /**
     * @brief Load all plugins from the plugin directory
     * 
     * @return Number of plugins successfully loaded
     */
    int LoadAllPlugins();
    
    /**
     * @brief Unload a plugin by name
     * 
     * @param pluginName Name of the plugin to unload
     * @return true if the plugin was unloaded successfully, false otherwise
     */
    bool UnloadPlugin(const std::string& pluginName);
    
    /**
     * @brief Unload all plugins
     */
    void UnloadAllPlugins();
    
    /**
     * @brief Get a plugin by name
     * 
     * @param pluginName Name of the plugin to retrieve
     * @return Pointer to the plugin, or nullptr if not found
     */
    IPlugin* GetPlugin(const std::string& pluginName);
    
    /**
     * @brief Get a plugin by name with type casting
     * 
     * @tparam T Type to cast the plugin to
     * @param pluginName Name of the plugin to retrieve
     * @return Typed pointer to the plugin, or nullptr if not found or wrong type
     */
    template<typename T>
    T* GetPlugin(const std::string& pluginName) {
        IPlugin* plugin = GetPlugin(pluginName);
        return plugin ? dynamic_cast<T*>(plugin) : nullptr;
    }
    
    /**
     * @brief Get names of all loaded plugins
     * 
     * @return Vector of plugin names
     */
    std::vector<std::string> GetLoadedPluginNames() const;
    
    /**
     * @brief Check if a plugin is loaded
     * 
     * @param pluginName Name of the plugin to check
     * @return true if the plugin is loaded, false otherwise
     */
    bool IsPluginLoaded(const std::string& pluginName) const;
    
    /**
     * @brief Hot-reload a plugin
     * 
     * @param pluginName Name of the plugin to reload
     * @return true if the plugin was reloaded successfully, false otherwise
     */
    bool HotReloadPlugin(const std::string& pluginName);
    
    /**
     * @brief Resolve and initialize plugin dependencies
     * 
     * @return true if all dependencies were resolved and initialized successfully
     */
    bool ResolveDependencies();

private:
    /**
     * @struct PluginLibrary
     * @brief Internal structure to track loaded plugin libraries
     */
    struct PluginLibrary {
        void* handle;                  ///< Handle to the loaded library
        IPlugin* instance;             ///< Instance of the plugin
        CreatePluginFunc createFunc;   ///< Function to create plugin instances
        GetPluginInfoFunc infoFunc;    ///< Function to get plugin info
        std::string path;              ///< Path to the plugin file
    };
    
    /**
     * @brief Close a plugin library
     * 
     * @param library PluginLibrary to close
     */
    void CloseLibrary(PluginLibrary& library);
    
    /**
     * @brief Find plugin files in the plugin directory
     * 
     * @return Vector of paths to plugin files
     */
    std::vector<std::string> FindPluginFiles() const;
    
    std::string pluginDirectory_;  ///< Directory where plugins are located
    
    /**
     * @brief Map of plugin names to their library information
     */
    std::unordered_map<std::string, PluginLibrary> loadedPlugins_;
    
    /**
     * @brief Dependency resolver for managing plugin dependencies
     */
    DependencyResolver dependencyResolver_;
};
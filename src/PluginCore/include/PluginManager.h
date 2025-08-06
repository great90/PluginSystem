/**
 * @file PluginManager.h
 * @brief Defines the PluginManager class for managing plugin lifecycle
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>
#include <stdexcept>
#include <optional>
#include "IPlugin.h"
#include "DependencyResolver.h"
#include "PluginExport.h"

/**
 * @brief Exception thrown when plugin operations fail
 */
class PLUGIN_CORE_API PluginException : public std::runtime_error {
public:
    explicit PluginException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Result type for plugin operations
 */
template<typename T>
using PluginResult = std::optional<T>;

/**
 * @brief Callback type for plugin lifecycle events
 */
using PluginLifecycleCallback = std::function<void(const std::string& pluginName, const std::string& event)>;

/**
 * @class PluginManager
 * @brief Thread-safe manager for plugin lifecycle with smart pointer management
 * 
 * The PluginManager is responsible for loading plugin libraries, resolving dependencies,
 * initializing plugins in the correct order, and managing plugin lifecycle operations
 * such as hot-reloading. Uses RAII principles and exception-safe design.
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
     * @brief Get a plugin by name (thread-safe)
     * 
     * @param pluginName Name of the plugin to retrieve
     * @return Shared pointer to the plugin, or nullptr if not found
     */
    std::shared_ptr<IPlugin> GetPlugin(const std::string& pluginName);
    
    /**
     * @brief Get a plugin by name with type casting (thread-safe)
     * 
     * @tparam T Type to cast the plugin to
     * @param pluginName Name of the plugin to retrieve
     * @return Typed shared pointer to the plugin, or nullptr if not found or wrong type
     */
    template<typename T>
    std::shared_ptr<T> GetPlugin(const std::string& pluginName) {
        auto plugin = GetPlugin(pluginName);
        return plugin ? std::dynamic_pointer_cast<T>(plugin) : nullptr;
    }
    
    /**
     * @brief Get a weak reference to a plugin (for script layer integration)
     * 
     * @param pluginName Name of the plugin to retrieve
     * @return Weak pointer to the plugin
     */
    std::weak_ptr<IPlugin> GetPluginWeak(const std::string& pluginName);
    
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
    
    /**
     * @brief Register a callback for plugin lifecycle events
     * 
     * @param callback Function to call on plugin events
     */
    void RegisterLifecycleCallback(const PluginLifecycleCallback& callback);
    
    /**
     * @brief Get the last error message
     * 
     * @return Last error message, or empty string if no error
     */
    std::string GetLastError() const;
    
    /**
     * @brief Enable or disable detailed logging
     * 
     * @param enabled Whether to enable logging
     */
    void SetLoggingEnabled(bool enabled);
    
    /**
     * @brief Get plugin load order based on dependencies
     * 
     * @return Vector of plugin names in load order
     */
    PluginResult<std::vector<std::string>> GetLoadOrder() const;

private:
    /**
     * @struct PluginLibrary
     * @brief Internal structure to track loaded plugin libraries with RAII
     */
    struct PluginLibrary {
        void* handle;                           ///< Handle to the loaded library
        std::shared_ptr<IPlugin> instance;     ///< Shared instance of the plugin
        CreatePluginFunc createFunc;           ///< Function to create plugin instances
        GetPluginInfoFunc infoFunc;            ///< Function to get plugin info
        std::string path;                      ///< Path to the plugin file
        
        // RAII destructor
        ~PluginLibrary();
        
        // Move semantics for exception safety
        PluginLibrary() = default;
        PluginLibrary(const PluginLibrary&) = delete;
        PluginLibrary& operator=(const PluginLibrary&) = delete;
        PluginLibrary(PluginLibrary&& other) noexcept;
        PluginLibrary& operator=(PluginLibrary&& other) noexcept;
    };
    
    /**
     * @brief Close a plugin library (exception-safe)
     * 
     * @param library PluginLibrary to close
     */
    void CloseLibrary(PluginLibrary& library) noexcept;
    
    /**
     * @brief Find plugin files in the plugin directory
     * 
     * @return Vector of paths to plugin files
     */
    std::vector<std::string> FindPluginFiles() const;
    
    /**
     * @brief Log a message if logging is enabled
     * 
     * @param level Log level (INFO, WARNING, ERROR)
     * @param message Message to log
     */
    void LogMessage(const std::string& level, const std::string& message) const;
    
    /**
     * @brief Notify lifecycle callbacks
     * 
     * @param pluginName Name of the plugin
     * @param event Event type
     */
    void NotifyLifecycleCallbacks(const std::string& pluginName, const std::string& event) const;
    
    /**
     * @brief Set the last error message (thread-safe)
     * 
     * @param error Error message
     */
    void SetLastError(const std::string& error) const;
    
    // Thread-safe member variables
    mutable std::mutex mutex_;                                      ///< Mutex for thread safety
    std::string pluginDirectory_;                                   ///< Directory where plugins are located
    std::unordered_map<std::string, PluginLibrary> loadedPlugins_; ///< Map of plugin names to their library information
    DependencyResolver dependencyResolver_;                        ///< Dependency resolver for managing plugin dependencies
    
    // Error handling and logging
    mutable std::string lastError_;                                 ///< Last error message
    bool loggingEnabled_;                                           ///< Whether logging is enabled
    std::vector<PluginLifecycleCallback> lifecycleCallbacks_;      ///< Registered lifecycle callbacks
};
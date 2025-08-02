/**
 * @file Application.h
 * @brief Application class for the plugin system demo
 */

#pragma once

#include <string>
#include <memory>
#include <vector>

// Forward declarations
class PluginManager;
class LogPlugin;
class MathPlugin;
class ScriptPlugin;

/**
 * @class Application
 * @brief Main application class for the plugin system demo
 */
class Application {
public:
    /**
     * @brief Constructor
     * @param pluginDir Directory containing plugins
     */
    Application(const std::string& pluginDir = "./plugins");
    
    /**
     * @brief Destructor
     */
    ~Application();
    
    /**
     * @brief Initialize the application
     * @return True if initialization was successful, false otherwise
     */
    bool Initialize();
    
    /**
     * @brief Run the application
     * @return Exit code
     */
    int Run();
    
    /**
     * @brief Shutdown the application
     */
    void Shutdown();

private:
    // Helper methods
    void PrintPluginInfo(const class PluginInfo& info);
    void DemonstrateMathPlugin();
    void DemonstrateLogPlugin();
    void DemonstrateScriptPlugins();
    
    // Member variables
    std::string m_pluginDir;
    std::unique_ptr<PluginManager> m_pluginManager;
    
    // Plugin instances (non-owning pointers)
    MathPlugin* m_mathPlugin;
    LogPlugin* m_logPlugin;
    ScriptPlugin* m_pythonPlugin;
    ScriptPlugin* m_luaPlugin;
    
    // Loaded plugin names
    std::vector<std::string> m_loadedPlugins;
};
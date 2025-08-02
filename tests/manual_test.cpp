/**
 * @file manual_test.cpp
 * @brief Manual test for the plugin system
 */

#include "PluginManager.h"
#include "LogPlugin.h"
#include "MathPlugin.h"
#include "ScriptPlugin.h"
#include "PythonPlugin.h"
#include "LuaPlugin.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <chrono>
#include <thread>

// Helper function to print plugin information
void PrintPluginInfo(const PluginInfo& info) {
    std::cout << "Plugin: " << info.displayName << " (" << info.name << ")" << std::endl;
    std::cout << "  Version: " << info.version.major << "." << info.version.minor << "." << info.version.patch << std::endl;
    std::cout << "  Description: " << info.description << std::endl;
    std::cout << "  Author: " << info.author << std::endl;
    
    if (!info.dependencies.empty()) {
        std::cout << "  Dependencies:" << std::endl;
        for (const auto& dep : info.dependencies) {
            std::cout << "    - " << dep.name << " (>= " << dep.minVersion.major << "." 
                      << dep.minVersion.minor << "." << dep.minVersion.patch << ")";
            if (dep.optional) {
                std::cout << " (optional)";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

// Test hot reloading functionality
void TestHotReloading(PluginManager& pluginManager, const std::string& pluginName) {
    std::cout << "\n=== Testing Hot Reloading for " << pluginName << " ===" << std::endl;
    
    // Get the plugin before hot reload
    IPlugin* plugin = pluginManager.GetPlugin(pluginName);
    if (!plugin) {
        std::cerr << "Plugin not found: " << pluginName << std::endl;
        return;
    }
    
    PluginInfo infoBefore = plugin->GetPluginInfo();
    std::cout << "Plugin info before hot reload:" << std::endl;
    PrintPluginInfo(infoBefore);
    
    // Perform hot reload
    std::cout << "Performing hot reload..." << std::endl;
    if (pluginManager.HotReloadPlugin(pluginName)) {
        std::cout << "Hot reload successful" << std::endl;
        
        // Get the plugin after hot reload
        plugin = pluginManager.GetPlugin(pluginName);
        if (plugin) {
            PluginInfo infoAfter = plugin->GetPluginInfo();
            std::cout << "Plugin info after hot reload:" << std::endl;
            PrintPluginInfo(infoAfter);
        } else {
            std::cerr << "Failed to get plugin after hot reload" << std::endl;
        }
    } else {
        std::cerr << "Hot reload failed" << std::endl;
    }
}

// Test dependency resolution
void TestDependencyResolution(PluginManager& pluginManager) {
    std::cout << "\n=== Testing Dependency Resolution ===" << std::endl;
    
    // Get all loaded plugins
    std::vector<std::string> loadedPlugins = pluginManager.GetLoadedPluginNames();
    
    // Print dependency graph
    std::cout << "Dependency graph:" << std::endl;
    for (const auto& name : loadedPlugins) {
        IPlugin* plugin = pluginManager.GetPlugin(name);
        if (plugin) {
            PluginInfo info = plugin->GetPluginInfo();
            std::cout << info.name << ":" << std::endl;
            
            if (info.dependencies.empty()) {
                std::cout << "  No dependencies" << std::endl;
            } else {
                for (const auto& dep : info.dependencies) {
                    std::cout << "  -> " << dep.name;
                    if (pluginManager.IsPluginLoaded(dep.name)) {
                        std::cout << " (Loaded)";
                    } else {
                        std::cout << " (Not loaded)";
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    
    // Test unloading and reloading with dependencies
    std::cout << "\nTesting unload and reload with dependencies:" << std::endl;
    
    // Try to unload a plugin that others depend on
    std::string basePlugin = "MathPlugin";
    std::cout << "Attempting to unload " << basePlugin << " which others may depend on..." << std::endl;
    
    if (pluginManager.UnloadPlugin(basePlugin)) {
        std::cout << "Unloaded " << basePlugin << std::endl;
        
        // Check if dependent plugins were also unloaded
        std::cout << "Checking dependent plugins:" << std::endl;
        for (const auto& name : loadedPlugins) {
            if (name != basePlugin) {
                bool isLoaded = pluginManager.IsPluginLoaded(name);
                std::cout << "  " << name << ": " << (isLoaded ? "Still loaded" : "Unloaded") << std::endl;
            }
        }
        
        // Reload the plugin
        std::cout << "Reloading " << basePlugin << "..." << std::endl;
        if (pluginManager.LoadPlugin(basePlugin)) {
            std::cout << "Reloaded " << basePlugin << std::endl;
            
            // Resolve dependencies to reload dependent plugins
            std::cout << "Resolving dependencies..." << std::endl;
            if (pluginManager.ResolveDependencies()) {
                std::cout << "Dependencies resolved" << std::endl;
                
                // Check if dependent plugins were reloaded
                std::cout << "Checking dependent plugins:" << std::endl;
                for (const auto& name : loadedPlugins) {
                    bool isLoaded = pluginManager.IsPluginLoaded(name);
                    std::cout << "  " << name << ": " << (isLoaded ? "Loaded" : "Not loaded") << std::endl;
                }
            } else {
                std::cerr << "Failed to resolve dependencies" << std::endl;
            }
        } else {
            std::cerr << "Failed to reload " << basePlugin << std::endl;
        }
    } else {
        std::cout << "Could not unload " << basePlugin << " (possibly due to dependencies)" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== Plugin System Manual Test ===" << std::endl;
    
    // Create the plugin manager
    PluginManager pluginManager;
    
    // Set the plugin directory
    std::string pluginDir = "./plugins";
    if (argc > 1) {
        pluginDir = argv[1];
    }
    
    std::cout << "Using plugin directory: " << pluginDir << std::endl;
    pluginManager.SetPluginDirectory(pluginDir);
    
    // Create plugin directory if it doesn't exist
    if (!std::filesystem::exists(pluginDir)) {
        std::filesystem::create_directories(pluginDir);
        std::cout << "Created plugin directory" << std::endl;
    }
    
    // Load all plugins
    std::cout << "\nLoading plugins..." << std::endl;
    if (!pluginManager.LoadAllPlugins()) {
        std::cerr << "Failed to load all plugins" << std::endl;
    }
    
    // Get loaded plugin names
    std::vector<std::string> loadedPlugins = pluginManager.GetLoadedPluginNames();
    std::cout << "Loaded " << loadedPlugins.size() << " plugins:" << std::endl;
    for (const auto& name : loadedPlugins) {
        IPlugin* plugin = pluginManager.GetPlugin(name);
        if (plugin) {
            PrintPluginInfo(plugin->GetPluginInfo());
        }
    }
    
    // Test hot reloading
    if (!loadedPlugins.empty()) {
        TestHotReloading(pluginManager, loadedPlugins[0]);
    }
    
    // Test dependency resolution
    TestDependencyResolution(pluginManager);
    
    // Unload all plugins
    std::cout << "\nUnloading plugins..." << std::endl;
    pluginManager.UnloadAllPlugins();
    
    std::cout << "\nManual test completed successfully" << std::endl;
    return 0;
}
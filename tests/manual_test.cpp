/**
 * @file manual_test.cpp
 * @brief Manual test program for the Plugin System
 */

#include <iostream>
#include <string>
#include <vector>
#include "PluginManager.h"
#include "IPlugin.h"

// Helper function to print plugin information
void PrintPluginInfo(const PluginInfo& info) {
    std::cout << "Plugin Information:" << std::endl;
    std::cout << "  Name: " << info.name << std::endl;
    std::cout << "  Display Name: " << info.displayName << std::endl;
    std::cout << "  Description: " << info.description << std::endl;
    std::cout << "  Author: " << info.author << std::endl;
    std::cout << "  Version: " << info.version.ToString() << std::endl;
    std::cout << "  Dependencies: ";
    
    if (info.dependencies.empty()) {
        std::cout << "None" << std::endl;
    } else {
        std::cout << std::endl;
        for (const auto& dep : info.dependencies) {
            std::cout << "    - " << dep.name << " (v" << dep.minVersion.ToString() << ")";
            if (dep.optional) {
                std::cout << " [Optional]";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

// Helper function to print a list of loaded plugins
void PrintLoadedPlugins(PluginManager& pluginManager) {
    std::vector<std::string> pluginNames = pluginManager.GetLoadedPluginNames();
    
    std::cout << "Loaded Plugins (" << pluginNames.size() << " total):" << std::endl;
    for (const auto& name : pluginNames) {
        IPlugin* plugin = pluginManager.GetPlugin(name);
        if (plugin) {
            std::cout << "  - " << name << " (" << plugin->GetPluginInfo().displayName << ")" << std::endl;
        } else {
            std::cout << "  - " << name << " (Error: Plugin pointer is null)" << std::endl;
        }
    }
    std::cout << std::endl;
}

// Helper function to test the MathPlugin
void TestMathPlugin(PluginManager& pluginManager) {
    std::cout << "Testing MathPlugin..." << std::endl;
    
    // Get the MathPlugin
    IPlugin* plugin = pluginManager.GetPlugin("MathPlugin");
    if (!plugin) {
        std::cout << "Error: MathPlugin not found!" << std::endl;
        return;
    }
    
    // Print plugin info
    PrintPluginInfo(plugin->GetPluginInfo());
    
    // We would need to cast to MathPlugin* to use its specific methods
    // This is just a demonstration of how to access a plugin
    std::cout << "MathPlugin successfully loaded and accessed." << std::endl;
    std::cout << std::endl;
}

// Helper function to test the LogPlugin
void TestLogPlugin(PluginManager& pluginManager) {
    std::cout << "Testing LogPlugin..." << std::endl;
    
    // Get the LogPlugin
    IPlugin* plugin = pluginManager.GetPlugin("LogPlugin");
    if (!plugin) {
        std::cout << "Error: LogPlugin not found!" << std::endl;
        return;
    }
    
    // Print plugin info
    PrintPluginInfo(plugin->GetPluginInfo());
    
    // We would need to cast to LogPlugin* to use its specific methods
    // This is just a demonstration of how to access a plugin
    std::cout << "LogPlugin successfully loaded and accessed." << std::endl;
    std::cout << std::endl;
}

// Helper function to test the ScriptPlugin
void TestScriptPlugin(PluginManager& pluginManager) {
    std::cout << "Testing ScriptPlugin..." << std::endl;
    
    // Get the ScriptPlugin
    IPlugin* plugin = pluginManager.GetPlugin("ScriptPlugin");
    if (!plugin) {
        std::cout << "Error: ScriptPlugin not found!" << std::endl;
        return;
    }
    
    // Print plugin info
    PrintPluginInfo(plugin->GetPluginInfo());
    
    // We would need to cast to ScriptPlugin* to use its specific methods
    // This is just a demonstration of how to access a plugin
    std::cout << "ScriptPlugin successfully loaded and accessed." << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Plugin System Manual Test ===" << std::endl;
    std::cout << std::endl;
    
    // Create a plugin manager
    PluginManager pluginManager;
    
    // Set the plugin directory
    pluginManager.SetPluginDirectory("./plugins");
    std::cout << "Plugin directory set to: " << pluginManager.GetPluginDirectory() << std::endl;
    std::cout << std::endl;
    
    // Load all plugins
    std::cout << "Loading all plugins..." << std::endl;
    int loadedCount = pluginManager.LoadAllPlugins();
    std::cout << "Loaded " << loadedCount << " plugins." << std::endl;
    std::cout << std::endl;
    
    // Print loaded plugins
    PrintLoadedPlugins(pluginManager);
    
    // Test individual plugins
    TestMathPlugin(pluginManager);
    TestLogPlugin(pluginManager);
    TestScriptPlugin(pluginManager);
    
    // Unload all plugins
    std::cout << "Unloading all plugins..." << std::endl;
    pluginManager.UnloadAllPlugins();
    std::cout << "All plugins unloaded." << std::endl;
    
    std::cout << "\nManual test completed successfully." << std::endl;
    return 0;
}
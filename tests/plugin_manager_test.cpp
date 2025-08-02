/**
 * @file plugin_manager_test.cpp
 * @brief Unit tests for the PluginManager class
 */

#include <gtest/gtest.h>
#include "PluginManager.h"
#include "IPlugin.h"
#include "MathPlugin.h"
#include "LogPlugin.h"
#include <string>
#include <vector>
#include <algorithm>

// Test fixture for PluginManager tests
class PluginManagerTest : public ::testing::Test {
protected:
    PluginManager pluginManager;
    
    void SetUp() override {
        // Set the plugin directory to the plugins directory in the build output
        pluginManager.SetPluginDirectory("plugins");
    }
    
    void TearDown() override {
        // Unload all plugins to clean up
        pluginManager.UnloadAllPlugins();
    }
};

// Test setting and getting the plugin directory
TEST_F(PluginManagerTest, PluginDirectoryTest) {
    // Set a new plugin directory
    pluginManager.SetPluginDirectory("test_plugins");
    
    // Verify the directory was set correctly
    EXPECT_EQ("test_plugins", pluginManager.GetPluginDirectory());
}

// Test loading a plugin
TEST_F(PluginManagerTest, LoadPluginTest) {
    // Load the MathPlugin
    bool result = pluginManager.LoadPlugin("plugins/MathPlugin" + std::string(PLUGIN_EXTENSION));
    
    // Verify the plugin was loaded successfully
    EXPECT_TRUE(result);
    EXPECT_TRUE(pluginManager.IsPluginLoaded("MathPlugin"));
    
    // Get the plugin and verify it's not null
    IPlugin* plugin = pluginManager.GetPlugin("MathPlugin");
    EXPECT_NE(nullptr, plugin);
    
    // Verify plugin info
    const PluginInfo& info = plugin->GetPluginInfo();
    EXPECT_EQ("MathPlugin", info.name);
    EXPECT_EQ("Math Utilities Plugin", info.displayName);
}

// Test loading all plugins
TEST_F(PluginManagerTest, LoadAllPluginsTest) {
    // Load all plugins
    int count = pluginManager.LoadAllPlugins();
    
    // Verify plugins were loaded
    EXPECT_GT(count, 0);
    
    // Get the names of loaded plugins
    std::vector<std::string> pluginNames = pluginManager.GetLoadedPluginNames();
    
    // Verify expected plugins are loaded
    EXPECT_TRUE(std::find(pluginNames.begin(), pluginNames.end(), "MathPlugin") != pluginNames.end());
    EXPECT_TRUE(std::find(pluginNames.begin(), pluginNames.end(), "LogPlugin") != pluginNames.end());
    EXPECT_TRUE(std::find(pluginNames.begin(), pluginNames.end(), "ScriptPlugin") != pluginNames.end());
}

// Test unloading a plugin
TEST_F(PluginManagerTest, UnloadPluginTest) {
    // First load a plugin
    pluginManager.LoadPlugin("plugins/MathPlugin" + std::string(PLUGIN_EXTENSION));
    EXPECT_TRUE(pluginManager.IsPluginLoaded("MathPlugin"));
    
    // Now unload it
    bool result = pluginManager.UnloadPlugin("MathPlugin");
    
    // Verify the plugin was unloaded
    EXPECT_TRUE(result);
    EXPECT_FALSE(pluginManager.IsPluginLoaded("MathPlugin"));
}

// Test unloading all plugins
TEST_F(PluginManagerTest, UnloadAllPluginsTest) {
    // First load all plugins
    pluginManager.LoadAllPlugins();
    EXPECT_GT(pluginManager.GetLoadedPluginNames().size(), 0);
    
    // Now unload all
    pluginManager.UnloadAllPlugins();
    
    // Verify all plugins were unloaded
    EXPECT_EQ(0, pluginManager.GetLoadedPluginNames().size());
}

// Test getting a plugin with type casting
TEST_F(PluginManagerTest, GetPluginWithCastTest) {
    // Load the MathPlugin
    pluginManager.LoadPlugin("plugins/MathPlugin" + std::string(PLUGIN_EXTENSION));
    
    // Try to get it with the wrong type (should return nullptr)
    auto* wrongType = pluginManager.GetPlugin<LogPlugin>("MathPlugin");
    EXPECT_EQ(nullptr, wrongType);
    
    // Get a plugin that doesn't exist
    auto* nonExistent = pluginManager.GetPlugin<IPlugin>("NonExistentPlugin");
    EXPECT_EQ(nullptr, nonExistent);
}
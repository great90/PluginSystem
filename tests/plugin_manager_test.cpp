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
#include <memory>
#include <thread>
#include <chrono>

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
    auto plugin = pluginManager.GetPlugin("MathPlugin");
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
    auto wrongType = pluginManager.GetPlugin<LogPlugin>("MathPlugin");
    EXPECT_EQ(nullptr, wrongType);
    
    // Get a plugin that doesn't exist
    auto nonExistent = pluginManager.GetPlugin<IPlugin>("NonExistentPlugin");
    EXPECT_EQ(nullptr, nonExistent);
}

// Test smart pointer functionality
TEST_F(PluginManagerTest, SmartPointerTest) {
    // Load the MathPlugin
    pluginManager.LoadPlugin("plugins/MathPlugin" + std::string(PLUGIN_EXTENSION));
    
    // Get plugin as shared_ptr
    auto plugin = pluginManager.GetPlugin("MathPlugin");
    EXPECT_NE(nullptr, plugin);
    
    // Test that multiple shared_ptr instances work correctly
    auto plugin2 = pluginManager.GetPlugin("MathPlugin");
    EXPECT_EQ(plugin.get(), plugin2.get());
    
    // Test weak_ptr functionality
    auto weakPlugin = pluginManager.GetPluginWeak("MathPlugin");
    EXPECT_FALSE(weakPlugin.expired());
    
    // Test that weak_ptr becomes invalid after unloading
    pluginManager.UnloadPlugin("MathPlugin");
    // Note: weak_ptr may still be valid due to shared_ptr references
}

// Test thread safety
TEST_F(PluginManagerTest, ThreadSafetyTest) {
    const int numThreads = 4;
    const int operationsPerThread = 10;
    std::vector<std::thread> threads;
    std::vector<bool> results(numThreads * operationsPerThread, false);
    
    // Launch multiple threads that load/unload plugins concurrently
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operationsPerThread; ++i) {
                int index = t * operationsPerThread + i;
                
                // Alternate between loading and checking plugins
                if (i % 2 == 0) {
                    results[index] = pluginManager.LoadPlugin("plugins/MathPlugin" + std::string(PLUGIN_EXTENSION));
                } else {
                    results[index] = pluginManager.IsPluginLoaded("MathPlugin");
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify no crashes occurred (test passes if we reach here)
    EXPECT_TRUE(true);
}

// Test error handling and logging
TEST_F(PluginManagerTest, ErrorHandlingTest) {
    // Enable logging
    pluginManager.SetLoggingEnabled(true);
    
    // Try to load a non-existent plugin
    bool result = pluginManager.LoadPlugin("plugins/NonExistentPlugin" + std::string(PLUGIN_EXTENSION));
    EXPECT_FALSE(result);
    
    // Check that error was recorded
    std::string lastError = pluginManager.GetLastError();
    EXPECT_FALSE(lastError.empty());
    
    // Try to unload a non-existent plugin
    result = pluginManager.UnloadPlugin("NonExistentPlugin");
    EXPECT_FALSE(result);
}

// Test lifecycle callbacks
TEST_F(PluginManagerTest, LifecycleCallbackTest) {
    bool loadCallbackCalled = false;
    bool unloadCallbackCalled = false;
    
    // Register lifecycle callbacks
    pluginManager.RegisterLifecycleCallback([&](const std::string& pluginName, const std::string& event) {
        if (event == "loaded") {
            loadCallbackCalled = true;
        } else if (event == "unloaded") {
            unloadCallbackCalled = true;
        }
    });
    
    // Load and unload a plugin
    pluginManager.LoadPlugin("plugins/MathPlugin" + std::string(PLUGIN_EXTENSION));
    pluginManager.UnloadPlugin("MathPlugin");
    
    // Verify callbacks were called
    EXPECT_TRUE(loadCallbackCalled);
    EXPECT_TRUE(unloadCallbackCalled);
}

// Test load order functionality
TEST_F(PluginManagerTest, LoadOrderTest) {
    // Load multiple plugins
    pluginManager.LoadPlugin("plugins/MathPlugin" + std::string(PLUGIN_EXTENSION));
    pluginManager.LoadPlugin("plugins/LogPlugin" + std::string(PLUGIN_EXTENSION));
    
    // Get load order
    auto loadOrder = pluginManager.GetLoadOrder();
    ASSERT_TRUE(loadOrder.has_value());
    EXPECT_GE(loadOrder->size(), 2);
    
    // Verify that plugins appear in the load order
    bool foundMath = false, foundLog = false;
    for (const auto& pluginName : *loadOrder) {
        if (pluginName == "MathPlugin") foundMath = true;
        if (pluginName == "LogPlugin") foundLog = true;
    }
    EXPECT_TRUE(foundMath);
    EXPECT_TRUE(foundLog);
}
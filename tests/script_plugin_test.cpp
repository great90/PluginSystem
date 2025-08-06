/**
 * @file script_plugin_test.cpp
 * @brief Unit tests for the ScriptPlugin, PythonPlugin, and LuaPlugin classes
 */

#include <gtest/gtest.h>
#include "PluginManager.h"
#include "ScriptPlugin.h"
#include "PythonPlugin.h"
#include "LuaPlugin.h"
#include <string>
#include <filesystem>

// Define plugin extension based on platform
#ifdef _WIN32
#define PLUGIN_EXTENSION ".dll"
#elif defined(__APPLE__)
#define PLUGIN_EXTENSION ".dylib"
#else
#define PLUGIN_EXTENSION ".so"
#endif

// Test fixture for ScriptPlugin tests
class ScriptPluginTest : public ::testing::Test {
protected:
    PluginManager pluginManager;
    std::shared_ptr<ScriptPlugin> scriptPlugin;
    std::shared_ptr<PythonPlugin> pythonPlugin;
    std::shared_ptr<LuaPlugin> luaPlugin;
    std::string pythonExamplePath;
    std::string luaExamplePath;
    
    void SetUp() override {
        // Set the plugin directory and load the ScriptPlugin
        pluginManager.SetPluginDirectory("plugins");
        pluginManager.LoadPlugin("plugins/ScriptPlugin" + std::string(PLUGIN_EXTENSION));
        
        // Get the ScriptPlugin instance
        scriptPlugin = pluginManager.GetPlugin<ScriptPlugin>("ScriptPlugin");
        ASSERT_NE(nullptr, scriptPlugin);
        
        // Get the PythonPlugin and LuaPlugin instances
        pythonPlugin = pluginManager.GetPlugin<PythonPlugin>("PythonPlugin");
        luaPlugin = pluginManager.GetPlugin<LuaPlugin>("LuaPlugin");
        
        // Set paths to example scripts
        pythonExamplePath = "examples/python_example.py";
        luaExamplePath = "examples/lua_example.lua";
        
        // Verify example scripts exist
        ASSERT_TRUE(std::filesystem::exists(pythonExamplePath));
        ASSERT_TRUE(std::filesystem::exists(luaExamplePath));
    }
    
    void TearDown() override {
        // Unload all plugins to clean up
        pluginManager.UnloadAllPlugins();
        scriptPlugin.reset();
        pythonPlugin.reset();
        luaPlugin.reset();
    }
};

// Test the ScriptPlugin info
TEST_F(ScriptPluginTest, ScriptPluginInfoTest) {
    const PluginInfo& info = scriptPlugin->GetPluginInfo();
    
    EXPECT_EQ("ScriptPlugin", info.name);
    EXPECT_EQ("Scripting Plugin", info.displayName);
    EXPECT_EQ("Provides scripting capabilities", info.description);
    EXPECT_EQ("Plugin System Team", info.author);
    
    // Check version
    EXPECT_EQ(1, info.version.major);
    EXPECT_EQ(0, info.version.minor);
    EXPECT_EQ(0, info.version.patch);
    EXPECT_EQ("1.0.0", info.version.ToString());
}

// Test the PythonPlugin
TEST_F(ScriptPluginTest, PythonPluginTest) {
    // Skip if PythonPlugin is not available
    if (!pythonPlugin) {
        GTEST_SKIP() << "PythonPlugin not available";
    }
    
    // Test plugin info
    const PluginInfo& info = pythonPlugin->GetPluginInfo();
    EXPECT_EQ("PythonPlugin", info.name);
    
    // Test language info
    EXPECT_EQ("Python", pythonPlugin->GetLanguageName());
    EXPECT_FALSE(pythonPlugin->GetLanguageVersion().empty());
    
    // Test file extensions
    auto extensions = pythonPlugin->GetSupportedExtensions();
    EXPECT_TRUE(std::find(extensions.begin(), extensions.end(), ".py") != extensions.end());
    
    // Test executing a Python script file
    bool result = pythonPlugin->ExecuteFile(pythonExamplePath);
    EXPECT_TRUE(result);
    
    // Test executing a Python script string
    result = pythonPlugin->ExecuteString("print('Hello from Python!')");
    EXPECT_TRUE(result);
    
    // Test evaluating a Python expression
    std::string evalResult;
    result = pythonPlugin->EvaluateExpression("2 + 3", evalResult);
    EXPECT_TRUE(result);
    EXPECT_EQ("5", evalResult);
    
    // Test registering and calling a C++ function from Python
    // For testing purposes, we'll just execute a string that defines a function
    result = pythonPlugin->ExecuteString("def add_numbers(a, b): return a + b");
    EXPECT_TRUE(result);
    
    result = pythonPlugin->ExecuteString("result = add_numbers(5, 7)");
    EXPECT_TRUE(result);
    
    result = pythonPlugin->EvaluateExpression("result", evalResult);
    EXPECT_TRUE(result);
    EXPECT_EQ("12", evalResult);
    
    // Test calling a Python function from C++
    // First define a function in Python
    result = pythonPlugin->ExecuteString("def multiply(a, b): return a * b");
    EXPECT_TRUE(result);
    
    // Now call it from C++
    std::string multiplyResultStr;
    // First prepare the arguments
    result = pythonPlugin->ExecuteString("result = multiply(6, 7)");
    EXPECT_TRUE(result);
    // Then get the result
    result = pythonPlugin->EvaluateExpression("result", multiplyResultStr);
    int multiplyResult = std::stoi(multiplyResultStr);
    EXPECT_TRUE(result);
    EXPECT_EQ(42, multiplyResult);
}

// Test the LuaPlugin
TEST_F(ScriptPluginTest, LuaPluginTest) {
    // Skip if LuaPlugin is not available
    if (!luaPlugin) {
        GTEST_SKIP() << "LuaPlugin not available";
    }
    
    // Test plugin info
    const PluginInfo& info = luaPlugin->GetPluginInfo();
    EXPECT_EQ("LuaPlugin", info.name);
    
    // Test language info
    EXPECT_EQ("Lua", luaPlugin->GetLanguageName());
    EXPECT_FALSE(luaPlugin->GetLanguageVersion().empty());
    
    // Test file extensions
    auto extensions = luaPlugin->GetSupportedExtensions();
    EXPECT_TRUE(std::find(extensions.begin(), extensions.end(), ".lua") != extensions.end());
    
    // Test executing a Lua script file
    bool result = luaPlugin->ExecuteFile(luaExamplePath);
    EXPECT_TRUE(result);
    
    // Test executing a Lua script string
    result = luaPlugin->ExecuteString("print('Hello from Lua!')");
    EXPECT_TRUE(result);
    
    // Test evaluating a Lua expression
    std::string evalResult;
    result = luaPlugin->EvaluateExpression("2 + 3", evalResult);
    EXPECT_TRUE(result);
    EXPECT_EQ("5", evalResult);
    
    // Test registering and calling a C++ function from Lua
    // For testing purposes, we'll just execute a string that defines a function
    result = luaPlugin->ExecuteString("function subtract_numbers(a, b) return a - b end");
    EXPECT_TRUE(result);
    
    result = luaPlugin->ExecuteString("result = subtract_numbers(10, 4)");
    EXPECT_TRUE(result);
    
    result = luaPlugin->EvaluateExpression("result", evalResult);
    EXPECT_TRUE(result);
    EXPECT_EQ("6", evalResult);
    
    // Test calling a Lua function from C++
    // First define a function in Lua
    result = luaPlugin->ExecuteString("function divide(a, b) return a / b end");
    EXPECT_TRUE(result);
    
    // Now call it from C++
    // First push the arguments to the Lua stack
    result = luaPlugin->ExecuteString("result = divide(10.0, 2.0)");
    EXPECT_TRUE(result);
    // Then get the result
    std::string divideResultStr;
    result = luaPlugin->EvaluateExpression("result", divideResultStr);
    double divideResult = std::stod(divideResultStr);
    EXPECT_TRUE(result);
    EXPECT_DOUBLE_EQ(5.0, divideResult);
}

// Test interoperability between Python and Lua
TEST_F(ScriptPluginTest, ScriptInteropTest) {
    // Skip if either plugin is not available
    if (!pythonPlugin || !luaPlugin) {
        GTEST_SKIP() << "Both PythonPlugin and LuaPlugin are required for this test";
    }
    
    // Define a value in Python
    bool result = pythonPlugin->ExecuteString("interop_value = 42");
    EXPECT_TRUE(result);
    
    // Get the value from Python
    std::string pythonValueStr;
    result = pythonPlugin->EvaluateExpression("interop_value", pythonValueStr);
    EXPECT_TRUE(result);
    int pythonValue = std::stoi(pythonValueStr);
    EXPECT_EQ(42, pythonValue);
    
    // Pass the value to Lua
    result = luaPlugin->ExecuteString("interop_value = " + std::to_string(pythonValue));
    EXPECT_TRUE(result);
    
    // Modify the value in Lua
    result = luaPlugin->ExecuteString("interop_value = interop_value * 2");
    EXPECT_TRUE(result);
    
    // Get the value from Lua
    std::string luaValueStr;
    result = luaPlugin->EvaluateExpression("interop_value", luaValueStr);
    EXPECT_TRUE(result);
    int luaValue = std::stoi(luaValueStr);
    EXPECT_EQ(84, luaValue);
    
    // Pass the value back to Python
    result = pythonPlugin->ExecuteString("interop_value = " + std::to_string(luaValue));
    EXPECT_TRUE(result);
    
    // Verify the value in Python
    result = pythonPlugin->EvaluateExpression("interop_value", pythonValueStr);
    EXPECT_TRUE(result);
    pythonValue = std::stoi(pythonValueStr);
    EXPECT_EQ(84, pythonValue);
}
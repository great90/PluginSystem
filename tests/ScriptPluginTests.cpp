/**
 * @file ScriptPluginTests.cpp
 * @brief Unit tests for the ScriptPlugin, PythonPlugin, and LuaPlugin
 */

#include <gtest/gtest.h>
#include "PluginManager.h"
#include "ScriptPlugin.h"
#include "PythonPlugin.h"
#include "LuaPlugin.h"
#include <filesystem>
#include <fstream>
#include <string>

class ScriptPluginTest : public ::testing::Test {
protected:
    PluginManager pluginManager;
    ScriptPlugin* scriptPlugin;
    PythonPlugin* pythonPlugin;
    LuaPlugin* luaPlugin;
    std::string pythonTestScript;
    std::string luaTestScript;

    void SetUp() override {
        // Set plugin directory
        pluginManager.SetPluginDirectory("./plugins");
        
        // Load plugins
        ASSERT_TRUE(pluginManager.LoadPlugin("ScriptPlugin"));
        ASSERT_TRUE(pluginManager.LoadPlugin("PythonPlugin"));
        ASSERT_TRUE(pluginManager.LoadPlugin("LuaPlugin"));
        
        // Get plugin instances
        IPlugin* scriptPluginBase = pluginManager.GetPlugin("ScriptPlugin");
        IPlugin* pythonPluginBase = pluginManager.GetPlugin("PythonPlugin");
        IPlugin* luaPluginBase = pluginManager.GetPlugin("LuaPlugin");
        
        ASSERT_NE(scriptPluginBase, nullptr);
        ASSERT_NE(pythonPluginBase, nullptr);
        ASSERT_NE(luaPluginBase, nullptr);
        
        // Cast to specific plugin types
        scriptPlugin = dynamic_cast<ScriptPlugin*>(scriptPluginBase);
        pythonPlugin = dynamic_cast<PythonPlugin*>(pythonPluginBase);
        luaPlugin = dynamic_cast<LuaPlugin*>(luaPluginBase);
        
        ASSERT_NE(scriptPlugin, nullptr);
        ASSERT_NE(pythonPlugin, nullptr);
        ASSERT_NE(luaPlugin, nullptr);
        
        // Create test scripts
        pythonTestScript = "test_script.py";
        luaTestScript = "test_script.lua";
        
        // Create Python test script
        std::ofstream pythonFile(pythonTestScript);
        pythonFile << "def add(a, b):\n"
                  << "    return a + b\n\n"
                  << "def multiply(a, b):\n"
                  << "    return a * b\n\n"
                  << "def calculate_fibonacci(n):\n"
                  << "    if n <= 0:\n"
                  << "        return 0\n"
                  << "    elif n == 1:\n"
                  << "        return 1\n"
                  << "    else:\n"
                  << "        return calculate_fibonacci(n-1) + calculate_fibonacci(n-2)\n\n"
                  << "result = 42\n";
        pythonFile.close();
        
        // Create Lua test script
        std::ofstream luaFile(luaTestScript);
        luaFile << "function add(a, b)\n"
               << "    return a + b\n"
               << "end\n\n"
               << "function multiply(a, b)\n"
               << "    return a * b\n"
               << "end\n\n"
               << "function calculate_fibonacci(n)\n"
               << "    if n <= 0 then\n"
               << "        return 0\n"
               << "    elseif n == 1 then\n"
               << "        return 1\n"
               << "    else\n"
               << "        return calculate_fibonacci(n-1) + calculate_fibonacci(n-2)\n"
               << "    end\n"
               << "end\n\n"
               << "result = 42\n";
        luaFile.close();
    }

    void TearDown() override {
        // Unload all plugins
        pluginManager.UnloadAllPlugins();
        scriptPlugin = nullptr;
        pythonPlugin = nullptr;
        luaPlugin = nullptr;
        
        // Clean up test scripts
        if (std::filesystem::exists(pythonTestScript)) {
            std::filesystem::remove(pythonTestScript);
        }
        
        if (std::filesystem::exists(luaTestScript)) {
            std::filesystem::remove(luaTestScript);
        }
    }
};

// Test ScriptPlugin base functionality
TEST_F(ScriptPluginTest, ScriptPluginBaseFunctionality) {
    // Check plugin info
    PluginInfo info = scriptPlugin->GetPluginInfo();
    EXPECT_EQ(info.name, "ScriptPlugin");
    EXPECT_FALSE(info.displayName.empty());
    EXPECT_FALSE(info.description.empty());
    
    // Test script registration
    EXPECT_TRUE(scriptPlugin->RegisterScriptEngine("python", pythonPlugin));
    EXPECT_TRUE(scriptPlugin->RegisterScriptEngine("lua", luaPlugin));
    
    // Test getting registered engines
    std::vector<std::string> engines = scriptPlugin->GetRegisteredEngines();
    EXPECT_EQ(engines.size(), 2);
    EXPECT_TRUE(std::find(engines.begin(), engines.end(), "python") != engines.end());
    EXPECT_TRUE(std::find(engines.begin(), engines.end(), "lua") != engines.end());
    
    // Test script execution through ScriptPlugin
    EXPECT_TRUE(scriptPlugin->ExecuteScript("python", pythonTestScript));
    EXPECT_TRUE(scriptPlugin->ExecuteScript("lua", luaTestScript));
    
    // Test invalid engine
    EXPECT_FALSE(scriptPlugin->ExecuteScript("invalid_engine", pythonTestScript));
}

// Test PythonPlugin functionality
TEST_F(ScriptPluginTest, PythonPluginFunctionality) {
    // Check plugin info
    PluginInfo info = pythonPlugin->GetPluginInfo();
    EXPECT_EQ(info.name, "PythonPlugin");
    EXPECT_FALSE(info.displayName.empty());
    EXPECT_FALSE(info.description.empty());
    
    // Test script execution
    EXPECT_TRUE(pythonPlugin->ExecuteScript(pythonTestScript));
    
    // Test function execution
    int result = 0;
    EXPECT_TRUE(pythonPlugin->CallFunction(pythonTestScript, "add", {10, 20}, result));
    EXPECT_EQ(result, 30);
    
    EXPECT_TRUE(pythonPlugin->CallFunction(pythonTestScript, "multiply", {6, 7}, result));
    EXPECT_EQ(result, 42);
    
    // Test Fibonacci calculation
    EXPECT_TRUE(pythonPlugin->CallFunction(pythonTestScript, "calculate_fibonacci", {10}, result));
    EXPECT_EQ(result, 55); // 10th Fibonacci number is 55
    
    // Test getting global variable
    EXPECT_TRUE(pythonPlugin->GetGlobalVariable(pythonTestScript, "result", result));
    EXPECT_EQ(result, 42);
    
    // Test setting global variable
    EXPECT_TRUE(pythonPlugin->SetGlobalVariable(pythonTestScript, "result", 100));
    EXPECT_TRUE(pythonPlugin->GetGlobalVariable(pythonTestScript, "result", result));
    EXPECT_EQ(result, 100);
    
    // Test invalid function
    EXPECT_FALSE(pythonPlugin->CallFunction(pythonTestScript, "non_existent_function", {1, 2}, result));
    
    // Test invalid script
    EXPECT_FALSE(pythonPlugin->ExecuteScript("non_existent_script.py"));
}

// Test LuaPlugin functionality
TEST_F(ScriptPluginTest, LuaPluginFunctionality) {
    // Check plugin info
    PluginInfo info = luaPlugin->GetPluginInfo();
    EXPECT_EQ(info.name, "LuaPlugin");
    EXPECT_FALSE(info.displayName.empty());
    EXPECT_FALSE(info.description.empty());
    
    // Test script execution
    EXPECT_TRUE(luaPlugin->ExecuteScript(luaTestScript));
    
    // Test function execution
    int result = 0;
    EXPECT_TRUE(luaPlugin->CallFunction(luaTestScript, "add", {10, 20}, result));
    EXPECT_EQ(result, 30);
    
    EXPECT_TRUE(luaPlugin->CallFunction(luaTestScript, "multiply", {6, 7}, result));
    EXPECT_EQ(result, 42);
    
    // Test Fibonacci calculation
    EXPECT_TRUE(luaPlugin->CallFunction(luaTestScript, "calculate_fibonacci", {10}, result));
    EXPECT_EQ(result, 55); // 10th Fibonacci number is 55
    
    // Test getting global variable
    EXPECT_TRUE(luaPlugin->GetGlobalVariable(luaTestScript, "result", result));
    EXPECT_EQ(result, 42);
    
    // Test setting global variable
    EXPECT_TRUE(luaPlugin->SetGlobalVariable(luaTestScript, "result", 100));
    EXPECT_TRUE(luaPlugin->GetGlobalVariable(luaTestScript, "result", result));
    EXPECT_EQ(result, 100);
    
    // Test invalid function
    EXPECT_FALSE(luaPlugin->CallFunction(luaTestScript, "non_existent_function", {1, 2}, result));
    
    // Test invalid script
    EXPECT_FALSE(luaPlugin->ExecuteScript("non_existent_script.lua"));
}

// Test script interoperability
TEST_F(ScriptPluginTest, ScriptInteroperability) {
    // Register script engines
    EXPECT_TRUE(scriptPlugin->RegisterScriptEngine("python", pythonPlugin));
    EXPECT_TRUE(scriptPlugin->RegisterScriptEngine("lua", luaPlugin));
    
    // Execute both scripts
    EXPECT_TRUE(scriptPlugin->ExecuteScript("python", pythonTestScript));
    EXPECT_TRUE(scriptPlugin->ExecuteScript("lua", luaTestScript));
    
    // Test calling functions from both scripts
    int pythonResult = 0;
    int luaResult = 0;
    
    // Call the same function from both scripts
    EXPECT_TRUE(pythonPlugin->CallFunction(pythonTestScript, "add", {10, 20}, pythonResult));
    EXPECT_TRUE(luaPlugin->CallFunction(luaTestScript, "add", {10, 20}, luaResult));
    
    // Results should be the same
    EXPECT_EQ(pythonResult, luaResult);
    EXPECT_EQ(pythonResult, 30);
    
    // Test Fibonacci calculation in both scripts
    EXPECT_TRUE(pythonPlugin->CallFunction(pythonTestScript, "calculate_fibonacci", {7}, pythonResult));
    EXPECT_TRUE(luaPlugin->CallFunction(luaTestScript, "calculate_fibonacci", {7}, luaResult));
    
    // Results should be the same
    EXPECT_EQ(pythonResult, luaResult);
    EXPECT_EQ(pythonResult, 13); // 7th Fibonacci number is 13
}

// Test script error handling
TEST_F(ScriptPluginTest, ScriptErrorHandling) {
    // Create invalid Python script
    std::string invalidPythonScript = "invalid_python.py";
    std::ofstream invalidPyFile(invalidPythonScript);
    invalidPyFile << "def broken_function():\n"
                 << "    return syntax error here\n"; // Syntax error
    invalidPyFile.close();
    
    // Create invalid Lua script
    std::string invalidLuaScript = "invalid_lua.lua";
    std::ofstream invalidLuaFile(invalidLuaScript);
    invalidLuaFile << "function broken_function()\n"
                  << "    return syntax error here\n"; // Syntax error
    invalidLuaFile.close();
    
    // Test executing invalid scripts
    EXPECT_FALSE(pythonPlugin->ExecuteScript(invalidPythonScript));
    EXPECT_FALSE(luaPlugin->ExecuteScript(invalidLuaScript));
    
    // Clean up invalid scripts
    if (std::filesystem::exists(invalidPythonScript)) {
        std::filesystem::remove(invalidPythonScript);
    }
    
    if (std::filesystem::exists(invalidLuaScript)) {
        std::filesystem::remove(invalidLuaScript);
    }
}
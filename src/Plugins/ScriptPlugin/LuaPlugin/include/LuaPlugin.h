/**
 * @file LuaPlugin.h
 * @brief Defines the LuaPlugin class for Lua script integration
 */

#pragma once

#include "ScriptPlugin.h"
#include "LuaPluginExport.h"
#include <string>
#include <vector>
#include <unordered_map>

// Forward declarations to avoid including Lua headers in this header
typedef struct lua_State lua_State;
typedef int (*lua_CFunction)(lua_State* L);

/**
 * @class LuaPlugin
 * @brief Plugin for executing Lua scripts and integrating with Lua interpreter
 * 
 * This plugin provides integration with the Lua interpreter, allowing
 * C++ code to execute Lua scripts and Lua scripts to call C++ functions.
 */
class LUA_PLUGIN_API LuaPlugin : public ScriptPlugin {
public:
    /**
     * @brief Constructor
     */
    LuaPlugin();
    
    /**
     * @brief Destructor
     */
    virtual ~LuaPlugin();
    
    // IPlugin interface implementation
    bool Initialize() override;
    void Shutdown() override;
    const PluginInfo& GetPluginInfo() const override;
    std::string Serialize() override;
    bool Deserialize(const std::string& data) override;
    bool PrepareForHotReload() override;
    bool CompleteHotReload() override;
    
    // ScriptPlugin interface implementation
    bool ExecuteFile(const std::string& filePath) override;
    bool ExecuteString(const std::string& script) override;
    bool EvaluateExpression(const std::string& expression, std::string& result) override;
    bool RegisterFunction(const std::string& name, void* function) override;
    bool RegisterObject(const std::string& name, void* object) override;
    std::vector<std::string> GetSupportedExtensions() const override;
    std::string GetLanguageName() const override;
    std::string GetLanguageVersion() const override;
    
    /**
     * @brief Get the Lua state
     * 
     * @return Pointer to the Lua state
     */
    lua_State* GetLuaState() const;
    
    /**
     * @brief Register a C function with Lua
     * 
     * @param name Name of the function in Lua
     * @param function C function to register
     * @return true if registration was successful, false otherwise
     */
    bool RegisterCFunction(const std::string& name, lua_CFunction function);
    
    /**
     * @brief Push a value onto the Lua stack
     * 
     * @tparam T Type of the value
     * @param value Value to push
     * @return true if successful, false otherwise
     */
    template<typename T>
    bool PushValue(const T& value);
    
    /**
     * @brief Get a value from the Lua stack
     * 
     * @tparam T Type of the value to get
     * @param index Stack index
     * @param value Output parameter to store the value
     * @return true if successful, false otherwise
     */
    template<typename T>
    bool GetValue(int index, T& value);
    
    /**
     * @brief Call a Lua function
     * 
     * @param functionName Name of the function to call
     * @param numArgs Number of arguments on the stack
     * @param numResults Number of expected results
     * @return true if call was successful, false otherwise
     */
    bool CallFunction(const std::string& functionName, int numArgs, int numResults);
    
    /**
     * @brief Register a C++ class with Lua
     * 
     * @tparam T Type of the C++ class
     * @param name Name to use for the class in Lua
     * @return true if registration was successful, false otherwise
     */
    template<typename T>
    bool RegisterClass(const std::string& name);

    /**
     * @brief Get static plugin information
     * 
     * @return Static plugin information
     */
    static const PluginInfo& GetPluginStaticInfo() {
        return pluginInfo_;
    }
    
    static PluginInfo pluginInfo_; ///< Static plugin information

private:
    /**
     * @brief Initialize the Lua state
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool InitializeLua();
    
    /**
     * @brief Finalize the Lua state
     */
    void FinalizeLua();
    
    /**
     * @brief Handle Lua errors and log them
     * 
     * @param result Lua error code
     * @return true if an error occurred, false otherwise
     */
    bool HandleLuaError(int result);
    
    /**
     * @brief Register built-in libraries and functions
     * 
     * @return true if registration was successful, false otherwise
     */
    bool RegisterBuiltins();
    
    /**
     * @brief Register math plugin functionality with Lua
     * 
     * @return true if registration was successful, false otherwise
     */
    bool RegisterMathFunctions();
    
    lua_State* luaState_;       ///< Lua state
    bool initialized_;          ///< Whether the Lua interpreter is initialized
};

// Template implementations

template<typename T>
bool LuaPlugin::PushValue(const T& value) {
    // Default implementation for unsupported types
    return false;
}

template<typename T>
bool LuaPlugin::GetValue(int index, T& value) {
    // Default implementation for unsupported types
    return false;
}

template<typename T>
bool LuaPlugin::RegisterClass(const std::string& name) {
    // Default implementation
    return false;
}
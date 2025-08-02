/**
 * @file ScriptPlugin.h
 * @brief Defines the interface for script language plugins
 */

#pragma once

#include <string>
#include <vector>
#include "IPlugin.h"
#include "ScriptPluginExport.h"

/**
 * @class ScriptPlugin
 * @brief Interface for script language plugins like Python and Lua
 * 
 * This interface extends the base IPlugin interface with methods specific
 * to script language integration, such as executing scripts and evaluating
 * expressions.
 */
class SCRIPT_PLUGIN_API ScriptPlugin : public IPlugin {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~ScriptPlugin() = default;
    
    /**
     * @brief Execute a script file
     * 
     * @param filePath Path to the script file to execute
     * @return true if execution was successful, false otherwise
     */
    virtual bool ExecuteFile(const std::string& filePath) = 0;
    
    /**
     * @brief Execute a script string
     * 
     * @param script String containing script code to execute
     * @return true if execution was successful, false otherwise
     */
    virtual bool ExecuteString(const std::string& script) = 0;
    
    /**
     * @brief Evaluate an expression and return the result as a string
     * 
     * @param expression Expression to evaluate
     * @param result Output parameter to store the result
     * @return true if evaluation was successful, false otherwise
     */
    virtual bool EvaluateExpression(const std::string& expression, std::string& result) = 0;
    
    /**
     * @brief Register a C++ function to be callable from scripts
     * 
     * @param name Name of the function in the script environment
     * @param function Pointer to the C++ function
     * @return true if registration was successful, false otherwise
     */
    virtual bool RegisterFunction(const std::string& name, void* function) = 0;
    
    /**
     * @brief Register a C++ object to be accessible from scripts
     * 
     * @param name Name of the object in the script environment
     * @param object Pointer to the C++ object
     * @return true if registration was successful, false otherwise
     */
    virtual bool RegisterObject(const std::string& name, void* object) = 0;
    
    /**
     * @brief Get the file extensions supported by this script plugin
     * 
     * @return Vector of supported file extensions (e.g., ".py", ".lua")
     */
    virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    
    /**
     * @brief Get the name of the script language
     * 
     * @return Name of the script language (e.g., "Python", "Lua")
     */
    virtual std::string GetLanguageName() const = 0;
    
    /**
     * @brief Get the version of the script language
     * 
     * @return Version string of the script language
     */
    virtual std::string GetLanguageVersion() const = 0;
};
/**
 * @file PythonPlugin.h
 * @brief Defines the PythonPlugin class for Python script integration
 */

#pragma once

#include "ScriptPlugin.h"
#include "PythonPluginExport.h"
#include <string>
#include <vector>
#include <unordered_map>

// Forward declarations to avoid including Python headers in this header
typedef struct _object PyObject;
typedef struct _ts PyThreadState;

/**
 * @class PythonPlugin
 * @brief Plugin for executing Python scripts and integrating with Python interpreter
 * 
 * This plugin provides integration with the Python interpreter, allowing
 * C++ code to execute Python scripts and Python scripts to call C++ functions.
 */
class PYTHON_PLUGIN_API PythonPlugin : public ScriptPlugin {
public:
    /**
     * @brief Constructor
     */
    PythonPlugin();
    
    /**
     * @brief Destructor
     */
    virtual ~PythonPlugin();
    
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
    
    /**
     * @brief Get static plugin information
     * 
     * @return Static plugin information
     */
    static const PluginInfo& GetPluginStaticInfo() {
        return pluginInfo_;
    }
    bool RegisterObject(const std::string& name, void* object) override;
    std::vector<std::string> GetSupportedExtensions() const override;
    std::string GetLanguageName() const override;
    std::string GetLanguageVersion() const override;
    
    /**
     * @brief Add a directory to the Python path
     * 
     * @param path Directory path to add
     * @return true if successful, false otherwise
     */
    bool AddToPath(const std::string& path);
    
    /**
     * @brief Import a Python module
     * 
     * @param moduleName Name of the module to import
     * @return Pointer to the imported module, or nullptr if import failed
     */
    PyObject* ImportModule(const std::string& moduleName);
    
    /**
     * @brief Get a Python attribute from an object
     * 
     * @param object Python object to get attribute from
     * @param attributeName Name of the attribute to get
     * @return Pointer to the attribute, or nullptr if not found
     */
    PyObject* GetAttribute(PyObject* object, const std::string& attributeName);
    
    /**
     * @brief Call a Python function
     * 
     * @param function Python function to call
     * @param args Arguments to pass to the function
     * @return Result of the function call, or nullptr if call failed
     */
    PyObject* CallFunction(PyObject* function, PyObject* args);
    
    /**
     * @brief Convert a C++ value to a Python object
     * 
     * @tparam T Type of the C++ value
     * @param value C++ value to convert
     * @return Python object representing the value
     */
    template<typename T>
    PyObject* ToPython(const T& value);
    
    /**
     * @brief Convert a Python object to a C++ value
     * 
     * @tparam T Type to convert to
     * @param object Python object to convert
     * @param value Output parameter to store the converted value
     * @return true if conversion was successful, false otherwise
     */
    template<typename T>
    bool FromPython(PyObject* object, T& value);
    
    /**
     * @brief Get the Python interpreter's main module
     * 
     * @return Pointer to the main module
     */
    PyObject* GetMainModule() const;
    
    /**
     * @brief Get the Python interpreter's main namespace
     * 
     * @return Pointer to the main namespace dictionary
     */
    PyObject* GetMainNamespace() const;
    
    /**
     * @brief Register a C++ class with Python
     * 
     * @tparam T Type of the C++ class
     * @param name Name to use for the class in Python
     * @return true if registration was successful, false otherwise
     */
    template<typename T>
    bool RegisterClass(const std::string& name);

private:
    /**
     * @brief Initialize the Python interpreter
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool InitializePython();
    
    /**
     * @brief Finalize the Python interpreter
     */
    void FinalizePython();
    
    /**
     * @brief Handle Python errors and log them
     * 
     * @return true if an error occurred, false otherwise
     */
    bool HandlePythonError();
    
    /**
     * @brief Register built-in modules and functions
     * 
     * @return true if registration was successful, false otherwise
     */
    bool RegisterBuiltins();
    
    /**
     * @brief Register math plugin functionality with Python
     * 
     * @return true if registration was successful, false otherwise
     */
    bool RegisterMathFunctions();
    
    PyObject* mainModule_;      ///< Python's __main__ module
    PyObject* mainNamespace_;   ///< Python's __main__ module namespace
    PyThreadState* threadState_; ///< Python thread state
    bool initialized_;          ///< Whether the Python interpreter is initialized
    
    static PluginInfo pluginInfo_; ///< Static plugin information
};

// Template implementations

template<typename T>
PyObject* PythonPlugin::ToPython(const T& value) {
    // Default implementation for unsupported types
    return nullptr;
}

template<typename T>
bool PythonPlugin::FromPython(PyObject* object, T& value) {
    // Default implementation for unsupported types
    return false;
}

template<typename T>
bool PythonPlugin::RegisterClass(const std::string& name) {
    // Default implementation
    return false;
}
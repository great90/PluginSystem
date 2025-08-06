/**
 * @file PythonPlugin.cpp
 * @brief Implementation of the PythonPlugin class
 */

#include "PythonPlugin.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/embed.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <mutex>
#include "MathPlugin.h"
#include "ScriptObjectWrapper.h"

// For convenience
namespace py = pybind11;
namespace fs = std::filesystem;

// Static plugin info definition
PluginInfo PythonPlugin::pluginInfo_(
    "PythonPlugin",
    "Python Script Plugin",
    "Provides Python scripting capabilities",
    PluginInfo::Version(1, 0, 0),
    "Plugin System"
);

// Constructor
PythonPlugin::PythonPlugin()
    : mainModule_(nullptr)
    , mainNamespace_(nullptr)
    , threadState_(nullptr)
    , initialized_(false) {
    // Add dependencies
    pluginInfo_.AddDependency(PluginInfo::Dependency("ScriptPlugin", PluginInfo::Version(1, 0, 0)));
    pluginInfo_.AddDependency(PluginInfo::Dependency("MathPlugin", PluginInfo::Version(1, 0, 0)));
    
    // Register cleanup callback
    RegisterCleanupCallback();
}

// Destructor
PythonPlugin::~PythonPlugin() {
    if (initialized_) {
        Shutdown();
    }
}

bool PythonPlugin::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // Initialize Python interpreter
    if (!InitializePython()) {
        return false;
    }
    
    // Register built-in modules and functions
    if (!RegisterBuiltins()) {
        FinalizePython();
        return false;
    }
    
    // Register math functions
    if (!RegisterMathFunctions()) {
        FinalizePython();
        return false;
    }
    
    initialized_ = true;
    return true;
}

void PythonPlugin::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // Clean up script objects first
    CleanupScriptObjects();
    
    FinalizePython();
    initialized_ = false;
}

const PluginInfo& PythonPlugin::GetPluginInfo() const {
    return pluginInfo_;
}

std::string PythonPlugin::Serialize() {
    // In a real implementation, you would serialize the Python state
    return "";
}

bool PythonPlugin::Deserialize(const std::string& data) {
    // In a real implementation, you would deserialize the Python state
    return true;
}

bool PythonPlugin::PrepareForHotReload() {
    // In a real implementation, you would prepare the Python state for hot reload
    return true;
}

bool PythonPlugin::CompleteHotReload() {
    // In a real implementation, you would complete the hot reload of the Python state
    return true;
}

bool PythonPlugin::ExecuteFile(const std::string& filePath) {
    if (!initialized_) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Check if file exists
        if (!fs::exists(filePath)) {
            return false;
        }
        
        // Execute the file
        py::eval_file(filePath, *mainNamespace_);
        return true;
    } catch (const std::exception& e) {
        // Handle exception
        return false;
    }
}

bool PythonPlugin::ExecuteString(const std::string& script) {
    if (!initialized_) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Execute the script
        py::exec(script, *mainNamespace_);
        return true;
    } catch (const std::exception& e) {
        // Handle exception
        return false;
    }
}

bool PythonPlugin::EvaluateExpression(const std::string& expression, std::string& result) {
    if (!initialized_) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Evaluate the expression
        py::object obj = py::eval(expression, *mainNamespace_);
        
        // Convert the result to string
        result = py::str(obj);
        return true;
    } catch (const std::exception& e) {
        // Handle exception
        result = e.what();
        return false;
    }
}

bool PythonPlugin::RegisterFunction(const std::string& name, void* function) {
    if (!initialized_) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Register the function
        // Note: This is a simplified implementation
        // In a real implementation, you would need to wrap the function properly
        return true;
    } catch (const std::exception& e) {
        // Handle exception
        return false;
    }
}

bool PythonPlugin::RegisterObject(const std::string& name, void* object) {
    if (!initialized_) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Register the object
        // Note: This is a simplified implementation
        // In a real implementation, you would need to wrap the object properly
        return true;
    } catch (const std::exception& e) {
        // Handle exception
        return false;
    }
}

std::vector<std::string> PythonPlugin::GetSupportedExtensions() const {
    return {".py"};
}

std::string PythonPlugin::GetLanguageName() const {
    return "Python";
}

std::string PythonPlugin::GetLanguageVersion() const {
    if (!initialized_) {
        return "Unknown";
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Get Python version
        py::module_ sys = py::module_::import("sys");
        std::string version = py::str(sys.attr("version"));
        return version;
    } catch (const std::exception& e) {
        return "Unknown";
    }
}

bool PythonPlugin::AddToPath(const std::string& path) {
    if (!initialized_) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Add path to sys.path
        py::module_ sys = py::module_::import("sys");
        py::list paths = sys.attr("path").cast<py::list>();
        paths.append(path);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

PyObject* PythonPlugin::ImportModule(const std::string& moduleName) {
    if (!initialized_) {
        return nullptr;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Import module
        py::module_ module = py::module_::import(moduleName.c_str());
        return module.ptr();
    } catch (const std::exception& e) {
        return nullptr;
    }
}

PyObject* PythonPlugin::GetAttribute(PyObject* object, const std::string& attributeName) {
    if (!initialized_ || !object) {
        return nullptr;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Get attribute
        py::handle handle(object);
        py::object attr = handle.attr(attributeName.c_str());
        return attr.ptr();
    } catch (const std::exception& e) {
        return nullptr;
    }
}

PyObject* PythonPlugin::CallFunction(PyObject* function, PyObject* args) {
    if (!initialized_ || !function) {
        return nullptr;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Call function
        py::handle func_handle(function);
        py::handle args_handle(args);
        py::object result = func_handle(py::reinterpret_borrow<py::tuple>(args_handle));
        return result.ptr();
    } catch (const std::exception& e) {
        return nullptr;
    }
}

bool PythonPlugin::InitializePython() {
    try {
        // Initialize Python interpreter
        py::initialize_interpreter();
        
        // Create main module and namespace
        mainModule_ = new py::module_(py::module_::import("__main__"));
        mainNamespace_ = new py::dict(mainModule_->attr("__dict__"));
        
        // Save thread state
        threadState_ = PyThreadState_Get();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void PythonPlugin::FinalizePython() {
    try {
        // Clean up resources
        if (mainNamespace_) {
            delete mainNamespace_;
            mainNamespace_ = nullptr;
        }
        
        if (mainModule_) {
            delete mainModule_;
            mainModule_ = nullptr;
        }
        
        // Finalize Python interpreter
        py::finalize_interpreter();
    } catch (const std::exception& e) {
        // Handle exception
    }
}

bool PythonPlugin::HandlePythonError() {
    try {
        py::gil_scoped_acquire gil;
        
        if (PyErr_Occurred()) {
            PyObject *type, *value, *traceback;
            PyErr_Fetch(&type, &value, &traceback);
            PyErr_NormalizeException(&type, &value, &traceback);
            
            py::handle type_handle(type);
            py::handle value_handle(value);
            py::handle traceback_handle(traceback);
            
            std::string error_msg = py::str(value_handle);
            // In a real implementation, you would log this error
            
            PyErr_Clear();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        PyErr_Clear();
        return true;
    }
}

bool PythonPlugin::RegisterBuiltins() {
    try {
        py::gil_scoped_acquire gil;
        
        // Add plugin directory to Python path
        AddToPath(fs::current_path().string());
        
        // Import commonly used modules
        py::module::import("os");
        py::module::import("sys");
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool PythonPlugin::RegisterMathFunctions() {
    try {
        py::gil_scoped_acquire gil;
        
        // For now, skip Vector3 registration due to SIMD type compatibility issues
        // TODO: Implement proper Vector3 wrapper for Python bindings
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

template<typename T>
bool PythonPlugin::RegisterSharedObject(const std::string& name, std::shared_ptr<T> obj) {
    if (!initialized_ || !obj) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        
        // Create script object wrapper
        auto wrapper = std::make_shared<ScriptObjectWrapper<T>>(MakeScriptWrapper(obj));
        
        // Register cleanup function
        {
            std::lock_guard<std::mutex> lock(scriptObjectMutex_);
            scriptObjectCleanups_.push_back([wrapper]() {
                wrapper->Invalidate();
            });
        }
        
        // Register with Python using pybind11
        (*mainNamespace_)[name.c_str()] = py::cast(*wrapper);
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool PythonPlugin::RegisterMathPlugin(std::shared_ptr<MathPlugin> mathPlugin) {
    return RegisterSharedObject("math_plugin_instance", mathPlugin);
}

void PythonPlugin::CleanupScriptObjects() {
    std::lock_guard<std::mutex> lock(scriptObjectMutex_);
    
    // Execute all cleanup functions
    for (auto& cleanup : scriptObjectCleanups_) {
        try {
            cleanup();
        } catch (const std::exception& e) {
            // Log error but continue cleanup
        }
    }
    
    scriptObjectCleanups_.clear();
}

void PythonPlugin::RegisterCleanupCallback() {
    // Register with ScriptObjectManager for plugin-wide cleanup
    ScriptObjectManager::GetInstance().RegisterCleanupCallback(
        "PythonPlugin",
        [this]() {
            CleanupScriptObjects();
        }
    );
}

// Register the plugin
REGISTER_PLUGIN(PythonPlugin)

// Explicit template instantiations for common types
template bool PythonPlugin::RegisterSharedObject<MathPlugin>(const std::string&, std::shared_ptr<MathPlugin>);
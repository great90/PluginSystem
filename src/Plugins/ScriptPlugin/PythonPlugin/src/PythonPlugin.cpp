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
#include "Vector3.h"

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
    
    FinalizePython();
    initialized_ = false;
}

const PluginInfo& PythonPlugin::GetPluginInfo() const {
    return pluginInfo_;
}

std::string PythonPlugin::Serialize() {
    // For Python plugin, we don't have much state to serialize
    // In a real implementation, you might want to serialize loaded modules,
    // global variables, etc.
    return "{}";
}

bool PythonPlugin::Deserialize(const std::string& data) {
    // For Python plugin, we don't have much state to deserialize
    return true;
}

bool PythonPlugin::PrepareForHotReload() {
    // Nothing special needed for hot reload preparation
    return true;
}

bool PythonPlugin::CompleteHotReload() {
    // Nothing special needed for hot reload completion
    return true;
}

bool PythonPlugin::ExecuteFile(const std::string& filePath) {
    if (!initialized_) {
        return false;
    }
    
    try {
        // Check if file exists
        if (!fs::exists(filePath)) {
            return false;
        }
        
        // Read file content
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string script = buffer.str();
        
        // Execute the script
        py::gil_scoped_acquire gil;
        py::exec(script, py::globals(), py::globals());
        
        return true;
    } catch (const py::error_already_set& e) {
        // Handle Python exception
        HandlePythonError();
        return false;
    } catch (const std::exception& e) {
        // Handle C++ exception
        return false;
    }
}

bool PythonPlugin::ExecuteString(const std::string& script) {
    if (!initialized_) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        py::exec(script, py::globals(), py::globals());
        return true;
    } catch (const py::error_already_set& e) {
        HandlePythonError();
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}

bool PythonPlugin::EvaluateExpression(const std::string& expression, std::string& result) {
    if (!initialized_) {
        return false;
    }
    
    try {
        py::gil_scoped_acquire gil;
        py::object eval_result = py::eval(expression, py::globals(), py::globals());
        result = py::str(eval_result);
        return true;
    } catch (const py::error_already_set& e) {
        HandlePythonError();
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}

bool PythonPlugin::RegisterFunction(const std::string& name, void* function) {
    if (!initialized_ || !function) {
        return false;
    }
    
    try {
        // This is a simplified implementation
        // In a real implementation, you would need to wrap the function
        // using pybind11 or similar
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool PythonPlugin::RegisterObject(const std::string& name, void* object) {
    if (!initialized_ || !object) {
        return false;
    }
    
    try {
        // This is a simplified implementation
        // In a real implementation, you would need to wrap the object
        // using pybind11 or similar
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<std::string> PythonPlugin::GetSupportedExtensions() const {
    return {".py", ".pyw"};
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
        py::module sys = py::module::import("sys");
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
        py::module sys = py::module::import("sys");
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
        py::module module = py::module::import(moduleName.c_str());
        return module.ptr();
    } catch (const py::error_already_set& e) {
        HandlePythonError();
        return nullptr;
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
        py::handle handle(object);
        py::object attr = handle.attr(attributeName.c_str());
        return attr.ptr();
    } catch (const py::error_already_set& e) {
        HandlePythonError();
        return nullptr;
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
        py::handle func_handle(function);
        py::handle args_handle(args);
        py::object args_obj = py::reinterpret_borrow<py::object>(args_handle);
        py::object result = func_handle(*py::tuple(args_obj));
        return result.ptr();
    } catch (const py::error_already_set& e) {
        HandlePythonError();
        return nullptr;
    } catch (const std::exception& e) {
        return nullptr;
    }
}

PyObject* PythonPlugin::GetMainModule() const {
    return mainModule_;
}

PyObject* PythonPlugin::GetMainNamespace() const {
    return mainNamespace_;
}

bool PythonPlugin::InitializePython() {
    try {
        // Initialize Python interpreter
        pybind11::initialize_interpreter();
        
        // Get main module and namespace
        py::gil_scoped_acquire gil;
        mainModule_ = PyImport_AddModule("__main__");
        if (!mainModule_) {
            return false;
        }
        
        mainNamespace_ = PyModule_GetDict(mainModule_);
        if (!mainNamespace_) {
            return false;
        }
        
        // Save thread state
        threadState_ = PyThreadState_Get();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void PythonPlugin::FinalizePython() {
    try {
        // Release references
        mainModule_ = nullptr;
        mainNamespace_ = nullptr;
        threadState_ = nullptr;
        
        // Finalize Python interpreter
        pybind11::finalize_interpreter();
    } catch (const std::exception& e) {
        // Ignore exceptions during shutdown
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
        
        // Create a math module
        static py::module_::module_def math_module_def;
        py::module_ math_module = py::module_::create_extension_module("math_plugin", nullptr, &math_module_def);
        
        // Example: Register Vector3 class
        py::class_<Vector3>(math_module, "Vector3")
            .def(py::init<>())
            .def(py::init<float, float, float>())
            .def_readwrite("x", &Vector3::x)
            .def_readwrite("y", &Vector3::y)
            .def_readwrite("z", &Vector3::z)
            .def("__add__", [](const Vector3& self, const Vector3& other) {
                return Vector3(self.x + other.x, self.y + other.y, self.z + other.z);
            })
            .def("__sub__", [](const Vector3& self, const Vector3& other) {
                return Vector3(self.x - other.x, self.y - other.y, self.z - other.z);
            })
            .def("dot", [](const Vector3& self, const Vector3& other) {
                return self.x * other.x + self.y * other.y + self.z * other.z;
            })
            .def("cross", [](const Vector3& self, const Vector3& other) {
                return Vector3(
                    self.y * other.z - self.z * other.y,
                    self.z * other.x - self.x * other.z,
                    self.x * other.y - self.y * other.x
                );
            })
            .def("length", [](const Vector3& self) {
                return std::sqrt(self.x * self.x + self.y * self.y + self.z * self.z);
            })
            .def("normalize", [](const Vector3& self) {
                float len = std::sqrt(self.x * self.x + self.y * self.y + self.z * self.z);
                if (len > 0) {
                    return Vector3(self.x / len, self.y / len, self.z / len);
                }
                return self;
            })
            .def("__repr__", [](const Vector3& self) {
                return "Vector3(" + std::to_string(self.x) + ", " + 
                       std::to_string(self.y) + ", " + 
                       std::to_string(self.z) + ")";
            });
        
        // Add the module to sys.modules
        py::module::import("sys").attr("modules")["math_plugin"] = math_module;
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// Register the plugin
REGISTER_PLUGIN(PythonPlugin)
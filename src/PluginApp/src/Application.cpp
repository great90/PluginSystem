/**
 * @file Application.cpp
 * @brief Implementation of the Application class
 */

#include "Application.h"
#include "PluginManager.h"
#include "LogPlugin.h"
#include "MathPlugin.h"
#include "ScriptPlugin.h"
#include "PythonPlugin.h"
#include "LuaPlugin.h"
#include <iostream>
#include <filesystem>

// Use Vector3 from MathPlugin
using Vector3 = math::Vector3;

Application::Application(const std::string& pluginDir)
    : m_pluginDir(pluginDir)
    , m_pluginManager(nullptr)
    , m_mathPlugin(nullptr)
    , m_logPlugin(nullptr)
    , m_pythonPlugin(nullptr)
    , m_luaPlugin(nullptr)
{
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    std::cout << "=== Plugin System Demo ===" << std::endl;
    
    // Create the plugin manager
    m_pluginManager = std::make_unique<PluginManager>();
    
    std::cout << "Using plugin directory: " << m_pluginDir << std::endl;
    m_pluginManager->SetPluginDirectory(m_pluginDir);
    
    // Create plugin directory if it doesn't exist
    if (!std::filesystem::exists(m_pluginDir)) {
        std::filesystem::create_directories(m_pluginDir);
        std::cout << "Created plugin directory" << std::endl;
    }
    
    // Load all plugins
    std::cout << "\nLoading plugins..." << std::endl;
    if (!m_pluginManager->LoadAllPlugins()) {
        std::cerr << "Failed to load all plugins" << std::endl;
        return false;
    }
    
    // Get loaded plugin names
    m_loadedPlugins = m_pluginManager->GetLoadedPluginNames();
    std::cout << "Loaded " << m_loadedPlugins.size() << " plugins:" << std::endl;
    for (const auto& name : m_loadedPlugins) {
        auto plugin = m_pluginManager->GetPlugin(name);
        if (plugin) {
            PrintPluginInfo(plugin->GetPluginInfo());
        }
    }
    
    // Get specific plugin instances
    auto mathPluginPtr = m_pluginManager->GetPlugin("MathPlugin");
    m_mathPlugin = mathPluginPtr ? static_cast<math::MathPlugin*>(mathPluginPtr.get()) : nullptr;
    
    auto logPluginPtr = m_pluginManager->GetPlugin("LogPlugin");
    m_logPlugin = logPluginPtr ? static_cast<LogPlugin*>(logPluginPtr.get()) : nullptr;
    
    auto pythonPluginPtr = m_pluginManager->GetPlugin("PythonPlugin");
    m_pythonPlugin = pythonPluginPtr ? static_cast<PythonPlugin*>(pythonPluginPtr.get()) : nullptr;
    
    auto luaPluginPtr = m_pluginManager->GetPlugin("LuaPlugin");
    m_luaPlugin = luaPluginPtr ? static_cast<LuaPlugin*>(luaPluginPtr.get()) : nullptr;
    
    return true;
}

int Application::Run() {
    // Demonstrate plugins
    if (m_mathPlugin) {
        DemonstrateMathPlugin();
    }
    
    if (m_logPlugin) {
        DemonstrateLogPlugin();
    }
    
    if (m_pythonPlugin || m_luaPlugin) {
        DemonstrateScriptPlugins();
    }
    
    std::cout << "\nPlugin System Demo completed successfully" << std::endl;
    return 0;
}

void Application::Shutdown() {
    // Unload all plugins
    if (m_pluginManager) {
        std::cout << "\nUnloading plugins..." << std::endl;
        m_pluginManager->UnloadAllPlugins();
    }
}

void Application::PrintPluginInfo(const PluginInfo& info) {
    std::cout << "Plugin: " << info.displayName << " (" << info.name << ")" << std::endl;
    std::cout << "  Version: " << info.version.major << "." << info.version.minor << "." << info.version.patch << std::endl;
    std::cout << "  Description: " << info.description << std::endl;
    std::cout << "  Author: " << info.author << std::endl;
    
    if (!info.dependencies.empty()) {
        std::cout << "  Dependencies:" << std::endl;
        for (const auto& dep : info.dependencies) {
            std::cout << "    - " << dep.name << " (>= " << dep.minVersion.major << "." 
                      << dep.minVersion.minor << "." << dep.minVersion.patch << ")";
            if (dep.optional) {
                std::cout << " (optional)";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

void Application::DemonstrateMathPlugin() {
    using namespace math;

    if (!m_mathPlugin) {
        std::cerr << "Math plugin not available" << std::endl;
        return;
    }
    
    std::cout << "\n=== Math Plugin Demonstration ===" << std::endl;
    
    // Create and manipulate vectors
    Vector3 v1 = m_mathPlugin->MakeVector3(1.0f, 2.0f, 3.0f);
    Vector3 v2 = m_mathPlugin->MakeVector3(4.0f, 5.0f, 6.0f);
    
    Vector3 sum = rtm::vector_add(v1, v2);
    Vector3 diff = rtm::vector_sub(v2, v1);
    Vector3 scaled = rtm::vector_mul(v1, rtm::scalar_set(2.0f));
    
    std::cout << "Vector operations:" << std::endl;
    std::cout << "  v1 = (" << m_mathPlugin->GetVector3X(v1) << ", " << m_mathPlugin->GetVector3Y(v1) << ", " << m_mathPlugin->GetVector3Z(v1) << ")" << std::endl;
    std::cout << "  v2 = (" << m_mathPlugin->GetVector3X(v2) << ", " << m_mathPlugin->GetVector3Y(v2) << ", " << m_mathPlugin->GetVector3Z(v2) << ")" << std::endl;
    std::cout << "  v1 + v2 = (" << m_mathPlugin->GetVector3X(sum) << ", " << m_mathPlugin->GetVector3Y(sum) << ", " << m_mathPlugin->GetVector3Z(sum) << ")" << std::endl;
    std::cout << "  v2 - v1 = (" << m_mathPlugin->GetVector3X(diff) << ", " << m_mathPlugin->GetVector3Y(diff) << ", " << m_mathPlugin->GetVector3Z(diff) << ")" << std::endl;
    std::cout << "  v1 * 2 = (" << m_mathPlugin->GetVector3X(scaled) << ", " << m_mathPlugin->GetVector3Y(scaled) << ", " << m_mathPlugin->GetVector3Z(scaled) << ")" << std::endl;
    
    float dot = rtm::vector_dot3(v1, v2);
    Vector3 cross = rtm::vector_cross3(v1, v2);
    float length = math::MathPlugin::Vector3Length(v1);
    Vector3 normalized = math::MathPlugin::Vector3Normalize(v1);
    
    std::cout << "Vector methods:" << std::endl;
    std::cout << "  v1.Dot(v2) = " << dot << std::endl;
    std::cout << "  v1.Cross(v2) = (" << m_mathPlugin->GetVector3X(cross) << ", " << m_mathPlugin->GetVector3Y(cross) << ", " << m_mathPlugin->GetVector3Z(cross) << ")" << std::endl;
    std::cout << "  v1.Length() = " << length << std::endl;
    std::cout << "  v1.Normalized() = (" << m_mathPlugin->GetVector3X(normalized) << ", " << m_mathPlugin->GetVector3Y(normalized) << ", " << m_mathPlugin->GetVector3Z(normalized) << ")" << std::endl;
    
    // Demonstrate quaternion operations
    Quaternion q1 = m_mathPlugin->QuaternionFromAxisAngle(m_mathPlugin->MakeVector3(0, 1, 0), m_mathPlugin->DegreesToRadians(45.0f));
    Quaternion q2 = m_mathPlugin->QuaternionFromEulerAngles(
        m_mathPlugin->DegreesToRadians(30.0f),
        m_mathPlugin->DegreesToRadians(45.0f),
        m_mathPlugin->DegreesToRadians(60.0f)
    );
    
    Quaternion qMul = m_mathPlugin->QuaternionMultiply(q1, q2);
    Vector3 rotated = m_mathPlugin->QuaternionRotateVector(q1, v1);
    
    std::cout << "\nQuaternion operations:" << std::endl;
    std::cout << "  q1 = (" << m_mathPlugin->GetQuaternionX(q1) << ", " << m_mathPlugin->GetQuaternionY(q1) << ", " 
              << m_mathPlugin->GetQuaternionZ(q1) << ", " << m_mathPlugin->GetQuaternionW(q1) << ")" << std::endl;
    std::cout << "  q2 = (" << m_mathPlugin->GetQuaternionX(q2) << ", " << m_mathPlugin->GetQuaternionY(q2) << ", " 
              << m_mathPlugin->GetQuaternionZ(q2) << ", " << m_mathPlugin->GetQuaternionW(q2) << ")" << std::endl;
    std::cout << "  q1 * q2 = (" << m_mathPlugin->GetQuaternionX(qMul) << ", " << m_mathPlugin->GetQuaternionY(qMul) << ", " 
              << m_mathPlugin->GetQuaternionZ(qMul) << ", " << m_mathPlugin->GetQuaternionW(qMul) << ")" << std::endl;
    std::cout << "  q1.RotateVector(v1) = (" << m_mathPlugin->GetVector3X(rotated) << ", " << m_mathPlugin->GetVector3Y(rotated) << ", " 
              << m_mathPlugin->GetVector3Z(rotated) << ")" << std::endl;
    
    // Demonstrate matrix operations
    Matrix4x4 translationMatrix = m_mathPlugin->MakeTranslationMatrix(v1);
    Matrix4x4 scaleMatrix = m_mathPlugin->MakeScalingMatrix(m_mathPlugin->MakeVector3(2.0f, 2.0f, 2.0f));
    Matrix4x4 rotationMatrix = m_mathPlugin->MakeRotationYMatrix(m_mathPlugin->DegreesToRadians(45.0f));
    
    Matrix4x4 combined = m_mathPlugin->MatrixMultiply(m_mathPlugin->MatrixMultiply(translationMatrix, rotationMatrix), scaleMatrix);
    Vector3 transformed = m_mathPlugin->MatrixTransformVector(combined, v2);
    
    std::cout << "\nMatrix operations:" << std::endl;
    std::cout << "  TransformVector result = (" << m_mathPlugin->GetVector3X(transformed) << ", " 
              << m_mathPlugin->GetVector3Y(transformed) << ", " << m_mathPlugin->GetVector3Z(transformed) << ")" << std::endl;
    
    // Demonstrate utility functions
    float lerped = m_mathPlugin->Lerp(0.0f, 10.0f, 0.5f);
    Vector3 lerpedVec = m_mathPlugin->Lerp(v1, v2, 0.5f);
    float random = m_mathPlugin->Random(1.0f, 10.0f);
    int randomInt = m_mathPlugin->RandomInt(1, 100);
    
    std::cout << "\nUtility functions:" << std::endl;
    std::cout << "  Lerp(0, 10, 0.5) = " << lerped << std::endl;
    std::cout << "  Lerp(v1, v2, 0.5) = (" << m_mathPlugin->GetVector3X(lerpedVec) << ", " << m_mathPlugin->GetVector3Y(lerpedVec) << ", " << m_mathPlugin->GetVector3Z(lerpedVec) << ")" << std::endl;
    std::cout << "  Random(1, 10) = " << random << std::endl;
    std::cout << "  RandomInt(1, 100) = " << randomInt << std::endl;
}

void Application::DemonstrateLogPlugin() {
    if (!m_logPlugin) {
        std::cerr << "Log plugin not available" << std::endl;
        return;
    }
    
    std::cout << "\n=== Log Plugin Demonstration ===" << std::endl;
    
    // Add a file sink
    std::string logFilePath = "plugin_system.log";
    if (m_logPlugin->AddFileSink(logFilePath, true)) {
        std::cout << "Added file sink: " << logFilePath << std::endl;
    }
    
    // Log messages at different levels
    m_logPlugin->SetLevel(LogLevel::Trace);
    std::cout << "Set log level to Trace" << std::endl;
    
    m_logPlugin->Trace("This is a trace message");
    m_logPlugin->Debug("This is a debug message");
    m_logPlugin->Info("This is an info message");
    m_logPlugin->Warning("This is a warning message");
    m_logPlugin->Error("This is an error message");
    m_logPlugin->Critical("This is a critical message");
    
    // Change log level and pattern
    m_logPlugin->SetLevel(LogLevel::Warning);
    m_logPlugin->SetPattern("%Y-%m-%d %H:%M:%S.%e [%^%l%$] %v");
    
    std::cout << "Set log level to Warning with colored level pattern" << std::endl;
    
    m_logPlugin->Trace("This trace message should not appear");
    m_logPlugin->Debug("This debug message should not appear");
    m_logPlugin->Info("This info message should not appear");
    m_logPlugin->Warning("This warning message should appear");
    m_logPlugin->Error("This error message should appear");
    m_logPlugin->Critical("This critical message should appear");
    
    // Flush logs
    m_logPlugin->Flush();
    std::cout << "Logs have been written to " << logFilePath << std::endl;
}

void Application::DemonstrateScriptPlugins() {
    std::cout << "\n=== Script Plugins Demonstration ===" << std::endl;
    
    // Demonstrate Python plugin
    if (m_pythonPlugin) {
        std::cout << "\n--- Python Plugin ---" << std::endl;
        std::cout << "Language: " << m_pythonPlugin->GetLanguageName() << " " 
                  << m_pythonPlugin->GetLanguageVersion() << std::endl;
        std::cout << "Supported extensions: ";
        for (const auto& ext : m_pythonPlugin->GetSupportedExtensions()) {
            std::cout << ext << " ";
        }
        std::cout << std::endl;
        
        // Execute Python code
        std::string pythonCode = R"(
# Python test script
print("Hello from Python!")

# Test Vector3 class
v1 = Vector3(1, 2, 3)
v2 = Vector3(4, 5, 6)
v3 = v1 + v2
print(f"v1 + v2 = {v3}")

# Test math operations
dot = v1.dot(v2)
print(f"v1.dot(v2) = {dot}")

cross = v1.cross(v2)
print(f"v1.cross(v2) = {cross}")

length = v1.length()
print(f"v1.length() = {length}")

normalized = v1.normalize()
print(f"v1.normalize() = {normalized}")

# Return a value
result = "Python script executed successfully"
)";
        
        std::cout << "\nExecuting Python code:" << std::endl;
        if (m_pythonPlugin->ExecuteString(pythonCode)) {
            std::string result;
            if (m_pythonPlugin->EvaluateExpression("result", result)) {
                std::cout << "Result: " << result << std::endl;
            }
        } else {
            std::cout << "Failed to execute Python code" << std::endl;
        }
    } else {
        std::cout << "Python plugin not available" << std::endl;
    }
    
    // Demonstrate Lua plugin
    if (m_luaPlugin) {
        std::cout << "\n--- Lua Plugin ---" << std::endl;
        std::cout << "Language: " << m_luaPlugin->GetLanguageName() << " " 
                  << m_luaPlugin->GetLanguageVersion() << std::endl;
        std::cout << "Supported extensions: ";
        for (const auto& ext : m_luaPlugin->GetSupportedExtensions()) {
            std::cout << ext << " ";
        }
        std::cout << std::endl;
        
        // Execute Lua code
        std::string luaCode = R"(
-- Lua test script
print("Hello from Lua!")

-- Test Vector3 class
local v1 = Vector3(1, 2, 3)
local v2 = Vector3(4, 5, 6)
local v3 = v1 + v2
print("v1 + v2 = " .. tostring(v3))

-- Test math operations
local dot = v1:dot(v2)
print("v1:dot(v2) = " .. dot)

local cross = v1:cross(v2)
print("v1:cross(v2) = " .. tostring(cross))

local length = v1:length()
print("v1:length() = " .. length)

local normalized = v1:normalize()
print("v1:normalize() = " .. tostring(normalized))

-- Return a value
result = "Lua script executed successfully"
)";
        
        std::cout << "\nExecuting Lua code:" << std::endl;
        if (m_luaPlugin->ExecuteString(luaCode)) {
            std::string result;
            if (m_luaPlugin->EvaluateExpression("result", result)) {
                std::cout << "Result: " << result << std::endl;
            }
        } else {
            std::cout << "Failed to execute Lua code" << std::endl;
        }
    } else {
        std::cout << "Lua plugin not available" << std::endl;
    }
}
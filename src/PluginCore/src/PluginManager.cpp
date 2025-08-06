/**
 * @file PluginManager.cpp
 * @brief Implementation of the PluginManager class
 */

#include "PluginManager.h"
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

// Platform-specific library loading
#ifdef _WIN32
    #include <windows.h>
    #define LOAD_LIBRARY(path) LoadLibraryA(path)
    #define GET_PROC_ADDRESS(handle, name) GetProcAddress((HMODULE)handle, name)
    #define CLOSE_LIBRARY(handle) FreeLibrary((HMODULE)handle)
    #define PLUGIN_EXTENSION ".dll"
#else
    #include <dlfcn.h>
    #define LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
    #define GET_PROC_ADDRESS(handle, name) dlsym(handle, name)
    #define CLOSE_LIBRARY(handle) dlclose(handle)
    #ifdef __APPLE__
        #define PLUGIN_EXTENSION ".dylib"
    #else
        #define PLUGIN_EXTENSION ".so"
    #endif
#endif

namespace fs = std::filesystem;

// PluginLibrary RAII implementation
PluginManager::PluginLibrary::~PluginLibrary() {
    if (instance) {
        try {
            instance->Shutdown();
        } catch (...) {
            // Ignore exceptions during shutdown
        }
        instance.reset();
    }
    
    if (handle) {
        CLOSE_LIBRARY(handle);
        handle = nullptr;
    }
}

PluginManager::PluginLibrary::PluginLibrary(PluginLibrary&& other) noexcept
    : handle(other.handle)
    , instance(std::move(other.instance))
    , createFunc(other.createFunc)
    , infoFunc(other.infoFunc)
    , path(std::move(other.path)) {
    other.handle = nullptr;
    other.createFunc = nullptr;
    other.infoFunc = nullptr;
}

PluginManager::PluginLibrary& PluginManager::PluginLibrary::operator=(PluginLibrary&& other) noexcept {
    if (this != &other) {
        // Clean up current resources
        if (instance) {
            try {
                instance->Shutdown();
            } catch (...) {}
            instance.reset();
        }
        if (handle) {
            CLOSE_LIBRARY(handle);
        }
        
        // Move from other
        handle = other.handle;
        instance = std::move(other.instance);
        createFunc = other.createFunc;
        infoFunc = other.infoFunc;
        path = std::move(other.path);
        
        // Reset other
        other.handle = nullptr;
        other.createFunc = nullptr;
        other.infoFunc = nullptr;
    }
    return *this;
}

PluginManager::PluginManager() 
    : pluginDirectory_("./plugins")
    , loggingEnabled_(false) {
    LogMessage("INFO", "PluginManager initialized");
}

PluginManager::~PluginManager() {
    try {
        UnloadAllPlugins();
        LogMessage("INFO", "PluginManager destroyed");
    } catch (...) {
        // Ignore exceptions during destruction
    }
}

void PluginManager::SetPluginDirectory(const std::string& directory) {
    std::lock_guard<std::mutex> lock(mutex_);
    pluginDirectory_ = directory;
    LogMessage("INFO", "Plugin directory set to: " + directory);
}

std::string PluginManager::GetPluginDirectory() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pluginDirectory_;
}

bool PluginManager::LoadPlugin(const std::string& pluginPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if file exists
        if (!fs::exists(pluginPath)) {
            SetLastError("Plugin file does not exist: " + pluginPath);
            return false;
        }
        
        LogMessage("INFO", "Loading plugin: " + pluginPath);
        
        // Load the library
        void* handle = LOAD_LIBRARY(pluginPath.c_str());
        if (!handle) {
            SetLastError("Failed to load library: " + pluginPath);
            return false;
        }
        
        // Get the plugin creation and info functions
        CreatePluginFunc createFunc = reinterpret_cast<CreatePluginFunc>(
            GET_PROC_ADDRESS(handle, "CreatePlugin"));
        GetPluginInfoFunc infoFunc = reinterpret_cast<GetPluginInfoFunc>(
            GET_PROC_ADDRESS(handle, "GetPluginInfo"));
        
        // Validate that we got the required functions
        if (!createFunc || !infoFunc) {
            CLOSE_LIBRARY(handle);
            SetLastError("Plugin missing required functions: " + pluginPath);
            return false;
        }
        
        // Get plugin info to check if it's already loaded
        const PluginInfo& info = infoFunc();
        if (loadedPlugins_.find(info.name) != loadedPlugins_.end()) {
            CLOSE_LIBRARY(handle);
            SetLastError("Plugin already loaded: " + info.name);
            return false;
        }
        
        // Create the plugin instance
        IPlugin* rawInstance = createFunc();
        if (!rawInstance) {
            CLOSE_LIBRARY(handle);
            SetLastError("Failed to create plugin instance: " + info.name);
            return false;
        }
        
        // Wrap in shared_ptr with custom deleter
        std::shared_ptr<IPlugin> instance(rawInstance, [](IPlugin* p) {
            delete p;
        });
        
        // Store the plugin
        PluginLibrary library;
        library.handle = handle;
        library.instance = instance;
        library.createFunc = createFunc;
        library.infoFunc = infoFunc;
        library.path = pluginPath;
        
        loadedPlugins_[info.name] = std::move(library);
        
        // Add dependencies to the resolver
        for (const auto& dep : info.dependencies) {
            dependencyResolver_.AddDependency(info.name, dep.name, dep.optional);
        }
        
        LogMessage("INFO", "Successfully loaded plugin: " + info.name);
        NotifyLifecycleCallbacks(info.name, "loaded");
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception during plugin loading: " + std::string(e.what()));
        return false;
    } catch (...) {
        SetLastError("Unknown exception during plugin loading");
        return false;
    }
}

int PluginManager::LoadAllPlugins() {
    int count = 0;
    std::vector<std::string> pluginFiles = FindPluginFiles();
    
    for (const auto& file : pluginFiles) {
        if (LoadPlugin(file)) {
            count++;
        }
    }
    
    // Initialize plugins in dependency order
    if (count > 0) {
        ResolveDependencies();
    }
    
    return count;
}

bool PluginManager::UnloadPlugin(const std::string& pluginName) {
    auto it = loadedPlugins_.find(pluginName);
    if (it == loadedPlugins_.end()) {
        return false;
    }
    
    // Check if other plugins depend on this one
    for (const auto& pair : loadedPlugins_) {
        if (pair.first == pluginName) continue;
        
        const auto& deps = dependencyResolver_.GetDependencies(pair.first);
        if (std::find(deps.begin(), deps.end(), pluginName) != deps.end()) {
            // Found a non-optional dependency
            return false;
        }
    }
    
    // Shutdown the plugin
    it->second.instance->Shutdown();
    
    // Close the library and remove from maps
    CloseLibrary(it->second);
    loadedPlugins_.erase(it);
    
    return true;
}

void PluginManager::UnloadAllPlugins() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        LogMessage("INFO", "Unloading all plugins");
        
        // Get plugin names in reverse dependency order for proper shutdown
        std::vector<std::string> pluginNames;
        pluginNames.reserve(loadedPlugins_.size());
        for (const auto& pair : loadedPlugins_) {
            pluginNames.push_back(pair.first);
        }
        
        std::vector<std::string> reverseOrder;
        
        try {
            reverseOrder = dependencyResolver_.ResolveLoadOrder(pluginNames);
            std::reverse(reverseOrder.begin(), reverseOrder.end());
        } catch (const std::exception& e) {
            // If there's a dependency issue, just use the unordered list
            LogMessage("WARNING", "Dependency resolution failed during unload: " + std::string(e.what()));
            reverseOrder = pluginNames;
        }
        
        // Shutdown all plugins first (RAII will handle cleanup)
        for (const auto& name : reverseOrder) {
            auto it = loadedPlugins_.find(name);
            if (it != loadedPlugins_.end()) {
                try {
                    if (it->second.instance) {
                        it->second.instance->Shutdown();
                        LogMessage("INFO", "Shutdown plugin: " + name);
                        NotifyLifecycleCallbacks(name, "unloaded");
                    }
                } catch (const std::exception& e) {
                    LogMessage("ERROR", "Exception during plugin shutdown: " + name + " - " + e.what());
                } catch (...) {
                    LogMessage("ERROR", "Unknown exception during plugin shutdown: " + name);
                }
            }
        }
        
        // Clear all plugins (RAII destructors will handle library cleanup)
        loadedPlugins_.clear();
        dependencyResolver_.Clear();
        
        LogMessage("INFO", "All plugins unloaded successfully");
        
    } catch (const std::exception& e) {
        SetLastError("Exception during unload all plugins: " + std::string(e.what()));
        LogMessage("ERROR", "Exception during unload all plugins: " + std::string(e.what()));
    } catch (...) {
        SetLastError("Unknown exception during unload all plugins");
        LogMessage("ERROR", "Unknown exception during unload all plugins");
    }
}

std::shared_ptr<IPlugin> PluginManager::GetPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loadedPlugins_.find(pluginName);
    if (it != loadedPlugins_.end()) {
        return it->second.instance;
    }
    return nullptr;
}

std::weak_ptr<IPlugin> PluginManager::GetPluginWeak(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loadedPlugins_.find(pluginName);
    if (it != loadedPlugins_.end()) {
        return std::weak_ptr<IPlugin>(it->second.instance);
    }
    return std::weak_ptr<IPlugin>();
}

std::vector<std::string> PluginManager::GetLoadedPluginNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    names.reserve(loadedPlugins_.size());
    
    for (const auto& pair : loadedPlugins_) {
        names.push_back(pair.first);
    }
    
    return names;
}

bool PluginManager::IsPluginLoaded(const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return loadedPlugins_.find(pluginName) != loadedPlugins_.end();
}

bool PluginManager::HotReloadPlugin(const std::string& pluginName) {
    auto it = loadedPlugins_.find(pluginName);
    if (it == loadedPlugins_.end()) {
        return false;
    }
    
    PluginLibrary& library = it->second;
    std::string pluginPath = library.path;
    
    // Check if the plugin supports hot-reloading
    if (!library.instance->PrepareForHotReload()) {
        return false;
    }
    
    // Serialize the plugin state
    std::string serializedState = library.instance->Serialize();
    
    // Get dependencies to reload
    std::vector<std::string> dependentPlugins;
    for (const auto& pair : loadedPlugins_) {
        const auto& deps = dependencyResolver_.GetDependencies(pair.first);
        if (std::find(deps.begin(), deps.end(), pluginName) != deps.end()) {
            dependentPlugins.push_back(pair.first);
        }
    }
    
    // Unload dependent plugins first
    std::unordered_map<std::string, std::string> dependentStates;
    for (const auto& depName : dependentPlugins) {
        auto depIt = loadedPlugins_.find(depName);
        if (depIt != loadedPlugins_.end()) {
            // Prepare and serialize dependent plugin
            if (!depIt->second.instance->PrepareForHotReload()) {
                // Abort if any dependent plugin can't be hot-reloaded
                return false;
            }
            dependentStates[depName] = depIt->second.instance->Serialize();
            
            // Shutdown and close the dependent plugin
            depIt->second.instance->Shutdown();
            CloseLibrary(depIt->second);
            loadedPlugins_.erase(depIt);
        }
    }
    
    // Unload the plugin
    library.instance->Shutdown();
    CloseLibrary(library);
    loadedPlugins_.erase(it);
    
    // Reload the plugin
    if (!LoadPlugin(pluginPath)) {
        // Failed to reload, dependencies will also fail
        return false;
    }
    
    // Get the newly loaded plugin
    auto newIt = loadedPlugins_.find(pluginName);
    if (newIt == loadedPlugins_.end()) {
        return false;
    }
    
    // Deserialize the plugin state
    if (!newIt->second.instance->Deserialize(serializedState)) {
        return false;
    }
    
    // Complete the hot-reload process
    if (!newIt->second.instance->CompleteHotReload()) {
        return false;
    }
    
    // Reload dependent plugins
    bool allDependentsReloaded = true;
    for (const auto& depName : dependentPlugins) {
        auto depPath = pluginDirectory_ + "/" + depName + PLUGIN_EXTENSION;
        if (!LoadPlugin(depPath)) {
            allDependentsReloaded = false;
            continue;
        }
        
        // Deserialize dependent plugin state
        auto depIt = loadedPlugins_.find(depName);
        if (depIt != loadedPlugins_.end() && dependentStates.find(depName) != dependentStates.end()) {
            if (!depIt->second.instance->Deserialize(dependentStates[depName])) {
                allDependentsReloaded = false;
            }
            if (!depIt->second.instance->CompleteHotReload()) {
                allDependentsReloaded = false;
            }
        }
    }
    
    // Re-initialize all plugins in dependency order
    ResolveDependencies();
    
    return true;
}

bool PluginManager::ResolveDependencies() {
    try {
        std::vector<std::string> pluginNames = GetLoadedPluginNames();
        std::vector<std::string> loadOrder = dependencyResolver_.ResolveLoadOrder(pluginNames);
        
        // Initialize plugins in dependency order
        for (const auto& name : loadOrder) {
            auto it = loadedPlugins_.find(name);
            if (it != loadedPlugins_.end()) {
                if (!it->second.instance->Initialize()) {
                    return false;
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        // Circular dependency or other error
        return false;
    }
}

void PluginManager::CloseLibrary(PluginLibrary& library) noexcept {
    try {
        if (library.instance) {
            library.instance->Shutdown();
            library.instance.reset();
        }
        
        if (library.handle) {
            CLOSE_LIBRARY(library.handle);
            library.handle = nullptr;
        }
    } catch (...) {
        // Ignore all exceptions during cleanup
    }
}

std::vector<std::string> PluginManager::FindPluginFiles() const {
    std::vector<std::string> pluginFiles;
    
    try {
        if (!fs::exists(pluginDirectory_) || !fs::is_directory(pluginDirectory_)) {
            LogMessage("WARNING", "Plugin directory does not exist or is not a directory: " + pluginDirectory_);
            return pluginFiles;
        }
        
        for (const auto& entry : fs::directory_iterator(pluginDirectory_)) {
            if (entry.is_regular_file() && entry.path().extension() == PLUGIN_EXTENSION) {
                pluginFiles.push_back(entry.path().string());
            }
        }
        
        LogMessage("INFO", "Found " + std::to_string(pluginFiles.size()) + " plugin files");
        
    } catch (const std::exception& e) {
        LogMessage("ERROR", "Exception while finding plugin files: " + std::string(e.what()));
    }
    
    return pluginFiles;
}

void PluginManager::LogMessage(const std::string& level, const std::string& message) const {
    if (!loggingEnabled_) return;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
    ss << "[" << level << "] PluginManager: " << message;
    
    std::cout << ss.str() << std::endl;
}

void PluginManager::NotifyLifecycleCallbacks(const std::string& pluginName, const std::string& event) const {
    for (const auto& callback : lifecycleCallbacks_) {
        try {
            callback(pluginName, event);
        } catch (const std::exception& e) {
            LogMessage("ERROR", "Exception in lifecycle callback: " + std::string(e.what()));
        } catch (...) {
            LogMessage("ERROR", "Unknown exception in lifecycle callback");
        }
    }
}

void PluginManager::SetLastError(const std::string& error) const {
    lastError_ = error;
}

void PluginManager::RegisterLifecycleCallback(const PluginLifecycleCallback& callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    lifecycleCallbacks_.push_back(callback);
}

std::string PluginManager::GetLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void PluginManager::SetLoggingEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    loggingEnabled_ = enabled;
    LogMessage("INFO", "Logging " + std::string(enabled ? "enabled" : "disabled"));
}

PluginResult<std::vector<std::string>> PluginManager::GetLoadOrder() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        std::vector<std::string> pluginNames;
        pluginNames.reserve(loadedPlugins_.size());
        for (const auto& pair : loadedPlugins_) {
            pluginNames.push_back(pair.first);
        }
        
        return dependencyResolver_.ResolveLoadOrder(pluginNames);
    } catch (const std::exception& e) {
        SetLastError("Failed to resolve load order: " + std::string(e.what()));
        return std::nullopt;
    }
}
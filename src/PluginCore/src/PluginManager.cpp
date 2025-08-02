/**
 * @file PluginManager.cpp
 * @brief Implementation of the PluginManager class
 */

#include "PluginManager.h"
#include <filesystem>
#include <algorithm>
#include <stdexcept>

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

PluginManager::PluginManager() : pluginDirectory_("./plugins") {
}

PluginManager::~PluginManager() {
    UnloadAllPlugins();
}

void PluginManager::SetPluginDirectory(const std::string& directory) {
    pluginDirectory_ = directory;
}

std::string PluginManager::GetPluginDirectory() const {
    return pluginDirectory_;
}

bool PluginManager::LoadPlugin(const std::string& pluginPath) {
    // Check if file exists
    if (!fs::exists(pluginPath)) {
        return false;
    }
    
    // Load the library
    void* handle = LOAD_LIBRARY(pluginPath.c_str());
    if (!handle) {
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
        return false;
    }
    
    // Get plugin info to check if it's already loaded
    const PluginInfo& info = infoFunc();
    if (IsPluginLoaded(info.name)) {
        CLOSE_LIBRARY(handle);
        return false;
    }
    
    // Create the plugin instance
    IPlugin* instance = createFunc();
    if (!instance) {
        CLOSE_LIBRARY(handle);
        return false;
    }
    
    // Store the plugin
    PluginLibrary library;
    library.handle = handle;
    library.instance = instance;
    library.createFunc = createFunc;
    library.infoFunc = infoFunc;
    library.path = pluginPath;
    
    loadedPlugins_[info.name] = library;
    
    // Add dependencies to the resolver
    for (const auto& dep : info.dependencies) {
        dependencyResolver_.AddDependency(info.name, dep.name, dep.optional);
    }
    
    return true;
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
    // Get plugin names in reverse dependency order for proper shutdown
    std::vector<std::string> pluginNames = GetLoadedPluginNames();
    std::vector<std::string> reverseOrder;
    
    try {
        reverseOrder = dependencyResolver_.ResolveLoadOrder(pluginNames);
        std::reverse(reverseOrder.begin(), reverseOrder.end());
    } catch (const std::exception&) {
        // If there's a dependency issue, just use the unordered list
        reverseOrder = pluginNames;
    }
    
    // Shutdown all plugins first
    for (const auto& name : reverseOrder) {
        auto it = loadedPlugins_.find(name);
        if (it != loadedPlugins_.end()) {
            it->second.instance->Shutdown();
        }
    }
    
    // Then close all libraries
    for (auto& pair : loadedPlugins_) {
        CloseLibrary(pair.second);
    }
    
    loadedPlugins_.clear();
    dependencyResolver_.Clear();
}

IPlugin* PluginManager::GetPlugin(const std::string& pluginName) {
    auto it = loadedPlugins_.find(pluginName);
    if (it != loadedPlugins_.end()) {
        return it->second.instance;
    }
    return nullptr;
}

std::vector<std::string> PluginManager::GetLoadedPluginNames() const {
    std::vector<std::string> names;
    names.reserve(loadedPlugins_.size());
    
    for (const auto& pair : loadedPlugins_) {
        names.push_back(pair.first);
    }
    
    return names;
}

bool PluginManager::IsPluginLoaded(const std::string& pluginName) const {
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

void PluginManager::CloseLibrary(PluginLibrary& library) {
    delete library.instance;
    library.instance = nullptr;
    
    if (library.handle) {
        CLOSE_LIBRARY(library.handle);
        library.handle = nullptr;
    }
}

std::vector<std::string> PluginManager::FindPluginFiles() const {
    std::vector<std::string> pluginFiles;
    
    if (!fs::exists(pluginDirectory_) || !fs::is_directory(pluginDirectory_)) {
        return pluginFiles;
    }
    
    for (const auto& entry : fs::directory_iterator(pluginDirectory_)) {
        if (entry.is_regular_file() && entry.path().extension() == PLUGIN_EXTENSION) {
            pluginFiles.push_back(entry.path().string());
        }
    }
    
    return pluginFiles;
}
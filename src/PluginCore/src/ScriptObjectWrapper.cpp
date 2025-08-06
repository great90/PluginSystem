/**
 * @file ScriptObjectWrapper.cpp
 * @brief Implementation of the ScriptObjectWrapper and ScriptObjectManager
 */

#include "ScriptObjectWrapper.h"
#include <mutex>
#include <unordered_map>
#include <vector>
#include <iostream>

void ScriptObjectManager::RegisterCleanupCallback(const std::string& pluginName, const CleanupCallback& callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    cleanupCallbacks_[pluginName].push_back(callback);
}

void ScriptObjectManager::CleanupPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cleanupCallbacks_.find(pluginName);
    if (it != cleanupCallbacks_.end()) {
        // Execute all cleanup callbacks for this plugin
        for (const auto& callback : it->second) {
            try {
                callback();
            } catch (const std::exception& e) {
                std::cerr << "Exception during script object cleanup for plugin " 
                         << pluginName << ": " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown exception during script object cleanup for plugin " 
                         << pluginName << std::endl;
            }
        }
        
        // Remove the callbacks for this plugin
        cleanupCallbacks_.erase(it);
    }
}

void ScriptObjectManager::CleanupAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Execute all cleanup callbacks
    for (const auto& pair : cleanupCallbacks_) {
        for (const auto& callback : pair.second) {
            try {
                callback();
            } catch (const std::exception& e) {
                std::cerr << "Exception during script object cleanup for plugin " 
                         << pair.first << ": " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown exception during script object cleanup for plugin " 
                         << pair.first << std::endl;
            }
        }
    }
    
    // Clear all callbacks
    cleanupCallbacks_.clear();
}

ScriptObjectManager& ScriptObjectManager::GetInstance() {
    static ScriptObjectManager instance;
    return instance;
}
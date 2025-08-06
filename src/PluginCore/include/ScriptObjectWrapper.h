/**
 * @file ScriptObjectWrapper.h
 * @brief Defines the ScriptObjectWrapper for safe script-layer object management
 */

#pragma once

#include <memory>
#include <string>
#include <functional>
#include <stdexcept>
#include "PluginExport.h"

/**
 * @brief Exception thrown when script object operations fail
 */
class PLUGIN_CORE_API ScriptObjectException : public std::runtime_error {
public:
    explicit ScriptObjectException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Template wrapper for safely exposing C++ objects to script layers
 * 
 * This class provides a safe way to expose C++ objects managed by shared_ptr
 * to script languages while maintaining proper lifetime management.
 * 
 * @tparam T The type of object being wrapped
 */
template<typename T>
class PLUGIN_CORE_API ScriptObjectWrapper {
public:
    /**
     * @brief Construct wrapper from shared_ptr
     * 
     * @param obj Shared pointer to the object
     * @param name Optional name for debugging
     */
    explicit ScriptObjectWrapper(std::shared_ptr<T> obj, const std::string& name = "")
        : weakPtr_(obj), name_(name), isValid_(true) {
        if (!obj) {
            throw ScriptObjectException("Cannot wrap null object");
        }
    }
    
    /**
     * @brief Construct wrapper from weak_ptr
     * 
     * @param weakObj Weak pointer to the object
     * @param name Optional name for debugging
     */
    explicit ScriptObjectWrapper(std::weak_ptr<T> weakObj, const std::string& name = "")
        : weakPtr_(weakObj), name_(name), isValid_(true) {
    }
    
    /**
     * @brief Check if the wrapped object is still valid
     * 
     * @return true if object is valid, false otherwise
     */
    bool IsValid() const {
        return isValid_ && !weakPtr_.expired();
    }
    
    /**
     * @brief Get the wrapped object (throws if invalid)
     * 
     * @return Shared pointer to the object
     * @throws ScriptObjectException if object is invalid
     */
    std::shared_ptr<T> Get() const {
        if (!isValid_) {
            throw ScriptObjectException("Object wrapper has been invalidated: " + name_);
        }
        
        auto shared = weakPtr_.lock();
        if (!shared) {
            throw ScriptObjectException("Object has been destroyed: " + name_);
        }
        
        return shared;
    }
    
    /**
     * @brief Try to get the wrapped object (returns nullptr if invalid)
     * 
     * @return Shared pointer to the object, or nullptr if invalid
     */
    std::shared_ptr<T> TryGet() const {
        if (!isValid_) {
            return nullptr;
        }
        return weakPtr_.lock();
    }
    
    /**
     * @brief Invalidate this wrapper (called when plugin is unloaded)
     */
    void Invalidate() {
        isValid_ = false;
    }
    
    /**
     * @brief Get the name of this wrapper
     * 
     * @return Name string
     */
    const std::string& GetName() const {
        return name_;
    }
    
    /**
     * @brief Operator-> for direct access (throws if invalid)
     * 
     * @return Pointer to the object
     * @throws ScriptObjectException if object is invalid
     */
    T* operator->() const {
        return Get().get();
    }
    
    /**
     * @brief Operator* for direct access (throws if invalid)
     * 
     * @return Reference to the object
     * @throws ScriptObjectException if object is invalid
     */
    T& operator*() const {
        auto shared = Get();
        return *shared;
    }
    
    /**
     * @brief Boolean conversion operator
     * 
     * @return true if object is valid, false otherwise
     */
    explicit operator bool() const {
        return IsValid();
    }

private:
    std::weak_ptr<T> weakPtr_;  ///< Weak pointer to the wrapped object
    std::string name_;          ///< Name for debugging
    bool isValid_;              ///< Whether this wrapper is still valid
};

/**
 * @brief Manager for script object wrappers
 * 
 * This class manages the lifecycle of script object wrappers and provides
 * cleanup functionality when plugins are unloaded.
 */
class PLUGIN_CORE_API ScriptObjectManager {
public:
    /**
     * @brief Callback type for wrapper cleanup
     */
    using CleanupCallback = std::function<void()>;
    
    /**
     * @brief Register a cleanup callback for a plugin
     * 
     * @param pluginName Name of the plugin
     * @param callback Cleanup function to call when plugin is unloaded
     */
    void RegisterCleanupCallback(const std::string& pluginName, const CleanupCallback& callback);
    
    /**
     * @brief Clean up all wrappers for a specific plugin
     * 
     * @param pluginName Name of the plugin being unloaded
     */
    void CleanupPlugin(const std::string& pluginName);
    
    /**
     * @brief Clean up all wrappers
     */
    void CleanupAll();
    
    /**
     * @brief Get singleton instance
     * 
     * @return Reference to the singleton instance
     */
    static ScriptObjectManager& GetInstance();

private:
    std::unordered_map<std::string, std::vector<CleanupCallback>> cleanupCallbacks_;
    mutable std::mutex mutex_;
};

/**
 * @brief Helper function to create a script wrapper
 * 
 * @tparam T Type of object to wrap
 * @param obj Shared pointer to the object
 * @param name Optional name for debugging
 * @return ScriptObjectWrapper for the object
 */
template<typename T>
ScriptObjectWrapper<T> MakeScriptWrapper(std::shared_ptr<T> obj, const std::string& name = "") {
    return ScriptObjectWrapper<T>(obj, name);
}

/**
 * @brief Helper function to create a script wrapper from weak_ptr
 * 
 * @tparam T Type of object to wrap
 * @param weakObj Weak pointer to the object
 * @param name Optional name for debugging
 * @return ScriptObjectWrapper for the object
 */
template<typename T>
ScriptObjectWrapper<T> MakeScriptWrapper(std::weak_ptr<T> weakObj, const std::string& name = "") {
    return ScriptObjectWrapper<T>(weakObj, name);
}
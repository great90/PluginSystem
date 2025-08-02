/**
 * @file PluginInfo.h
 * @brief Defines the PluginInfo structure for plugin metadata
 */

#pragma once

#include <string>
#include <vector>
#include "PluginExport.h"

/**
 * @struct PluginInfo
 * @brief Contains metadata about a plugin including its name, version, and dependencies
 */
struct PLUGIN_CORE_API PluginInfo {
    /**
     * @brief Semantic version structure
     */
    struct Version {
        int major;      ///< Major version number (breaking changes)
        int minor;      ///< Minor version number (new features, non-breaking)
        int patch;      ///< Patch version number (bug fixes)
        
        /**
         * @brief Default constructor
         */
        Version() : major(0), minor(0), patch(0) {}
        
        /**
         * @brief Constructor with version components
         * @param maj Major version
         * @param min Minor version
         * @param pat Patch version
         */
        Version(int maj, int min, int pat) : major(maj), minor(min), patch(pat) {}
        
        /**
         * @brief Convert version to string format (e.g., "1.2.3")
         * @return String representation of the version
         */
        std::string ToString() const {
            return std::to_string(major) + "." + 
                   std::to_string(minor) + "." + 
                   std::to_string(patch);
        }
        
        /**
         * @brief Compare versions
         * @param other Version to compare with
         * @return true if this version is less than other
         */
        bool operator<(const Version& other) const {
            if (major != other.major) return major < other.major;
            if (minor != other.minor) return minor < other.minor;
            return patch < other.patch;
        }
        
        /**
         * @brief Check if versions are equal
         * @param other Version to compare with
         * @return true if versions are equal
         */
        bool operator==(const Version& other) const {
            return major == other.major && 
                   minor == other.minor && 
                   patch == other.patch;
        }
    };
    
    /**
     * @brief Dependency information structure
     */
    struct Dependency {
        std::string name;       ///< Name of the dependency plugin
        Version minVersion;    ///< Minimum required version
        bool optional;         ///< Whether this dependency is optional
        
        /**
         * @brief Constructor for required dependency
         * @param n Dependency name
         * @param v Minimum version required
         */
        Dependency(const std::string& n, const Version& v) 
            : name(n), minVersion(v), optional(false) {}
        
        /**
         * @brief Constructor with optional flag
         * @param n Dependency name
         * @param v Minimum version required
         * @param opt Whether dependency is optional
         */
        Dependency(const std::string& n, const Version& v, bool opt) 
            : name(n), minVersion(v), optional(opt) {}
    };
    
    std::string name;                  ///< Unique name of the plugin
    std::string displayName;           ///< Human-readable name
    std::string description;           ///< Description of plugin functionality
    Version version;                   ///< Plugin version
    std::string author;                ///< Plugin author
    std::vector<Dependency> dependencies; ///< List of plugin dependencies
    
    /**
     * @brief Default constructor
     */
    PluginInfo() = default;
    
    /**
     * @brief Constructor with basic plugin information
     * @param n Plugin name
     * @param dn Display name
     * @param desc Description
     * @param v Version
     * @param auth Author
     */
    PluginInfo(const std::string& n, 
               const std::string& dn, 
               const std::string& desc, 
               const Version& v, 
               const std::string& auth) 
        : name(n), displayName(dn), description(desc), 
          version(v), author(auth) {}
    
    /**
     * @brief Add a dependency to this plugin
     * @param dep Dependency to add
     */
    void AddDependency(const Dependency& dep) {
        dependencies.push_back(dep);
    }
};
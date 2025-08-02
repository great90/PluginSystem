/**
 * @file DependencyResolver.h
 * @brief Defines the DependencyResolver class for managing plugin dependencies
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "PluginExport.h"

/**
 * @class DependencyResolver
 * @brief Resolves dependencies between plugins and determines load order
 * 
 * This class is responsible for tracking dependencies between plugins,
 * detecting circular dependencies, and determining the correct order
 * in which plugins should be loaded.
 */
class PLUGIN_CORE_API DependencyResolver {
public:
    /**
     * @brief Default constructor
     */
    DependencyResolver() = default;
    
    /**
     * @brief Add a dependency relationship
     * 
     * @param plugin Name of the plugin
     * @param dependency Name of the plugin it depends on
     * @param optional Whether this dependency is optional
     */
    void AddDependency(const std::string& plugin, const std::string& dependency, bool optional = false);
    
    /**
     * @brief Get dependencies for a specific plugin
     * 
     * @param plugin Name of the plugin
     * @return Vector of dependency names
     */
    std::vector<std::string> GetDependencies(const std::string& plugin) const;
    
    /**
     * @brief Get optional dependencies for a specific plugin
     * 
     * @param plugin Name of the plugin
     * @return Vector of optional dependency names
     */
    std::vector<std::string> GetOptionalDependencies(const std::string& plugin) const;
    
    /**
     * @brief Resolve the order in which plugins should be loaded
     * 
     * @param plugins List of available plugin names
     * @return Ordered list of plugin names in the order they should be loaded
     * @throws std::runtime_error if circular dependencies are detected
     */
    std::vector<std::string> ResolveLoadOrder(const std::vector<std::string>& plugins) const;
    
    /**
     * @brief Check if there are circular dependencies
     * 
     * @return true if circular dependencies exist, false otherwise
     */
    bool HasCircularDependencies() const;
    
    /**
     * @brief Clear all dependency information
     */
    void Clear();

private:
    /**
     * @brief Perform depth-first search to detect cycles and build load order
     * 
     * @param node Current plugin being visited
     * @param visited Set of plugins that have been visited
     * @param temp Set of plugins in the current DFS path
     * @param order Vector to store the resulting load order
     * @param availablePlugins Set of available plugins
     * @return true if no cycles were detected, false otherwise
     */
    bool DFS(const std::string& node, 
             std::unordered_set<std::string>& visited,
             std::unordered_set<std::string>& temp,
             std::vector<std::string>& order,
             const std::unordered_set<std::string>& availablePlugins) const;

    // Maps plugin names to their dependencies
    std::unordered_map<std::string, std::vector<std::string>> dependencies_;
    
    // Maps plugin names to their optional dependencies
    std::unordered_map<std::string, std::vector<std::string>> optionalDependencies_;
};
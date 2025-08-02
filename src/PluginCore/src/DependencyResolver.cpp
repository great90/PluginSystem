/**
 * @file DependencyResolver.cpp
 * @brief Implementation of the DependencyResolver class
 */

#include "DependencyResolver.h"
#include <stdexcept>
#include <algorithm>

void DependencyResolver::AddDependency(const std::string& plugin, const std::string& dependency, bool optional) {
    // Add the plugin to our dependency map if it doesn't exist
    if (dependencies_.find(plugin) == dependencies_.end()) {
        dependencies_[plugin] = std::vector<std::string>();
        optionalDependencies_[plugin] = std::vector<std::string>();
    }
    
    // Add the dependency to the appropriate list
    if (optional) {
        optionalDependencies_[plugin].push_back(dependency);
    } else {
        dependencies_[plugin].push_back(dependency);
    }
}

std::vector<std::string> DependencyResolver::GetDependencies(const std::string& plugin) const {
    auto it = dependencies_.find(plugin);
    if (it != dependencies_.end()) {
        return it->second;
    }
    return std::vector<std::string>();
}

std::vector<std::string> DependencyResolver::GetOptionalDependencies(const std::string& plugin) const {
    auto it = optionalDependencies_.find(plugin);
    if (it != optionalDependencies_.end()) {
        return it->second;
    }
    return std::vector<std::string>();
}

std::vector<std::string> DependencyResolver::ResolveLoadOrder(const std::vector<std::string>& plugins) const {
    std::vector<std::string> order;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> temp;
    
    // Create a set of available plugins for quick lookup
    std::unordered_set<std::string> availablePlugins(plugins.begin(), plugins.end());
    
    // Perform topological sort using DFS
    for (const auto& plugin : plugins) {
        if (visited.find(plugin) == visited.end()) {
            if (!DFS(plugin, visited, temp, order, availablePlugins)) {
                throw std::runtime_error("Circular dependency detected involving plugin: " + plugin);
            }
        }
    }
    
    // Reverse the order to get the correct load sequence
    std::reverse(order.begin(), order.end());
    return order;
}

bool DependencyResolver::HasCircularDependencies() const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> temp;
    std::vector<std::string> order;
    
    // Create a set of all plugins
    std::unordered_set<std::string> allPlugins;
    for (const auto& pair : dependencies_) {
        allPlugins.insert(pair.first);
        for (const auto& dep : pair.second) {
            allPlugins.insert(dep);
        }
    }
    
    // Check for cycles
    for (const auto& plugin : allPlugins) {
        if (visited.find(plugin) == visited.end()) {
            if (!DFS(plugin, visited, temp, order, allPlugins)) {
                return true; // Cycle detected
            }
        }
    }
    
    return false; // No cycles
}

void DependencyResolver::Clear() {
    dependencies_.clear();
    optionalDependencies_.clear();
}

bool DependencyResolver::DFS(const std::string& node, 
                             std::unordered_set<std::string>& visited,
                             std::unordered_set<std::string>& temp,
                             std::vector<std::string>& order,
                             const std::unordered_set<std::string>& availablePlugins) const {
    // If node is in temp, we have a cycle
    if (temp.find(node) != temp.end()) {
        return false;
    }
    
    // If node is already visited, we're done with it
    if (visited.find(node) != visited.end()) {
        return true;
    }
    
    // Mark node as temporarily visited
    temp.insert(node);
    
    // Visit all dependencies
    auto depIt = dependencies_.find(node);
    if (depIt != dependencies_.end()) {
        for (const auto& dep : depIt->second) {
            // Only process dependency if it's in the available plugins
            if (availablePlugins.find(dep) != availablePlugins.end()) {
                if (!DFS(dep, visited, temp, order, availablePlugins)) {
                    return false; // Cycle detected
                }
            }
        }
    }
    
    // Visit all optional dependencies
    auto optDepIt = optionalDependencies_.find(node);
    if (optDepIt != optionalDependencies_.end()) {
        for (const auto& dep : optDepIt->second) {
            // Only process dependency if it's in the available plugins
            if (availablePlugins.find(dep) != availablePlugins.end()) {
                if (!DFS(dep, visited, temp, order, availablePlugins)) {
                    return false; // Cycle detected
                }
            }
        }
    }
    
    // Mark node as visited and add to order
    temp.erase(node);
    visited.insert(node);
    order.push_back(node);
    
    return true;
}
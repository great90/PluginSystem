# PluginSystem

A modular C++ plugin system that provides a flexible framework for extending applications through plugins.

## Features

- **Plugin Interface**: Defines a common interface for all plugins with lifecycle management, serialization, hot-reloading, and dependency management.
- **Plugin Manager**: Handles plugin loading, unloading, dependency resolution, and hot-reloading.
- **Standard Plugins**:
  - **Math Plugin**: A 3D math library providing vector, matrix, and quaternion operations.
  - **Log Plugin**: A logging system built on top of spdlog.
  - **Script Plugin**: Support for Python and Lua scripting languages.
- **Cross-Language Support**: Extend the system using C++, Python, or Lua.
- **Flexible Build System**: Build plugins as either static or dynamic libraries.

## Requirements

- C++17 compatible compiler
- CMake 3.14 or higher
- External dependencies (included as submodules):
  - spdlog
  - pybind11
  - Lua
  - GoogleTest

## Building

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/yourusername/PluginSystem.git
cd PluginSystem

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .
```

## Usage

### Creating a Plugin

```cpp
// MyPlugin.h
#include "IPlugin.h"

class MyPlugin : public IPlugin {
public:
    // Implement the IPlugin interface
    bool Initialize() override;
    void Shutdown() override;
    // ... other methods
    
    // Plugin-specific functionality
    void DoSomething();
    
    // Plugin info
    static PluginInfo GetPluginStaticInfo();
};

// Register the plugin
REGISTER_PLUGIN(MyPlugin)
```

### Using Plugins in an Application

```cpp
#include "PluginManager.h"
#include "MyPlugin.h"

int main() {
    // Initialize plugin manager
    auto& manager = PluginManager::Instance();
    
    // Load plugins
    manager.LoadPlugin("MyPlugin");
    manager.ResolveDependencies();
    manager.InitializeAll();
    
    // Use a plugin
    auto* myPlugin = manager.GetPlugin<MyPlugin>("MyPlugin");
    if (myPlugin) {
        myPlugin->DoSomething();
    }
    
    // Shutdown
    manager.ShutdownAll();
    
    return 0;
}
```

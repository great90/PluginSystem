# 插件开发模板

本文档提供了创建新插件的标准模板和指南。

## 目录结构

新插件应遵循以下目录结构：

```
src/Plugins/YourPlugin/
├── CMakeLists.txt
├── include/
│   ├── YourPlugin.h
│   └── YourPluginExport.h
└── src/
    └── YourPlugin.cpp
```

## CMakeLists.txt 模板

```cmake
# src/Plugins/YourPlugin/CMakeLists.txt

# Support standalone build
if(NOT TARGET PluginCore)
    cmake_minimum_required(VERSION 3.16)
    project(YourPlugin VERSION 1.0.0 LANGUAGES CXX)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Find PluginCore
    find_package(PluginCore REQUIRED)
endif()

# Define source files
set(YOUR_PLUGIN_SOURCES
    src/YourPlugin.cpp
)

# Define header files
set(YOUR_PLUGIN_HEADERS
    include/YourPlugin.h
    include/YourPluginExport.h
)

# Create library target
if(BUILD_SHARED_LIBS)
    add_library(YourPlugin SHARED ${YOUR_PLUGIN_SOURCES} ${YOUR_PLUGIN_HEADERS})
    target_compile_definitions(YourPlugin PRIVATE YOUR_PLUGIN_EXPORTS)
    target_compile_definitions(YourPlugin PUBLIC YOUR_PLUGIN_SHARED)
else()
    add_library(YourPlugin STATIC ${YOUR_PLUGIN_SOURCES} ${YOUR_PLUGIN_HEADERS})
endif()

# Set include directories
target_include_directories(YourPlugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(YourPlugin PRIVATE 
    PluginCore
    # Add other dependencies here
)

# Installation rules
install(TARGETS YourPlugin
    EXPORT YourPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${YOUR_PLUGIN_HEADERS} DESTINATION include/YourPlugin)
```

## 头文件模板

### YourPluginExport.h

```cpp
#pragma once

#ifdef YOUR_PLUGIN_SHARED
    #ifdef YOUR_PLUGIN_EXPORTS
        #define YOUR_PLUGIN_API __declspec(dllexport)
    #else
        #define YOUR_PLUGIN_API __declspec(dllimport)
    #endif
#else
    #define YOUR_PLUGIN_API
#endif
```

### YourPlugin.h

```cpp
#pragma once

#include "YourPluginExport.h"
#include "PluginCore/IPlugin.h"

class YOUR_PLUGIN_API YourPlugin : public IPlugin {
public:
    YourPlugin();
    virtual ~YourPlugin();

    // IPlugin interface
    bool initialize() override;
    void shutdown() override;
    const char* getName() const override;
    const char* getVersion() const override;

    // Plugin-specific methods
    void yourMethod();

private:
    bool m_initialized;
};

// Plugin factory function
extern "C" YOUR_PLUGIN_API IPlugin* createPlugin();
extern "C" YOUR_PLUGIN_API void destroyPlugin(IPlugin* plugin);
```

## 源文件模板

### YourPlugin.cpp

```cpp
#include "YourPlugin.h"
#include <iostream>

YourPlugin::YourPlugin() : m_initialized(false) {
}

YourPlugin::~YourPlugin() {
    if (m_initialized) {
        shutdown();
    }
}

bool YourPlugin::initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize your plugin here
    std::cout << "Initializing " << getName() << " v" << getVersion() << std::endl;
    
    m_initialized = true;
    return true;
}

void YourPlugin::shutdown() {
    if (!m_initialized) {
        return;
    }

    // Cleanup your plugin here
    std::cout << "Shutting down " << getName() << std::endl;
    
    m_initialized = false;
}

const char* YourPlugin::getName() const {
    return "YourPlugin";
}

const char* YourPlugin::getVersion() const {
    return "1.0.0";
}

void YourPlugin::yourMethod() {
    if (!m_initialized) {
        std::cerr << "Plugin not initialized!" << std::endl;
        return;
    }

    // Implement your plugin functionality here
    std::cout << "YourPlugin method called" << std::endl;
}

// Plugin factory functions
extern "C" YOUR_PLUGIN_API IPlugin* createPlugin() {
    return new YourPlugin();
}

extern "C" YOUR_PLUGIN_API void destroyPlugin(IPlugin* plugin) {
    delete plugin;
}
```

## 构建指南

### 1. 完整构建

```bash
# 构建所有插件
cmake -B build
cmake --build build

# 或者只构建特定插件
cmake -B build -DBUILD_YOURPLUGIN=ON -DBUILD_ALL_PLUGINS=OFF
cmake --build build
```

### 2. 独立构建

```bash
# 在插件目录中独立构建
cd src/Plugins/YourPlugin
cmake -B build
cmake --build build
```

### 3. 测试构建

```bash
# 构建包含新插件的测试
cmake -B build -DBUILD_YOURPLUGIN=ON -DBUILD_PLUGIN_TESTS=ON
cmake --build build
./build/tests/PluginSystemTests
```

## 最佳实践

1. **命名约定**：使用 PascalCase 命名插件类和文件
2. **错误处理**：在所有公共方法中添加适当的错误检查
3. **资源管理**：确保在 shutdown() 中正确清理所有资源
4. **线程安全**：如果插件需要多线程支持，确保线程安全
5. **文档**：为所有公共接口添加详细的文档注释
6. **测试**：为插件功能编写单元测试

## 集成到构建系统

新插件会被自动发现并集成到构建系统中，无需手动修改 CMakeLists.txt 文件。插件发现机制会：

1. 扫描 `src/Plugins/` 目录
2. 为每个发现的插件创建构建选项
3. 根据构建选项条件性地编译插件
4. 自动配置测试系统链接

## 故障排除

### 常见问题

1. **链接错误**：确保 PluginCore 已正确安装或在构建路径中
2. **符号导出问题**：检查 Export 宏定义是否正确
3. **依赖问题**：确保所有外部依赖都已正确配置

### 调试技巧

1. 使用 `CMAKE_VERBOSE_MAKEFILE=ON` 查看详细构建信息
2. 检查 CMake 配置输出中的插件发现信息
3. 使用调试器逐步调试插件初始化过程
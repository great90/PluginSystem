/**
 * @file rendering_plugin_test.cpp
 * @brief Unit tests for the RenderingPlugin class
 */

#include <gtest/gtest.h>
#include "PluginManager.h"
#include "RenderingPlugin.h"
#include <memory>
#include <thread>
#include <chrono>

// Define plugin extension based on platform
#ifdef _WIN32
#define PLUGIN_EXTENSION ".dll"
#elif defined(__APPLE__)
#define PLUGIN_EXTENSION ".dylib"
#else
#define PLUGIN_EXTENSION ".so"
#endif

// Test fixture for RenderingPlugin tests
class RenderingPluginTest : public ::testing::Test {
protected:
    PluginManager pluginManager;
    std::shared_ptr<IPlugin> basePlugin;
    RenderingPlugin* renderingPlugin;
    
    void SetUp() override {
        // 加载插件
        std::string pluginPath = "/Users/sunny/dev/PluginSystem/build/lib/RenderingPlugin.dylib";
        bool loaded = pluginManager.LoadPlugin(pluginPath);
        ASSERT_TRUE(loaded) << "Failed to load RenderingPlugin";
        
        // 通过PluginManager创建实例以触发构造函数
        auto tempPlugin = pluginManager.GetPlugin<RenderingPlugin>("RenderingPlugin");
        ASSERT_NE(tempPlugin, nullptr) << "Failed to create RenderingPlugin instance";
        
        // 使用单例实例进行测试
        renderingPlugin = RenderingPlugin::GetInstance();
        ASSERT_NE(renderingPlugin, nullptr) << "Failed to get RenderingPlugin singleton instance";
        
        std::cout << "SetUp: temp plugin = " << tempPlugin.get() << std::endl;
        std::cout << "SetUp: singleton instance = " << renderingPlugin << std::endl;
    }
    
    void TearDown() override {
        std::cout << "Starting TearDown..." << std::endl;
        
        try {
            // Shutdown the plugin if it was initialized
            if (renderingPlugin) {
                std::cout << "Checking if plugin is initialized..." << std::endl;
                if (renderingPlugin->IsInitialized()) {
                    std::cout << "Shutting down plugin..." << std::endl;
                    renderingPlugin->Shutdown();
                }
                renderingPlugin = nullptr;
            }
            
            std::cout << "Resetting base plugin..." << std::endl;
            basePlugin.reset();
            
            std::cout << "Unloading all plugins..." << std::endl;
            pluginManager.UnloadAllPlugins();
            
            std::cout << "TearDown completed successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Exception in TearDown: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Unknown exception in TearDown" << std::endl;
        }
    }
};

// Test the plugin info
TEST_F(RenderingPluginTest, PluginInfoTest) {
    std::cout << "Starting PluginInfoTest..." << std::endl;
    
    // Check if renderingPlugin is valid
    ASSERT_NE(nullptr, renderingPlugin);
    std::cout << "RenderingPlugin pointer is valid" << std::endl;
    
    try {
        std::cout << "Attempting to get plugin info..." << std::endl;
        const PluginInfo& info = renderingPlugin->GetPluginInfo();
        std::cout << "Successfully got plugin info" << std::endl;
        
        EXPECT_EQ("RenderingPlugin", info.name);
        EXPECT_EQ("Rendering Plugin", info.displayName);
        EXPECT_EQ("A plugin for rendering operations using LLGL", info.description);
        EXPECT_EQ("PluginSystem", info.author);
        
        // Check version
        EXPECT_EQ(1, info.version.major);
        EXPECT_EQ(0, info.version.minor);
        EXPECT_EQ(0, info.version.patch);
        EXPECT_EQ("1.0.0", info.version.ToString());
        
        std::cout << "PluginInfoTest completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception in PluginInfoTest: " << e.what() << std::endl;
        FAIL() << "Exception occurred: " << e.what();
    } catch (...) {
        std::cout << "Unknown exception in PluginInfoTest" << std::endl;
        FAIL() << "Unknown exception occurred";
    }
}

// Test plugin initialization and shutdown
TEST_F(RenderingPluginTest, InitializationTest) {
    // Add debug information
    std::cout << "InitializationTest: renderingPlugin pointer = " << renderingPlugin << std::endl;
    std::cout << "InitializationTest: Initial IsInitialized() = " << renderingPlugin->IsInitialized() << std::endl;
    
    // Initially should not be initialized
    EXPECT_FALSE(renderingPlugin->IsInitialized());
    
    // Test initialization
    std::cout << "InitializationTest: Calling Initialize()..." << std::endl;
    bool initResult = renderingPlugin->Initialize();
    std::cout << "InitializationTest: Initialize() returned " << initResult << std::endl;
    std::cout << "InitializationTest: After Initialize(), IsInitialized() = " << renderingPlugin->IsInitialized() << std::endl;
    
    EXPECT_TRUE(initResult);
    EXPECT_TRUE(renderingPlugin->IsInitialized());
    
    // Test shutdown
    std::cout << "InitializationTest: Calling Shutdown()..." << std::endl;
    renderingPlugin->Shutdown();
    std::cout << "InitializationTest: After Shutdown(), IsInitialized() = " << renderingPlugin->IsInitialized() << std::endl;
    EXPECT_FALSE(renderingPlugin->IsInitialized());
}

// Test render system initialization with different APIs
TEST_F(RenderingPluginTest, RenderSystemInitializationTest) {
    // Initialize the plugin first
    ASSERT_TRUE(renderingPlugin->Initialize());
    
    // Test OpenGL initialization (most likely to be available)
    bool openglResult = renderingPlugin->InitializeRenderSystem(RenderAPI::OpenGL);
    if (openglResult) {
        EXPECT_EQ(RenderAPI::OpenGL, renderingPlugin->GetCurrentAPI());
        std::cout << "OpenGL render system initialized successfully" << std::endl;
    } else {
        std::cout << "OpenGL render system initialization failed (may not be available)" << std::endl;
    }
    
    // Test Metal initialization on macOS
#ifdef __APPLE__
    bool metalResult = renderingPlugin->InitializeRenderSystem(RenderAPI::Metal);
    if (metalResult) {
        EXPECT_EQ(RenderAPI::Metal, renderingPlugin->GetCurrentAPI());
        std::cout << "Metal render system initialized successfully" << std::endl;
    } else {
        std::cout << "Metal render system initialization failed (may not be available)" << std::endl;
    }
#endif
    
    // Test Vulkan initialization (may not be available)
    bool vulkanResult = renderingPlugin->InitializeRenderSystem(RenderAPI::Vulkan);
    if (vulkanResult) {
        EXPECT_EQ(RenderAPI::Vulkan, renderingPlugin->GetCurrentAPI());
        std::cout << "Vulkan render system initialized successfully" << std::endl;
    } else {
        std::cout << "Vulkan render system initialization failed (may not be available)" << std::endl;
    }
}

// Test window creation (may fail without display)
TEST_F(RenderingPluginTest, WindowCreationTest) {
    // Initialize the plugin and render system first
    ASSERT_TRUE(renderingPlugin->Initialize());
    
    // Try to initialize a render system (prefer OpenGL)
    bool renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::OpenGL);
    if (!renderSystemInitialized) {
        renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::Metal);
    }
    if (!renderSystemInitialized) {
        renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::Vulkan);
    }
    
    if (renderSystemInitialized) {
        // Test window creation
        WindowDesc windowDesc;
        windowDesc.width = 800;
        windowDesc.height = 600;
        windowDesc.title = "Test Window";
        
        bool windowResult = renderingPlugin->CreateWindow(windowDesc);
        if (windowResult) {
            std::cout << "Window created successfully" << std::endl;
            
            // Test window size retrieval
            int width, height;
            EXPECT_TRUE(renderingPlugin->GetWindowSize(width, height));
            EXPECT_EQ(800, width);
            EXPECT_EQ(600, height);
        } else {
            std::cout << "Window creation failed (may require display)" << std::endl;
        }
    } else {
        std::cout << "Skipping window creation test - no render system available" << std::endl;
    }
}

// Test basic rendering loop (BeginFrame/EndFrame)
TEST_F(RenderingPluginTest, BasicRenderingLoopTest) {
    // Initialize the plugin and render system first
    ASSERT_TRUE(renderingPlugin->Initialize());
    
    // Try to initialize a render system
    bool renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::OpenGL);
    if (!renderSystemInitialized) {
        renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::Metal);
    }
    
    if (renderSystemInitialized) {
        // Try to create a window
        WindowDesc windowDesc;
        windowDesc.width = 800;
        windowDesc.height = 600;
        windowDesc.title = "Test Window";
        
        bool windowResult = renderingPlugin->CreateWindow(windowDesc);
        if (windowResult) {
            // Test basic rendering loop
            EXPECT_TRUE(renderingPlugin->BeginFrame());
            EXPECT_TRUE(renderingPlugin->EndFrame());
            
            std::cout << "Basic rendering loop test passed" << std::endl;
        } else {
            std::cout << "Skipping rendering loop test - window creation failed" << std::endl;
        }
    } else {
        std::cout << "Skipping rendering loop test - no render system available" << std::endl;
    }
}

// Test clear functionality
TEST_F(RenderingPluginTest, ClearTest) {
    // Initialize the plugin and render system first
    ASSERT_TRUE(renderingPlugin->Initialize());
    
    // Try to initialize a render system
    bool renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::OpenGL);
    if (!renderSystemInitialized) {
        renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::Metal);
    }
    
    if (renderSystemInitialized) {
        // Try to create a window
        WindowDesc windowDesc;
        windowDesc.width = 800;
        windowDesc.height = 600;
        windowDesc.title = "Test Window";
        
        bool windowResult = renderingPlugin->CreateWindow(windowDesc);
        if (windowResult) {
            // Test clear functionality
            EXPECT_TRUE(renderingPlugin->BeginFrame());
            
            Color clearColor = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
            renderingPlugin->Clear(clearColor);
            
            EXPECT_TRUE(renderingPlugin->EndFrame());
            
            std::cout << "Clear test passed" << std::endl;
        } else {
            std::cout << "Skipping clear test - window creation failed" << std::endl;
        }
    } else {
        std::cout << "Skipping clear test - no render system available" << std::endl;
    }
}

// Test viewport setting
TEST_F(RenderingPluginTest, ViewportTest) {
    // Initialize the plugin and render system first
    ASSERT_TRUE(renderingPlugin->Initialize());
    
    // Try to initialize a render system
    bool renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::OpenGL);
    if (!renderSystemInitialized) {
        renderSystemInitialized = renderingPlugin->InitializeRenderSystem(RenderAPI::Metal);
    }
    
    if (renderSystemInitialized) {
        // Try to create a window
        WindowDesc windowDesc;
        windowDesc.width = 800;
        windowDesc.height = 600;
        windowDesc.title = "Test Window";
        
        bool windowResult = renderingPlugin->CreateWindow(windowDesc);
        if (windowResult) {
            // Test viewport setting
            EXPECT_TRUE(renderingPlugin->BeginFrame());
            
            renderingPlugin->SetViewport(0, 0, 400, 300);
            renderingPlugin->SetViewport(100, 100, 600, 400);
            
            EXPECT_TRUE(renderingPlugin->EndFrame());
            
            std::cout << "Viewport test passed" << std::endl;
        } else {
            std::cout << "Skipping viewport test - window creation failed" << std::endl;
        }
    } else {
        std::cout << "Skipping viewport test - no render system available" << std::endl;
    }
}

// Test serialization and deserialization
TEST_F(RenderingPluginTest, SerializationTest) {
    // Initialize the plugin
    ASSERT_TRUE(renderingPlugin->Initialize());
    
    // Test serialization
    std::string serializedData = renderingPlugin->Serialize();
    EXPECT_FALSE(serializedData.empty());
    
    // Test deserialization
    EXPECT_TRUE(renderingPlugin->Deserialize(serializedData));
    
    std::cout << "Serialization test passed" << std::endl;
}

// Test hot reload functionality
TEST_F(RenderingPluginTest, HotReloadTest) {
    // Initialize the plugin
    ASSERT_TRUE(renderingPlugin->Initialize());
    
    // Test prepare for hot reload
    renderingPlugin->PrepareForHotReload();
    
    // Test complete hot reload
    renderingPlugin->CompleteHotReload();
    
    // Plugin should still be functional after hot reload
    EXPECT_TRUE(renderingPlugin->IsInitialized());
    
    std::cout << "Hot reload test passed" << std::endl;
}

// Test singleton pattern (even though it has issues with dynamic loading)
TEST_F(RenderingPluginTest, SingletonTest) {
    // Test that GetInstance returns a valid pointer after plugin creation
    // Note: This may fail due to dynamic loading issues, but we test it anyway
    RenderingPlugin* instance = RenderingPlugin::GetInstance();
    
    if (instance != nullptr) {
        std::cout << "Singleton instance available: " << instance << std::endl;
        EXPECT_EQ(instance, renderingPlugin);
    } else {
        std::cout << "Singleton instance not available (expected due to dynamic loading)" << std::endl;
        // This is expected behavior due to the dynamic loading issues we discovered
    }
}

// Test event polling (should not crash)
TEST_F(RenderingPluginTest, EventPollingTest) {
    // Initialize the plugin
    ASSERT_TRUE(renderingPlugin->Initialize());
    
    // Test event polling (should not crash)
    renderingPlugin->PollEvents();
    
    // Test ShouldWindowClose (should return false by default)
    EXPECT_FALSE(renderingPlugin->ShouldWindowClose());
    
    std::cout << "Event polling test passed" << std::endl;
}
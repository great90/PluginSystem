/**
 * @file RenderingPlugin.h
 * @brief Defines the RenderingPlugin class for rendering functionality
 */

#pragma once

#include "IPlugin.h"
#include "RenderingPluginExport.h"
#include <string>
#include <memory>
#include <vector>

// Forward declarations for LLGL
namespace LLGL {
    class RenderSystem;
    class SwapChain;
    class CommandBuffer;
    class RenderContext;
    class Surface;
    class Buffer;
    class Texture;
    class Shader;
    class PipelineState;
    class RenderTarget;
    struct Extent2D;
    struct Viewport;
    struct ClearValue;
}

/**
 * @enum RenderAPI
 * @brief Defines the available rendering APIs
 */
enum class RenderAPI {
    None,       ///< No rendering API selected
    OpenGL,     ///< OpenGL rendering API
    Vulkan,     ///< Vulkan rendering API
    Direct3D11, ///< Direct3D 11 rendering API
    Direct3D12, ///< Direct3D 12 rendering API
    Metal       ///< Metal rendering API
};

/**
 * @struct WindowDesc
 * @brief Describes window properties for rendering
 */
struct WindowDesc {
    std::string title = "Rendering Window";  ///< Window title
    int width = 800;                         ///< Window width
    int height = 600;                        ///< Window height
    bool fullscreen = false;                 ///< Fullscreen mode
    bool resizable = true;                   ///< Resizable window
    bool vsync = true;                       ///< Vertical synchronization
};

/**
 * @struct Color
 * @brief Represents an RGBA color
 */
struct Color {
    float r, g, b, a;
    
    Color(float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 1.0f)
        : r(red), g(green), b(blue), a(alpha) {}
};

/**
 * @struct SystemGraphicsInfo
 * @brief Contains system graphics capability information
 */
struct SystemGraphicsInfo {
    bool hasDisplay = false;              ///< Whether a display is available
    bool isHeadless = false;              ///< Whether running in headless mode
    std::string osVersion;                ///< Operating system version
    std::string graphicsCard;             ///< Graphics card information
    std::vector<RenderAPI> supportedAPIs; ///< List of potentially supported APIs
    bool metalSupported = false;          ///< Metal framework availability
    bool openglSupported = false;         ///< OpenGL support
    bool vulkanSupported = false;         ///< Vulkan support
    std::string errorDetails;             ///< Detailed error information
};

/**
 * @enum RenderingMode
 * @brief Defines the rendering mode options
 */
enum class RenderingMode {
    Hardware,    ///< Hardware-accelerated rendering
    Software,    ///< Software rendering fallback
    Headless     ///< Headless rendering (no display)
};

/**
 * @class RenderingPlugin
 * @brief Plugin providing rendering functionality using LLGL
 * 
 * This plugin provides rendering capabilities using the LLGL library.
 * It supports multiple rendering APIs and provides basic rendering operations.
 */
class RENDERING_PLUGIN_API RenderingPlugin : public IPlugin {
public:
    /**
     * @brief Constructor
     */
    RenderingPlugin();
    
    /**
     * @brief Destructor
     */
    virtual ~RenderingPlugin();
    
    // IPlugin interface implementation
    bool Initialize() override;
    void Shutdown() override;
    const PluginInfo& GetPluginInfo() const override;
    std::string Serialize() override;
    bool Deserialize(const std::string& data) override;
    bool PrepareForHotReload() override;
    bool CompleteHotReload() override;
    
    /**
     * @brief Get the singleton instance of the RenderingPlugin
     * 
     * @return Pointer to the RenderingPlugin instance
     */
    static RenderingPlugin* GetInstance();
    
    /**
     * @brief Get static plugin information
     * 
     * @return Static plugin information
     */
    static const PluginInfo& GetPluginStaticInfo() {
        return pluginInfo_;
    }
    
    /**
     * @brief Set the singleton instance of RenderingPlugin
     * 
     * @param instance Pointer to the instance to set as singleton
     */
    static void SetInstance(RenderingPlugin* instance);
    
    /**
     * @brief Initialize the render system with specified API
     * 
     * @param api The rendering API to use
     * @return true if initialization was successful, false otherwise
     */
    bool InitializeRenderSystem(RenderAPI api = RenderAPI::OpenGL);
    
    /**
     * @brief Create a rendering window
     * 
     * @param desc Window description
     * @return true if window creation was successful, false otherwise
     */
    bool CreateWindow(const WindowDesc& desc);
    
    /**
     * @brief Begin a new frame for rendering
     * 
     * @return true if frame begin was successful, false otherwise
     */
    bool BeginFrame();
    
    /**
     * @brief End the current frame and present it
     * 
     * @return true if frame end was successful, false otherwise
     */
    bool EndFrame();
    
    /**
     * @brief Clear the current render target
     * 
     * @param color Clear color
     */
    void Clear(const Color& color = Color(0.0f, 0.0f, 0.0f, 1.0f));
    
    /**
     * @brief Set the viewport for rendering
     * 
     * @param x X coordinate of the viewport
     * @param y Y coordinate of the viewport
     * @param width Width of the viewport
     * @param height Height of the viewport
     */
    void SetViewport(int x, int y, int width, int height);
    
    /**
     * @brief Check if the rendering system is initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const;
    
    /**
     * @brief Get the current rendering API
     * 
     * @return The current rendering API
     */
    RenderAPI GetCurrentAPI() const;
    
    /**
     * @brief Get the window size
     * 
     * @param width Output parameter for width
     * @param height Output parameter for height
     * @return true if successful, false otherwise
     */
    bool GetWindowSize(int& width, int& height) const;
    
    /**
     * @brief Check if the window should close
     * 
     * @return true if window should close, false otherwise
     */
    bool ShouldWindowClose() const;
    
    /**
     * @brief Poll window events
     */
    void PollEvents();
    
    /**
     * @brief Detect system graphics capabilities
     * 
     * @return SystemGraphicsInfo containing detailed system information
     */
    static SystemGraphicsInfo DetectSystemGraphics();
    
    /**
     * @brief Check if running in headless environment
     * 
     * @return true if headless, false otherwise
     */
    static bool IsHeadlessEnvironment();
    
    /**
     * @brief Get detailed macOS graphics information
     * 
     * @return String containing macOS-specific graphics details
     */
    static std::string GetMacOSGraphicsInfo();
    
    /**
     * @brief Initialize with software rendering fallback
     * 
     * @param mode The rendering mode to use
     * @return true if initialization was successful, false otherwise
     */
    bool InitializeWithFallback(RenderingMode mode = RenderingMode::Hardware);
    
    /**
     * @brief Get current rendering mode
     * 
     * @return The current rendering mode
     */
    RenderingMode GetRenderingMode() const;
    
    /**
     * @brief Initialize software renderer
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool InitializeSoftwareRenderer();
    
    /**
     * @brief Initialize headless renderer
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool InitializeHeadlessRenderer();
    
    /**
     * @brief Create offscreen buffer for headless rendering
     * 
     * @param width Width of the offscreen buffer
     * @param height Height of the offscreen buffer
     * @return true if creation was successful, false otherwise
     */
    bool CreateOffscreenBuffer(int width, int height);
    
    /**
     * @brief Check if software rendering is enabled
     * 
     * @return true if software rendering is enabled, false otherwise
     */
    bool IsSoftwareRenderingEnabled() const;
    
    /**
     * @brief Check if running in headless mode
     * 
     * @return true if in headless mode, false otherwise
     */
    bool IsHeadlessMode() const;
    
public:
    // Plugin info (made public for export function access)
    static PluginInfo pluginInfo_;
    
private:
    // Static instance for singleton pattern
    static RenderingPlugin* instance_;
    
    // LLGL objects
    LLGL::RenderSystem* renderSystem_;
    std::unique_ptr<LLGL::SwapChain> swapChain_;
    LLGL::CommandBuffer* commandBuffer_;
    LLGL::Surface* surface_;
    
    // State
    bool initialized_;
    RenderAPI currentAPI_;
    WindowDesc windowDesc_;
    RenderingMode currentMode_;
    SystemGraphicsInfo systemInfo_;
    bool softwareRenderingEnabled_;
    
    // Offscreen rendering support
    LLGL::RenderTarget* offscreenRenderTarget_;
    
    /**
     * @brief Convert RenderAPI enum to LLGL module name
     * 
     * @param api The rendering API
     * @return Module name string
     */
    std::string GetModuleName(RenderAPI api) const;
    
    /**
     * @brief Try to initialize a specific graphics API
     * 
     * @param api The rendering API to try
     * @param errorMessage Output parameter for detailed error message
     * @return true if initialization was successful, false otherwise
     */
    bool TryInitializeAPI(RenderAPI api, std::string& errorMessage);
};

// Plugin export functions
extern "C" {
    /**
     * @brief Create an instance of the RenderingPlugin
     * 
     * @return Pointer to the created plugin instance
     */
    RENDERING_PLUGIN_API IPlugin* CreatePlugin();
    
    /**
     * @brief Get plugin information without creating an instance
     * 
     * @return Pointer to the plugin information
     */
    RENDERING_PLUGIN_API const PluginInfo* GetPluginInfo();
}
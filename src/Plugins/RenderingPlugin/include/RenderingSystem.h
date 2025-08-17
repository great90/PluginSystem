/**
 * @file RenderingSystem.h
 * @brief Core rendering system management class
 * @details Manages LLGL render system initialization, window creation, and frame operations
 */

#pragma once

#include "RenderingPluginExport.h"
#include <LLGL/LLGL.h>
#include <memory>
#include <string>

namespace RenderingPlugin {

/**
 * @brief Enumeration of supported rendering APIs
 */
enum class RenderAPI {
    None = 0,
    OpenGL,
    Vulkan,
    Direct3D11,
    Direct3D12,
    Metal
};

/**
 * @brief Enumeration of rendering modes
 */
enum class RenderingMode {
    Hardware = 0,
    Software,
    Headless
};

/**
 * @brief Window description structure
 */
struct WindowDesc {
    int width = 800;
    int height = 600;
    std::string title = "Rendering Window";
    bool resizable = true;
    bool fullscreen = false;
};

/**
 * @brief Color structure for clear operations
 */
struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
    
    Color() = default;
    Color(float red, float green, float blue, float alpha = 1.0f)
        : r(red), g(green), b(blue), a(alpha) {}
};

/**
 * @brief System graphics information
 */
struct SystemGraphicsInfo {
    bool isHeadless = false;
    std::string osVersion;
    std::string deviceName;
    std::string vendorName;
    std::string rendererName;
    std::string shadingLanguageName;
    std::string driverVersion;
    bool supportsVulkan = false;
    bool supportsOpenGL = false;
    bool supportsMetal = false;
    bool supportsDirectX = false;
};

/**
 * @brief Core rendering system management class
 * @details Handles LLGL render system initialization, API selection, window creation, and frame operations
 */
class RENDERING_PLUGIN_API RenderingSystem {
public:
    /**
     * @brief Constructor
     */
    RenderingSystem();
    
    /**
     * @brief Destructor
     */
    ~RenderingSystem();
    
    // === Initialization and Shutdown ===
    
    /**
     * @brief Initialize the rendering system with specified API
     * @param api The rendering API to use
     * @return true if initialization was successful, false otherwise
     */
    bool Initialize(RenderAPI api = RenderAPI::None);
    
    /**
     * @brief Initialize with automatic fallback to available APIs
     * @param preferredAPI The preferred API to try first
     * @return true if initialization was successful, false otherwise
     */
    bool InitializeWithFallback(RenderAPI preferredAPI = RenderAPI::None);
    
    /**
     * @brief Initialize software rendering mode
     * @return true if software rendering was enabled, false otherwise
     */
    bool InitializeSoftwareRenderer();
    
    /**
     * @brief Initialize headless rendering mode
     * @return true if headless rendering was enabled, false otherwise
     */
    bool InitializeHeadlessRenderer();
    
    /**
     * @brief Shutdown the rendering system
     */
    void Shutdown();
    
    // === Window Management ===
    
    /**
     * @brief Create a rendering window
     * @param desc Window description
     * @return true if window was created successfully, false otherwise
     */
    bool CreateWindow(const WindowDesc& desc);
    
    /**
     * @brief Get current window size
     * @param width Output parameter for window width
     * @param height Output parameter for window height
     * @return true if window size was retrieved successfully, false otherwise
     */
    bool GetWindowSize(int& width, int& height) const;
    
    /**
     * @brief Check if window should close
     * @return true if window should close, false otherwise
     */
    bool ShouldWindowClose() const;
    
    /**
     * @brief Poll window events
     */
    void PollEvents();
    
    // === Frame Operations ===
    
    /**
     * @brief Begin a new frame
     * @return true if frame was started successfully, false otherwise
     */
    bool BeginFrame();
    
    /**
     * @brief End the current frame and present
     * @return true if frame was ended successfully, false otherwise
     */
    bool EndFrame();
    
    /**
     * @brief Clear the frame buffer
     * @param color Clear color
     */
    void Clear(const Color& color = Color(0.1f, 0.1f, 0.2f, 1.0f));
    
    /**
     * @brief Set viewport
     * @param x X coordinate
     * @param y Y coordinate
     * @param width Viewport width
     * @param height Viewport height
     */
    void SetViewport(int x, int y, int width, int height);
    
    // === System Information ===
    
    /**
     * @brief Check if rendering system is initialized
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const;
    
    /**
     * @brief Get current rendering API
     * @return Current rendering API
     */
    RenderAPI GetCurrentAPI() const;
    
    /**
     * @brief Get current rendering mode
     * @return Current rendering mode
     */
    RenderingMode GetCurrentMode() const;
    
    /**
     * @brief Detect system graphics capabilities
     * @return System graphics information
     */
    SystemGraphicsInfo DetectSystemGraphics();
    
    /**
     * @brief Check if running in headless environment
     * @return true if headless, false otherwise
     */
    bool IsHeadlessEnvironment() const;
    
    // === LLGL Access ===
    
    /**
     * @brief Get the underlying LLGL render system
     * @return Pointer to LLGL render system, or nullptr if not initialized
     */
    LLGL::RenderSystem* GetRenderSystem() const;
    
    /**
     * @brief Get the swap chain
     * @return Pointer to swap chain, or nullptr if not available
     */
    LLGL::SwapChain* GetSwapChain() const;
    
    /**
     * @brief Get the command buffer
     * @return Pointer to command buffer, or nullptr if not available
     */
    LLGL::CommandBuffer* GetCommandBuffer() const;
    
    /**
     * @brief Get the surface
     * @return Pointer to surface, or nullptr if not available
     */
    LLGL::Surface* GetSurface() const;

private:
    // === Private Methods ===
    
    /**
     * @brief Convert RenderAPI enum to LLGL module name
     * @param api The rendering API
     * @return Module name string
     */
    std::string GetModuleName(RenderAPI api) const;
    
    /**
     * @brief Get API name string
     * @param api The rendering API
     * @return API name string
     */
    std::string GetAPIName(RenderAPI api) const;
    
    /**
     * @brief Try to initialize a specific graphics API
     * @param api The rendering API to try
     * @param errorMessage Output parameter for detailed error message
     * @return true if initialization was successful, false otherwise
     */
    bool TryInitializeAPI(RenderAPI api, std::string& errorMessage);
    
    /**
     * @brief Get macOS-specific graphics information
     * @return macOS graphics information string
     */
    std::string GetMacOSGraphicsInfo() const;
    
    // === Private Members ===
    
    // LLGL objects
    std::unique_ptr<LLGL::RenderSystem, LLGL::RenderSystemDeleter> renderSystem_;
    std::unique_ptr<LLGL::SwapChain> swapChain_;
    LLGL::CommandBuffer* commandBuffer_;
    LLGL::Surface* surface_;
    std::unique_ptr<LLGL::Window> window_;
    
    // State
    bool initialized_;
    RenderAPI currentAPI_;
    RenderingMode currentMode_;
    WindowDesc windowDesc_;
    SystemGraphicsInfo systemInfo_;
    bool softwareRenderingEnabled_;
    
    // Offscreen rendering support
    LLGL::RenderTarget* offscreenRenderTarget_;
};

} // namespace RenderingPlugin
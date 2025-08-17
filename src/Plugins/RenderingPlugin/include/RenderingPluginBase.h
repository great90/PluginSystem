/**
 * @file RenderingPluginBase.h
 * @brief Abstract base class for rendering plugins
 * 
 * This file defines the abstract interface for rendering plugins, providing
 * a common API that can be implemented by different rendering backends.
 * This design follows the same pattern as ScriptPlugin for modularity.
 */

#pragma once

#include "../../../PluginCore/include/IPlugin.h"
#include "RenderingPluginExport.h"
#include <Gauss/Matrix.h>
#include <string>
#include <vector>
#include <memory>

// Gauss library types are included via headers

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
 * @enum RenderingMode
 * @brief Defines the rendering mode options
 */
enum class RenderingMode {
    Hardware,    ///< Hardware-accelerated rendering
    Software,    ///< Software rendering fallback
    Headless     ///< Headless rendering (no display)
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
 * @struct Vertex
 * @brief Basic vertex structure with position, normal, and texture coordinates
 */
struct Vertex {
    float position[3];  ///< Vertex position (x, y, z)
    float normal[3];    ///< Vertex normal (x, y, z)
    float texCoord[2];  ///< Texture coordinates (u, v)
    
    Vertex() {
        position[0] = position[1] = position[2] = 0.0f;
        normal[0] = normal[1] = normal[2] = 0.0f;
        texCoord[0] = texCoord[1] = 0.0f;
    }
    
    Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v) {
        position[0] = px; position[1] = py; position[2] = pz;
        normal[0] = nx; normal[1] = ny; normal[2] = nz;
        texCoord[0] = u; texCoord[1] = v;
    }
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
 * @class RenderingPluginBase
 * @brief Abstract base class for rendering plugins
 * 
 * This class defines the common interface that all rendering plugins must implement.
 * It extends the base IPlugin interface with methods specific to rendering operations,
 * such as initializing render systems, creating windows, and managing rendering resources.
 * 
 * Different rendering backends (LLGL, DirectX, etc.) can inherit from this class
 * and provide their own implementations.
 */
class RENDERING_PLUGIN_API RenderingPluginBase : public IPlugin {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~RenderingPluginBase() = default;
    
    // === Core Rendering System Management ===
    
    /**
     * @brief Initialize the render system with specified API
     * 
     * @param api The rendering API to use
     * @return true if initialization was successful, false otherwise
     */
    virtual bool InitializeRenderSystem(RenderAPI api = RenderAPI::OpenGL) = 0;
    
    /**
     * @brief Create a rendering window
     * 
     * @param desc Window description
     * @return true if window creation was successful, false otherwise
     */
    virtual bool CreateWindow(const WindowDesc& desc) = 0;
    
    /**
     * @brief Begin a new frame for rendering
     * 
     * @return true if frame begin was successful, false otherwise
     */
    virtual bool BeginFrame() = 0;
    
    /**
     * @brief End the current frame and present it
     * 
     * @return true if frame end was successful, false otherwise
     */
    virtual bool EndFrame() = 0;
    
    /**
     * @brief Clear the current render target
     * 
     * @param color Clear color
     */
    virtual void Clear(const Color& color = Color(0.0f, 0.0f, 0.0f, 1.0f)) = 0;
    
    /**
     * @brief Set the viewport for rendering
     * 
     * @param x X coordinate of the viewport
     * @param y Y coordinate of the viewport
     * @param width Width of the viewport
     * @param height Height of the viewport
     */
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    
    // === System Information and State ===
    
    /**
     * @brief Check if the rendering system is initialized
     * 
     * @return true if initialized, false otherwise
     */
    virtual bool IsInitialized() const = 0;
    
    /**
     * @brief Get the current rendering API
     * 
     * @return The current rendering API
     */
    virtual RenderAPI GetCurrentAPI() const = 0;
    
    /**
     * @brief Get the window size
     * 
     * @param width Output parameter for width
     * @param height Output parameter for height
     * @return true if successful, false otherwise
     */
    virtual bool GetWindowSize(int& width, int& height) const = 0;
    
    /**
     * @brief Check if the window should close
     * 
     * @return true if window should close, false otherwise
     */
    virtual bool ShouldWindowClose() const = 0;
    
    /**
     * @brief Poll window events
     */
    virtual void PollEvents() = 0;
    
    /**
     * @brief Get current rendering mode
     * 
     * @return The current rendering mode
     */
    virtual RenderingMode GetRenderingMode() const = 0;
    
    /**
     * @brief Check if software rendering is enabled
     * 
     * @return true if software rendering is enabled, false otherwise
     */
    virtual bool IsSoftwareRenderingEnabled() const = 0;
    
    /**
     * @brief Check if running in headless mode
     * 
     * @return true if in headless mode, false otherwise
     */
    virtual bool IsHeadlessMode() const = 0;
    
    // === Advanced Rendering Features ===
    
    /**
     * @brief Initialize with software rendering fallback
     * 
     * @param mode The rendering mode to use
     * @return true if initialization was successful, false otherwise
     */
    virtual bool InitializeWithFallback(RenderingMode mode = RenderingMode::Hardware) = 0;
    
    /**
     * @brief Create offscreen buffer for headless rendering
     * 
     * @param width Width of the offscreen buffer
     * @param height Height of the offscreen buffer
     * @return true if creation was successful, false otherwise
     */
    virtual bool CreateOffscreenBuffer(int width, int height) = 0;
    
    // === Geometry Generation ===
    
    /**
     * @brief Generate vertices for a textured cube
     * 
     * @return Vector of cube vertices
     */
    virtual std::vector<Vertex> GenerateCubeVertices() = 0;
    
    /**
     * @brief Generate indices for a textured cube
     * 
     * @return Vector of cube indices
     */
    virtual std::vector<std::uint32_t> GenerateCubeIndices() = 0;
    
    /**
     * @brief Generate vertices for a triangle
     * 
     * @return Vector of triangle vertices
     */
    virtual std::vector<Vertex> GenerateTriangleVertices() = 0;
    
    /**
     * @brief Generate indices for a triangle
     * 
     * @return Vector of triangle indices
     */
    virtual std::vector<std::uint32_t> GenerateTriangleIndices() = 0;
    
    // === Matrix Operations ===
    
    /**
     * @brief Build a perspective projection matrix
     * 
     * @param aspectRatio Aspect ratio (width/height)
     * @param nearPlane Near clipping plane
     * @param farPlane Far clipping plane
     * @param fieldOfView Field of view in degrees
     * @return Perspective projection matrix
     */
    virtual Gs::Matrix4f BuildPerspectiveProjection(float aspectRatio, float nearPlane, 
                                                   float farPlane, float fieldOfView) const = 0;
    
    // === Demo and Testing ===
    
    /**
     * @brief Renders a simple demo scene with a rotating cube
     * 
     * @return true if demo rendering was successful, false otherwise
     */
    virtual bool RenderDemo() = 0;
    
    // === Static Utility Functions ===
    
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
     * @brief Get the supported rendering APIs for this plugin
     * 
     * @return Vector of supported rendering APIs
     */
    virtual std::vector<RenderAPI> GetSupportedAPIs() const = 0;
    
    /**
     * @brief Get the name of the rendering backend
     * 
     * @return Name of the rendering backend (e.g., "LLGL", "DirectX")
     */
    virtual std::string GetBackendName() const = 0;
    
    /**
     * @brief Get the version of the rendering backend
     * 
     * @return Version string of the rendering backend
     */
    virtual std::string GetBackendVersion() const = 0;
};
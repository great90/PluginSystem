/**
 * @file RenderingPlugin.h
 * @brief Modular rendering plugin with LLGL backend
 * 
 * This file provides a comprehensive rendering plugin with modular architecture:
 * - RenderingSystem: Core rendering system management
 * - ResourceManager: Rendering resource management
 * - RenderCommands: Rendering command interface
 * - GeometryGenerator: Geometry generation utilities
 * - ShaderManager: Shader management and compilation
 */

#pragma once

#include "IPlugin.h"
#include "RenderingPluginExport.h"

// Include modular components
#include "RenderingSystem.h"
#include "ResourceManager.h"
#include "RenderCommands.h"
#include "GeometryGenerator.h"
#include "ShaderManager.h"

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
    class ResourceHeap;
    class PipelineLayout;
    class Sampler;
    struct Extent2D;
    struct Viewport;
    struct ClearValue;
    struct VertexFormat;
}

// Forward declarations for Gauss Math (Gs)
namespace Gs {
    template<typename T> class Matrix4;
    using Matrix4f = Matrix4<float>;
    template<typename T> class Vector3;
    using Vector3f = Vector3<float>;
    template<typename T> class Vector2;
    using Vector2f = Vector2<float>;
}

// Namespace for the rendering plugin
namespace RenderingPlugin {
    // Forward declarations of modular components
    class RenderingSystem;
    class ResourceManager;
    class RenderCommands;
    class GeometryGenerator;
    class ShaderManager;
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
 * @struct Matrices
 * @brief Matrix structure for transformations
 */
struct Matrices {
    Gs::Matrix4f wvpMatrix;  ///< World-View-Projection matrix
    Gs::Matrix4f wMatrix;    ///< World matrix
};

/**
 * @struct RenderObject
 * @brief Represents a renderable object with its resources
 */
struct RenderObject {
    LLGL::Buffer* vertexBuffer = nullptr;     ///< Vertex buffer
    LLGL::Buffer* indexBuffer = nullptr;      ///< Index buffer
    LLGL::Buffer* constantBuffer = nullptr;   ///< Constant buffer for matrices
    LLGL::Texture* texture = nullptr;         ///< Texture
    LLGL::Sampler* sampler = nullptr;         ///< Texture sampler
    LLGL::ResourceHeap* resourceHeap = nullptr; ///< Resource heap
    LLGL::PipelineState* pipeline = nullptr;  ///< Graphics pipeline
    LLGL::PipelineLayout* layout = nullptr;   ///< Pipeline layout
    std::uint32_t indexCount = 0;             ///< Number of indices
    
    RenderObject() = default;
    
    // Disable copy constructor and assignment
    RenderObject(const RenderObject&) = delete;
    RenderObject& operator=(const RenderObject&) = delete;
    
    // Enable move constructor and assignment
    RenderObject(RenderObject&& other) noexcept
        : vertexBuffer(other.vertexBuffer), indexBuffer(other.indexBuffer),
          constantBuffer(other.constantBuffer), texture(other.texture),
          sampler(other.sampler), resourceHeap(other.resourceHeap),
          pipeline(other.pipeline), layout(other.layout),
          indexCount(other.indexCount) {
        other.vertexBuffer = nullptr;
        other.indexBuffer = nullptr;
        other.constantBuffer = nullptr;
        other.texture = nullptr;
        other.sampler = nullptr;
        other.resourceHeap = nullptr;
        other.pipeline = nullptr;
        other.layout = nullptr;
        other.indexCount = 0;
    }
    
    RenderObject& operator=(RenderObject&& other) noexcept {
        if (this != &other) {
            vertexBuffer = other.vertexBuffer;
            indexBuffer = other.indexBuffer;
            constantBuffer = other.constantBuffer;
            texture = other.texture;
            sampler = other.sampler;
            resourceHeap = other.resourceHeap;
            pipeline = other.pipeline;
            layout = other.layout;
            indexCount = other.indexCount;
            
            other.vertexBuffer = nullptr;
            other.indexBuffer = nullptr;
            other.constantBuffer = nullptr;
            other.texture = nullptr;
            other.sampler = nullptr;
            other.resourceHeap = nullptr;
            other.pipeline = nullptr;
            other.layout = nullptr;
            other.indexCount = 0;
        }
        return *this;
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
 * @brief Modular rendering plugin with comprehensive LLGL backend
 * 
 * This plugin provides a complete rendering solution with modular architecture:
 * - Supports multiple rendering APIs (OpenGL, Vulkan, Direct3D, Metal)
 * - Provides high-level abstraction over LLGL
 * - Includes resource management, shader compilation, and geometry generation
 * - Maintains backward compatibility with existing API
 * - Offers both simple and advanced rendering interfaces
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
    
    // === Modular Components Access ===
    
    /**
     * @brief Get the rendering system component
     * 
     * @return Pointer to the rendering system, or nullptr if not initialized
     */
    RenderingPlugin::RenderingSystem* GetRenderingSystem() const;
    
    /**
     * @brief Get the resource manager component
     * 
     * @return Pointer to the resource manager, or nullptr if not initialized
     */
    RenderingPlugin::ResourceManager* GetResourceManager() const;
    
    /**
     * @brief Get the render commands component
     * 
     * @return Pointer to the render commands, or nullptr if not initialized
     */
    RenderingPlugin::RenderCommands* GetRenderCommands() const;
    
    /**
     * @brief Get the geometry generator component
     * 
     * @return Pointer to the geometry generator, or nullptr if not initialized
     */
    RenderingPlugin::GeometryGenerator* GetGeometryGenerator() const;
    
    /**
     * @brief Get the shader manager component
     * 
     * @return Pointer to the shader manager, or nullptr if not initialized
     */
    RenderingPlugin::ShaderManager* GetShaderManager() const;
    
    // === Core LLGL Objects (Legacy Support) ===
    
    /**
     * @brief Get the LLGL render system for advanced rendering operations
     * 
     * @return Pointer to the LLGL render system, or nullptr if not initialized
     */
    LLGL::RenderSystem* GetRenderSystem() const;
    
    /**
     * @brief Get the LLGL swap chain for render pass operations
     * 
     * @return Pointer to the LLGL swap chain, or nullptr if not initialized
     */
    LLGL::SwapChain* GetSwapChain() const;
    
    /**
     * @brief Get the LLGL command buffer for rendering commands
     * 
     * @return Pointer to the LLGL command buffer, or nullptr if not initialized
     */
    LLGL::CommandBuffer* GetCommandBuffer() const;
    
    // === Rendering Resource Management ===
    
    /**
     * @brief Create a vertex buffer from vertex data
     * 
     * @param vertices Array of vertex data
     * @param vertexCount Number of vertices
     * @return Pointer to created vertex buffer, or nullptr on failure
     */
    LLGL::Buffer* CreateVertexBuffer(const Vertex* vertices, std::uint32_t vertexCount);
    
    /**
     * @brief Create an index buffer from index data
     * 
     * @param indices Array of index data
     * @param indexCount Number of indices
     * @return Pointer to created index buffer, or nullptr on failure
     */
    LLGL::Buffer* CreateIndexBuffer(const std::uint32_t* indices, std::uint32_t indexCount);
    
    /**
     * @brief Create a constant buffer for matrix data
     * 
     * @return Pointer to created constant buffer, or nullptr on failure
     */
    LLGL::Buffer* CreateConstantBuffer();
    
    /**
     * @brief Create a texture from file
     * 
     * @param filename Path to the texture file
     * @return Pointer to created texture, or nullptr on failure
     */
    LLGL::Texture* CreateTexture(const std::string& filename);
    
    /**
     * @brief Create a texture sampler
     * 
     * @param maxAnisotropy Maximum anisotropy level (default: 8)
     * @return Pointer to created sampler, or nullptr on failure
     */
    LLGL::Sampler* CreateSampler(int maxAnisotropy = 8);
    
    /**
     * @brief Create vertex and fragment shaders
     * 
     * @param vertexShaderOut Output pointer for vertex shader
     * @param fragmentShaderOut Output pointer for fragment shader
     * @return true if shaders were created successfully, false otherwise
     */
    bool CreateShaders(LLGL::Shader*& vertexShaderOut, LLGL::Shader*& fragmentShaderOut);
    
    /**
     * @brief Create a pipeline layout for resource binding
     * 
     * @return Pointer to created pipeline layout, or nullptr on failure
     */
    LLGL::PipelineLayout* CreatePipelineLayout();
    
    /**
     * @brief Create a resource heap for binding resources
     * 
     * @param layout Pipeline layout
     * @param constantBuffer Constant buffer
     * @param texture Texture
     * @param sampler Sampler
     * @return Pointer to created resource heap, or nullptr on failure
     */
    LLGL::ResourceHeap* CreateResourceHeap(LLGL::PipelineLayout* layout, 
                                           LLGL::Buffer* constantBuffer,
                                           LLGL::Texture* texture, 
                                           LLGL::Sampler* sampler);
    
    /**
     * @brief Create a graphics pipeline state
     * 
     * @param vertexShader Vertex shader
     * @param fragmentShader Fragment shader
     * @param layout Pipeline layout
     * @param enableDepthTest Enable depth testing (default: true)
     * @param enableMultiSample Enable multi-sampling (default: true)
     * @return Pointer to created pipeline state, or nullptr on failure
     */
    LLGL::PipelineState* CreatePipelineState(LLGL::Shader* vertexShader,
                                             LLGL::Shader* fragmentShader,
                                             LLGL::PipelineLayout* layout,
                                             bool enableDepthTest = true,
                                             bool enableMultiSample = true);
    
    /**
     * @brief Create a complete render object with all necessary resources
     * 
     * @param vertices Array of vertex data
     * @param vertexCount Number of vertices
     * @param indices Array of index data
     * @param indexCount Number of indices
     * @param texturePath Path to texture file (optional)
     * @return RenderObject with all resources created
     */
    RenderObject CreateRenderObject(const Vertex* vertices, std::uint32_t vertexCount,
                                   const std::uint32_t* indices, std::uint32_t indexCount,
                                   const std::string& texturePath = "");
    
    /**
     * @brief Release a render object and all its resources
     * 
     * @param renderObject The render object to release
     */
    void ReleaseRenderObject(RenderObject& renderObject);
    
    // === Geometry Generation ===
    
    /**
     * @brief Generate vertices for a textured cube
     * 
     * @return Vector of cube vertices
     */
    std::vector<Vertex> GenerateCubeVertices();
    
    /**
     * @brief Generate indices for a textured cube
     * 
     * @return Vector of cube indices
     */
    std::vector<std::uint32_t> GenerateCubeIndices();
    
    /**
     * @brief Generate vertices for a triangle
     * 
     * @return Vector of triangle vertices
     */
    std::vector<Vertex> GenerateTriangleVertices();
    
    /**
     * @brief Generate indices for a triangle
     * 
     * @return Vector of triangle indices
     */
    std::vector<std::uint32_t> GenerateTriangleIndices();
    
    // === Rendering Operations ===
    
    /**
     * @brief Update constant buffer with matrix data
     * 
     * @param constantBuffer The constant buffer to update
     * @param matrices The matrix data to upload
     */
    void UpdateConstantBuffer(LLGL::Buffer* constantBuffer, const Matrices& matrices);
    
    /**
     * @brief Render a render object
     * 
     * @param renderObject The render object to render
     * @param worldMatrix World transformation matrix
     * @param viewMatrix View transformation matrix
     * @param projectionMatrix Projection transformation matrix
     */
    void RenderObject(const RenderObject& renderObject, 
                     const Gs::Matrix4f& worldMatrix,
                     const Gs::Matrix4f& viewMatrix, 
                     const Gs::Matrix4f& projectionMatrix);
    
    /**
     * @brief Build a perspective projection matrix
     * 
     * @param aspectRatio Aspect ratio (width/height)
     * @param nearPlane Near clipping plane
     * @param farPlane Far clipping plane
     * @param fieldOfView Field of view in degrees
     * @return Perspective projection matrix
     */
    Gs::Matrix4f BuildPerspectiveProjection(float aspectRatio, float nearPlane, 
                                           float farPlane, float fieldOfView) const;
    
    /**
     * @brief Renders a simple demo scene with a rotating cube
     * 
     * @return true if demo rendering was successful, false otherwise
     */
    bool RenderDemo();
    
    // === High-Level Rendering API ===
    
    /**
     * @brief Initialize all modular components
     * 
     * @return true if all components were initialized successfully, false otherwise
     */
    bool InitializeComponents();
    
    /**
     * @brief Shutdown and cleanup all modular components
     */
    void ShutdownComponents();
    
    /**
     * @brief Create a mesh using the geometry generator
     * 
     * @param type Type of geometry to generate
     * @param params Parameters for geometry generation
     * @return Generated mesh data
     */
    template<typename GeometryType, typename... Args>
    auto CreateMesh(Args&&... args) -> decltype(auto);
    
    /**
     * @brief Load and compile a shader program
     * 
     * @param vertexShaderPath Path to vertex shader file
     * @param fragmentShaderPath Path to fragment shader file
     * @param defines Optional preprocessor defines
     * @return Shader program handle, or nullptr on failure
     */
    LLGL::PipelineState* LoadShaderProgram(const std::string& vertexShaderPath,
                                           const std::string& fragmentShaderPath,
                                           const std::vector<std::pair<std::string, std::string>>& defines = {});
    
    /**
     * @brief Begin a render pass with the specified commands
     * 
     * @return Render commands interface for this frame
     */
    RenderingPlugin::RenderCommands* BeginRenderPass();
    
    /**
     * @brief End the current render pass
     */
    void EndRenderPass();
    
public:
    // Plugin info (made public for export function access)
    static PluginInfo pluginInfo_;
    
private:
    // Static instance for singleton pattern
    static RenderingPlugin* instance_;
    
    // Modular components
    RenderingPlugin::RenderingSystem* renderingSystem_;
    RenderingPlugin::ResourceManager* resourceManager_;
    RenderingPlugin::RenderCommands* renderCommands_;
    RenderingPlugin::GeometryGenerator* geometryGenerator_;
    RenderingPlugin::ShaderManager* shaderManager_;
    
    // LLGL objects (legacy support)
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
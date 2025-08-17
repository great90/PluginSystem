#pragma once

#include "RenderingPluginBase.h"
#include "PluginExport.h"

// Forward declarations for LLGL
namespace LLGL {
    class RenderSystem;
    class SwapChain;
    class CommandBuffer;
    class Surface;
    class Buffer;
    class Texture;
    class Sampler;
    class Shader;
    class PipelineLayout;
    class ResourceHeap;
    class PipelineState;
}

// Forward declarations for modular components
class RenderingSystem;
class ResourceManager;
class RenderCommands;
class GeometryGenerator;
class ShaderManager;
class RenderObject;
struct Matrices;

class LLGL_RENDERING_PLUGIN_API LLGLRenderingPlugin : public RenderingPluginBase
{
public:
    LLGLRenderingPlugin();
    virtual ~LLGLRenderingPlugin();

    // IPlugin interface
    bool Initialize() override;
    void Shutdown() override;
    const PluginInfo& GetPluginInfo() const override;
    std::string Serialize() override;
    bool Deserialize(const std::string& data) override;
    bool PrepareForHotReload() override;
    bool CompleteHotReload() override;
    
    // Plugin information methods (not from IPlugin)
    const char* GetName() const;
    const char* GetVersion() const;
    const char* GetDescription() const;

    // RenderingPluginBase interface - Core rendering functions
    bool InitializeRenderSystem(RenderAPI api = RenderAPI::OpenGL) override;
    bool InitializeRenderingSystem(RenderAPI api, RenderingMode mode = RenderingMode::Hardware);
    bool CreateWindow(const WindowDesc& desc) override;
    bool BeginFrame() override;
    bool EndFrame() override;
    void Clear(const Color& color) override;
    void SetViewport(int x, int y, int width, int height) override;
    bool GetWindowSize(int& width, int& height) const override;
    bool IsInitialized() const override;
    RenderAPI GetCurrentAPI() const override;
    bool ShouldWindowClose() const override;
    void PollEvents() override;
    RenderingMode GetRenderingMode() const override;
    bool IsSoftwareRenderingEnabled() const override;
    bool IsHeadlessMode() const override;
    bool InitializeWithFallback(RenderingMode mode = RenderingMode::Hardware) override;
    bool CreateOffscreenBuffer(int width, int height) override;
    std::vector<Vertex> GenerateCubeVertices() override;
    std::vector<std::uint32_t> GenerateCubeIndices() override;
    std::vector<Vertex> GenerateTriangleVertices() override;
    std::vector<std::uint32_t> GenerateTriangleIndices() override;
    Gs::Matrix4f BuildPerspectiveProjection(float aspectRatio, float nearPlane, float farPlane, float fieldOfView) const override;
    bool RenderDemo() override;
    std::vector<RenderAPI> GetSupportedAPIs() const override;
    std::string GetBackendName() const override;
    std::string GetBackendVersion() const override;

    // Non-virtual functions (remove override)
    SystemGraphicsInfo GetSystemGraphicsInfo() const;
    bool InitializeSoftwareRenderer(int width, int height);
    bool InitializeHeadlessRenderer(int width, int height);

    // Modular component access (non-virtual)
    RenderingSystem* GetRenderingSystem();
    ResourceManager* GetResourceManager();
    RenderCommands* GetRenderCommands();
    GeometryGenerator* GetGeometryGenerator();
    ShaderManager* GetShaderManager();

    // Core LLGL object access (non-virtual)
    LLGL::RenderSystem* GetLLGLRenderSystem();
    LLGL::SwapChain* GetSwapChain();
    LLGL::CommandBuffer* GetCommandBuffer();

    // Resource creation and management (non-virtual)
    LLGL::Buffer* CreateVertexBuffer(const void* data, size_t size);
    LLGL::Buffer* CreateIndexBuffer(const void* data, size_t size);
    LLGL::Buffer* CreateConstantBuffer(size_t size);
    LLGL::Texture* CreateTexture(int width, int height, const void* data = nullptr);
    LLGL::Sampler* CreateSampler();
    LLGL::Shader* CreateShader(const char* source, const char* entryPoint, const char* profile);
    LLGL::PipelineLayout* CreatePipelineLayout();
    LLGL::ResourceHeap* CreateResourceHeap(LLGL::PipelineLayout* layout);
    LLGL::PipelineState* CreateGraphicsPipelineState();
    RenderObject* CreateRenderObject();
    void ReleaseRenderObject(RenderObject* obj);

    // Geometry generation (non-virtual, different signature from base class)
    void GenerateCubeVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    void GenerateTriangleVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    // Utility functions (non-virtual)
    void UpdateConstantBuffer(LLGL::Buffer* buffer, const void* data, size_t size);
    void RenderMesh(RenderObject* obj, const Matrices& matrices);
    Matrices BuildPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane);

    // Modular component management (non-virtual)
    bool InitializeModularComponents();
    void ShutdownModularComponents();
    RenderObject* CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    LLGL::Shader* LoadAndCompileShader(const std::string& filename, const std::string& entryPoint, const std::string& profile);
    void BeginRenderPass();
    void EndRenderPass();

private:
    // Modular components
    RenderingSystem* m_renderingSystem;
    ResourceManager* m_resourceManager;
    RenderCommands* m_renderCommands;
    GeometryGenerator* m_geometryGenerator;
    ShaderManager* m_shaderManager;

    // Core LLGL objects
    LLGL::RenderSystem* m_renderSystem;
    LLGL::SwapChain* m_swapChain;
    LLGL::CommandBuffer* m_commandBuffer;
    LLGL::Surface* m_surface;

    // Plugin state
    bool m_initialized;
    bool m_windowCreated;
    RenderAPI m_currentAPI;
    RenderingMode m_currentMode;
    int m_windowWidth;
    int m_windowHeight;

    // Offscreen rendering
    LLGL::Texture* m_offscreenTarget;

    // Plugin information
    PluginInfo pluginInfo;

    // Static instance for singleton pattern
    static LLGLRenderingPlugin* instance_;
};
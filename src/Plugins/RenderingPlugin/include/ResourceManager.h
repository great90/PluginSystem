/**
 * @file ResourceManager.h
 * @brief Rendering resource management class
 * @details Manages buffers, textures, shaders, and other rendering resources
 */

#pragma once

#include "RenderingPluginExport.h"
#include "MathTypes.h"
#include <LLGL/LLGL.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace RenderingPlugin {

/**
 * @brief Resource ID type for internal resource tracking
 */
using ResourceId = std::uint32_t;

/**
 * @brief Vertex structure for rendering
 */
struct Vertex {
    Gs::Vector3f position;  ///< Vertex position
    Gs::Vector3f normal;    ///< Vertex normal
    Gs::Vector2f texCoord;  ///< Texture coordinates
    Gs::Vector3f color;     ///< Vertex color
    
    Vertex() = default;
    Vertex(const Gs::Vector3f& pos, const Gs::Vector3f& norm = Gs::Vector3f(0, 1, 0),
           const Gs::Vector2f& uv = Gs::Vector2f(0, 0), const Gs::Vector3f& col = Gs::Vector3f(1, 1, 1))
        : position(pos), normal(norm), texCoord(uv), color(col) {}
};

/**
 * @brief Matrix data structure for constant buffers
 */
struct Matrices {
    Gs::Matrix4f world;      ///< World transformation matrix
    Gs::Matrix4f view;       ///< View transformation matrix
    Gs::Matrix4f projection; ///< Projection transformation matrix
    
    Matrices() {
        world.LoadIdentity();
        view.LoadIdentity();
        projection.LoadIdentity();
    }
    
    // Copy constructor
    Matrices(const Matrices&) = default;
    
    // Assignment operator
    Matrices& operator=(const Matrices&) = default;
    
    // Move constructor
    Matrices(Matrices&&) = default;
    
    // Move assignment operator
    Matrices& operator=(Matrices&&) = default;
};

/**
 * @brief Complete render object with all necessary resources
 */
struct RenderObject {
    ResourceId vertexBufferId = 0;
    ResourceId indexBufferId = 0;
    ResourceId pipelineStateId = 0;
    uint32_t indexCount = 0;
    Matrices transform;
    bool visible = true;
    
    // Default constructor
    RenderObject() = default;
    
    // Copy constructor
    RenderObject(const RenderObject&) = default;
    
    // Assignment operator
    RenderObject& operator=(const RenderObject&) = default;
    
    // Move constructor
    RenderObject(RenderObject&&) = default;
    
    // Move assignment operator
    RenderObject& operator=(RenderObject&&) = default;
};

/**
 * @brief Resource statistics structure
 */
struct ResourceStats {
    size_t vertexBufferCount;
    size_t indexBufferCount;
    size_t constantBufferCount;
    size_t textureCount;
    size_t samplerCount;
    size_t shaderCount;
    size_t pipelineLayoutCount;
    size_t resourceHeapCount;
    size_t pipelineStateCount;
    size_t renderObjectCount;
    size_t totalResourceCount;
};

/**
 * @brief Resource handle for tracking managed resources
 */
using ResourceHandle = std::uint32_t;
static constexpr ResourceHandle INVALID_HANDLE = 0;

/**
 * @brief Rendering resource management class
 * @details Manages creation, tracking, and cleanup of rendering resources
 */
class RENDERING_PLUGIN_API ResourceManager {
public:
    /**
     * @brief Constructor
     * @param renderSystem Pointer to the LLGL render system
     */
    explicit ResourceManager(LLGL::RenderSystem* renderSystem);
    
    /**
     * @brief Destructor
     */
    ~ResourceManager();
    
    // === Buffer Management ===
    
    /**
     * @brief Create a vertex buffer
     * @param data Vertex data
     * @param size Size of data in bytes
     * @param format Vertex format
     * @return Resource ID of created vertex buffer, or 0 on failure
     */
    ResourceId CreateVertexBuffer(const void* data, size_t size, const LLGL::VertexFormat& format);
    
    /**
     * @brief Create an index buffer
     * @param data Index data
     * @param size Size of data in bytes
     * @param format Index format
     * @return Resource ID of created index buffer, or 0 on failure
     */
    ResourceId CreateIndexBuffer(const void* data, size_t size, const LLGL::Format format);
    
    /**
     * @brief Create a constant buffer
     * @param size Size of buffer in bytes
     * @param initialData Initial data (optional)
     * @return Resource ID of created constant buffer, or 0 on failure
     */
    ResourceId CreateConstantBuffer(size_t size, const void* initialData = nullptr);
    
    /**
     * @brief Update buffer data
     * @param bufferId Resource ID of buffer to update
     * @param data New data
     * @param size Size of data in bytes
     * @param offset Offset in buffer
     * @return true on success, false on failure
     */
    bool UpdateBuffer(ResourceId bufferId, const void* data, size_t size, size_t offset = 0);
    
    /**
     * @brief Update constant buffer with matrix data
     * @param constantBuffer The constant buffer to update
     * @param matrices The matrix data to upload
     */
    void UpdateConstantBuffer(LLGL::Buffer* constantBuffer, const Matrices& matrices);
    
    // === Texture Management ===
    
    /**
     * @brief Create a 2D texture
     * @param width Texture width
     * @param height Texture height
     * @param format Texture format
     * @param data Raw texture data (optional)
     * @return Resource ID of created texture, or 0 on failure
     */
    ResourceId CreateTexture2D(int width, int height, LLGL::Format format, const void* data = nullptr);
    
    /**
     * @brief Create a cube texture
     * @param size Texture size (width and height)
     * @param format Texture format
     * @param data Array of 6 face data pointers (optional)
     * @return Resource ID of created texture, or 0 on failure
     */
    ResourceId CreateTextureCube(int size, LLGL::Format format, const void* data[6] = nullptr);
    
    /**
     * @brief Create a texture sampler
     * @param desc Sampler descriptor
     * @return Resource ID of created sampler, or 0 on failure
     */
    ResourceId CreateSampler(const LLGL::SamplerDescriptor& desc);
    
    // === Shader Management ===
    
    /**
     * @brief Create a shader from source code
     * @param type Shader type (vertex, fragment, etc.)
     * @param source Shader source code
     * @param entryPoint Shader entry point (default: "main")
     * @return Resource ID of created shader, or 0 on failure
     */
    ResourceId CreateShader(LLGL::ShaderType type, const std::string& source, const std::string& entryPoint = "main");
    
    /**
     * @brief Create a shader from file
     * @param type Shader type (vertex, fragment, etc.)
     * @param filename Path to shader file
     * @param entryPoint Shader entry point (default: "main")
     * @return Resource ID of created shader, or 0 on failure
     */
    ResourceId CreateShaderFromFile(LLGL::ShaderType type, const std::string& filename, const std::string& entryPoint = "main");
    
    // === Pipeline Management ===
    
    // Pipeline management
    ResourceId CreatePipelineLayout(const LLGL::PipelineLayoutDescriptor& desc);
    ResourceId CreateResourceHeap(const LLGL::ResourceHeapDescriptor& desc);
    ResourceId CreateGraphicsPipelineState(const LLGL::GraphicsPipelineDescriptor& desc);
    ResourceId CreateComputePipelineState(const LLGL::ComputePipelineDescriptor& desc);
    
    /**
     * @brief Create a pipeline layout for resource binding
     * @return Pointer to created pipeline layout, or nullptr on failure
     */
    LLGL::PipelineLayout* CreatePipelineLayout();
    
    /**
     * @brief Create a graphics pipeline state
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
     * @brief Create a resource heap for binding resources
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
    
    // === High-Level Resource Management ===
    
    /**
     * @brief Create a complete render object with all necessary resources
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
     * @param renderObject The render object to release
     */
    void ReleaseRenderObject(RenderObject& renderObject);
    
    // RenderObject management
    ResourceId CreateRenderObject(ResourceId vertexBufferId, ResourceId indexBufferId, 
                                 ResourceId pipelineStateId, uint32_t indexCount);
    bool UpdateRenderObjectTransform(ResourceId objectId, const Matrices& transform);
    bool SetRenderObjectVisibility(ResourceId objectId, bool visible);
    void ReleaseRenderObject(ResourceId objectId);
    
    /**
     * @brief Create a render object with handle tracking
     * @param vertices Array of vertex data
     * @param vertexCount Number of vertices
     * @param indices Array of index data
     * @param indexCount Number of indices
     * @param texturePath Path to texture file (optional)
     * @return Handle to the created render object
     */
    ResourceHandle CreateManagedRenderObject(const Vertex* vertices, std::uint32_t vertexCount,
                                            const std::uint32_t* indices, std::uint32_t indexCount,
                                            const std::string& texturePath = "");
    
    /**
     * @brief Get a managed render object by handle
     * @param handle Resource handle
     * @return Pointer to render object, or nullptr if not found
     */
    RenderObject* GetRenderObject(ResourceHandle handle);
    
    /**
     * @brief Release a managed render object by handle
     * @param handle Resource handle
     */
    void ReleaseManagedRenderObject(ResourceHandle handle);
    
    // === Resource Cleanup ===
    
    /**
     * @brief Release all managed resources
     */
    void ReleaseAllResources();
    
    // Resource access
    LLGL::Buffer* GetVertexBuffer(ResourceId id) const;
    LLGL::Buffer* GetIndexBuffer(ResourceId id) const;
    LLGL::Buffer* GetConstantBuffer(ResourceId id) const;
    LLGL::Texture* GetTexture(ResourceId id) const;
    LLGL::Sampler* GetSampler(ResourceId id) const;
    LLGL::Shader* GetShader(ResourceId id) const;
    LLGL::PipelineLayout* GetPipelineLayout(ResourceId id) const;
    LLGL::ResourceHeap* GetResourceHeap(ResourceId id) const;
    LLGL::PipelineState* GetPipelineState(ResourceId id) const;
    const RenderObject* GetRenderObject(ResourceId id) const;
    
    // Resource release
    void ReleaseBuffer(ResourceId id);
    void ReleaseTexture(ResourceId id);
    void ReleaseSampler(ResourceId id);
    void ReleaseShader(ResourceId id);
    void ReleasePipelineLayout(ResourceId id);
    void ReleaseResourceHeap(ResourceId id);
    void ReleasePipelineState(ResourceId id);
    
    // Resource statistics
    ResourceStats GetResourceStats() const;
    void PrintResourceStats() const;
    std::vector<ResourceId> GetAllRenderObjects() const;
    std::vector<ResourceId> GetVisibleRenderObjects() const;
     
    /**
     * @brief Get resource statistics
     * @param bufferCount Output parameter for buffer count
     * @param textureCount Output parameter for texture count
     * @param shaderCount Output parameter for shader count
     * @param pipelineCount Output parameter for pipeline count
     */
    void GetResourceStats(std::uint32_t& bufferCount, std::uint32_t& textureCount,
                         std::uint32_t& shaderCount, std::uint32_t& pipelineCount) const;

private:
    // === Private Methods ===
    
    /**
     * @brief Generate a new resource handle
     * @return New unique resource handle
     */
    ResourceHandle GenerateHandle();
    
    /**
     * @brief Track a resource for automatic cleanup
     * @param resource Pointer to resource
     */
    void TrackResource(void* resource);
    
    /**
     * @brief Untrack a resource
     * @param resource Pointer to resource
     */
    void UntrackResource(void* resource);
    
    // === Private Members ===
    
    LLGL::RenderSystem* renderSystem_;              ///< Pointer to LLGL render system
    
    // Resource tracking
    std::unordered_map<ResourceHandle, RenderObject> managedRenderObjects_;
    std::vector<void*> trackedResources_;
    ResourceHandle nextHandle_;
    
    // Resource counters
    std::uint32_t bufferCount_;
    std::uint32_t textureCount_;
    std::uint32_t shaderCount_;
    std::uint32_t pipelineCount_;
    
    // Resource storage maps
    ResourceId nextResourceId_;
    std::unordered_map<ResourceId, LLGL::Buffer*> vertexBuffers_;
    std::unordered_map<ResourceId, LLGL::Buffer*> indexBuffers_;
    std::unordered_map<ResourceId, LLGL::Buffer*> constantBuffers_;
    std::unordered_map<ResourceId, LLGL::Texture*> textures_;
    std::unordered_map<ResourceId, LLGL::Sampler*> samplers_;
    std::unordered_map<ResourceId, LLGL::Shader*> shaders_;
    std::unordered_map<ResourceId, LLGL::PipelineLayout*> pipelineLayouts_;
    std::unordered_map<ResourceId, LLGL::PipelineState*> pipelineStates_;
    std::unordered_map<ResourceId, LLGL::ResourceHeap*> resourceHeaps_;
    std::unordered_map<ResourceId, std::unique_ptr<RenderObject>> renderObjects_;
};

} // namespace RenderingPlugin
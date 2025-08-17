/**
 * @file RenderCommands.h
 * @brief Rendering commands interface class
 * @details Provides high-level rendering command interface for drawing operations
 */

#pragma once

#include "RenderingPluginExport.h"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

// Include LLGL headers
#include <LLGL/LLGL.h>
#include "RenderingPluginBase.h"
#include <Gauss/Matrix.h>

namespace RenderingPlugin {

// Forward declarations
class ResourceManager;
struct RenderObject;
struct Matrices;

/**
 * @brief Rendering commands interface class
 * @details Provides high-level interface for rendering operations and draw calls
 */
class RENDERING_PLUGIN_API RenderCommands {
public:
    /**
     * @brief Constructor
     * @param commandBuffer Pointer to LLGL command buffer
     * @param resourceManager Pointer to resource manager
     */
    RenderCommands(LLGL::CommandBuffer* commandBuffer, ResourceManager* resourceManager);
    
    /**
     * @brief Destructor
     */
    ~RenderCommands();
    
    // === Basic Rendering Commands ===
    
    /**
     * @brief Clear the frame buffer
     * @param color Clear color
     * @param clearDepth Whether to clear depth buffer (default: true)
     * @param clearStencil Whether to clear stencil buffer (default: false)
     */
    void Clear(const Color& color, bool clearDepth = true, bool clearStencil = false);
    
    /**
     * @brief Set viewport
     * @param x X coordinate
     * @param y Y coordinate
     * @param width Viewport width
     * @param height Viewport height
     * @param minDepth Minimum depth value (default: 0.0f)
     * @param maxDepth Maximum depth value (default: 1.0f)
     */
    void SetViewport(int x, int y, int width, int height, float minDepth = 0.0f, float maxDepth = 1.0f);
    
    /**
     * @brief Set scissor rectangle
     * @param x X coordinate
     * @param y Y coordinate
     * @param width Scissor width
     * @param height Scissor height
     */
    void SetScissor(int x, int y, int width, int height);
    
    /**
     * @brief Enable or disable scissor test
     * @param enable Whether to enable scissor test
     */
    void EnableScissor(bool enable);
    
    // === Pipeline State Commands ===
    
    /**
     * @brief Bind a pipeline state
     * @param pipelineState Pointer to pipeline state
     */
    void BindPipelineState(LLGL::PipelineState* pipelineState);
    
    /**
     * @brief Bind a resource heap
     * @param resourceHeap Pointer to resource heap
     * @param firstSet First descriptor set index (default: 0)
     */
    void BindResourceHeap(LLGL::ResourceHeap* resourceHeap, std::uint32_t firstSet = 0);
    
    /**
     * @brief Bind vertex buffer
     * @param vertexBuffer Pointer to vertex buffer
     * @param slot Buffer slot index (default: 0)
     */
    void BindVertexBuffer(LLGL::Buffer* vertexBuffer, std::uint32_t slot = 0);
    
    /**
     * @brief Bind index buffer
     * @param indexBuffer Pointer to index buffer
     * @param format Index format (default: UInt32)
     */
    void BindIndexBuffer(LLGL::Buffer* indexBuffer, LLGL::Format format = LLGL::Format::R32UInt);
    
    // === Drawing Commands ===
    
    /**
     * @brief Draw primitives
     * @param vertexCount Number of vertices to draw
     * @param firstVertex First vertex index (default: 0)
     */
    void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex = 0);
    
    /**
     * @brief Draw indexed primitives
     * @param indexCount Number of indices to draw
     * @param firstIndex First index (default: 0)
     * @param vertexOffset Vertex offset (default: 0)
     */
    void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex = 0, std::int32_t vertexOffset = 0);
    
    /**
     * @brief Draw instanced primitives
     * @param vertexCount Number of vertices per instance
     * @param instanceCount Number of instances
     * @param firstVertex First vertex index (default: 0)
     * @param firstInstance First instance index (default: 0)
     */
    void DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount,
                      std::uint32_t firstVertex = 0, std::uint32_t firstInstance = 0);
    
    /**
     * @brief Draw indexed instanced primitives
     * @param indexCount Number of indices per instance
     * @param instanceCount Number of instances
     * @param firstIndex First index (default: 0)
     * @param vertexOffset Vertex offset (default: 0)
     * @param firstInstance First instance index (default: 0)
     */
    void DrawIndexedInstanced(std::uint32_t indexCount, std::uint32_t instanceCount,
                             std::uint32_t firstIndex = 0, std::int32_t vertexOffset = 0,
                             std::uint32_t firstInstance = 0);
    
    // === High-Level Rendering Commands ===
    
    /**
     * @brief Render a render object
     * @param renderObject The render object to render
     * @param worldMatrix World transformation matrix
     * @param viewMatrix View transformation matrix
     * @param projectionMatrix Projection transformation matrix
     */
    void RenderObject(const struct RenderObject& renderObject,
                     const Gs::Matrix4f& worldMatrix,
                     const Gs::Matrix4f& viewMatrix,
                     const Gs::Matrix4f& projectionMatrix);
    
    /**
     * @brief Render a render object with pre-computed matrices
     * @param renderObject The render object to render
     * @param matrices Pre-computed transformation matrices
     */
    void RenderObject(const struct RenderObject& renderObject, const Matrices& matrices);
    
    /**
     * @brief Render multiple objects with the same pipeline state
     * @param renderObjects Array of render objects
     * @param objectCount Number of objects
     * @param matrices Array of transformation matrices (one per object)
     */
    void RenderObjects(const struct RenderObject* renderObjects, std::uint32_t objectCount,
                      const Matrices* matrices);
    
    /**
     * @brief Render a mesh with custom transformation
     * @param vertexBuffer Vertex buffer
     * @param indexBuffer Index buffer
     * @param indexCount Number of indices
     * @param pipelineState Pipeline state
     * @param resourceHeap Resource heap (optional)
     * @param worldMatrix World transformation matrix
     * @param viewMatrix View transformation matrix
     * @param projectionMatrix Projection transformation matrix
     */
    void RenderMesh(LLGL::Buffer* vertexBuffer, LLGL::Buffer* indexBuffer, std::uint32_t indexCount,
                   LLGL::PipelineState* pipelineState, LLGL::ResourceHeap* resourceHeap,
                   const Gs::Matrix4f& worldMatrix, const Gs::Matrix4f& viewMatrix,
                   const Gs::Matrix4f& projectionMatrix);
    
    // === Utility Commands ===
    
    /**
     * @brief Begin a debug group for debugging/profiling
     * @param name Debug group name
     */
    void BeginDebugGroup(const std::string& name);
    
    /**
     * @brief End the current debug group
     */
    void EndDebugGroup();
    
    /**
     * @brief Insert a debug marker
     * @param name Marker name
     */
    void InsertDebugMarker(const std::string& name);
    
    /**
     * @brief Set a uniform value (for simple constant updates)
     * @param location Uniform location
     * @param data Pointer to uniform data
     * @param size Size of uniform data in bytes
     */
    void SetUniform(std::uint32_t location, const void* data, std::uint32_t size);
    
    /**
     * @brief Update a buffer with new data
     * @param buffer Buffer to update
     * @param data New data
     * @param size Size of data in bytes
     * @param offset Offset in buffer (default: 0)
     */
    void UpdateBuffer(LLGL::Buffer* buffer, const void* data, std::uint32_t size, std::uint32_t offset = 0);
    
    // === State Queries ===
    
    /**
     * @brief Check if command buffer is in recording state
     * @return true if recording, false otherwise
     */
    bool IsRecording() const;
    
    /**
     * @brief Get the underlying command buffer
     * @return Pointer to LLGL command buffer
     */
    LLGL::CommandBuffer* GetCommandBuffer() const;
    
    /**
     * @brief Get the resource manager
     * @return Pointer to resource manager
     */
    ResourceManager* GetResourceManager() const;
    
    // === Batch Rendering Support ===
    
    /**
     * @brief Begin a batch rendering session
     * @param pipelineState Pipeline state to use for the batch
     */
    void BeginBatch(LLGL::PipelineState* pipelineState);
    
    /**
     * @brief Add an object to the current batch
     * @param renderObject Object to add to batch
     * @param worldMatrix World transformation matrix
     */
    void AddToBatch(const struct RenderObject& renderObject, const Gs::Matrix4f& worldMatrix);
    
    /**
     * @brief End the current batch and render all batched objects
     * @param viewMatrix View transformation matrix
     * @param projectionMatrix Projection transformation matrix
     */
    void EndBatch(const Gs::Matrix4f& viewMatrix, const Gs::Matrix4f& projectionMatrix);

private:
    // === Private Methods ===
    
    /**
     * @brief Validate command buffer state
     * @return true if command buffer is valid, false otherwise
     */
    bool ValidateCommandBuffer() const;
    
    /**
     * @brief Setup matrices for rendering
     * @param matrices Matrix data to setup
     */
    void SetupMatrices(const Matrices& matrices);
    
    // === Private Members ===
    
    LLGL::CommandBuffer* commandBuffer_;     ///< Pointer to LLGL command buffer
    ResourceManager* resourceManager_;       ///< Pointer to resource manager
    
    // State tracking
    LLGL::PipelineState* currentPipelineState_;
    LLGL::ResourceHeap* currentResourceHeap_;
    
    // Debug and utility state
    int debugGroupDepth_;
    bool batchingEnabled_;
};

} // namespace RenderingPlugin
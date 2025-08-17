/**
 * @file ResourceManager.cpp
 * @brief Implementation of ResourceManager class
 */

#include "../include/ResourceManager.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <LLGL/Utils/VertexFormat.h>

namespace RenderingPlugin {

// === ResourceManager Implementation ===

ResourceManager::ResourceManager(LLGL::RenderSystem* renderSystem)
    : renderSystem_(renderSystem)
    , nextResourceId_(1) {
    
    if (!renderSystem_) {
        throw std::invalid_argument("RenderSystem cannot be null");
    }
    
    std::cout << "ResourceManager initialized" << std::endl;
}

ResourceManager::~ResourceManager() {
    ReleaseAllResources();
    std::cout << "ResourceManager destroyed" << std::endl;
}

void ResourceManager::ReleaseAllResources() {
    // Release render objects first (they may reference other resources)
    renderObjects_.clear();
    
    // Release LLGL resources
    for (auto& buffer : vertexBuffers_) {
        if (buffer.second) {
            renderSystem_->Release(*buffer.second);
        }
    }
    vertexBuffers_.clear();
    
    for (auto& buffer : indexBuffers_) {
        if (buffer.second) {
            renderSystem_->Release(*buffer.second);
        }
    }
    indexBuffers_.clear();
    
    for (auto& buffer : constantBuffers_) {
        if (buffer.second) {
            renderSystem_->Release(*buffer.second);
        }
    }
    constantBuffers_.clear();
    
    for (auto& texture : textures_) {
        if (texture.second) {
            renderSystem_->Release(*texture.second);
        }
    }
    textures_.clear();
    
    for (auto& sampler : samplers_) {
        if (sampler.second) {
            renderSystem_->Release(*sampler.second);
        }
    }
    samplers_.clear();
    
    for (auto& shader : shaders_) {
        if (shader.second) {
            renderSystem_->Release(*shader.second);
        }
    }
    shaders_.clear();
    
    for (auto& layout : pipelineLayouts_) {
        if (layout.second) {
            renderSystem_->Release(*layout.second);
        }
    }
    pipelineLayouts_.clear();
    
    for (auto& heap : resourceHeaps_) {
        if (heap.second) {
            renderSystem_->Release(*heap.second);
        }
    }
    resourceHeaps_.clear();
    
    for (auto& pipeline : pipelineStates_) {
        if (pipeline.second) {
            renderSystem_->Release(*pipeline.second);
        }
    }
    pipelineStates_.clear();
    
    std::cout << "All resources released" << std::endl;
}

// === Buffer Management ===

ResourceId ResourceManager::CreateVertexBuffer(const void* data, size_t size, const LLGL::VertexFormat& format) {
    try {
        LLGL::BufferDescriptor bufferDesc;
        bufferDesc.size = size;
        bufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        bufferDesc.vertexAttribs = format.attributes;
        
        if (data) {
            bufferDesc.miscFlags = LLGL::MiscFlags::DynamicUsage;
        }
        
        LLGL::Buffer* buffer = renderSystem_->CreateBuffer(bufferDesc, data);
        if (!buffer) {
            std::cerr << "Failed to create vertex buffer" << std::endl;
            return 0;
        }
        
        ResourceId id = nextResourceId_++;
        vertexBuffers_[id] = buffer;
        
        std::cout << "Created vertex buffer (ID: " << id << ", Size: " << size << " bytes)" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating vertex buffer: " << e.what() << std::endl;
        return 0;
    }
}

ResourceId ResourceManager::CreateIndexBuffer(const void* data, size_t size, const LLGL::Format format) {
    try {
        LLGL::BufferDescriptor bufferDesc;
        bufferDesc.size = size;
        bufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
        bufferDesc.format = format;
        
        if (data) {
            bufferDesc.miscFlags = LLGL::MiscFlags::DynamicUsage;
        }
        
        LLGL::Buffer* buffer = renderSystem_->CreateBuffer(bufferDesc, data);
        if (!buffer) {
            std::cerr << "Failed to create index buffer" << std::endl;
            return 0;
        }
        
        ResourceId id = nextResourceId_++;
        indexBuffers_[id] = buffer;
        
        std::cout << "Created index buffer (ID: " << id << ", Size: " << size << " bytes)" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating index buffer: " << e.what() << std::endl;
        return 0;
    }
}

ResourceId ResourceManager::CreateConstantBuffer(size_t size, const void* initialData) {
    try {
        LLGL::BufferDescriptor bufferDesc;
        bufferDesc.size = size;
        bufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
        
        if (!initialData) {
            bufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
        }
        
        LLGL::Buffer* buffer = renderSystem_->CreateBuffer(bufferDesc, initialData);
        if (!buffer) {
            std::cerr << "Failed to create constant buffer" << std::endl;
            return 0;
        }
        
        ResourceId id = nextResourceId_++;
        constantBuffers_[id] = buffer;
        
        std::cout << "Created constant buffer (ID: " << id << ", Size: " << size << " bytes)" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating constant buffer: " << e.what() << std::endl;
        return 0;
    }
}

bool ResourceManager::UpdateBuffer(ResourceId bufferId, const void* data, size_t size, size_t offset) {
    LLGL::Buffer* buffer = nullptr;
    
    // Find the buffer in any of the buffer maps
    auto vertexIt = vertexBuffers_.find(bufferId);
    if (vertexIt != vertexBuffers_.end()) {
        buffer = vertexIt->second;
    } else {
        auto indexIt = indexBuffers_.find(bufferId);
        if (indexIt != indexBuffers_.end()) {
            buffer = indexIt->second;
        } else {
            auto constantIt = constantBuffers_.find(bufferId);
            if (constantIt != constantBuffers_.end()) {
                buffer = constantIt->second;
            }
        }
    }
    
    if (!buffer) {
        std::cerr << "Buffer with ID " << bufferId << " not found" << std::endl;
        return false;
    }
    
    try {
        renderSystem_->WriteBuffer(*buffer, offset, data, size);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception updating buffer: " << e.what() << std::endl;
        return false;
    }
}

// === Texture Management ===

ResourceId ResourceManager::CreateTexture2D(int width, int height, LLGL::Format format, const void* data) {
    try {
        LLGL::TextureDescriptor textureDesc;
        textureDesc.type = LLGL::TextureType::Texture2D;
        textureDesc.bindFlags = LLGL::BindFlags::Sampled;
        textureDesc.format = format;
        textureDesc.extent.width = static_cast<std::uint32_t>(width);
        textureDesc.extent.height = static_cast<std::uint32_t>(height);
        textureDesc.extent.depth = 1;
        textureDesc.mipLevels = 1;
        
        LLGL::ImageView* imageView = nullptr;
        if (data) {
            // Create ImageView for initial data
            static LLGL::ImageView tempImageView;
            tempImageView.data = data;
            tempImageView.dataSize = width * height * 4; // Assuming RGBA format
            tempImageView.format = LLGL::ImageFormat::RGBA;
            imageView = &tempImageView;
        }
        LLGL::Texture* texture = renderSystem_->CreateTexture(textureDesc, imageView);
        if (!texture) {
            std::cerr << "Failed to create 2D texture" << std::endl;
            return 0;
        }
        
        ResourceId id = nextResourceId_++;
        textures_[id] = texture;
        
        std::cout << "Created 2D texture (ID: " << id << ", Size: " << width << "x" << height << ")" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating 2D texture: " << e.what() << std::endl;
        return 0;
    }
}

ResourceId ResourceManager::CreateTextureCube(int size, LLGL::Format format, const void* data[6]) {
    try {
        LLGL::TextureDescriptor textureDesc;
        textureDesc.type = LLGL::TextureType::TextureCube;
        textureDesc.bindFlags = LLGL::BindFlags::Sampled;
        textureDesc.format = format;
        textureDesc.extent.width = static_cast<std::uint32_t>(size);
        textureDesc.extent.height = static_cast<std::uint32_t>(size);
        textureDesc.extent.depth = 1;
        textureDesc.arrayLayers = 6;
        textureDesc.mipLevels = 1;
        
        // For cube textures, we need to handle the data differently
        LLGL::Texture* texture = renderSystem_->CreateTexture(textureDesc);
        if (!texture) {
            std::cerr << "Failed to create cube texture" << std::endl;
            return 0;
        }
        
        // Upload each face if data is provided
        if (data) {
            for (int face = 0; face < 6; ++face) {
                if (data[face]) {
                    LLGL::TextureRegion region;
                    region.subresource.baseMipLevel = 0;
                    region.subresource.numMipLevels = 1;
                    region.subresource.baseArrayLayer = face;
                    region.subresource.numArrayLayers = 1;
                    region.extent.width = size;
                    region.extent.height = size;
                    region.extent.depth = 1;
                    
                    LLGL::ImageView imageView;
                    imageView.format = LLGL::ImageFormat::RGBA; // Use appropriate format
                    imageView.dataType = LLGL::DataType::UInt8;
                    imageView.data = data[face];
                    imageView.dataSize = size * size * 4; // Assuming RGBA format
                    
                    renderSystem_->WriteTexture(*texture, region, imageView);
                }
            }
        }
        
        ResourceId id = nextResourceId_++;
        textures_[id] = texture;
        
        std::cout << "Created cube texture (ID: " << id << ", Size: " << size << "x" << size << ")" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating cube texture: " << e.what() << std::endl;
        return 0;
    }
}

ResourceId ResourceManager::CreateSampler(const LLGL::SamplerDescriptor& desc) {
    try {
        LLGL::Sampler* sampler = renderSystem_->CreateSampler(desc);
        if (!sampler) {
            std::cerr << "Failed to create sampler" << std::endl;
            return 0;
        }
        
        ResourceId id = nextResourceId_++;
        samplers_[id] = sampler;
        
        std::cout << "Created sampler (ID: " << id << ")" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating sampler: " << e.what() << std::endl;
        return 0;
    }
}

// === Shader Management ===

ResourceId ResourceManager::CreateShader(LLGL::ShaderType type, const std::string& source, const std::string& entryPoint) {
    try {
        LLGL::ShaderDescriptor shaderDesc;
        shaderDesc.type = type;
        shaderDesc.source = source.c_str();
        shaderDesc.sourceSize = source.length();
        shaderDesc.entryPoint = entryPoint.c_str();
        
        LLGL::Shader* shader = renderSystem_->CreateShader(shaderDesc);
        if (!shader) {
            std::cerr << "Failed to create shader" << std::endl;
            return 0;
        }
        
        // Check for compilation errors
        const LLGL::Report* report = shader->GetReport();
        if (report && strlen(report->GetText()) > 0) {
            std::cout << "Shader compilation log: " << report->GetText() << std::endl;
        }
        
        ResourceId id = nextResourceId_++;
        shaders_[id] = shader;
        
        std::cout << "Created shader (ID: " << id << ", Type: " << static_cast<int>(type) << ")" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating shader: " << e.what() << std::endl;
        return 0;
    }
}

ResourceId ResourceManager::CreateShaderFromFile(LLGL::ShaderType type, const std::string& filename, const std::string& entryPoint) {
    // Read shader source from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << std::endl;
        return 0;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    return CreateShader(type, source, entryPoint);
}

// === Pipeline Management ===

ResourceId ResourceManager::CreatePipelineLayout(const LLGL::PipelineLayoutDescriptor& desc) {
    try {
        LLGL::PipelineLayout* layout = renderSystem_->CreatePipelineLayout(desc);
        if (!layout) {
            std::cerr << "Failed to create pipeline layout" << std::endl;
            return 0;
        }
        
        ResourceId id = nextResourceId_++;
        pipelineLayouts_[id] = layout;
        
        std::cout << "Created pipeline layout (ID: " << id << ")" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating pipeline layout: " << e.what() << std::endl;
        return 0;
    }
}

ResourceId ResourceManager::CreateResourceHeap(const LLGL::ResourceHeapDescriptor& desc) {
    try {
        LLGL::ResourceHeap* heap = renderSystem_->CreateResourceHeap(desc);
        if (!heap) {
            std::cerr << "Failed to create resource heap" << std::endl;
            return 0;
        }
        
        ResourceId id = nextResourceId_++;
        resourceHeaps_[id] = heap;
        
        std::cout << "Created resource heap (ID: " << id << ")" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating resource heap: " << e.what() << std::endl;
        return 0;
    }
}

ResourceId ResourceManager::CreateGraphicsPipelineState(const LLGL::GraphicsPipelineDescriptor& desc) {
    try {
        LLGL::PipelineState* pipeline = renderSystem_->CreatePipelineState(desc);
        if (!pipeline) {
            std::cerr << "Failed to create graphics pipeline state" << std::endl;
            return 0;
        }
        
        // Check for pipeline creation errors
        const LLGL::Report* report = pipeline->GetReport();
        if (report && strlen(report->GetText()) > 0) {
            std::cout << "Pipeline creation log: " << report->GetText() << std::endl;
        }
        
        ResourceId id = nextResourceId_++;
        pipelineStates_[id] = pipeline;
        
        std::cout << "Created graphics pipeline state (ID: " << id << ")" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating graphics pipeline state: " << e.what() << std::endl;
        return 0;
    }
}

ResourceId ResourceManager::CreateComputePipelineState(const LLGL::ComputePipelineDescriptor& desc) {
    try {
        LLGL::PipelineState* pipeline = renderSystem_->CreatePipelineState(desc);
        if (!pipeline) {
            std::cerr << "Failed to create compute pipeline state" << std::endl;
            return 0;
        }
        
        ResourceId id = nextResourceId_++;
        pipelineStates_[id] = pipeline;
        
        std::cout << "Created compute pipeline state (ID: " << id << ")" << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception creating compute pipeline state: " << e.what() << std::endl;
        return 0;
    }
}

// === RenderObject Management ===

ResourceId ResourceManager::CreateRenderObject(ResourceId vertexBufferId, ResourceId indexBufferId, 
                                              ResourceId pipelineStateId, uint32_t indexCount) {
    // Validate that all required resources exist
    if (vertexBuffers_.find(vertexBufferId) == vertexBuffers_.end()) {
        std::cerr << "Vertex buffer with ID " << vertexBufferId << " not found" << std::endl;
        return 0;
    }
    
    if (indexBufferId != 0 && indexBuffers_.find(indexBufferId) == indexBuffers_.end()) {
        std::cerr << "Index buffer with ID " << indexBufferId << " not found" << std::endl;
        return 0;
    }
    
    if (pipelineStates_.find(pipelineStateId) == pipelineStates_.end()) {
        std::cerr << "Pipeline state with ID " << pipelineStateId << " not found" << std::endl;
        return 0;
    }
    
    RenderObject renderObj;
    renderObj.vertexBufferId = vertexBufferId;
    renderObj.indexBufferId = indexBufferId;
    renderObj.pipelineStateId = pipelineStateId;
    renderObj.indexCount = indexCount;
    renderObj.visible = true;
    
    ResourceId id = nextResourceId_++;
    renderObjects_[id] = std::make_unique<RenderObject>(renderObj);
    
    std::cout << "Created render object (ID: " << id << ")" << std::endl;
    return id;
}

bool ResourceManager::UpdateRenderObjectTransform(ResourceId objectId, const Matrices& transform) {
    auto it = renderObjects_.find(objectId);
    if (it == renderObjects_.end()) {
        std::cerr << "Render object with ID " << objectId << " not found" << std::endl;
        return false;
    }
    
    it->second->transform = transform;
    return true;
}

bool ResourceManager::SetRenderObjectVisibility(ResourceId objectId, bool visible) {
    auto it = renderObjects_.find(objectId);
    if (it == renderObjects_.end()) {
        std::cerr << "Render object with ID " << objectId << " not found" << std::endl;
        return false;
    }
    
    it->second->visible = visible;
    return true;
}

void ResourceManager::ReleaseRenderObject(ResourceId objectId) {
    auto it = renderObjects_.find(objectId);
    if (it != renderObjects_.end()) {
        renderObjects_.erase(it);
        std::cout << "Released render object (ID: " << objectId << ")" << std::endl;
    }
}

// === Resource Access ===

LLGL::Buffer* ResourceManager::GetVertexBuffer(ResourceId id) const {
    auto it = vertexBuffers_.find(id);
    return (it != vertexBuffers_.end()) ? it->second : nullptr;
}

LLGL::Buffer* ResourceManager::GetIndexBuffer(ResourceId id) const {
    auto it = indexBuffers_.find(id);
    return (it != indexBuffers_.end()) ? it->second : nullptr;
}

LLGL::Buffer* ResourceManager::GetConstantBuffer(ResourceId id) const {
    auto it = constantBuffers_.find(id);
    return (it != constantBuffers_.end()) ? it->second : nullptr;
}

LLGL::Texture* ResourceManager::GetTexture(ResourceId id) const {
    auto it = textures_.find(id);
    return (it != textures_.end()) ? it->second : nullptr;
}

LLGL::Sampler* ResourceManager::GetSampler(ResourceId id) const {
    auto it = samplers_.find(id);
    return (it != samplers_.end()) ? it->second : nullptr;
}

LLGL::Shader* ResourceManager::GetShader(ResourceId id) const {
    auto it = shaders_.find(id);
    return (it != shaders_.end()) ? it->second : nullptr;
}

LLGL::PipelineLayout* ResourceManager::GetPipelineLayout(ResourceId id) const {
    auto it = pipelineLayouts_.find(id);
    return (it != pipelineLayouts_.end()) ? it->second : nullptr;
}

LLGL::ResourceHeap* ResourceManager::GetResourceHeap(ResourceId id) const {
    auto it = resourceHeaps_.find(id);
    return (it != resourceHeaps_.end()) ? it->second : nullptr;
}

LLGL::PipelineState* ResourceManager::GetPipelineState(ResourceId id) const {
    auto it = pipelineStates_.find(id);
    return (it != pipelineStates_.end()) ? it->second : nullptr;
}

const RenderObject* ResourceManager::GetRenderObject(ResourceId id) const {
    auto it = renderObjects_.find(id);
    return (it != renderObjects_.end()) ? it->second.get() : nullptr;
}

// === Resource Release ===

void ResourceManager::ReleaseBuffer(ResourceId id) {
    // Try to find and release from vertex buffers
    auto vertexIt = vertexBuffers_.find(id);
    if (vertexIt != vertexBuffers_.end()) {
        if (vertexIt->second) {
            renderSystem_->Release(*vertexIt->second);
        }
        vertexBuffers_.erase(vertexIt);
        std::cout << "Released vertex buffer (ID: " << id << ")" << std::endl;
        return;
    }
    
    // Try to find and release from index buffers
    auto indexIt = indexBuffers_.find(id);
    if (indexIt != indexBuffers_.end()) {
        if (indexIt->second) {
            renderSystem_->Release(*indexIt->second);
        }
        indexBuffers_.erase(indexIt);
        std::cout << "Released index buffer (ID: " << id << ")" << std::endl;
        return;
    }
    
    // Try to find and release from constant buffers
    auto constantIt = constantBuffers_.find(id);
    if (constantIt != constantBuffers_.end()) {
        if (constantIt->second) {
            renderSystem_->Release(*constantIt->second);
        }
        constantBuffers_.erase(constantIt);
        std::cout << "Released constant buffer (ID: " << id << ")" << std::endl;
        return;
    }
    
    std::cerr << "Buffer with ID " << id << " not found" << std::endl;
}

void ResourceManager::ReleaseTexture(ResourceId id) {
    auto it = textures_.find(id);
    if (it != textures_.end()) {
        if (it->second) {
            renderSystem_->Release(*it->second);
        }
        textures_.erase(it);
        std::cout << "Released texture (ID: " << id << ")" << std::endl;
    } else {
        std::cerr << "Texture with ID " << id << " not found" << std::endl;
    }
}

void ResourceManager::ReleaseSampler(ResourceId id) {
    auto it = samplers_.find(id);
    if (it != samplers_.end()) {
        if (it->second) {
            renderSystem_->Release(*it->second);
        }
        samplers_.erase(it);
        std::cout << "Released sampler (ID: " << id << ")" << std::endl;
    } else {
        std::cerr << "Sampler with ID " << id << " not found" << std::endl;
    }
}

void ResourceManager::ReleaseShader(ResourceId id) {
    auto it = shaders_.find(id);
    if (it != shaders_.end()) {
        if (it->second) {
            renderSystem_->Release(*it->second);
        }
        shaders_.erase(it);
        std::cout << "Released shader (ID: " << id << ")" << std::endl;
    } else {
        std::cerr << "Shader with ID " << id << " not found" << std::endl;
    }
}

void ResourceManager::ReleasePipelineLayout(ResourceId id) {
    auto it = pipelineLayouts_.find(id);
    if (it != pipelineLayouts_.end()) {
        if (it->second) {
            renderSystem_->Release(*it->second);
        }
        pipelineLayouts_.erase(it);
        std::cout << "Released pipeline layout (ID: " << id << ")" << std::endl;
    } else {
        std::cerr << "Pipeline layout with ID " << id << " not found" << std::endl;
    }
}

void ResourceManager::ReleaseResourceHeap(ResourceId id) {
    auto it = resourceHeaps_.find(id);
    if (it != resourceHeaps_.end()) {
        if (it->second) {
            renderSystem_->Release(*it->second);
        }
        resourceHeaps_.erase(it);
        std::cout << "Released resource heap (ID: " << id << ")" << std::endl;
    } else {
        std::cerr << "Resource heap with ID " << id << " not found" << std::endl;
    }
}

void ResourceManager::ReleasePipelineState(ResourceId id) {
    auto it = pipelineStates_.find(id);
    if (it != pipelineStates_.end()) {
        if (it->second) {
            renderSystem_->Release(*it->second);
        }
        pipelineStates_.erase(it);
        std::cout << "Released pipeline state (ID: " << id << ")" << std::endl;
    } else {
        std::cerr << "Pipeline state with ID " << id << " not found" << std::endl;
    }
}

// === Resource Statistics ===

ResourceStats ResourceManager::GetResourceStats() const {
    ResourceStats stats;
    stats.vertexBufferCount = vertexBuffers_.size();
    stats.indexBufferCount = indexBuffers_.size();
    stats.constantBufferCount = constantBuffers_.size();
    stats.textureCount = textures_.size();
    stats.samplerCount = samplers_.size();
    stats.shaderCount = shaders_.size();
    stats.pipelineLayoutCount = pipelineLayouts_.size();
    stats.resourceHeapCount = resourceHeaps_.size();
    stats.pipelineStateCount = pipelineStates_.size();
    stats.renderObjectCount = renderObjects_.size();
    stats.totalResourceCount = stats.vertexBufferCount + stats.indexBufferCount + 
                              stats.constantBufferCount + stats.textureCount + 
                              stats.samplerCount + stats.shaderCount + 
                              stats.pipelineLayoutCount + stats.resourceHeapCount + 
                              stats.pipelineStateCount + stats.renderObjectCount;
    
    return stats;
}

void ResourceManager::PrintResourceStats() const {
    ResourceStats stats = GetResourceStats();
    
    std::cout << "=== Resource Statistics ===" << std::endl;
    std::cout << "Vertex Buffers: " << stats.vertexBufferCount << std::endl;
    std::cout << "Index Buffers: " << stats.indexBufferCount << std::endl;
    std::cout << "Constant Buffers: " << stats.constantBufferCount << std::endl;
    std::cout << "Textures: " << stats.textureCount << std::endl;
    std::cout << "Samplers: " << stats.samplerCount << std::endl;
    std::cout << "Shaders: " << stats.shaderCount << std::endl;
    std::cout << "Pipeline Layouts: " << stats.pipelineLayoutCount << std::endl;
    std::cout << "Resource Heaps: " << stats.resourceHeapCount << std::endl;
    std::cout << "Pipeline States: " << stats.pipelineStateCount << std::endl;
    std::cout << "Render Objects: " << stats.renderObjectCount << std::endl;
    std::cout << "Total Resources: " << stats.totalResourceCount << std::endl;
    std::cout << "==========================" << std::endl;
}

std::vector<ResourceId> ResourceManager::GetAllRenderObjects() const {
    std::vector<ResourceId> objectIds;
    objectIds.reserve(renderObjects_.size());
    
    for (const auto& pair : renderObjects_) {
        objectIds.push_back(pair.first);
    }
    
    return objectIds;
}

std::vector<ResourceId> ResourceManager::GetVisibleRenderObjects() const {
    std::vector<ResourceId> visibleObjects;
    
    for (const auto& pair : renderObjects_) {
        if (pair.second->visible) {
            visibleObjects.push_back(pair.first);
        }
    }
    
    return visibleObjects;
}

} // namespace RenderingPlugin
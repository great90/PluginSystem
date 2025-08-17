/**
 * @file GeometryGenerator.cpp
 * @brief Implementation of GeometryGenerator class
 */

#include "../include/GeometryGenerator.h"
#include "ResourceManager.h"
#include "MathTypes.h"
#include <LLGL/LLGL.h>
#include <LLGL/Utils/VertexFormat.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <unordered_map>

namespace RenderingPlugin {

// === Constants ===
static const float PI = 3.14159265359f;
static const float TWO_PI = 2.0f * PI;

// === Helper Functions ===

static Gs::Vector3f CalculateNormal(const Gs::Vector3f& v0, const Gs::Vector3f& v1, const Gs::Vector3f& v2) {
    Gs::Vector3f edge1 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
    Gs::Vector3f edge2 = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
    
    // Cross product
    Gs::Vector3f normal = {
        edge1.y * edge2.z - edge1.z * edge2.y,
        edge1.z * edge2.x - edge1.x * edge2.z,
        edge1.x * edge2.y - edge1.y * edge2.x
    };
    
    // Normalize
    float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length > 0.0f) {
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }
    
    return normal;
}

static Gs::Vector3f Normalize(const Gs::Vector3f& v) {
    float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length > 0.0f) {
        return { v.x / length, v.y / length, v.z / length };
    }
    return { 0.0f, 0.0f, 0.0f };
}

static float Dot(const Gs::Vector3f& a, const Gs::Vector3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Gs::Vector3f Cross(const Gs::Vector3f& a, const Gs::Vector3f& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// === GeometryGenerator Implementation ===

// === Basic Primitives ===

MeshData GeometryGenerator::GenerateTriangle(float size, const GeometryParams& params) {
    MeshData mesh;
    
    // Use provided size parameter
    
    // Create equilateral triangle vertices
    mesh.vertices = {
        {{ 0.0f,  size * 0.866f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.0f}},  // Top
        {{-size * 0.5f, -size * 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // Bottom left
        {{ size * 0.5f, -size * 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}   // Bottom right
    };
    
    mesh.indices = { 0, 1, 2 };
    
    return mesh;
}

MeshData GeometryGenerator::GenerateQuad(float width, float height, const GeometryParams& params) {
    MeshData mesh;
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Create quad vertices
    mesh.vertices = {
        {{-halfWidth, -halfHeight, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // Bottom left
        {{ halfWidth, -halfHeight, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},  // Bottom right
        {{ halfWidth,  halfHeight, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // Top right
        {{-halfWidth,  halfHeight, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}   // Top left
    };
    
    mesh.indices = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };
    
    return mesh;
}

MeshData GeometryGenerator::GenerateCube(float size, const GeometryParams& params) {
    MeshData mesh;
    
    float halfSize = size * 0.5f;
    
    // Create cube vertices (24 vertices for proper normals and UVs)
    mesh.vertices = {
        // Front face
        {{-halfSize, -halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ halfSize, -halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ halfSize,  halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-halfSize,  halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        
        // Back face
        {{ halfSize, -halfSize, -halfSize}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-halfSize, -halfSize, -halfSize}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-halfSize,  halfSize, -halfSize}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{ halfSize,  halfSize, -halfSize}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        
        // Left face
        {{-halfSize, -halfSize, -halfSize}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{-halfSize, -halfSize,  halfSize}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-halfSize,  halfSize,  halfSize}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-halfSize,  halfSize, -halfSize}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        
        // Right face
        {{ halfSize, -halfSize,  halfSize}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{ halfSize, -halfSize, -halfSize}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{ halfSize,  halfSize, -halfSize}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{ halfSize,  halfSize,  halfSize}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        
        // Top face
        {{-halfSize,  halfSize,  halfSize}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ halfSize,  halfSize,  halfSize}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{ halfSize,  halfSize, -halfSize}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-halfSize,  halfSize, -halfSize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        
        // Bottom face
        {{-halfSize, -halfSize, -halfSize}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ halfSize, -halfSize, -halfSize}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{ halfSize, -halfSize,  halfSize}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-halfSize, -halfSize,  halfSize}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}
    };
    
    // Create cube indices
    mesh.indices = {
        // Front face
        0, 1, 2,   2, 3, 0,
        // Back face
        4, 5, 6,   6, 7, 4,
        // Left face
        8, 9, 10,  10, 11, 8,
        // Right face
        12, 13, 14, 14, 15, 12,
        // Top face
        16, 17, 18, 18, 19, 16,
        // Bottom face
        20, 21, 22, 22, 23, 20
    };
    
    return mesh;
}

MeshData GeometryGenerator::GenerateBox(float width, float height, float depth, const GeometryParams& params) {
    MeshData mesh;
    
    float halfX = width * 0.5f;
    float halfY = height * 0.5f;
    float halfZ = depth * 0.5f;
    
    // Create box vertices (24 vertices for proper normals and UVs)
    mesh.vertices = {
        // Front face
        {{-halfX, -halfY,  halfZ}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ halfX, -halfY,  halfZ}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ halfX,  halfY,  halfZ}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-halfX,  halfY,  halfZ}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        
        // Back face
        {{ halfX, -halfY, -halfZ}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-halfX, -halfY, -halfZ}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-halfX,  halfY, -halfZ}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{ halfX,  halfY, -halfZ}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        
        // Left face
        {{-halfX, -halfY, -halfZ}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{-halfX, -halfY,  halfZ}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-halfX,  halfY,  halfZ}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-halfX,  halfY, -halfZ}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        
        // Right face
        {{ halfX, -halfY,  halfZ}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{ halfX, -halfY, -halfZ}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{ halfX,  halfY, -halfZ}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{ halfX,  halfY,  halfZ}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        
        // Top face
        {{-halfX,  halfY,  halfZ}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ halfX,  halfY,  halfZ}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{ halfX,  halfY, -halfZ}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-halfX,  halfY, -halfZ}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        
        // Bottom face
        {{-halfX, -halfY, -halfZ}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ halfX, -halfY, -halfZ}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{ halfX, -halfY,  halfZ}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-halfX, -halfY,  halfZ}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}
    };
    
    // Create box indices (same as cube)
    mesh.indices = {
        // Front face
        0, 1, 2,   2, 3, 0,
        // Back face
        4, 5, 6,   6, 7, 4,
        // Left face
        8, 9, 10,  10, 11, 8,
        // Right face
        12, 13, 14, 14, 15, 12,
        // Top face
        16, 17, 18, 18, 19, 16,
        // Bottom face
        20, 21, 22, 22, 23, 20
    };
    
    return mesh;
}

// === Spheres ===

MeshData GeometryGenerator::GenerateSphere(float radius, std::uint32_t slices, std::uint32_t stacks, const GeometryParams& params) {
    MeshData mesh;
    
    // Generate vertices
    for (uint32_t stack = 0; stack <= stacks; ++stack) {
        float phi = PI * static_cast<float>(stack) / static_cast<float>(stacks);
        float y = radius * std::cos(phi);
        float radiusAtY = radius * std::sin(phi);
        
        for (uint32_t slice = 0; slice <= slices; ++slice) {
            float theta = TWO_PI * static_cast<float>(slice) / static_cast<float>(slices);
            float x = radiusAtY * std::cos(theta);
            float z = radiusAtY * std::sin(theta);
            
            Gs::Vector3f position = { x, y, z };
            Gs::Vector3f normal = Normalize(position);
            Gs::Vector2f texCoord = {
                static_cast<float>(slice) / static_cast<float>(slices),
                static_cast<float>(stack) / static_cast<float>(stacks)
            };
            
            mesh.vertices.push_back({ position, normal, texCoord });
        }
    }
    
    // Generate indices
    for (uint32_t stack = 0; stack < stacks; ++stack) {
        for (uint32_t slice = 0; slice < slices; ++slice) {
            uint32_t current = stack * (slices + 1) + slice;
            uint32_t next = current + slices + 1;
            
            // First triangle
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);
            
            // Second triangle
            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }
    
    return mesh;
}

MeshData GeometryGenerator::GenerateUVSphere(float radius, std::uint32_t longitudeSegments, std::uint32_t latitudeSegments, const GeometryParams& params) {
    return GenerateSphere(radius, longitudeSegments, latitudeSegments, params);
}

MeshData GeometryGenerator::GenerateIcosphere(float radius, std::uint32_t subdivisions, const GeometryParams& params) {
    MeshData mesh;
    
    // Golden ratio
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
    
    // Create initial icosahedron vertices
    std::vector<Gs::Vector3f> vertices = {
        {-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
        {0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
        {t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1}
    };
    
    // Normalize to unit sphere
    for (auto& v : vertices) {
        v = Normalize(v);
        v.x *= radius;
        v.y *= radius;
        v.z *= radius;
    }
    
    // Create initial icosahedron faces
    std::vector<uint32_t> indices = {
        0, 11, 5,   0, 5, 1,    0, 1, 7,    0, 7, 10,   0, 10, 11,
        1, 5, 9,    5, 11, 4,   11, 10, 2,  10, 7, 6,   7, 1, 8,
        3, 9, 4,    3, 4, 2,    3, 2, 6,    3, 6, 8,    3, 8, 9,
        4, 9, 5,    2, 4, 11,   6, 2, 10,   8, 6, 7,    9, 8, 1
    };
    
    // Subdivide
    for (uint32_t i = 0; i < subdivisions; ++i) {
        std::vector<uint32_t> newIndices;
        std::unordered_map<uint64_t, uint32_t> midpointCache;
        
        auto getMidpoint = [&](uint32_t i1, uint32_t i2) -> uint32_t {
            uint64_t key = (static_cast<uint64_t>(std::min(i1, i2)) << 32) | std::max(i1, i2);
            auto it = midpointCache.find(key);
            if (it != midpointCache.end()) {
                return it->second;
            }
            
            Gs::Vector3f mid = {
                (vertices[i1].x + vertices[i2].x) * 0.5f,
                (vertices[i1].y + vertices[i2].y) * 0.5f,
                (vertices[i1].z + vertices[i2].z) * 0.5f
            };
            mid = Normalize(mid);
            mid.x *= radius;
            mid.y *= radius;
            mid.z *= radius;
            
            uint32_t index = static_cast<uint32_t>(vertices.size());
            vertices.push_back(mid);
            midpointCache[key] = index;
            return index;
        };
        
        for (size_t j = 0; j < indices.size(); j += 3) {
            uint32_t v1 = indices[j];
            uint32_t v2 = indices[j + 1];
            uint32_t v3 = indices[j + 2];
            
            uint32_t a = getMidpoint(v1, v2);
            uint32_t b = getMidpoint(v2, v3);
            uint32_t c = getMidpoint(v3, v1);
            
            newIndices.insert(newIndices.end(), {v1, a, c});
            newIndices.insert(newIndices.end(), {v2, b, a});
            newIndices.insert(newIndices.end(), {v3, c, b});
            newIndices.insert(newIndices.end(), {a, b, c});
        }
        
        indices = std::move(newIndices);
    }
    
    // Convert to mesh format
    for (const auto& v : vertices) {
        Gs::Vector3f normal = Normalize(v);
        Gs::Vector2f texCoord = {
            0.5f + std::atan2(normal.z, normal.x) / TWO_PI,
            0.5f - std::asin(normal.y) / PI
        };
        mesh.vertices.push_back({ v, normal, texCoord });
    }
    
    mesh.indices = indices;
    
    return mesh;
}

// === Cylinders ===

MeshData GeometryGenerator::GenerateCylinder(float topRadius, float bottomRadius, float height, 
                                           std::uint32_t slices, std::uint32_t stacks, const GeometryParams& params) {
    MeshData mesh;
    
    float stackHeight = height / static_cast<float>(stacks);
    float radiusStep = (topRadius - bottomRadius) / static_cast<float>(stacks);
    
    // Generate vertices for each stack
    for (uint32_t stack = 0; stack <= stacks; ++stack) {
        float y = -height * 0.5f + static_cast<float>(stack) * stackHeight;
        float radius = bottomRadius + static_cast<float>(stack) * radiusStep;
        
        for (uint32_t slice = 0; slice <= slices; ++slice) {
            float theta = TWO_PI * static_cast<float>(slice) / static_cast<float>(slices);
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);
            
            Gs::Vector3f position = { x, y, z };
            
            // Calculate normal (for cone/cylinder)
            Gs::Vector3f normal;
            if (topRadius == bottomRadius) {
                // Cylinder - normal is horizontal
                normal = Normalize({ x, 0.0f, z });
            } else {
                // Cone - calculate slanted normal
                float dr = bottomRadius - topRadius;
                normal = Normalize({ x * height, dr, z * height });
            }
            
            Gs::Vector2f texCoord = {
                static_cast<float>(slice) / static_cast<float>(slices),
                static_cast<float>(stack) / static_cast<float>(stacks)
            };
            
            mesh.vertices.push_back({ position, normal, texCoord });
        }
    }
    
    // Generate side indices
    for (uint32_t stack = 0; stack < stacks; ++stack) {
        for (uint32_t slice = 0; slice < slices; ++slice) {
            uint32_t current = stack * (slices + 1) + slice;
            uint32_t next = current + slices + 1;
            
            // First triangle
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);
            
            // Second triangle
            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }
    
    // Add top cap if topRadius > 0
    if (topRadius > 0.0f) {
        uint32_t topCenterIndex = static_cast<uint32_t>(mesh.vertices.size());
        mesh.vertices.push_back({ {0.0f, height * 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f} });
        
        for (uint32_t slice = 0; slice < slices; ++slice) {
            uint32_t current = stacks * (slices + 1) + slice;
            uint32_t next = current + 1;
            
            mesh.indices.push_back(topCenterIndex);
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
        }
    }
    
    // Add bottom cap if bottomRadius > 0
    if (bottomRadius > 0.0f) {
        uint32_t bottomCenterIndex = static_cast<uint32_t>(mesh.vertices.size());
        mesh.vertices.push_back({ {0.0f, -height * 0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f} });
        
        for (uint32_t slice = 0; slice < slices; ++slice) {
            uint32_t current = slice;
            uint32_t next = current + 1;
            
            mesh.indices.push_back(bottomCenterIndex);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current);
        }
    }
    
    return mesh;
}

MeshData GeometryGenerator::GenerateCone(float radius, float height, uint32_t slices, uint32_t stacks, const GeometryParams& params) {
    return GenerateCylinder(0.0f, radius, height, slices, stacks, params);
}

MeshData GeometryGenerator::GenerateCapsule(float radius, float height, uint32_t slices, uint32_t rings, const GeometryParams& params) {
    MeshData mesh;
    
    float cylinderHeight = height - 2.0f * radius;
    
    // Create cylinder part
    MeshData cylinder = GenerateCylinder(radius, radius, cylinderHeight, slices, 1, params);
    
    // Create top hemisphere
    MeshData topSphere = GenerateSphere(radius, slices, rings / 2, params);
    
    // Create bottom hemisphere
    MeshData bottomSphere = GenerateSphere(radius, slices, rings / 2, params);
    
    // Offset hemispheres
    float offset = cylinderHeight * 0.5f;
    for (auto& vertex : topSphere.vertices) {
        vertex.position.y += offset;
    }
    for (auto& vertex : bottomSphere.vertices) {
        vertex.position.y -= offset;
        // Flip bottom hemisphere
        vertex.position.y = -vertex.position.y - cylinderHeight;
    }
    
    // Combine meshes
    mesh = cylinder;
    
    uint32_t baseIndex = static_cast<uint32_t>(mesh.vertices.size());
    mesh.vertices.insert(mesh.vertices.end(), topSphere.vertices.begin(), topSphere.vertices.end());
    for (uint32_t index : topSphere.indices) {
        mesh.indices.push_back(index + baseIndex);
    }
    
    baseIndex = static_cast<uint32_t>(mesh.vertices.size());
    mesh.vertices.insert(mesh.vertices.end(), bottomSphere.vertices.begin(), bottomSphere.vertices.end());
    for (uint32_t index : bottomSphere.indices) {
        mesh.indices.push_back(index + baseIndex);
    }
    
    return mesh;
}

// === Torus ===

MeshData GeometryGenerator::GenerateTorus(float majorRadius, float minorRadius, uint32_t majorSegments, uint32_t minorSegments, const GeometryParams& params) {
    MeshData mesh;
    
    // Generate vertices
    for (uint32_t i = 0; i <= majorSegments; ++i) {
        float u = TWO_PI * static_cast<float>(i) / static_cast<float>(majorSegments);
        float cosU = std::cos(u);
        float sinU = std::sin(u);
        
        for (uint32_t j = 0; j <= minorSegments; ++j) {
            float v = TWO_PI * static_cast<float>(j) / static_cast<float>(minorSegments);
            float cosV = std::cos(v);
            float sinV = std::sin(v);
            
            Gs::Vector3f position = {
                (majorRadius + minorRadius * cosV) * cosU,
                minorRadius * sinV,
                (majorRadius + minorRadius * cosV) * sinU
            };
            
            Gs::Vector3f normal = {
                cosV * cosU,
                sinV,
                cosV * sinU
            };
            
            Gs::Vector2f texCoord = {
                static_cast<float>(i) / static_cast<float>(majorSegments),
                static_cast<float>(j) / static_cast<float>(minorSegments)
            };
            
            mesh.vertices.push_back({ position, normal, texCoord });
        }
    }
    
    // Generate indices
    for (uint32_t i = 0; i < majorSegments; ++i) {
        for (uint32_t j = 0; j < minorSegments; ++j) {
            uint32_t current = i * (minorSegments + 1) + j;
            uint32_t next = current + minorSegments + 1;
            
            // First triangle
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);
            
            // Second triangle
            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }
    
    return mesh;
}

// === Planes ===

MeshData GeometryGenerator::GeneratePlane(float width, float height, uint32_t widthSegments, uint32_t heightSegments, const GeometryParams& params) {
    MeshData mesh;
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Generate vertices
    for (uint32_t y = 0; y <= heightSegments; ++y) {
        for (uint32_t x = 0; x <= widthSegments; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(widthSegments);
            float v = static_cast<float>(y) / static_cast<float>(heightSegments);
            
            Gs::Vector3f position = {
                (u - 0.5f) * width,
                0.0f,
                (v - 0.5f) * height
            };
            
            Gs::Vector3f normal = { 0.0f, 1.0f, 0.0f };
            Gs::Vector2f texCoord = { u, v };
            
            mesh.vertices.push_back({ position, normal, texCoord });
        }
    }
    
    // Generate indices
    for (uint32_t y = 0; y < heightSegments; ++y) {
        for (uint32_t x = 0; x < widthSegments; ++x) {
            uint32_t current = y * (widthSegments + 1) + x;
            uint32_t next = current + widthSegments + 1;
            
            // First triangle
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);
            
            // Second triangle
            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }
    
    return mesh;
}

MeshData GeometryGenerator::GenerateCircle(float radius, uint32_t segments, const GeometryParams& params) {
    MeshData mesh;
    
    // Center vertex
    mesh.vertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.5f} });
    
    // Edge vertices
    for (uint32_t i = 0; i <= segments; ++i) {
        float angle = TWO_PI * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        
        Gs::Vector3f position = { x, y, 0.0f };
        Gs::Vector3f normal = { 0.0f, 0.0f, 1.0f };
        Gs::Vector2f texCoord = {
            0.5f + 0.5f * std::cos(angle),
            0.5f + 0.5f * std::sin(angle)
        };
        
        mesh.vertices.push_back({ position, normal, texCoord });
    }
    
    // Generate indices
    for (uint32_t i = 0; i < segments; ++i) {
        mesh.indices.push_back(0);
        mesh.indices.push_back(i + 1);
        mesh.indices.push_back(i + 2);
    }
    
    return mesh;
}

MeshData GeometryGenerator::GenerateRing(float innerRadius, float outerRadius, uint32_t segments, const GeometryParams& params) {
    MeshData mesh;
    
    // Generate vertices
    for (uint32_t i = 0; i <= segments; ++i) {
        float angle = TWO_PI * static_cast<float>(i) / static_cast<float>(segments);
        float cosAngle = std::cos(angle);
        float sinAngle = std::sin(angle);
        
        // Inner vertex
        Gs::Vector3f innerPos = { innerRadius * cosAngle, innerRadius * sinAngle, 0.0f };
        Gs::Vector3f normal = { 0.0f, 0.0f, 1.0f };
        Gs::Vector2f innerTexCoord = {
            0.5f + 0.5f * innerRadius * cosAngle / outerRadius,
            0.5f + 0.5f * innerRadius * sinAngle / outerRadius
        };
        mesh.vertices.push_back({ innerPos, normal, innerTexCoord });
        
        // Outer vertex
        Gs::Vector3f outerPos = { outerRadius * cosAngle, outerRadius * sinAngle, 0.0f };
        Gs::Vector2f outerTexCoord = {
            0.5f + 0.5f * cosAngle,
            0.5f + 0.5f * sinAngle
        };
        mesh.vertices.push_back({ outerPos, normal, outerTexCoord });
    }
    
    // Generate indices
    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t current = i * 2;
        uint32_t next = current + 2;
        
        // First triangle
        mesh.indices.push_back(current);
        mesh.indices.push_back(next);
        mesh.indices.push_back(current + 1);
        
        // Second triangle
        mesh.indices.push_back(current + 1);
        mesh.indices.push_back(next);
        mesh.indices.push_back(next + 1);
    }
    
    return mesh;
}

// === Advanced Primitives ===

MeshData GeometryGenerator::GenerateTetrahedron(float size, const GeometryParams& params) {
    MeshData mesh;
    
    float a = size / std::sqrt(3.0f);
    
    // Tetrahedron vertices
    mesh.vertices = {
        {{ a,  a,  a}, Normalize({ 1,  1,  1}), {0.0f, 0.0f}},
        {{-a, -a,  a}, Normalize({-1, -1,  1}), {1.0f, 0.0f}},
        {{-a,  a, -a}, Normalize({-1,  1, -1}), {0.5f, 1.0f}},
        {{ a, -a, -a}, Normalize({ 1, -1, -1}), {0.0f, 1.0f}}
    };
    
    mesh.indices = {
        0, 1, 2,  // Face 1
        0, 2, 3,  // Face 2
        0, 3, 1,  // Face 3
        1, 3, 2   // Face 4
    };
    
    return mesh;
}

MeshData GeometryGenerator::GenerateOctahedron(float size, const GeometryParams& params) {
    MeshData mesh;
    
    // Octahedron vertices
    mesh.vertices = {
        {{ 0,  size,  0}, { 0,  1,  0}, {0.5f, 0.0f}},  // Top
        {{ 0, -size,  0}, { 0, -1,  0}, {0.5f, 1.0f}},  // Bottom
        {{ size,  0,  0}, { 1,  0,  0}, {1.0f, 0.5f}},  // Right
        {{-size,  0,  0}, {-1,  0,  0}, {0.0f, 0.5f}},  // Left
        {{ 0,  0,  size}, { 0,  0,  1}, {0.5f, 0.5f}},  // Front
        {{ 0,  0, -size}, { 0,  0, -1}, {0.5f, 0.5f}}   // Back
    };
    
    mesh.indices = {
        0, 4, 2,  0, 2, 5,  0, 5, 3,  0, 3, 4,  // Top faces
        1, 2, 4,  1, 5, 2,  1, 3, 5,  1, 4, 3   // Bottom faces
    };
    
    return mesh;
}

MeshData GeometryGenerator::GenerateDodecahedron(float size, const GeometryParams& params) {
    // This is a simplified implementation - a full dodecahedron would be more complex
    MeshData mesh;
    
    const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f; // Golden ratio
    const float a = size / std::sqrt(3.0f);
    const float b = a / phi;
    const float c = a * phi;
    
    // Dodecahedron has 20 vertices - this is a simplified version
    // For a complete implementation, you would need all 20 vertices and 36 triangular faces
    
    // Return a simple cube for now as placeholder
    return GenerateCube(size, params);
}

MeshData GeometryGenerator::GenerateIcosahedron(float size, const GeometryParams& params) {
    MeshData mesh;
    
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f; // Golden ratio
    const float s = size / std::sqrt(1.0f + t * t);
    
    // Icosahedron vertices
    mesh.vertices = {
        {{-s, t*s, 0}, Normalize({-1, t, 0}), {0.0f, 0.0f}},
        {{ s, t*s, 0}, Normalize({ 1, t, 0}), {1.0f, 0.0f}},
        {{-s,-t*s, 0}, Normalize({-1,-t, 0}), {0.0f, 1.0f}},
        {{ s,-t*s, 0}, Normalize({ 1,-t, 0}), {1.0f, 1.0f}},
        
        {{ 0,-s, t*s}, Normalize({ 0,-1, t}), {0.5f, 0.0f}},
        {{ 0, s, t*s}, Normalize({ 0, 1, t}), {0.5f, 1.0f}},
        {{ 0,-s,-t*s}, Normalize({ 0,-1,-t}), {0.5f, 0.0f}},
        {{ 0, s,-t*s}, Normalize({ 0, 1,-t}), {0.5f, 1.0f}},
        
        {{ t*s, 0,-s}, Normalize({ t, 0,-1}), {0.0f, 0.5f}},
        {{ t*s, 0, s}, Normalize({ t, 0, 1}), {1.0f, 0.5f}},
        {{-t*s, 0,-s}, Normalize({-t, 0,-1}), {0.0f, 0.5f}},
        {{-t*s, 0, s}, Normalize({-t, 0, 1}), {1.0f, 0.5f}}
    };
    
    mesh.indices = {
        0, 11, 5,   0, 5, 1,    0, 1, 7,    0, 7, 10,   0, 10, 11,
        1, 5, 9,    5, 11, 4,   11, 10, 2,  10, 7, 6,   7, 1, 8,
        3, 9, 4,    3, 4, 2,    3, 2, 6,    3, 6, 8,    3, 8, 9,
        4, 9, 5,    2, 4, 11,   6, 2, 10,   8, 6, 7,    9, 8, 1
    };
    
    return mesh;
}

// === Utility Functions ===

bool GeometryGenerator::CreateBuffersFromMesh(const MeshData& meshData, ResourceManager* resourceManager,
                                            LLGL::Buffer*& vertexBuffer, LLGL::Buffer*& indexBuffer) {
    if (!resourceManager) {
        return false;
    }
    
    try {
        // Create vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "normal", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "color", LLGL::Format::RGB32Float });
        
        // Create vertex buffer using ResourceManager
        ResourceId vertexBufferId = resourceManager->CreateVertexBuffer(
            meshData.vertices.data(),
            meshData.vertices.size() * sizeof(Vertex),
            vertexFormat
        );
        
        // Create index buffer using ResourceManager
        ResourceId indexBufferId = resourceManager->CreateIndexBuffer(
            meshData.indices.data(),
            meshData.indices.size() * sizeof(uint32_t),
            LLGL::Format::R32UInt
        );
        
        // Get the actual buffer pointers
        vertexBuffer = resourceManager->GetVertexBuffer(vertexBufferId);
        indexBuffer = resourceManager->GetIndexBuffer(indexBufferId);
        
        return (vertexBuffer != nullptr && indexBuffer != nullptr);
    } catch (...) {
        return false;
    }
}

void GeometryGenerator::CalculateBounds(const MeshData& meshData, Gs::Vector3f& minBounds, Gs::Vector3f& maxBounds) {
    if (meshData.vertices.empty()) {
        minBounds = Gs::Vector3f(0, 0, 0);
        maxBounds = Gs::Vector3f(0, 0, 0);
        return;
    }
    
    minBounds = meshData.vertices[0].position;
    maxBounds = meshData.vertices[0].position;
    
    for (const auto& vertex : meshData.vertices) {
        if (vertex.position.x < minBounds.x) minBounds.x = vertex.position.x;
        if (vertex.position.y < minBounds.y) minBounds.y = vertex.position.y;
        if (vertex.position.z < minBounds.z) minBounds.z = vertex.position.z;
        
        if (vertex.position.x > maxBounds.x) maxBounds.x = vertex.position.x;
        if (vertex.position.y > maxBounds.y) maxBounds.y = vertex.position.y;
        if (vertex.position.z > maxBounds.z) maxBounds.z = vertex.position.z;
    }
}

Gs::Vector3f GeometryGenerator::CalculateCenter(const MeshData& meshData) {
    Gs::Vector3f minBounds, maxBounds;
    CalculateBounds(meshData, minBounds, maxBounds);
    return {
        (minBounds.x + maxBounds.x) * 0.5f,
        (minBounds.y + maxBounds.y) * 0.5f,
        (minBounds.z + maxBounds.z) * 0.5f
    };
}

void GeometryGenerator::TransformMesh(MeshData& meshData, const Gs::Matrix4f& transform) {
    // This is a simplified transform - in practice you'd want proper matrix math
    // For now, just apply translation from the matrix
    float tx = transform.At(3, 0);
    float ty = transform.At(3, 1);
    float tz = transform.At(3, 2);
    
    for (auto& vertex : meshData.vertices) {
        vertex.position.x += tx;
        vertex.position.y += ty;
        vertex.position.z += tz;
    }
}

MeshData GeometryGenerator::MergeMeshes(const MeshData* meshes, std::size_t meshCount) {
    MeshData result;
    
    uint32_t vertexOffset = 0;
    
    for (std::size_t i = 0; i < meshCount; ++i) {
        const auto& mesh = meshes[i];
        // Add vertices
        result.vertices.insert(result.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
        
        // Add indices with offset
        for (uint32_t index : mesh.indices) {
            result.indices.push_back(index + vertexOffset);
        }
        
        vertexOffset += static_cast<uint32_t>(mesh.vertices.size());
    }
    
    return result;
}

void GeometryGenerator::GenerateNormals(MeshData& meshData, bool smooth) {
    // Reset all normals
    for (auto& vertex : meshData.vertices) {
        vertex.normal = { 0.0f, 0.0f, 0.0f };
    }
    
    // Calculate face normals and accumulate
    for (size_t i = 0; i < meshData.indices.size(); i += 3) {
        uint32_t i0 = meshData.indices[i];
        uint32_t i1 = meshData.indices[i + 1];
        uint32_t i2 = meshData.indices[i + 2];
        
        Gs::Vector3f normal = CalculateFaceNormal(
            meshData.vertices[i0].position,
            meshData.vertices[i1].position,
            meshData.vertices[i2].position
        );
        
        if (smooth) {
            meshData.vertices[i0].normal = meshData.vertices[i0].normal + normal;
            meshData.vertices[i1].normal = meshData.vertices[i1].normal + normal;
            meshData.vertices[i2].normal = meshData.vertices[i2].normal + normal;
        } else {
            meshData.vertices[i0].normal = normal;
            meshData.vertices[i1].normal = normal;
            meshData.vertices[i2].normal = normal;
        }
    }
    
    // Normalize all normals
    if (smooth) {
        for (auto& vertex : meshData.vertices) {
            vertex.normal = Normalize(vertex.normal);
        }
    }
}

void GeometryGenerator::GenerateTangents(MeshData& mesh) {
    // This is a simplified tangent generation
    // In practice, you'd want to calculate proper tangent and bitangent vectors
    for (auto& vertex : mesh.vertices) {
        // Generate a tangent perpendicular to the normal
        Gs::Vector3f up = { 0.0f, 1.0f, 0.0f };
        if (std::abs(Dot(vertex.normal, up)) > 0.9f) {
            up = { 1.0f, 0.0f, 0.0f };
        }
        
        Gs::Vector3f tangent = Normalize(Cross(vertex.normal, up));
        // Store tangent in unused vertex data if needed
    }
}

void GeometryGenerator::FlipWindingOrder(MeshData& mesh) {
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        std::swap(mesh.indices[i + 1], mesh.indices[i + 2]);
    }
}

bool GeometryGenerator::ValidateMesh(const MeshData& mesh) {
    if (mesh.vertices.empty()) {
        std::cerr << "Mesh validation failed: No vertices" << std::endl;
        return false;
    }
    
    if (mesh.indices.size() % 3 != 0) {
        std::cerr << "Mesh validation failed: Index count not divisible by 3" << std::endl;
        return false;
    }
    
    // Check for out-of-bounds indices
    uint32_t maxIndex = static_cast<uint32_t>(mesh.vertices.size() - 1);
    for (uint32_t index : mesh.indices) {
        if (index > maxIndex) {
            std::cerr << "Mesh validation failed: Index " << index << " out of bounds (max: " << maxIndex << ")" << std::endl;
            return false;
        }
    }
    
    std::cout << "Mesh validation passed: " << mesh.vertices.size() << " vertices, " 
              << mesh.indices.size() / 3 << " triangles" << std::endl;
    
    return true;
}

} // namespace RenderingPlugin
/**
 * @file GeometryGenerator.h
 * @brief Geometry generation utility class
 * @details Provides utilities for generating common geometric shapes and meshes
 */

#pragma once

#include "RenderingPluginExport.h"
#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <vector>
#include <memory>

namespace RenderingPlugin {

// Forward declarations
struct Vertex;
class ResourceManager;

/**
 * @brief Mesh data structure
 * @details Contains vertex and index data for a generated mesh
 */
struct RENDERING_PLUGIN_API MeshData {
    std::vector<Vertex> vertices;    ///< Vertex data
    std::vector<std::uint32_t> indices;  ///< Index data
    
    /**
     * @brief Get vertex count
     * @return Number of vertices
     */
    std::size_t GetVertexCount() const { return vertices.size(); }
    
    /**
     * @brief Get index count
     * @return Number of indices
     */
    std::size_t GetIndexCount() const { return indices.size(); }
    
    /**
     * @brief Get triangle count
     * @return Number of triangles
     */
    std::size_t GetTriangleCount() const { return indices.size() / 3; }
    
    /**
     * @brief Clear all data
     */
    void Clear() {
        vertices.clear();
        indices.clear();
    }
    
    /**
     * @brief Check if mesh data is empty
     * @return true if empty, false otherwise
     */
    bool IsEmpty() const {
        return vertices.empty() || indices.empty();
    }
};

/**
 * @brief Geometry generation parameters
 */
struct RENDERING_PLUGIN_API GeometryParams {
    bool generateNormals = true;     ///< Generate vertex normals
    bool generateTexCoords = true;   ///< Generate texture coordinates
    bool generateTangents = false;   ///< Generate tangent vectors
    bool flipWindingOrder = false;   ///< Flip triangle winding order
    float textureScale = 1.0f;       ///< Texture coordinate scale factor
};

/**
 * @brief Geometry generator utility class
 * @details Provides static methods for generating common geometric shapes
 */
class RENDERING_PLUGIN_API GeometryGenerator {
public:
    // === Basic Primitives ===
    
    /**
     * @brief Generate a triangle mesh
     * @param size Triangle size
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateTriangle(float size = 1.0f, const GeometryParams& params = {});
    
    /**
     * @brief Generate a quad mesh
     * @param width Quad width
     * @param height Quad height
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateQuad(float width = 1.0f, float height = 1.0f, const GeometryParams& params = {});
    
    /**
     * @brief Generate a cube mesh
     * @param size Cube size
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateCube(float size = 1.0f, const GeometryParams& params = {});
    
    /**
     * @brief Generate a box mesh with different dimensions
     * @param width Box width
     * @param height Box height
     * @param depth Box depth
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateBox(float width, float height, float depth, const GeometryParams& params = {});
    
    // === Spherical Primitives ===
    
    /**
     * @brief Generate a sphere mesh
     * @param radius Sphere radius
     * @param slices Number of horizontal slices
     * @param stacks Number of vertical stacks
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateSphere(float radius = 1.0f, std::uint32_t slices = 32, std::uint32_t stacks = 16,
                                  const GeometryParams& params = {});
    
    /**
     * @brief Generate a UV sphere mesh (alternative sphere generation)
     * @param radius Sphere radius
     * @param longitudeSegments Number of longitude segments
     * @param latitudeSegments Number of latitude segments
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateUVSphere(float radius = 1.0f, std::uint32_t longitudeSegments = 32,
                                    std::uint32_t latitudeSegments = 16, const GeometryParams& params = {});
    
    /**
     * @brief Generate an icosphere mesh
     * @param radius Sphere radius
     * @param subdivisions Number of subdivisions
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateIcosphere(float radius = 1.0f, std::uint32_t subdivisions = 2,
                                     const GeometryParams& params = {});
    
    // === Cylindrical Primitives ===
    
    /**
     * @brief Generate a cylinder mesh
     * @param topRadius Top radius
     * @param bottomRadius Bottom radius
     * @param height Cylinder height
     * @param slices Number of slices around the axis
     * @param stacks Number of stacks along the height
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateCylinder(float topRadius = 1.0f, float bottomRadius = 1.0f, float height = 2.0f,
                                   std::uint32_t slices = 32, std::uint32_t stacks = 1,
                                   const GeometryParams& params = {});
    
    /**
     * @brief Generate a cone mesh
     * @param radius Base radius
     * @param height Cone height
     * @param slices Number of slices around the base
     * @param stacks Number of stacks along the height
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateCone(float radius = 1.0f, float height = 2.0f, std::uint32_t slices = 32,
                               std::uint32_t stacks = 1, const GeometryParams& params = {});
    
    /**
     * @brief Generate a capsule mesh
     * @param radius Capsule radius
     * @param height Capsule height (excluding hemispheres)
     * @param slices Number of slices
     * @param stacks Number of stacks for hemispheres
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateCapsule(float radius = 1.0f, float height = 2.0f, std::uint32_t slices = 32,
                                  std::uint32_t stacks = 8, const GeometryParams& params = {});
    
    // === Torus and Ring Primitives ===
    
    /**
     * @brief Generate a torus mesh
     * @param majorRadius Major radius (distance from center to tube center)
     * @param minorRadius Minor radius (tube radius)
     * @param majorSegments Number of segments around the major radius
     * @param minorSegments Number of segments around the minor radius
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateTorus(float majorRadius = 1.0f, float minorRadius = 0.3f,
                                std::uint32_t majorSegments = 32, std::uint32_t minorSegments = 16,
                                const GeometryParams& params = {});
    
    // === Planar Primitives ===
    
    /**
     * @brief Generate a plane mesh
     * @param width Plane width
     * @param depth Plane depth
     * @param widthSegments Number of width segments
     * @param depthSegments Number of depth segments
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GeneratePlane(float width = 1.0f, float depth = 1.0f, std::uint32_t widthSegments = 1,
                                std::uint32_t depthSegments = 1, const GeometryParams& params = {});
    
    /**
     * @brief Generate a circle mesh
     * @param radius Circle radius
     * @param segments Number of segments
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateCircle(float radius = 1.0f, std::uint32_t segments = 32,
                                 const GeometryParams& params = {});
    
    /**
     * @brief Generate a ring mesh
     * @param innerRadius Inner radius
     * @param outerRadius Outer radius
     * @param segments Number of segments
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateRing(float innerRadius = 0.5f, float outerRadius = 1.0f, std::uint32_t segments = 32,
                               const GeometryParams& params = {});
    
    // === Advanced Primitives ===
    
    /**
     * @brief Generate a tetrahedron mesh
     * @param size Tetrahedron size
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateTetrahedron(float size = 1.0f, const GeometryParams& params = {});
    
    /**
     * @brief Generate an octahedron mesh
     * @param size Octahedron size
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateOctahedron(float size = 1.0f, const GeometryParams& params = {});
    
    /**
     * @brief Generate a dodecahedron mesh
     * @param size Dodecahedron size
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateDodecahedron(float size = 1.0f, const GeometryParams& params = {});
    
    /**
     * @brief Generate an icosahedron mesh
     * @param size Icosahedron size
     * @param params Generation parameters
     * @return Generated mesh data
     */
    static MeshData GenerateIcosahedron(float size = 1.0f, const GeometryParams& params = {});
    
    // === Utility Functions ===
    
    /**
     * @brief Create LLGL buffers from mesh data
     * @param meshData Mesh data to create buffers from
     * @param resourceManager Resource manager to use for buffer creation
     * @param vertexBuffer Output vertex buffer
     * @param indexBuffer Output index buffer
     * @return true if successful, false otherwise
     */
    static bool CreateBuffersFromMesh(const MeshData& meshData, ResourceManager* resourceManager,
                                    LLGL::Buffer*& vertexBuffer, LLGL::Buffer*& indexBuffer);
    
    /**
     * @brief Calculate mesh bounding box
     * @param meshData Mesh data
     * @param minBounds Output minimum bounds
     * @param maxBounds Output maximum bounds
     */
    static void CalculateBounds(const MeshData& meshData, Gs::Vector3f& minBounds, Gs::Vector3f& maxBounds);
    
    /**
     * @brief Calculate mesh center point
     * @param meshData Mesh data
     * @return Center point
     */
    static Gs::Vector3f CalculateCenter(const MeshData& meshData);
    
    /**
     * @brief Transform mesh vertices
     * @param meshData Mesh data to transform
     * @param transform Transformation matrix
     */
    static void TransformMesh(MeshData& meshData, const Gs::Matrix4f& transform);
    
    /**
     * @brief Merge multiple meshes into one
     * @param meshes Array of meshes to merge
     * @param meshCount Number of meshes
     * @return Merged mesh data
     */
    static MeshData MergeMeshes(const MeshData* meshes, std::size_t meshCount);
    
    /**
     * @brief Generate normals for mesh data
     * @param meshData Mesh data to generate normals for
     * @param smooth Whether to generate smooth normals (true) or flat normals (false)
     */
    static void GenerateNormals(MeshData& meshData, bool smooth = true);
    
    /**
     * @brief Generate tangent vectors for mesh data
     * @param meshData Mesh data to generate tangents for
     */
    static void GenerateTangents(MeshData& meshData);
    
    /**
     * @brief Flip triangle winding order
     * @param meshData Mesh data to flip winding order for
     */
    static void FlipWindingOrder(MeshData& meshData);
    
    /**
     * @brief Validate mesh data
     * @param meshData Mesh data to validate
     * @return true if valid, false otherwise
     */
    static bool ValidateMesh(const MeshData& meshData);

private:
    // === Private Helper Methods ===
    
    /**
     * @brief Add vertex to mesh data
     * @param meshData Mesh data
     * @param position Vertex position
     * @param normal Vertex normal
     * @param texCoord Texture coordinate
     */
    static void AddVertex(MeshData& meshData, const Gs::Vector3f& position,
                         const Gs::Vector3f& normal, const Gs::Vector2f& texCoord);
    
    /**
     * @brief Add triangle to mesh data
     * @param meshData Mesh data
     * @param i0 First vertex index
     * @param i1 Second vertex index
     * @param i2 Third vertex index
     * @param flipWinding Whether to flip winding order
     */
    static void AddTriangle(MeshData& meshData, std::uint32_t i0, std::uint32_t i1, std::uint32_t i2,
                           bool flipWinding = false);
    
    /**
     * @brief Calculate face normal
     * @param v0 First vertex position
     * @param v1 Second vertex position
     * @param v2 Third vertex position
     * @return Face normal
     */
    static Gs::Vector3f CalculateFaceNormal(const Gs::Vector3f& v0, const Gs::Vector3f& v1, const Gs::Vector3f& v2);
    
    /**
     * @brief Subdivide triangle for icosphere generation
     * @param meshData Mesh data
     * @param v0 First vertex index
     * @param v1 Second vertex index
     * @param v2 Third vertex index
     * @param subdivisions Number of subdivisions remaining
     * @param radius Sphere radius
     */
    static void SubdivideTriangle(MeshData& meshData, std::uint32_t v0, std::uint32_t v1, std::uint32_t v2,
                                 std::uint32_t subdivisions, float radius);
};

} // namespace RenderingPlugin
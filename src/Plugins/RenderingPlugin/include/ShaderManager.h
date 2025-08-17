/**
 * @file ShaderManager.h
 * @brief Shader management utility class
 * @details Provides utilities for loading, compiling, and managing shaders
 */

#pragma once

#include "RenderingPluginExport.h"
#include <LLGL/LLGL.h>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <filesystem>
#include <ctime>
#include <regex>

namespace RenderingPlugin {

// Forward declarations
class ResourceManager;

// Type aliases
using IncludeResolver = std::function<std::string(const std::string&, const std::string&)>;

/**
 * @brief Shader type enumeration
 */
enum class RENDERING_PLUGIN_API ShaderType {
    Vertex,         ///< Vertex shader
    Fragment,       ///< Fragment/Pixel shader
    Geometry,       ///< Geometry shader
    TessControl,    ///< Tessellation control shader
    TessEvaluation, ///< Tessellation evaluation shader
    Compute         ///< Compute shader
};

/**
 * @brief Shader source information
 */
struct RENDERING_PLUGIN_API ShaderSource {
    ShaderType type;        ///< Shader type
    std::string source;     ///< Shader source code
    std::string entryPoint; ///< Entry point function name (default: "main")
    std::string profile;    ///< Shader profile/version (optional)
    std::string filePath;   ///< Source file path (for hot reload)
    
    /**
     * @brief Default constructor
     */
    ShaderSource() : type(ShaderType::Vertex), source(""), entryPoint("main"), profile(""), filePath("") {}
    
    /**
     * @brief Constructor
     * @param shaderType Shader type
     * @param sourceCode Shader source code
     * @param entry Entry point (default: "main")
     * @param shaderProfile Shader profile (optional)
     * @param sourceFilePath Source file path (optional)
     */
    ShaderSource(ShaderType shaderType, const std::string& sourceCode,
                const std::string& entry = "main", const std::string& shaderProfile = "",
                const std::string& sourceFilePath = "")
        : type(shaderType), source(sourceCode), entryPoint(entry), profile(shaderProfile), filePath(sourceFilePath) {}
};

/**
 * @brief Shader compilation options
 */
struct RENDERING_PLUGIN_API ShaderCompileOptions {
    bool enableOptimization = true;     ///< Enable shader optimization
    bool enableDebugInfo = false;       ///< Include debug information
    bool treatWarningsAsErrors = false; ///< Treat warnings as errors
    std::vector<std::string> defines;   ///< Preprocessor defines
    std::vector<std::string> includePaths; ///< Include search paths
    
    /**
     * @brief Add a preprocessor define
     * @param define Define string (e.g., "FEATURE_ENABLED=1")
     */
    void AddDefine(const std::string& define) {
        defines.push_back(define);
    }
    
    /**
     * @brief Add an include path
     * @param path Include path
     */
    void AddIncludePath(const std::string& path) {
        includePaths.push_back(path);
    }
};

/**
 * @brief Shader program description
 */
struct RENDERING_PLUGIN_API ShaderProgramDesc {
    std::string name;                           ///< Program name/identifier
    ShaderSource vertexShader;                 ///< Vertex shader source
    ShaderSource fragmentShader;               ///< Fragment shader source
    ShaderSource geometryShader;               ///< Geometry shader source
    ShaderSource tessControlShader;            ///< Tessellation control shader source
    ShaderSource tessEvaluationShader;         ///< Tessellation evaluation shader source
    ShaderSource computeShader;                ///< Compute shader source
    std::vector<LLGL::BindingDescriptor> bindings; ///< Resource bindings
    ShaderCompileOptions compileOptions;       ///< Compilation options
    
    /**
      * @brief Constructor
      */
     ShaderProgramDesc() = default;
};

/**
 * @brief Compiled shader program information
 */
struct RENDERING_PLUGIN_API CompiledShaderProgram {
    std::string name;                           ///< Program name
    LLGL::PipelineState* pipelineState;        ///< LLGL pipeline state
    LLGL::Shader* vertexShader;                ///< Vertex shader
    LLGL::Shader* fragmentShader;              ///< Fragment shader
    LLGL::Shader* geometryShader;              ///< Geometry shader
    std::unordered_map<std::string, std::uint32_t> uniformLocations; ///< Uniform locations cache
    std::unordered_map<std::string, std::uint32_t> attributeLocations; ///< Attribute locations cache
    bool isValid;                               ///< Whether the program is valid
    std::string errorLog;                       ///< Compilation error log (if any)
    
    /**
     * @brief Constructor
     */
    CompiledShaderProgram() : pipelineState(nullptr), vertexShader(nullptr), 
                             fragmentShader(nullptr), geometryShader(nullptr), isValid(false) {}
    
    /**
     * @brief Get uniform location by name
     * @param name Uniform name
     * @return Uniform location, or UINT32_MAX if not found
     */
    std::uint32_t GetUniformLocation(const std::string& name) const {
        auto it = uniformLocations.find(name);
        return (it != uniformLocations.end()) ? it->second : UINT32_MAX;
    }
    
    /**
     * @brief Get attribute location by name
     * @param name Attribute name
     * @return Attribute location, or UINT32_MAX if not found
     */
    std::uint32_t GetAttributeLocation(const std::string& name) const {
        auto it = attributeLocations.find(name);
        return (it != attributeLocations.end()) ? it->second : UINT32_MAX;
    }
};

/**
 * @brief Cached shader information
 */
struct CachedShader {
    LLGL::Shader* shader;           ///< LLGL shader object
    ShaderSource source;            ///< Original shader source
    ShaderCompileOptions options;   ///< Compilation options used
    std::string cacheKey;           ///< Cache key for this shader
    
    CachedShader() : shader(nullptr) {}
};

/**
 * @brief Shader manager class
 * @details Manages shader loading, compilation, and caching
 */
class RENDERING_PLUGIN_API ShaderManager {
public:
    /**
     * @brief Constructor
     * @param renderSystem Pointer to LLGL render system
     * @param resourceManager Pointer to resource manager
     */
    ShaderManager(LLGL::RenderSystem* renderSystem, ResourceManager* resourceManager);
    
    /**
     * @brief Destructor
     */
    ~ShaderManager();
    
    // === Shader Loading ===
    
    /**
     * @brief Load shader from file
     * @param filePath Path to shader file
     * @param type Shader type
     * @param entryPoint Entry point function name (default: "main")
     * @return Shader source, or empty source on failure
     */
    ShaderSource LoadShaderFromFile(const std::string& filePath, ShaderType type,
                                   const std::string& entryPoint = "main");
    
    /**
     * @brief Load shader from string
     * @param source Shader source code
     * @param type Shader type
     * @param entryPoint Entry point function name (default: "main")
     * @return Shader source
     */
    ShaderSource LoadShaderFromString(const std::string& source, ShaderType type,
                                     const std::string& entryPoint = "main");
    
    /**
     * @brief Load multiple shaders from directory
     * @param directoryPath Directory containing shader files
     * @param fileExtensions Map of file extensions to shader types (e.g., {".vert", ShaderType::Vertex})
     * @return Vector of loaded shader sources
     */
    std::vector<ShaderSource> LoadShadersFromDirectory(const std::string& directoryPath,
        const std::unordered_map<std::string, ShaderType>& fileExtensions);
    
    // === Shader Compilation ===
    
    /**
     * @brief Compile shader program
     * @param programDesc Shader program description
     * @param options Compilation options
     * @return Compiled shader program
     */
    CompiledShaderProgram CompileShaderProgram(const ShaderProgramDesc& programDesc,
                                              const ShaderCompileOptions& options = {});
    
    /**
     * @brief Compile shader program from files
     * @param programName Program name
     * @param shaderFiles Map of shader types to file paths
     * @param vertexAttributes Vertex attributes (optional)
     * @param options Compilation options
     * @return Compiled shader program
     */
    CompiledShaderProgram CompileShaderProgramFromFiles(const std::string& programName,
        const std::unordered_map<ShaderType, std::string>& shaderFiles,
        const std::vector<LLGL::VertexAttribute>& vertexAttributes = {},
        const ShaderCompileOptions& options = {});
    
    // === Shader Management ===
    
    /**
     * @brief Register compiled shader program
     * @param program Compiled shader program
     * @return true if successful, false otherwise
     */
    bool RegisterShaderProgram(const CompiledShaderProgram& program);
    
    /**
     * @brief Get shader program by name
     * @param name Program name
     * @return Pointer to compiled shader program, or nullptr if not found
     */
    const CompiledShaderProgram* GetShaderProgram(const std::string& name) const;
    
    /**
     * @brief Remove shader program
     * @param name Program name
     * @return true if removed, false if not found
     */
    bool RemoveShaderProgram(const std::string& name);
    
    /**
     * @brief Check if shader program exists
     * @param name Program name
     * @return true if exists, false otherwise
     */
    bool HasShaderProgram(const std::string& name) const;
    
    /**
     * @brief Get all shader program names
     * @return Vector of program names
     */
    std::vector<std::string> GetShaderProgramNames() const;
    
    /**
     * @brief Clear all shader programs
     */
    void ClearAllShaderPrograms();
    
    // === Built-in Shaders ===
    
    /**
     * @brief Create basic vertex/fragment shader program
     * @param programName Program name
     * @param vertexShader Vertex shader source
     * @param fragmentShader Fragment shader source
     * @param vertexAttributes Vertex attributes
     * @return Compiled shader program
     */
    CompiledShaderProgram CreateBasicShaderProgram(const std::string& programName,
                                                   const std::string& vertexShader,
                                                   const std::string& fragmentShader,
                                                   const std::vector<LLGL::VertexAttribute>& vertexAttributes);
    
    /**
     * @brief Get default vertex shader source
     * @return Default vertex shader source code
     */
    std::string GetDefaultVertexShader() const;
    
    /**
     * @brief Get default fragment shader source
     * @return Default fragment shader source code
     */
    std::string GetDefaultFragmentShader() const;
    
    /**
     * @brief Create default shader program
     * @param vertexAttributes Vertex attributes
     * @return Compiled default shader program
     */
    CompiledShaderProgram CreateDefaultShaderProgram(const std::vector<LLGL::VertexAttribute>& vertexAttributes);
    
    // === Utility Functions ===
    
    /**
     * @brief Convert shader type to string
     * @param type Shader type
     * @return String representation
     */
    static std::string ShaderTypeToString(ShaderType type);
    
    /**
     * @brief Convert string to shader type
     * @param typeString String representation
     * @return Shader type, or ShaderType::Vertex if invalid
     */
    static ShaderType StringToShaderType(const std::string& typeString);
    
    /**
     * @brief Get file extension for shader type
     * @param type Shader type
     * @return Common file extension for the shader type
     */
    static std::string GetShaderFileExtension(ShaderType type);
    
    /**
     * @brief Preprocess shader source
     * @param source Original shader source
     * @param options Compilation options
     * @return Preprocessed shader source
     */
    std::string PreprocessShaderSource(const std::string& source, const ShaderCompileOptions& options);
    
    /**
     * @brief Validate shader source syntax
     * @param source Shader source code
     * @param type Shader type
     * @return true if valid, false otherwise
     */
    bool ValidateShaderSource(const std::string& source, ShaderType type);
    
    /**
     * @brief Get shader compilation statistics
     * @return Map of shader program names to compilation times (in milliseconds)
     */
    std::unordered_map<std::string, double> GetCompilationStatistics() const;
    
    /**
     * @brief Enable or disable shader caching
     * @param enable Whether to enable caching
     */
    void SetCachingEnabled(bool enable);
    
    /**
     * @brief Check if shader caching is enabled
     * @return true if caching is enabled, false otherwise
     */
    bool IsCachingEnabled() const;
    
    /**
     * @brief Clear shader cache
     */
    void ClearCache();
    
    // === Hot Reload Support ===
    
    /**
     * @brief Enable hot reload for shader files
     * @param enable Whether to enable hot reload
     */
    void SetHotReloadEnabled(bool enable);
    
    /**
     * @brief Check if hot reload is enabled
     * @return true if hot reload is enabled, false otherwise
     */
    bool IsHotReloadEnabled() const;
    
    /**
     * @brief Check for shader file changes and reload if necessary
     */
    bool CheckForShaderChanges();
    
    /**
     * @brief Reload shader program from files
     * @param programName Program name to reload
     * @return true if successful, false otherwise
     */
    bool ReloadShaderProgram(const std::string& programName);

private:
    // === Private Methods ===
    
    /**
     * @brief Convert ShaderType to LLGL::ShaderType
     * @param type Our shader type
     * @return LLGL shader type
     */
    LLGL::ShaderType ConvertShaderType(ShaderType type) const;
    
    /**
     * @brief Read file contents
     * @param filePath File path
     * @return File contents, or empty string on failure
     */
    std::string ReadFileContents(const std::string& filePath) const;
    
    /**
     * @brief Extract uniform and attribute locations from shader program
     * @param program Compiled shader program to extract from
     */
    void ExtractShaderReflection(CompiledShaderProgram& program);
    
    /**
     * @brief Generate cache key for shader source
     * @param source Shader source
     * @param options Compilation options
     * @return Cache key string
     */
    std::string GenerateCacheKey(const ShaderSource& source, const ShaderCompileOptions& options) const;
    
    /**
     * @brief Generate cache key for shader program
     * @param programDesc Program description
     * @return Cache key string
     */
    std::string GenerateProgramCacheKey(const ShaderProgramDesc& programDesc) const;
    
    /**
     * @brief Release all cached shaders and shader programs
     */
    void ReleaseAllShaders();
    
    /**
     * @brief Reload shader from file
     * @param filePath Path to shader file
     */
    void ReloadShader(const std::string& filePath);
    
    /**
     * @brief Reload affected shader programs
     * @param filePath Path to changed shader file
     */
    void ReloadAffectedPrograms(const std::string& filePath);
    
    /**
     * @brief Reload changed shaders
     */
    void ReloadChangedShaders();
    
    /**
     * @brief Create shader from source
     * @param source Shader source
     * @param options Compilation options
     * @return Created shader pointer
     */
    LLGL::Shader* CreateShader(const ShaderSource& source, const ShaderCompileOptions& options);
    
    /**
     * @brief Initialize built-in shaders
     */
    void InitializeBuiltInShaders();
    
    /**
     * @brief Get built-in shader source
     * @param name Shader name
     * @return Shader source code
     */
    std::string GetBuiltInShader(const std::string& name) const;
    
    /**
     * @brief Get all built-in shader names
     * @return Vector of shader names
     */
    std::vector<std::string> GetBuiltInShaderNames() const;
    
    /**
     * @brief Set include resolver for shader preprocessing
     * @param resolver Function to resolve include paths
     */
    void SetIncludeResolver(IncludeResolver resolver);
    
    /**
     * @brief Preprocess shader source
     * @param source Original source
     * @param defines Preprocessor defines
     * @return Preprocessed source
     */
    std::string PreprocessShader(const std::string& source, const std::vector<std::string>& defines);
    
    /**
     * @brief Extract uniforms from shader source
     * @param source Shader source code
     * @return Vector of uniform names
     */
    std::vector<std::string> ExtractUniforms(const std::string& source);
    
    /**
     * @brief Extract attributes from shader source
     * @param source Shader source code
     * @return Vector of attribute names
     */
    std::vector<std::string> ExtractAttributes(const std::string& source);
    
    /**
     * @brief Process include directives in shader source
     * @param source Shader source code
     * @param currentPath Current file path for relative includes
     * @return Processed source with includes resolved
     */
    std::string ProcessIncludes(const std::string& source, const std::string& currentPath);
    
    // === Private Members ===
    
    LLGL::RenderSystem* renderSystem_;         ///< Pointer to LLGL render system
    ResourceManager* resourceManager_;         ///< Pointer to resource manager
    
    // Shader program storage
    std::unordered_map<std::string, CompiledShaderProgram> shaderPrograms_;
    
    // Caching
    bool cachingEnabled_;
    std::unordered_map<std::string, CompiledShaderProgram> shaderCache_;
    
    // Hot reload
    bool hotReloadEnabled_;
    std::unordered_map<std::string, std::unordered_map<ShaderType, std::string>> shaderFilePaths_;
    std::unordered_map<std::string, std::filesystem::file_time_type> fileModificationTimes_;
    
    // Statistics
    std::unordered_map<std::string, double> compilationTimes_;
    
    // Built-in shaders
    std::unordered_map<std::string, std::string> builtInShaders_;
    
    // Include resolver for shader preprocessing
    IncludeResolver includeResolver_;
};

} // namespace RenderingPlugin
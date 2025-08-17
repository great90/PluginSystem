#include "../include/ShaderManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <regex>

namespace RenderingPlugin {

// ShaderManager Implementation
ShaderManager::ShaderManager(LLGL::RenderSystem* renderSystem, ResourceManager* resourceManager)
    : renderSystem_(renderSystem)
    , resourceManager_(resourceManager)
    , cachingEnabled_(true)
    , hotReloadEnabled_(false) {
    if (!renderSystem_) {
        throw std::runtime_error("ShaderManager: RenderSystem cannot be null");
    }
}

ShaderManager::~ShaderManager() {
    ReleaseAllShaders();
}

void ShaderManager::ReleaseAllShaders() {
    // Note: Individual shader caching is not implemented in this version
    // shaderCache_ stores CompiledShaderProgram, not individual shaders
    shaderCache_.clear();
    
    // Release all shader programs
    for (const auto& pair : shaderPrograms_) {
        const CompiledShaderProgram& program = pair.second;
        if (program.pipelineState) {
            renderSystem_->Release(*program.pipelineState);
        }
        if (program.vertexShader) {
            renderSystem_->Release(*program.vertexShader);
        }
        if (program.fragmentShader) {
            renderSystem_->Release(*program.fragmentShader);
        }
        if (program.geometryShader) {
            renderSystem_->Release(*program.geometryShader);
        }
    }
    shaderPrograms_.clear();
    
    // Clear file timestamps
    fileModificationTimes_.clear();
}

// Shader Loading
ShaderSource ShaderManager::LoadShaderFromFile(const std::string& filePath, ShaderType type,
                                               const std::string& entryPoint) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // Update file timestamp for hot reload
    if (hotReloadEnabled_) {
        auto writeTime = std::filesystem::last_write_time(filePath);
        fileModificationTimes_[filePath] = writeTime;
    }
    
    ShaderSource source;
    source.type = type;
    source.source = content;
    source.entryPoint = entryPoint;
    source.filePath = filePath;
    
    return source;
}

LLGL::Shader* ShaderManager::CreateShader(const ShaderSource& source, const ShaderCompileOptions& options) {
    // Generate cache key
    std::string cacheKey = GenerateCacheKey(source, options);
    
    // Note: Individual shader caching is not implemented in this version
    // since shaderCache_ stores CompiledShaderProgram instead of individual shaders
    
    // Create shader descriptor
    LLGL::ShaderDescriptor shaderDesc;
    shaderDesc.type = ConvertShaderType(source.type);
    shaderDesc.source = source.source.c_str();
    shaderDesc.sourceSize = source.source.size();
    shaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
    shaderDesc.entryPoint = source.entryPoint.c_str();
    shaderDesc.profile = source.profile.c_str();
    
    // Set compile flags
    if (options.enableDebugInfo) {
        shaderDesc.flags |= LLGL::ShaderCompileFlags::Debug;
    }
    if (options.enableOptimization) {
        shaderDesc.flags |= LLGL::ShaderCompileFlags::OptimizationLevel1;
    }
    if (options.treatWarningsAsErrors) {
        shaderDesc.flags |= LLGL::ShaderCompileFlags::WarningsAreErrors;
    }
    
    // Add preprocessor definitions
    std::vector<LLGL::ShaderMacro> macros;
    for (const auto& define : options.defines) {
        // Parse define string (e.g., "FEATURE_ENABLED=1" or "DEBUG")
        size_t pos = define.find('=');
        if (pos != std::string::npos) {
            std::string name = define.substr(0, pos);
            std::string value = define.substr(pos + 1);
            macros.push_back({name.c_str(), value.c_str()});
        } else {
            macros.push_back({define.c_str(), "1"});
        }
    }
    // Add null terminator for LLGL
    if (!macros.empty()) {
        macros.push_back({nullptr, nullptr});
        shaderDesc.defines = macros.data();
    }
    
    // Create shader
    LLGL::Shader* shader = renderSystem_->CreateShader(shaderDesc);
    if (!shader) {
        throw std::runtime_error("Failed to create shader");
    }
    
    // Check for compilation errors
    const LLGL::Report* report = shader->GetReport();
    if (report && report->HasErrors()) {
        std::string infoLog = report->GetText();
        renderSystem_->Release(*shader);
        throw std::runtime_error("Shader compilation failed: " + infoLog);
    }
    
    // Note: Individual shader caching is handled differently
    // For now, we'll skip individual shader caching and rely on program caching
    
    return shader;
}

// CreateShaderFromFile function removed - not declared in header

// Shader Program Management
CompiledShaderProgram ShaderManager::CompileShaderProgram(const ShaderProgramDesc& programDesc,
                                                              const ShaderCompileOptions& options) {
    CompiledShaderProgram program;
    program.name = programDesc.name;
    program.isValid = false;
    
    LLGL::Shader* vertexShader = nullptr;
    LLGL::Shader* fragmentShader = nullptr;
    LLGL::Shader* geometryShader = nullptr;
    
    // Create vertex shader
    if (!programDesc.vertexShader.source.empty()) {
        LLGL::ShaderDescriptor shaderDesc;
        shaderDesc.type = LLGL::ShaderType::Vertex;
        shaderDesc.source = programDesc.vertexShader.source.c_str();
        shaderDesc.sourceSize = programDesc.vertexShader.source.length();
        shaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
        shaderDesc.entryPoint = programDesc.vertexShader.entryPoint.c_str();
        shaderDesc.profile = programDesc.vertexShader.profile.c_str();
        
        vertexShader = renderSystem_->CreateShader(shaderDesc);
        if (!vertexShader) {
            program.errorLog += "Vertex shader compilation failed: Failed to create vertex shader\n";
            return program;
        }
        
        const LLGL::Report* vertexReport = vertexShader->GetReport();
        if (vertexReport && vertexReport->HasErrors()) {
            program.errorLog += "Vertex shader compilation failed: ";
            program.errorLog += vertexReport->GetText();
            program.errorLog += "\n";
            return program;
        }
    }
    
    // Create fragment shader
    if (!programDesc.fragmentShader.source.empty()) {
        LLGL::ShaderDescriptor shaderDesc;
        shaderDesc.type = LLGL::ShaderType::Fragment;
        shaderDesc.source = programDesc.fragmentShader.source.c_str();
        shaderDesc.sourceSize = programDesc.fragmentShader.source.length();
        shaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
        shaderDesc.entryPoint = programDesc.fragmentShader.entryPoint.c_str();
        shaderDesc.profile = programDesc.fragmentShader.profile.c_str();
        
        fragmentShader = renderSystem_->CreateShader(shaderDesc);
        if (!fragmentShader) {
            program.errorLog += "Fragment shader compilation failed: Failed to create fragment shader\n";
            return program;
        }
        
        const LLGL::Report* fragmentReport = fragmentShader->GetReport();
        if (fragmentReport && fragmentReport->HasErrors()) {
            program.errorLog += "Fragment shader compilation failed: ";
            program.errorLog += fragmentReport->GetText();
            program.errorLog += "\n";
            return program;
        }
    }
    
    // Create geometry shader if specified
    if (!programDesc.geometryShader.source.empty()) {
        LLGL::ShaderDescriptor shaderDesc;
        shaderDesc.type = LLGL::ShaderType::Geometry;
        shaderDesc.source = programDesc.geometryShader.source.c_str();
        shaderDesc.sourceSize = programDesc.geometryShader.source.length();
        shaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
        shaderDesc.entryPoint = programDesc.geometryShader.entryPoint.c_str();
        shaderDesc.profile = programDesc.geometryShader.profile.c_str();
        
        geometryShader = renderSystem_->CreateShader(shaderDesc);
        if (!geometryShader) {
            program.errorLog += "Geometry shader compilation failed: Failed to create geometry shader\n";
            return program;
        }
        
        const LLGL::Report* geometryReport = geometryShader->GetReport();
        if (geometryReport && geometryReport->HasErrors()) {
            program.errorLog += "Geometry shader compilation failed: ";
            program.errorLog += geometryReport->GetText();
            program.errorLog += "\n";
            return program;
        }
    }
    
    // Create graphics pipeline state
    if (vertexShader) {
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        pipelineDesc.vertexShader = vertexShader;
        pipelineDesc.fragmentShader = fragmentShader;
        pipelineDesc.geometryShader = geometryShader;
        
        program.pipelineState = renderSystem_->CreatePipelineState(pipelineDesc);
        if (program.pipelineState) {
            program.isValid = true;
        } else {
            program.errorLog = "Failed to create graphics pipeline state";
        }
    } else {
        program.errorLog = "Vertex shader is required for graphics pipeline";
    }
    
    // Store compiled shaders for cleanup
    program.vertexShader = vertexShader;
    program.fragmentShader = fragmentShader;
    program.geometryShader = geometryShader;
    
    return program;
}

CompiledShaderProgram ShaderManager::CompileShaderProgramFromFiles(const std::string& programName,
        const std::unordered_map<ShaderType, std::string>& shaderFiles,
        const std::vector<LLGL::VertexAttribute>& vertexAttributes,
        const ShaderCompileOptions& options) {
    
    ShaderProgramDesc desc;
    desc.compileOptions = options;
    
    for (const auto& pair : shaderFiles) {
        ShaderType type = pair.first;
        const std::string& filePath = pair.second;
        
        ShaderSource source = LoadShaderFromFile(filePath, type, "");
        source.filePath = filePath;
        
        // Set default entry points and profiles based on shader type
        switch (type) {
            case ShaderType::Vertex:
                source.entryPoint = "VSMain";
                source.profile = "vs_5_0";
                desc.vertexShader = source;
                break;
            case ShaderType::Fragment:
                source.entryPoint = "PSMain";
                source.profile = "ps_5_0";
                desc.fragmentShader = source;
                break;
            case ShaderType::Geometry:
                source.entryPoint = "GSMain";
                source.profile = "gs_5_0";
                desc.geometryShader = source;
                break;
            case ShaderType::TessControl:
                source.entryPoint = "HSMain";
                source.profile = "hs_5_0";
                desc.tessControlShader = source;
                break;
            case ShaderType::TessEvaluation:
                source.entryPoint = "DSMain";
                source.profile = "ds_5_0";
                desc.tessEvaluationShader = source;
                break;
            case ShaderType::Compute:
                source.entryPoint = "CSMain";
                source.profile = "cs_5_0";
                desc.computeShader = source;
                break;
        }
    }
    
    CompiledShaderProgram program = CompileShaderProgram(desc, options);
    program.name = programName;
    return program;
}

// Hot Reload
void ShaderManager::SetHotReloadEnabled(bool enable) {
    hotReloadEnabled_ = enable;
    if (enable) {
        // Update timestamps for all cached shader programs
        for (const auto& pair : shaderPrograms_) {
            // For simplicity, we'll track all shader files that might be used
            // This is a basic implementation - a more sophisticated version would
            // track individual shader file dependencies
        }
    } else {
        fileModificationTimes_.clear();
    }
}

bool ShaderManager::CheckForShaderChanges() {
    if (!hotReloadEnabled_) {
        return false;
    }
    
    bool hasChanges = false;
    
    for (const auto& pair : fileModificationTimes_) {
        const std::string& filePath = pair.first;
        const auto& cachedTime = pair.second;
        
        if (std::filesystem::exists(filePath)) {
            auto currentTime = std::filesystem::last_write_time(filePath);
            if (currentTime != cachedTime) {
                hasChanges = true;
                break;
            }
        }
    }
    
    return hasChanges;
}

void ShaderManager::ReloadChangedShaders() {
    if (!hotReloadEnabled_) {
        return;
    }
    
    std::vector<std::string> changedFiles;
    
    // Find changed files
    for (auto& pair : fileModificationTimes_) {
        const std::string& filePath = pair.first;
        auto& cachedTime = pair.second;
        
        if (std::filesystem::exists(filePath)) {
            auto currentTime = std::filesystem::last_write_time(filePath);
            if (currentTime != cachedTime) {
                changedFiles.push_back(filePath);
                cachedTime = currentTime;
            }
        }
    }
    
    // Reload affected shaders
    for (const std::string& filePath : changedFiles) {
        ReloadShader(filePath);
    }
}

void ShaderManager::ReloadShader(const std::string& filePath) {
    // Note: Individual shader reloading is not implemented in this version
    // since shaderCache_ stores CompiledShaderProgram instead of CachedShader
    // For now, we'll just reload affected shader programs
    ReloadAffectedPrograms(filePath);
}

// Built-in Shaders
void ShaderManager::InitializeBuiltInShaders() {
    // Basic vertex shader
    const std::string basicVertexShader = R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragNormal;
out vec2 fragTexCoord;
out vec3 fragWorldPos;

void main() {
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    fragWorldPos = worldPos.xyz;
    fragNormal = mat3(modelMatrix) * normal;
    fragTexCoord = texCoord;
    
    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
)";
    
    // Basic fragment shader
    const std::string basicFragmentShader = R"(
#version 330 core

in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragWorldPos;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform sampler2D diffuseTexture;

out vec4 fragColor;

void main() {
    vec3 normal = normalize(fragNormal);
    float NdotL = max(dot(normal, -lightDirection), 0.0);
    
    vec3 diffuse = lightColor * NdotL;
    vec3 ambient = ambientColor;
    
    vec4 texColor = texture(diffuseTexture, fragTexCoord);
    vec3 finalColor = (diffuse + ambient) * texColor.rgb;
    
    fragColor = vec4(finalColor, texColor.a);
}
)";
    
    // Store built-in shaders
    builtInShaders_["basic_vertex"] = basicVertexShader;
    builtInShaders_["basic_fragment"] = basicFragmentShader;
}

std::string ShaderManager::GetBuiltInShader(const std::string& name) const {
    auto it = builtInShaders_.find(name);
    if (it != builtInShaders_.end()) {
        return it->second;
    }
    throw std::runtime_error("Built-in shader not found: " + name);
}

std::vector<std::string> ShaderManager::GetBuiltInShaderNames() const {
    std::vector<std::string> names;
    for (const auto& pair : builtInShaders_) {
        names.push_back(pair.first);
    }
    return names;
}

// Utility Functions
std::string ShaderManager::PreprocessShader(const std::string& source, const std::vector<std::string>& defines) {
    std::string result = source;
    
    // Add defines at the beginning
    std::string defineBlock;
    for (const auto& define : defines) {
        defineBlock += "#define " + define + "\n";
    }
    
    // Insert defines after version directive
    size_t versionPos = result.find("#version");
    if (versionPos != std::string::npos) {
        size_t lineEnd = result.find('\n', versionPos);
        if (lineEnd != std::string::npos) {
            result.insert(lineEnd + 1, defineBlock);
        }
    } else {
        result = defineBlock + result;
    }
    
    return result;
}

bool ShaderManager::ValidateShaderSource(const std::string& source, ShaderType type) {
    // Basic validation - check for required elements
    switch (type) {
        case ShaderType::Vertex:
            return source.find("gl_Position") != std::string::npos;
        case ShaderType::Fragment:
            return source.find("gl_FragColor") != std::string::npos || 
                   source.find("out ") != std::string::npos;
        default:
            return true; // Basic validation for other types
    }
}

std::vector<std::string> ShaderManager::ExtractUniforms(const std::string& source) {
    std::vector<std::string> uniforms;
    std::regex uniformRegex(R"(uniform\s+\w+\s+(\w+)\s*;)");
    std::sregex_iterator iter(source.begin(), source.end(), uniformRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        uniforms.push_back((*iter)[1].str());
    }
    
    return uniforms;
}

std::vector<std::string> ShaderManager::ExtractAttributes(const std::string& source) {
    std::vector<std::string> attributes;
    std::regex attributeRegex(R"((?:attribute|in)\s+\w+\s+(\w+)\s*;)");
    std::sregex_iterator iter(source.begin(), source.end(), attributeRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        attributes.push_back((*iter)[1].str());
    }
    
    return attributes;
}

// Include Processing
void ShaderManager::SetIncludeResolver(IncludeResolver resolver) {
    includeResolver_ = resolver;
}

std::string ShaderManager::ProcessIncludes(const std::string& source, const std::string& currentPath) {
    if (!includeResolver_) {
        return source;
    }
    
    std::string result = source;
    std::regex includeRegex(R"(#include\s*["<]([^\s">]+)[">])");
    std::sregex_iterator iter(result.begin(), result.end(), includeRegex);
    std::sregex_iterator end;
    
    // Process includes from back to front to maintain positions
    std::vector<std::pair<size_t, size_t>> replacements;
    std::vector<std::string> includeContents;
    
    for (; iter != end; ++iter) {
        std::string includePath = (*iter)[1].str();
        std::string includeContent = includeResolver_(includePath, currentPath);
        
        // Recursively process includes in the included file
        includeContent = ProcessIncludes(includeContent, includePath);
        
        replacements.push_back({iter->position(), iter->length()});
        includeContents.push_back(includeContent);
    }
    
    // Apply replacements from back to front
    for (int i = static_cast<int>(replacements.size()) - 1; i >= 0; --i) {
        result.replace(replacements[i].first, replacements[i].second, includeContents[i]);
    }
    
    return result;
}

// Private Helper Methods
LLGL::ShaderType ShaderManager::ConvertShaderType(ShaderType type) const {
    switch (type) {
        case ShaderType::Vertex: return LLGL::ShaderType::Vertex;
        case ShaderType::Fragment: return LLGL::ShaderType::Fragment;
        case ShaderType::Geometry: return LLGL::ShaderType::Geometry;
        case ShaderType::TessControl: return LLGL::ShaderType::TessControl;
        case ShaderType::TessEvaluation: return LLGL::ShaderType::TessEvaluation;
        case ShaderType::Compute: return LLGL::ShaderType::Compute;
        default: return LLGL::ShaderType::Vertex;
    }
}

std::string ShaderManager::GenerateCacheKey(const ShaderSource& source, const ShaderCompileOptions& options) const {
    std::string key = source.source + "|" + source.entryPoint + "|" + source.profile + "|" + std::to_string(static_cast<int>(source.type));
    
    // Add compile options to key
    key += "|" + std::to_string(options.enableDebugInfo) + "|" + std::to_string(options.enableOptimization) + "|" + std::to_string(options.treatWarningsAsErrors);
    
    // Add defines to key
    for (const auto& define : options.defines) {
        key += "|" + define + "=1";
    }
    
    return key;
}

std::string ShaderManager::GenerateProgramCacheKey(const ShaderProgramDesc& programDesc) const {
    std::string key;
    
    if (!programDesc.vertexShader.source.empty()) {
        key += "VS:" + GenerateCacheKey(programDesc.vertexShader, programDesc.compileOptions) + "|";
    }
    if (!programDesc.fragmentShader.source.empty()) {
        key += "FS:" + GenerateCacheKey(programDesc.fragmentShader, programDesc.compileOptions) + "|";
    }
    if (!programDesc.geometryShader.source.empty()) {
        key += "GS:" + GenerateCacheKey(programDesc.geometryShader, programDesc.compileOptions) + "|";
    }
    if (!programDesc.tessControlShader.source.empty()) {
        key += "HS:" + GenerateCacheKey(programDesc.tessControlShader, programDesc.compileOptions) + "|";
    }
    if (!programDesc.tessEvaluationShader.source.empty()) {
        key += "DS:" + GenerateCacheKey(programDesc.tessEvaluationShader, programDesc.compileOptions) + "|";
    }
    if (!programDesc.computeShader.source.empty()) {
        key += "CS:" + GenerateCacheKey(programDesc.computeShader, programDesc.compileOptions) + "|";
    }
    
    return key;
}

void ShaderManager::ReloadAffectedPrograms(const std::string& filePath) {
    std::vector<std::string> programsToReload;
    
    // Find all programs that might use this shader file
    // This is a simplified implementation - in practice, you'd need to track
    // which shader files belong to which programs
    for (const auto& programPair : shaderPrograms_) {
        // For now, we'll reload all programs when any shader file changes
        // A more sophisticated implementation would track dependencies
        programsToReload.push_back(programPair.first);
    }
    
    // Reload affected programs
    for (const std::string& programName : programsToReload) {
        ReloadShaderProgram(programName);
    }
}

} // namespace RenderingPlugin
/**
 * @file RenderingPlugin.cpp
 * @brief Implementation of the RenderingPlugin class with modular architecture
 */

#include "RenderingPlugin.h"
#include "PluginExport.h"

// Include modular components
#include "RenderingSystem.h"
#include "ResourceManager.h"
#include "RenderCommands.h"
#include "GeometryGenerator.h"
#include "ShaderManager.h"

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdlib>
#include <dlfcn.h>
#include <LLGL/LLGL.h>

// Initialize static members
RenderingPlugin* RenderingPlugin::instance_ = nullptr;

// Static plugin info
PluginInfo RenderingPlugin::pluginInfo_ = {
    "RenderingPlugin",                // name
    "Rendering Plugin",               // displayName
    "A plugin for rendering operations using LLGL", // description
    PluginInfo::Version(1, 0, 0),     // version
    "PluginSystem"                    // author
};

RenderingPlugin::RenderingPlugin() 
    : renderingSystem_(nullptr), resourceManager_(nullptr), renderCommands_(nullptr),
      geometryGenerator_(nullptr), shaderManager_(nullptr),
      renderSystem_(nullptr), swapChain_(nullptr), commandBuffer_(nullptr), surface_(nullptr),
      currentAPI_(RenderAPI::None), initialized_(false), currentMode_(RenderingMode::Hardware),
      softwareRenderingEnabled_(false), offscreenRenderTarget_(nullptr) {
    std::cout << "RenderingPlugin constructor called, this = " << this << std::endl;
    
    // Set singleton instance if not already set
    if (instance_ == nullptr) {
        instance_ = this;
        std::cout << "Set instance_ to this = " << this << std::endl;
    } else {
        std::cout << "instance_ already set to " << instance_ << std::endl;
    }
}

RenderingPlugin::~RenderingPlugin() {
    // Don't call Shutdown in destructor to avoid issues with LLGL cleanup
    // Shutdown should be called explicitly before destruction
    
    // Clean up modular components (they should already be cleaned up in Shutdown)
    renderingSystem_ = nullptr;
    resourceManager_ = nullptr;
    renderCommands_ = nullptr;
    geometryGenerator_ = nullptr;
    shaderManager_ = nullptr;
    
    // Clear the singleton instance if it's this instance
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

bool RenderingPlugin::Initialize() {
    try {
        std::cout << "RenderingPlugin::Initialize() - Starting initialization..." << std::endl;
        
        // Initialize modular components
        renderingSystem_ = std::make_unique<RenderingPlugin::RenderingSystem>();
        if (!renderingSystem_) {
            std::cerr << "Failed to create RenderingSystem" << std::endl;
            return false;
        }
        
        geometryGenerator_ = std::make_unique<RenderingPlugin::GeometryGenerator>();
        if (!geometryGenerator_) {
            std::cerr << "Failed to create GeometryGenerator" << std::endl;
            return false;
        }
        
        std::cout << "RenderingPlugin::Initialize() - Modular components created successfully" << std::endl;
        
        initialized_ = true;
        std::cout << "RenderingPlugin initialized successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize RenderingPlugin: " << e.what() << std::endl;
        
        // Clean up on failure
        renderingSystem_.reset();
        geometryGenerator_.reset();
        
        initialized_ = false;
        return false;
    }
}

void RenderingPlugin::Shutdown() {
    // Prevent multiple shutdowns - check all critical resources
    if (!initialized_ && !renderingSystem_ && !renderSystem_ && !swapChain_) {
        return;
    }
    
    std::cout << "RenderingPlugin::Shutdown() - Starting shutdown process..." << std::endl;
    
    try {
        // Shutdown modular components first
        if (renderCommands_) {
            std::cout << "RenderingPlugin::Shutdown() - Cleaning up RenderCommands..." << std::endl;
            renderCommands_.reset();
        }
        
        if (shaderManager_) {
            std::cout << "RenderingPlugin::Shutdown() - Cleaning up ShaderManager..." << std::endl;
            shaderManager_.reset();
        }
        
        if (resourceManager_) {
            std::cout << "RenderingPlugin::Shutdown() - Cleaning up ResourceManager..." << std::endl;
            resourceManager_.reset();
        }
        
        if (geometryGenerator_) {
            std::cout << "RenderingPlugin::Shutdown() - Cleaning up GeometryGenerator..." << std::endl;
            geometryGenerator_.reset();
        }
        
        // Shutdown rendering system (this will handle LLGL cleanup)
        if (renderingSystem_) {
            std::cout << "RenderingPlugin::Shutdown() - Shutting down RenderingSystem..." << std::endl;
            renderingSystem_->Shutdown();
            renderingSystem_.reset();
            std::cout << "RenderingPlugin::Shutdown() - RenderingSystem shutdown completed" << std::endl;
        }
        
        // Legacy cleanup for backward compatibility
        surface_ = nullptr;
        commandBuffer_ = nullptr;
        if (swapChain_) {
            swapChain_.reset();
            swapChain_ = nullptr;
        }
        if (renderSystem_) {
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        
        initialized_ = false;
        currentAPI_ = RenderAPI::None;
        std::cout << "RenderingPlugin::Shutdown() - Shutdown completed successfully" << std::endl;
        
    } catch (const std::exception& e) {
        // Ignore exceptions during shutdown to prevent crashes
        std::cerr << "Exception during RenderingPlugin shutdown: " << e.what() << std::endl;
        // Still mark as shutdown to prevent further issues
        initialized_ = false;
        currentAPI_ = RenderAPI::None;
        
        // Clean up all components
        renderCommands_.reset();
        shaderManager_.reset();
        resourceManager_.reset();
        geometryGenerator_.reset();
        renderingSystem_.reset();
        
        renderSystem_ = nullptr;
        commandBuffer_ = nullptr;
        if (swapChain_) {
            swapChain_.reset();
            swapChain_ = nullptr;
        }
        surface_ = nullptr;
    } catch (...) {
        // Ignore all exceptions during shutdown
        std::cerr << "Unknown exception during RenderingPlugin shutdown" << std::endl;
        // Still mark as shutdown to prevent further issues
        initialized_ = false;
        currentAPI_ = RenderAPI::None;
        
        // Clean up all components
        renderCommands_.reset();
        shaderManager_.reset();
        resourceManager_.reset();
        geometryGenerator_.reset();
        renderingSystem_.reset();
        
        renderSystem_ = nullptr;
        commandBuffer_ = nullptr;
        if (swapChain_) {
            swapChain_.reset();
            swapChain_ = nullptr;
        }
        surface_ = nullptr;
    }
}

const PluginInfo& RenderingPlugin::GetPluginInfo() const {
    return pluginInfo_;
}

std::string RenderingPlugin::Serialize() {
    // Serialize current state (API type and window properties)
    std::string data;
    data += std::to_string(static_cast<int>(currentAPI_)) + ";";
    data += std::to_string(windowDesc_.width) + ";";
    data += std::to_string(windowDesc_.height) + ";";
    data += windowDesc_.title;
    return data;
}

bool RenderingPlugin::Deserialize(const std::string& data) {
    try {
        size_t pos = 0;
        size_t nextPos = data.find(';', pos);
        if (nextPos == std::string::npos) return false;
        
        // Parse API type
        int apiType = std::stoi(data.substr(pos, nextPos - pos));
        currentAPI_ = static_cast<RenderAPI>(apiType);
        
        // Parse width
        pos = nextPos + 1;
        nextPos = data.find(';', pos);
        if (nextPos == std::string::npos) return false;
        windowDesc_.width = std::stoi(data.substr(pos, nextPos - pos));
        
        // Parse height
        pos = nextPos + 1;
        nextPos = data.find(';', pos);
        if (nextPos == std::string::npos) return false;
        windowDesc_.height = std::stoi(data.substr(pos, nextPos - pos));
        
        // Parse title
        pos = nextPos + 1;
        windowDesc_.title = data.substr(pos);
        
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool RenderingPlugin::PrepareForHotReload() {
    // Prepare for hot reload by saving current state
    return true;
}

bool RenderingPlugin::CompleteHotReload() {
    // Complete hot reload by restoring state
    return true;
}

RenderingPlugin* RenderingPlugin::GetInstance() {
    std::cout << "GetInstance called, instance_ = " << instance_ << std::endl;
    return instance_;
}

void RenderingPlugin::SetInstance(RenderingPlugin* instance) {
    instance_ = instance;
}

bool RenderingPlugin::InitializeRenderSystem(RenderAPI api) {
    if (!initialized_) {
        std::cerr << "ERROR: InitializeRenderSystem: Plugin not initialized" << std::endl;
        std::cerr << "SOLUTION: Call Initialize() first before attempting to initialize render system" << std::endl;
        return false;
    }
    
    // Check environment variables for debugging modes
    const char* debugMode = std::getenv("RENDERING_DEBUG");
    const char* forceAPI = std::getenv("FORCE_RENDER_API");
    const char* softwareMode = std::getenv("SOFTWARE_RENDERING");
    
    if (debugMode && std::string(debugMode) == "1") {
        std::cout << "DEBUG MODE ENABLED" << std::endl;
        std::cout << "Environment variables:" << std::endl;
        std::cout << "  RENDERING_DEBUG=" << (debugMode ? debugMode : "not set") << std::endl;
        std::cout << "  FORCE_RENDER_API=" << (forceAPI ? forceAPI : "not set") << std::endl;
        std::cout << "  SOFTWARE_RENDERING=" << (softwareMode ? softwareMode : "not set") << std::endl;
    }
    
    if (softwareMode && std::string(softwareMode) == "1") {
        std::cout << "SOFTWARE_RENDERING environment variable detected, enabling software mode" << std::endl;
        currentMode_ = RenderingMode::Software;
        softwareRenderingEnabled_ = true;
        return true;
    }
    
    if (forceAPI) {
        std::cout << "FORCE_RENDER_API environment variable detected: " << forceAPI << std::endl;
        // Override the requested API based on environment variable
        std::string apiStr(forceAPI);
        if (apiStr == "Metal") api = RenderAPI::Metal;
        else if (apiStr == "OpenGL") api = RenderAPI::OpenGL;
        else if (apiStr == "Vulkan") api = RenderAPI::Vulkan;
        else if (apiStr == "Direct3D11") api = RenderAPI::Direct3D11;
        else if (apiStr == "Direct3D12") api = RenderAPI::Direct3D12;
    }
    
    std::cout << "InitializeRenderSystem: Attempting to initialize " << GetModuleName(api) << " API..." << std::endl;
    
    // If specific API fails, try fallback APIs
    std::vector<RenderAPI> apisToTry;
    if (api != RenderAPI::None) {
        apisToTry.push_back(api);
    }
    
    // Add platform-specific fallback APIs
#ifdef __APPLE__
    if (api != RenderAPI::Metal)
        apisToTry.push_back(RenderAPI::Metal);
#endif
    if (api != RenderAPI::Vulkan)
        apisToTry.push_back(RenderAPI::Vulkan);
    if (api != RenderAPI::OpenGL)
        apisToTry.push_back(RenderAPI::OpenGL);
#if defined(_WIN32)
    if (api != RenderAPI::Direct3D11)
        apisToTry.push_back(RenderAPI::Direct3D11);
    if (api != RenderAPI::Direct3D12)
        apisToTry.push_back(RenderAPI::Direct3D12);
#endif
    
    std::string lastError;
    
    for (RenderAPI tryApi : apisToTry) {
        std::cout << "\n=== Trying " << GetModuleName(tryApi) << " API ===" << std::endl;
        
        if (TryInitializeAPI(tryApi, lastError)) {
            currentAPI_ = tryApi;
            std::cout << "SUCCESS: " << GetModuleName(tryApi) << " API initialized successfully!" << std::endl;
            return true;
        }
        
        std::cout << "FAILED: " << GetModuleName(tryApi) << " initialization failed" << std::endl;
        if (!lastError.empty()) {
            std::cout << "ERROR DETAILS: " << lastError << std::endl;
        }
    }
    
    // All APIs failed - provide detailed error and recovery options
    std::cerr << "\n=== WARNING: NO HARDWARE GRAPHICS API AVAILABLE ===" << std::endl;
    std::cerr << "All supported graphics APIs failed to initialize:" << std::endl;
    
    #ifdef __APPLE__
    std::cerr << "- Metal: Not available or failed to load" << std::endl;
    std::cerr << "- OpenGL: Not available or failed to load" << std::endl;
    
    // Provide detailed macOS diagnostics
    std::cerr << "\n=== DETAILED macOS DIAGNOSTICS ===" << std::endl;
    std::string macOSInfo = GetMacOSGraphicsInfo();
    std::cerr << macOSInfo << std::endl;
    
    std::cerr << "\nPOSSIBLE SOLUTIONS:" << std::endl;
    std::cerr << "1. Update macOS to latest version" << std::endl;
    std::cerr << "2. Update graphics drivers" << std::endl;
    std::cerr << "3. Check if running in headless environment" << std::endl;
    std::cerr << "4. Verify LLGL library installation" << std::endl;
    std::cerr << "5. Set SOFTWARE_RENDERING=1 environment variable" << std::endl;
    std::cerr << "6. Set AUTO_SOFTWARE_FALLBACK=1 for automatic fallback" << std::endl;
    #elif defined(_WIN32)
    std::cerr << "- Direct3D 11: Not available or failed to load" << std::endl;
    std::cerr << "- Direct3D 12: Not available or failed to load" << std::endl;
    std::cerr << "- Vulkan: Not available or failed to load" << std::endl;
    std::cerr << "- OpenGL: Not available or failed to load" << std::endl;
    std::cerr << "\nPOSSIBLE SOLUTIONS:" << std::endl;
    std::cerr << "1. Update graphics drivers (NVIDIA/AMD/Intel)" << std::endl;
    std::cerr << "2. Install latest DirectX runtime" << std::endl;
    std::cerr << "3. Install Vulkan runtime" << std::endl;
    std::cerr << "4. Check Windows version compatibility" << std::endl;
    std::cerr << "5. Verify LLGL library installation" << std::endl;
    std::cerr << "6. Set SOFTWARE_RENDERING=1 environment variable" << std::endl;
    #else
    std::cerr << "- Vulkan: Not available or failed to load" << std::endl;
    std::cerr << "- OpenGL: Not available or failed to load" << std::endl;
    std::cerr << "\nPOSSIBLE SOLUTIONS:" << std::endl;
    std::cerr << "1. Install graphics drivers (mesa, nvidia, amd)" << std::endl;
    std::cerr << "2. Install Vulkan development packages" << std::endl;
    std::cerr << "3. Check X11/Wayland display server" << std::endl;
    std::cerr << "4. Verify LLGL library installation" << std::endl;
    std::cerr << "5. Set SOFTWARE_RENDERING=1 environment variable" << std::endl;
    #endif
    
    std::cerr << "\nLast error: " << lastError << std::endl;
    
    // Check if we should exit or try software rendering
    const char* noExit = std::getenv("NO_EXIT_ON_GRAPHICS_FAILURE");
    const char* autoSoftware = std::getenv("AUTO_SOFTWARE_FALLBACK");
    
    if (autoSoftware && std::string(autoSoftware) == "1") {
        std::cerr << "\nAUTO_SOFTWARE_FALLBACK enabled, switching to software rendering..." << std::endl;
        currentMode_ = RenderingMode::Software;
        softwareRenderingEnabled_ = true;
        return true;
    }
    
    if (noExit && std::string(noExit) == "1") {
        std::cerr << "\nNO_EXIT_ON_GRAPHICS_FAILURE enabled, returning false instead of exiting..." << std::endl;
        return false;
    }
    
    std::cerr << "\nTo prevent application exit, set NO_EXIT_ON_GRAPHICS_FAILURE=1" << std::endl;
    std::cerr << "To enable automatic software fallback, set AUTO_SOFTWARE_FALLBACK=1" << std::endl;
    std::cerr << "\nApplication cannot continue without graphics support. Exiting..." << std::endl;
    
    // Exit the application as no graphics API is available (unless overridden)
    std::exit(EXIT_FAILURE);
}

bool RenderingPlugin::CreateWindow(const WindowDesc& desc) {
    // Use new modular architecture if available
    if (renderingSystem_) {
        std::cout << "CreateWindow: Using new modular RenderingSystem..." << std::endl;
        bool success = renderingSystem_->CreateWindow(desc.width, desc.height, desc.title);
        if (success) {
            windowDesc_ = desc;
            
            // Update legacy members for backward compatibility
            if (renderingSystem_->GetSwapChain()) {
                swapChain_.reset(renderingSystem_->GetSwapChain());
                surface_ = renderingSystem_->GetSurface();
                commandBuffer_ = renderingSystem_->GetCommandBuffer();
            }
        }
        return success;
    }
    
    // Legacy implementation for backward compatibility
    if (!renderSystem_) {
        std::cout << "CreateWindow: No render system available" << std::endl;
        return false;
    }
    
    std::cout << "CreateWindow: Using legacy implementation..." << std::endl;
    std::cout << "  - Requested size: " << desc.width << "x" << desc.height << std::endl;
    std::cout << "  - Title: " << desc.title << std::endl;
    std::cout << "  - Current API: " << GetModuleName(currentAPI_) << std::endl;
    
    try {
        // Create swap chain descriptor with macOS-specific optimizations
        LLGL::SwapChainDescriptor swapChainDesc;
        swapChainDesc.resolution = { static_cast<std::uint32_t>(desc.width), static_cast<std::uint32_t>(desc.height) };
        swapChainDesc.colorBits = 32;
        swapChainDesc.depthBits = 24;
        swapChainDesc.stencilBits = 8;
        swapChainDesc.samples = 1;
        swapChainDesc.swapBuffers = 2;
        swapChainDesc.fullscreen = false;
        swapChainDesc.resizable = true;
        
        std::cout << "CreateWindow: Creating swap chain..." << std::endl;
        
        // Create swap chain (this will create its own surface)
        swapChain_.reset(renderSystem_->CreateSwapChain(swapChainDesc));
        if (!swapChain_) {
            std::cout << "CreateWindow: Failed to create swap chain" << std::endl;
            return false;
        }
        
        std::cout << "CreateWindow: Swap chain created successfully" << std::endl;
        
        // Get the surface from the swap chain
        surface_ = &swapChain_->GetSurface();
        if (!surface_) {
            std::cout << "CreateWindow: Failed to get surface from swap chain" << std::endl;
            return false;
        }
        
        std::cout << "CreateWindow: Surface obtained successfully" << std::endl;
        
        // Try to set window title if possible
        try {
            if (LLGL::IsInstanceOf<LLGL::Window>(surface_)) {
                LLGL::Window* window = LLGL::CastTo<LLGL::Window>(surface_);
                window->SetTitle(L"Example");
                window->Show();
            }
        } catch (...) {
            // Ignore title setting errors
        }
        
        // Create command buffer
        std::cout << "CreateWindow: Creating command buffer..." << std::endl;
        commandBuffer_ = renderSystem_->CreateCommandBuffer();
        if (!commandBuffer_) {
            std::cout << "CreateWindow: Failed to create command buffer" << std::endl;
            return false;
        }
        
        std::cout << "CreateWindow: Command buffer created successfully" << std::endl;
        
        // Store window description
        windowDesc_ = desc;
        
        // Process events to ensure window is properly initialized
        try {
            LLGL::Surface::ProcessEvents();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            LLGL::Surface::ProcessEvents();
        } catch (...) {
            // Ignore event processing errors
        }
        
        std::cout << "CreateWindow: Window creation completed successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "CreateWindow: Exception caught: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cout << "CreateWindow: Unknown exception caught" << std::endl;
        return false;
    }
}

bool RenderingPlugin::BeginFrame() {
    // Use new modular architecture if available
    if (renderingSystem_ && renderCommands_) {
        return renderingSystem_->BeginFrame() && renderCommands_->BeginFrame();
    }
    
    // Legacy implementation for backward compatibility
    if (!initialized_ || !commandBuffer_) {
        std::cerr << "BeginFrame: Plugin not initialized or no command buffer available" << std::endl;
        return false;
    }
    
    try {
        // Begin command buffer recording
        commandBuffer_->Begin();
        
        // Begin render pass if swap chain is available
        if (swapChain_) {
            commandBuffer_->BeginRenderPass(*swapChain_);
            
            // Clear color and depth buffers with a nice blue background
            LLGL::ClearValue clearValue;
            clearValue.color[0] = 0.1f;  // Red
            clearValue.color[1] = 0.1f;  // Green
            clearValue.color[2] = 0.2f;  // Blue
            clearValue.color[3] = 1.0f;  // Alpha
            clearValue.depth = 1.0f;
            clearValue.stencil = 0;
            
            commandBuffer_->Clear(LLGL::ClearFlags::ColorDepth, clearValue);
            
            // Set viewport to full window size
            const auto resolution = swapChain_->GetResolution();
            LLGL::Viewport viewport;
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(resolution.width);
            viewport.height = static_cast<float>(resolution.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            commandBuffer_->SetViewport(viewport);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "BeginFrame: Exception caught: " << e.what() << std::endl;
        return false;
    }
}

bool RenderingPlugin::EndFrame() {
    // Use new modular architecture if available
    if (renderingSystem_ && renderCommands_) {
        bool success = renderCommands_->EndFrame();
        if (success) {
            success = renderingSystem_->EndFrame();
        }
        return success;
    }
    
    // Legacy implementation for backward compatibility
    if (!initialized_ || !commandBuffer_) {
        std::cerr << "EndFrame: Plugin not initialized or no command buffer available" << std::endl;
        return false;
    }
    
    try {
        // End render pass if swap chain is available
        if (swapChain_) {
            commandBuffer_->EndRenderPass();
        }
        
        // End command buffer recording
        commandBuffer_->End();
        
        // Submit command buffer to render system
        if (renderSystem_) {
            auto commandQueue = renderSystem_->GetCommandQueue();
            if (commandQueue) {
                commandQueue->Submit(*commandBuffer_);
            } else {
                std::cerr << "EndFrame: No command queue available" << std::endl;
                return false;
            }
        }
        
        // Present the swap chain
        if (swapChain_) {
            swapChain_->Present();
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "EndFrame: Exception caught: " << e.what() << std::endl;
        return false;
    }
}

void RenderingPlugin::Clear(const Color& color) {
    // Use new modular architecture if available
    if (renderCommands_) {
        renderCommands_->Clear(color.r, color.g, color.b, color.a);
        return;
    }
    
    // Legacy implementation for backward compatibility
    if (!initialized_ || !commandBuffer_) {
        return;
    }
    
    try {
        LLGL::ClearValue clearValue;
        clearValue.color[0] = color.r;
        clearValue.color[1] = color.g;
        clearValue.color[2] = color.b;
        clearValue.color[3] = color.a;
        clearValue.depth = 1.0f;
        clearValue.stencil = 0;
        
        commandBuffer_->Clear(LLGL::ClearFlags::ColorDepth, clearValue);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to clear: " << e.what() << std::endl;
    }
}

void RenderingPlugin::SetViewport(int x, int y, int width, int height) {
    // Use new modular architecture if available
    if (renderCommands_) {
        renderCommands_->SetViewport(x, y, width, height);
        return;
    }
    
    // Legacy implementation for backward compatibility
    if (!initialized_ || !commandBuffer_) {
        return;
    }
    
    try {
        LLGL::Viewport viewport;
        viewport.x = static_cast<float>(x);
        viewport.y = static_cast<float>(y);
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        commandBuffer_->SetViewport(viewport);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to set viewport: " << e.what() << std::endl;
    }
}

bool RenderingPlugin::IsInitialized() const {
    // Use new modular architecture if available
    if (renderingSystem_) {
        return renderingSystem_->IsInitialized();
    }
    
    // Legacy implementation for backward compatibility
    return initialized_;
}

RenderAPI RenderingPlugin::GetCurrentAPI() const {
    // Use new modular architecture if available
    if (renderingSystem_) {
        return renderingSystem_->GetCurrentAPI();
    }
    
    // Legacy implementation for backward compatibility
    return currentAPI_;
}

bool RenderingPlugin::GetWindowSize(int& width, int& height) const {
    // Use new modular architecture if available
    if (renderingSystem_) {
        return renderingSystem_->GetWindowSize(width, height);
    }
    
    // Legacy implementation for backward compatibility
    if (!initialized_ || !swapChain_) {
        width = 0;
        height = 0;
        return false;
    }
    
    auto resolution = swapChain_->GetResolution();
    width = static_cast<int>(resolution.width);
    height = static_cast<int>(resolution.height);
    return true;
}

bool RenderingPlugin::ShouldWindowClose() const {
    // Use new modular architecture if available
    if (renderingSystem_) {
        return renderingSystem_->ShouldWindowClose();
    }
    
    // Legacy implementation for backward compatibility
    // For now, just return false. In a real implementation,
    // you would check if the window close button was pressed
    return false;
}

void RenderingPlugin::PollEvents() {
    // Use new modular architecture if available
    if (renderingSystem_) {
        renderingSystem_->PollEvents();
        return;
    }
    
    // Legacy implementation for backward compatibility
    // Poll window events using static method
    LLGL::Surface::ProcessEvents();
}

std::string RenderingPlugin::GetModuleName(RenderAPI api) const {
    switch (api) {
        case RenderAPI::OpenGL:
            return "OpenGL";
        case RenderAPI::Vulkan:
            return "Vulkan";
        case RenderAPI::Direct3D11:
            return "Direct3D11";
        case RenderAPI::Direct3D12:
            return "Direct3D12";
        case RenderAPI::Metal:
            return "Metal";
        default:
            return "OpenGL";
    }
}

bool RenderingPlugin::TryInitializeAPI(RenderAPI api, std::string& errorMessage) {
    errorMessage.clear();
    
    try {
        // Create render system descriptor
        LLGL::RenderSystemDescriptor renderSystemDesc;
        
        switch (api) {
            case RenderAPI::OpenGL:
                renderSystemDesc.moduleName = "OpenGL";
                std::cout << "  - Configuring OpenGL backend" << std::endl;
                break;
            case RenderAPI::Vulkan:
                renderSystemDesc.moduleName = "Vulkan";
                std::cout << "  - Configuring Vulkan backend" << std::endl;
                break;
            case RenderAPI::Direct3D11:
                renderSystemDesc.moduleName = "Direct3D11";
                std::cout << "  - Configuring Direct3D11 backend" << std::endl;
                break;
            case RenderAPI::Direct3D12:
                renderSystemDesc.moduleName = "Direct3D12";
                std::cout << "  - Configuring Direct3D12 backend" << std::endl;
                break;
            case RenderAPI::Metal:
                renderSystemDesc.moduleName = "Metal";
                std::cout << "  - Configuring Metal backend (macOS native)" << std::endl;
                break;
            default:
                errorMessage = "Unknown or unsupported API requested";
                return false;
        }
        
        // Platform-specific validation
        #ifdef __APPLE__
        if (api == RenderAPI::Direct3D11 || api == RenderAPI::Direct3D12) {
            errorMessage = "Direct3D APIs are not supported on macOS";
            return false;
        }
        if (api == RenderAPI::Metal) {
            std::cout << "  - macOS detected, Metal is the preferred API" << std::endl;
        }
        #elif defined(_WIN32)
        if (api == RenderAPI::Metal) {
            errorMessage = "Metal API is not supported on Windows";
            return false;
        }
        #else
        if (api == RenderAPI::Direct3D11 || api == RenderAPI::Direct3D12 || api == RenderAPI::Metal) {
            errorMessage = "Direct3D and Metal APIs are not supported on Linux";
            return false;
        }
        #endif
        
        std::cout << "  - Loading " << renderSystemDesc.moduleName.c_str() << " render system module..." << std::endl;
        
        // Clean up any existing render system
        if (renderSystem_) {
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        
        // Load render system
        auto renderSystemPtr = LLGL::RenderSystem::Load(renderSystemDesc);
        renderSystem_ = renderSystemPtr.release();
        
        if (!renderSystem_) {
            errorMessage = "Failed to load " + std::string(renderSystemDesc.moduleName.c_str()) + " render system. ";
            
            // Add specific error details based on API
            switch (api) {
                case RenderAPI::OpenGL:
                    errorMessage += "OpenGL drivers may not be installed or are outdated.";
                    break;
                case RenderAPI::Vulkan:
                    errorMessage += "Vulkan runtime may not be installed or graphics card doesn't support Vulkan.";
                    break;
                case RenderAPI::Direct3D11:
                    errorMessage += "DirectX 11 runtime may not be installed or graphics card doesn't support D3D11.";
                    break;
                case RenderAPI::Direct3D12:
                    errorMessage += "DirectX 12 runtime may not be installed or graphics card doesn't support D3D12.";
                    break;
                case RenderAPI::Metal:
                    errorMessage += "Metal framework may not be available or macOS version is too old.";
                    break;
                default:
                    errorMessage += "Unknown error occurred.";
                    break;
            }
            return false;
        }
        
        std::cout << "  - Render system loaded successfully" << std::endl;
        std::cout << "  - Renderer: " << renderSystem_->GetName() << std::endl;
        
        // Get and validate renderer info
        try {
            const auto& info = renderSystem_->GetRendererInfo();
            std::cout << "  - Device: " << info.deviceName << std::endl;
            std::cout << "  - Vendor: " << info.vendorName << std::endl;
            std::cout << "  - API Version: " << info.shadingLanguageName << std::endl;
            
            // Check if device name is empty (might indicate driver issues)
            if (info.deviceName.empty()) {
                errorMessage = "Graphics device information is unavailable. This may indicate driver issues.";
                delete renderSystem_;
                renderSystem_ = nullptr;
                return false;
            }
        } catch (const std::exception& e) {
            errorMessage = "Failed to get renderer information: " + std::string(e.what());
            delete renderSystem_;
            renderSystem_ = nullptr;
            return false;
        }
        
        std::cout << "  - " << GetModuleName(api) << " API validation successful" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        errorMessage = "Exception during " + GetModuleName(api) + " initialization: " + e.what();
        
        // Clean up on exception
        if (renderSystem_) {
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        return false;
    } catch (...) {
        errorMessage = "Unknown exception during " + GetModuleName(api) + " initialization";
        
        // Clean up on exception
        if (renderSystem_) {
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        return false;
    }
}

SystemGraphicsInfo RenderingPlugin::DetectSystemGraphics() {
    SystemGraphicsInfo info;
    
    // Check for headless environment
    info.isHeadless = IsHeadlessEnvironment();
    
    // Get OS version and graphics information
    #ifdef __APPLE__
    info.osVersion = GetMacOSGraphicsInfo();
    
    // Check Metal framework availability
    info.metalSupported = true; // Assume available on macOS 10.11+
    
    // Check OpenGL support
    info.openglSupported = true; // Generally available on macOS
    
    // Vulkan requires MoltenVK
    info.vulkanSupported = false; // Conservative assumption
    
    if (!info.isHeadless) {
        info.hasDisplay = true;
        info.supportedAPIs.push_back(RenderAPI::Metal);
        info.supportedAPIs.push_back(RenderAPI::OpenGL);
    }
    #elif defined(_WIN32)
    // Windows-specific detection
    info.openglSupported = true;
    info.vulkanSupported = true; // Usually available
    info.supportedAPIs.push_back(RenderAPI::Direct3D11);
    info.supportedAPIs.push_back(RenderAPI::OpenGL);
    info.supportedAPIs.push_back(RenderAPI::Vulkan);
    #else
    // Linux-specific detection
    info.openglSupported = true;
    info.vulkanSupported = true;
    info.supportedAPIs.push_back(RenderAPI::OpenGL);
    info.supportedAPIs.push_back(RenderAPI::Vulkan);
    #endif
    
    return info;
}

bool RenderingPlugin::IsHeadlessEnvironment() {
    #ifdef __APPLE__
    // Check for SSH session or no display
    const char* sshClient = std::getenv("SSH_CLIENT");
    const char* sshTty = std::getenv("SSH_TTY");
    const char* display = std::getenv("DISPLAY");
    
    if (sshClient || sshTty) {
        return true; // SSH session
    }
    
    // Check if running in terminal without window server access
    // This is a simplified check - in practice, you might want to use
    // CGSessionCopyCurrentDictionary or similar APIs
    return false;
    #elif defined(_WIN32)
    // Windows headless detection
    return GetSystemMetrics(SM_CMONITORS) == 0;
    #else
    // Linux headless detection
    const char* display = std::getenv("DISPLAY");
    const char* waylandDisplay = std::getenv("WAYLAND_DISPLAY");
    return (!display && !waylandDisplay);
    #endif
}

std::string RenderingPlugin::GetMacOSGraphicsInfo() {
    std::string info = "macOS Graphics Environment Information:\n";
    
    #ifdef __APPLE__
    // Get macOS version using system calls
    info += "  - macOS Version: ";
    try {
        FILE* pipe = popen("sw_vers -productVersion", "r");
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                // Remove newline
                std::string version(buffer);
                if (!version.empty() && version.back() == '\n') {
                    version.pop_back();
                }
                info += version;
            } else {
                info += "Unknown";
            }
            pclose(pipe);
        } else {
            info += "Unable to determine";
        }
    } catch (...) {
        info += "Error retrieving version";
    }
    info += "\n";
    
    // Check for Metal support
    info += "  - Metal Support: ";
    try {
        // Try to load Metal framework
        void* metalFramework = dlopen("/System/Library/Frameworks/Metal.framework/Metal", RTLD_LAZY);
        if (metalFramework) {
            info += "Available (Metal framework loaded)";
            dlclose(metalFramework);
        } else {
            info += "Not available (Metal framework not found)";
        }
    } catch (...) {
        info += "Error checking Metal support";
    }
    info += "\n";
    
    // Check for OpenGL support
    info += "  - OpenGL Support: ";
    try {
        void* openglFramework = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY);
        if (openglFramework) {
            info += "Available (OpenGL framework loaded)";
            dlclose(openglFramework);
        } else {
            info += "Not available (OpenGL framework not found)";
        }
    } catch (...) {
        info += "Error checking OpenGL support";
    }
    info += "\n";
    
    // Check display configuration
    info += "  - Display Configuration: ";
    try {
        FILE* pipe = popen("system_profiler SPDisplaysDataType | grep -E 'Displays:|Resolution:|Chipset Model:' | head -10", "r");
        if (pipe) {
            char buffer[256];
            bool hasDisplayInfo = false;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::string line(buffer);
                if (!line.empty() && line.back() == '\n') {
                    line.pop_back();
                }
                if (!hasDisplayInfo) {
                    info += "\n      " + line;
                    hasDisplayInfo = true;
                } else {
                    info += "\n      " + line;
                }
            }
            if (!hasDisplayInfo) {
                info += "No display information available";
            }
            pclose(pipe);
        } else {
            info += "Unable to retrieve display information";
        }
    } catch (...) {
        info += "Error retrieving display information";
    }
    info += "\n";
    
    // Check for headless environment indicators
    info += "  - Environment Type: ";
    const char* display = std::getenv("DISPLAY");
    const char* ssh = std::getenv("SSH_CONNECTION");
    const char* term = std::getenv("TERM");
    
    if (ssh) {
        info += "SSH session detected";
    } else if (!display && term) {
        info += "Terminal session (possibly headless)";
    } else if (display) {
        info += "Display environment available";
    } else {
        info += "Unknown environment type";
    }
    info += "\n";
    
    // Check graphics card information
    info += "  - Graphics Hardware: ";
    try {
        FILE* pipe = popen("system_profiler SPDisplaysDataType | grep 'Chipset Model:' | head -3", "r");
        if (pipe) {
            char buffer[256];
            bool hasGPUInfo = false;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::string line(buffer);
                if (!line.empty() && line.back() == '\n') {
                    line.pop_back();
                }
                if (!hasGPUInfo) {
                    info += "\n      " + line;
                    hasGPUInfo = true;
                } else {
                    info += "\n      " + line;
                }
            }
            if (!hasGPUInfo) {
                info += "No GPU information available";
            }
            pclose(pipe);
        } else {
            info += "Unable to retrieve GPU information";
        }
    } catch (...) {
        info += "Error retrieving GPU information";
    }
    info += "\n";
    
    // Check for virtualization
    info += "  - Virtualization: ";
    try {
        FILE* pipe = popen("sysctl -n machdep.cpu.features | grep -i vmx", "r");
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                info += "Hardware virtualization supported";
            } else {
                info += "Hardware virtualization not detected";
            }
            pclose(pipe);
        } else {
            info += "Unable to check virtualization support";
        }
    } catch (...) {
        info += "Error checking virtualization";
    }
    info += "\n";
    
    #else
    info = "Not running on macOS";
    #endif
    
    return info;
}

bool RenderingPlugin::InitializeWithFallback(RenderingMode mode) {
    if (!initialized_) {
        std::cerr << "ERROR: InitializeWithFallback: Plugin not initialized" << std::endl;
        return false;
    }
    
    currentMode_ = mode;
    
    // Detect system capabilities first
    systemInfo_ = DetectSystemGraphics();
    
    std::cout << "\n=== SYSTEM GRAPHICS DETECTION ===" << std::endl;
    std::cout << "Headless Environment: " << (systemInfo_.isHeadless ? "Yes" : "No") << std::endl;
    std::cout << "Display Available: " << (systemInfo_.hasDisplay ? "Yes" : "No") << std::endl;
    
    #ifdef __APPLE__
    std::cout << GetMacOSGraphicsInfo() << std::endl;
    #endif
    
    if (mode == RenderingMode::Software) {
        std::cout << "Software rendering mode requested" << std::endl;
        softwareRenderingEnabled_ = true;
        return InitializeSoftwareRenderer();
    }
    
    if (mode == RenderingMode::Headless) {
        std::cout << "Headless rendering mode requested" << std::endl;
        return InitializeHeadlessRenderer();
    }
    
    // Try to initialize with detected APIs
    if (systemInfo_.supportedAPIs.empty()) {
        std::cerr << "No supported graphics APIs detected" << std::endl;
        if (mode != RenderingMode::Software) {
            std::cerr << "Falling back to software rendering..." << std::endl;
            currentMode_ = RenderingMode::Software;
            softwareRenderingEnabled_ = true;
            return InitializeSoftwareRenderer();
        }
        return false;
    }
    
    // Try each detected API
    for (RenderAPI api : systemInfo_.supportedAPIs) {
        if (InitializeRenderSystem(api)) {
            return true;
        }
    }
    
    // All hardware APIs failed
    if (mode == RenderingMode::Hardware) {
        std::cerr << "\nHardware rendering failed, attempting software fallback..." << std::endl;
        currentMode_ = RenderingMode::Software;
        softwareRenderingEnabled_ = true;
        return InitializeSoftwareRenderer();
    }
    
    return false;
}

bool RenderingPlugin::InitializeSoftwareRenderer() {
    std::cerr << "\nAttempting software rendering fallback..." << std::endl;
    
    try {
        // Try to initialize with Null renderer as software fallback
        std::cerr << "   - Configuring Null renderer (software fallback)" << std::endl;
        
        LLGL::RenderSystemDescriptor renderSystemDesc;
        renderSystemDesc.moduleName = "Null";
        
        // Clean up any existing render system
        if (renderSystem_) {
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        
        // Create the render system
        auto renderSystemPtr = LLGL::RenderSystem::Load(renderSystemDesc);
        renderSystem_ = renderSystemPtr.release();
        
        if (!renderSystem_) {
            std::cerr << "   FAILED: Could not load Null renderer" << std::endl;
            return false;
        }
        
        std::cerr << "   - Null renderer loaded successfully" << std::endl;
        std::cerr << "   SUCCESS: Software rendering initialized" << std::endl;
        std::cerr << "   Note: Software rendering provides limited functionality" << std::endl;
        
        currentAPI_ = RenderAPI::None; // Mark as software rendering
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "   FAILED: Software rendering initialization failed: " << e.what() << std::endl;
        // Clean up on failure
        if (renderSystem_) {
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        return false;
    }
}

bool RenderingPlugin::InitializeHeadlessRenderer() {
    std::cerr << "\nAttempting headless rendering initialization..." << std::endl;
    
    try {
        // Try to initialize with Null renderer for headless mode
        std::cerr << "   - Configuring Null renderer (headless mode)" << std::endl;
        
        LLGL::RenderSystemDescriptor renderSystemDesc;
        renderSystemDesc.moduleName = "Null";
        
        // Clean up any existing render system
        if (renderSystem_) {
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        
        // Create the render system
        auto renderSystemPtr = LLGL::RenderSystem::Load(renderSystemDesc);
        renderSystem_ = renderSystemPtr.release();
        
        if (!renderSystem_) {
            std::cerr << "   FAILED: Could not load Null renderer for headless mode" << std::endl;
            return false;
        }
        
        std::cerr << "   - Null renderer loaded successfully for headless mode" << std::endl;
        std::cerr << "   SUCCESS: Headless rendering initialized" << std::endl;
        std::cerr << "   Note: Headless rendering provides offscreen rendering capabilities" << std::endl;
        
        currentAPI_ = RenderAPI::None; // Mark as headless rendering
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "   FAILED: Headless rendering initialization failed: " << e.what() << std::endl;
        // Clean up on failure
        if (renderSystem_) {
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        return false;
    }
}

RenderingMode RenderingPlugin::GetRenderingMode() const {
    return currentMode_;
}

bool RenderingPlugin::CreateOffscreenBuffer(int width, int height) {
    if (!renderSystem_) {
        std::cerr << "CreateOffscreenBuffer: No render system available" << std::endl;
        return false;
    }
    
    if (currentMode_ != RenderingMode::Headless && currentMode_ != RenderingMode::Software) {
        std::cerr << "CreateOffscreenBuffer: Only available in headless or software mode" << std::endl;
        return false;
    }
    
    try {
        // Create offscreen render target
        LLGL::RenderTargetDescriptor renderTargetDesc;
        renderTargetDesc.resolution = { static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height) };
        renderTargetDesc.samples = 1;
        
        // Add color attachment
        LLGL::AttachmentDescriptor colorAttachment;
        colorAttachment.format = LLGL::Format::RGBA8UNorm;
        renderTargetDesc.colorAttachments[0] = colorAttachment;
        
        // Add depth attachment
        LLGL::AttachmentDescriptor depthAttachment;
        depthAttachment.format = LLGL::Format::D24UNormS8UInt;
        renderTargetDesc.depthStencilAttachment = depthAttachment;
        
        // Create the render target
        offscreenRenderTarget_ = renderSystem_->CreateRenderTarget(renderTargetDesc);
        if (!offscreenRenderTarget_) {
            std::cerr << "CreateOffscreenBuffer: Failed to create render target" << std::endl;
            return false;
        }
        
        // Create command buffer if not already created
        if (!commandBuffer_) {
            commandBuffer_ = renderSystem_->CreateCommandBuffer();
            if (!commandBuffer_) {
                std::cerr << "CreateOffscreenBuffer: Failed to create command buffer" << std::endl;
                return false;
            }
        }
        
        std::cout << "CreateOffscreenBuffer: Offscreen buffer created successfully (" << width << "x" << height << ")" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateOffscreenBuffer: Exception caught: " << e.what() << std::endl;
        return false;
    }
}

bool RenderingPlugin::IsSoftwareRenderingEnabled() const {
    return softwareRenderingEnabled_;
}

bool RenderingPlugin::IsHeadlessMode() const {
    return currentMode_ == RenderingMode::Headless;
}

LLGL::RenderSystem* RenderingPlugin::GetRenderSystem() const {
    return renderSystem_;
}

LLGL::SwapChain* RenderingPlugin::GetSwapChain() const {
    return swapChain_.get();
}

LLGL::CommandBuffer* RenderingPlugin::GetCommandBuffer() const {
    return commandBuffer_;
}

// === Rendering Resource Management Implementation ===

LLGL::Buffer* RenderingPlugin::CreateVertexBuffer(const Vertex* vertices, std::uint32_t vertexCount) {
    // Try using new modular architecture first
    if (resourceManager_) {
        return resourceManager_->CreateVertexBuffer(vertices, vertexCount);
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_) {
        std::cerr << "CreateVertexBuffer: No render system available" << std::endl;
        return nullptr;
    }
    
    try {
        // Create vertex buffer descriptor
        LLGL::BufferDescriptor vertexBufferDesc;
        vertexBufferDesc.size = sizeof(Vertex) * vertexCount;
        vertexBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        vertexBufferDesc.usage = LLGL::Usage::Immutable;
        
        // Create the vertex buffer with initial data
        auto vertexBuffer = renderSystem_->CreateBuffer(vertexBufferDesc, vertices);
        if (!vertexBuffer) {
            std::cerr << "CreateVertexBuffer: Failed to create vertex buffer" << std::endl;
            return nullptr;
        }
        
        std::cout << "CreateVertexBuffer: Created vertex buffer with " << vertexCount << " vertices" << std::endl;
        return vertexBuffer;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateVertexBuffer: Exception caught: " << e.what() << std::endl;
        return nullptr;
    }
}

LLGL::Buffer* RenderingPlugin::CreateIndexBuffer(const std::uint32_t* indices, std::uint32_t indexCount) {
    // Try using new modular architecture first
    if (resourceManager_) {
        return resourceManager_->CreateIndexBuffer(indices, indexCount);
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_) {
        std::cerr << "CreateIndexBuffer: No render system available" << std::endl;
        return nullptr;
    }
    
    try {
        // Create index buffer descriptor
        LLGL::BufferDescriptor indexBufferDesc;
        indexBufferDesc.size = sizeof(std::uint32_t) * indexCount;
        indexBufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
        indexBufferDesc.usage = LLGL::Usage::Immutable;
        
        // Create the index buffer with initial data
        auto indexBuffer = renderSystem_->CreateBuffer(indexBufferDesc, indices);
        if (!indexBuffer) {
            std::cerr << "CreateIndexBuffer: Failed to create index buffer" << std::endl;
            return nullptr;
        }
        
        std::cout << "CreateIndexBuffer: Created index buffer with " << indexCount << " indices" << std::endl;
        return indexBuffer;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateIndexBuffer: Exception caught: " << e.what() << std::endl;
        return nullptr;
    }
}

LLGL::Buffer* RenderingPlugin::CreateConstantBuffer() {
    // Try using new modular architecture first
    if (resourceManager_) {
        return resourceManager_->CreateConstantBuffer(sizeof(Matrices));
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_) {
        std::cerr << "CreateConstantBuffer: No render system available" << std::endl;
        return nullptr;
    }
    
    try {
        // Create constant buffer descriptor
        LLGL::BufferDescriptor constantBufferDesc;
        constantBufferDesc.size = sizeof(Matrices);
        constantBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
        constantBufferDesc.usage = LLGL::Usage::Dynamic;
        
        // Create the constant buffer
        auto constantBuffer = renderSystem_->CreateBuffer(constantBufferDesc);
        if (!constantBuffer) {
            std::cerr << "CreateConstantBuffer: Failed to create constant buffer" << std::endl;
            return nullptr;
        }
        
        std::cout << "CreateConstantBuffer: Created constant buffer for matrix data" << std::endl;
        return constantBuffer;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateConstantBuffer: Exception caught: " << e.what() << std::endl;
        return nullptr;
    }
}

LLGL::Texture* RenderingPlugin::CreateTexture(const std::string& filename) {
    // Try using new modular architecture first
    if (resourceManager_) {
        return resourceManager_->CreateTexture(filename);
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_) {
        std::cerr << "CreateTexture: No render system available" << std::endl;
        return nullptr;
    }
    
    try {
        // For now, create a simple 2x2 white texture as placeholder
        // In a real implementation, you would load the texture from file
        std::uint32_t textureData[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
        
        LLGL::TextureDescriptor textureDesc;
        textureDesc.type = LLGL::TextureType::Texture2D;
        textureDesc.bindFlags = LLGL::BindFlags::Sampled;
        textureDesc.format = LLGL::Format::RGBA8UNorm;
        textureDesc.extent.width = 2;
        textureDesc.extent.height = 2;
        textureDesc.extent.depth = 1;
        textureDesc.arrayLayers = 1;
        textureDesc.mipLevels = 1;
        
        auto texture = renderSystem_->CreateTexture(textureDesc, LLGL::ImageView{ LLGL::ImageFormat::RGBA, LLGL::DataType::UInt8, textureData });
        if (!texture) {
            std::cerr << "CreateTexture: Failed to create texture" << std::endl;
            return nullptr;
        }
        
        std::cout << "CreateTexture: Created placeholder texture (2x2 white)" << std::endl;
        return texture;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateTexture: Exception caught: " << e.what() << std::endl;
        return nullptr;
    }
}

LLGL::Sampler* RenderingPlugin::CreateSampler(int maxAnisotropy) {
    // Try using new modular architecture first
    if (resourceManager_) {
        return resourceManager_->CreateSampler(maxAnisotropy);
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_) {
        std::cerr << "CreateSampler: No render system available" << std::endl;
        return nullptr;
    }
    
    try {
        LLGL::SamplerDescriptor samplerDesc;
        samplerDesc.addressModeU = LLGL::SamplerAddressMode::Repeat;
        samplerDesc.addressModeV = LLGL::SamplerAddressMode::Repeat;
        samplerDesc.addressModeW = LLGL::SamplerAddressMode::Repeat;
        samplerDesc.minFilter = LLGL::SamplerFilter::Linear;
        samplerDesc.magFilter = LLGL::SamplerFilter::Linear;
        samplerDesc.mipMapFilter = LLGL::SamplerFilter::Linear;
        samplerDesc.maxAnisotropy = static_cast<std::uint32_t>(maxAnisotropy);
        
        auto sampler = renderSystem_->CreateSampler(samplerDesc);
        if (!sampler) {
            std::cerr << "CreateSampler: Failed to create sampler" << std::endl;
            return nullptr;
        }
        
        std::cout << "CreateSampler: Created texture sampler with anisotropy " << maxAnisotropy << std::endl;
        return sampler;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateSampler: Exception caught: " << e.what() << std::endl;
        return nullptr;
    }
}

bool RenderingPlugin::CreateShaders(LLGL::Shader*& vertexShaderOut, LLGL::Shader*& fragmentShaderOut) {
    // Try using new modular architecture first
    if (shaderManager_) {
        return shaderManager_->CreateDefaultShaders(vertexShaderOut, fragmentShaderOut);
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_) {
        std::cerr << "CreateShaders: No render system available" << std::endl;
        return false;
    }
    
    try {
        const auto& renderCaps = renderSystem_->GetRenderingCaps();
        
        // Determine which shading language to use based on render system capabilities
        std::string vertexShaderSource, fragmentShaderSource;
        LLGL::ShaderDescriptor shaderDesc;
        
        if (renderCaps.shadingLanguages.find(LLGL::ShadingLanguage::HLSL) != renderCaps.shadingLanguages.end()) {
            // HLSL shaders for Direct3D
            shaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
            
            vertexShaderSource = R"(
struct VertexIn {
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOut {
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

cbuffer Matrices : register(b0) {
    float4x4 wvpMatrix;
    float4x4 worldMatrix;
};

VertexOut VS(VertexIn inp) {
    VertexOut outp;
    outp.position = mul(wvpMatrix, float4(inp.position, 1));
    outp.normal = normalize(mul((float3x3)worldMatrix, inp.normal));
    outp.texCoord = inp.texCoord;
    return outp;
}
)";
            
            fragmentShaderSource = R"(
struct VertexOut {
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

Texture2D colorMap : register(t0);
SamplerState colorMapSampler : register(s0);

float4 PS(VertexOut inp) : SV_Target {
    float3 lightDir = normalize(float3(0, 0, -1));
    float NdotL = max(0.2, dot(lightDir, normalize(inp.normal)));
    float4 albedo = colorMap.Sample(colorMapSampler, inp.texCoord);
    return float4(albedo.rgb * NdotL, albedo.a);
}
)";
            
            shaderDesc.entryPoint = "VS";
            
        } else if (renderCaps.shadingLanguages.find(LLGL::ShadingLanguage::GLSL) != renderCaps.shadingLanguages.end()) {
            // GLSL shaders for OpenGL
            shaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
            
            vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 vNormal;
out vec2 vTexCoord;

uniform Matrices {
    mat4 wvpMatrix;
    mat4 worldMatrix;
};

void main() {
    gl_Position = wvpMatrix * vec4(position, 1.0);
    vNormal = normalize(mat3(worldMatrix) * normal);
    vTexCoord = texCoord;
}
)";
            
            fragmentShaderSource = R"(
#version 330 core

in vec3 vNormal;
in vec2 vTexCoord;

out vec4 fragColor;

uniform sampler2D colorMap;

void main() {
    vec3 lightDir = normalize(vec3(0.0, 0.0, -1.0));
    float NdotL = max(0.2, dot(lightDir, normalize(vNormal)));
    vec4 albedo = texture(colorMap, vTexCoord);
    fragColor = vec4(albedo.rgb * NdotL, albedo.a);
}
)";
            
            shaderDesc.entryPoint = "main";
            
        } else if (renderCaps.shadingLanguages.find(LLGL::ShadingLanguage::Metal) != renderCaps.shadingLanguages.end()) {
            // Metal shaders for macOS/iOS
            shaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
            
            vertexShaderSource = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 normal;
    float2 texCoord;
};

struct Matrices {
    float4x4 wvpMatrix;
    float4x4 worldMatrix;
};

vertex VertexOut VS(VertexIn inp [[stage_in]], constant Matrices& matrices [[buffer(0)]]) {
    VertexOut outp;
    outp.position = matrices.wvpMatrix * float4(inp.position, 1.0);
    outp.normal = normalize((matrices.worldMatrix * float4(inp.normal, 0.0)).xyz);
    outp.texCoord = inp.texCoord;
    return outp;
}
)";
            
            fragmentShaderSource = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float3 normal;
    float2 texCoord;
};

fragment float4 PS(VertexOut inp [[stage_in]], texture2d<float> colorMap [[texture(0)]], sampler colorMapSampler [[sampler(0)]]) {
    float3 lightDir = normalize(float3(0.0, 0.0, -1.0));
    float NdotL = max(0.2, dot(lightDir, normalize(inp.normal)));
    float4 albedo = colorMap.sample(colorMapSampler, inp.texCoord);
    return float4(albedo.rgb * NdotL, albedo.a);
}
)";
            
            shaderDesc.entryPoint = "VS";
            
        } else {
            std::cerr << "CreateShaders: No supported shading language found" << std::endl;
            return false;
        }
        
        // Create vertex shader
        shaderDesc.type = LLGL::ShaderType::Vertex;
        shaderDesc.source = vertexShaderSource.c_str();
        shaderDesc.sourceSize = vertexShaderSource.size();
        
        if (std::find(renderCaps.shadingLanguages.begin(), renderCaps.shadingLanguages.end(), LLGL::ShadingLanguage::Metal) != renderCaps.shadingLanguages.end()) {
            shaderDesc.entryPoint = "VS";
        }
        
        vertexShaderOut = renderSystem_->CreateShader(shaderDesc);
        if (!vertexShaderOut) {
            std::cerr << "CreateShaders: Failed to create vertex shader" << std::endl;
            return false;
        }
        
        // Check for vertex shader compilation errors
        if (auto report = vertexShaderOut->GetReport()) {
            if (report->HasErrors()) {
                std::cerr << "CreateShaders: Vertex shader compilation errors:\n" << report->GetText() << std::endl;
                renderSystem_->Release(*vertexShaderOut);
                vertexShaderOut = nullptr;
                return false;
            }
        }
        
        // Create fragment shader
        shaderDesc.type = LLGL::ShaderType::Fragment;
        shaderDesc.source = fragmentShaderSource.c_str();
        shaderDesc.sourceSize = fragmentShaderSource.size();
        
        if (std::find(renderCaps.shadingLanguages.begin(), renderCaps.shadingLanguages.end(), LLGL::ShadingLanguage::Metal) != renderCaps.shadingLanguages.end()) {
            shaderDesc.entryPoint = "PS";
        } else if (std::find(renderCaps.shadingLanguages.begin(), renderCaps.shadingLanguages.end(), LLGL::ShadingLanguage::HLSL) != renderCaps.shadingLanguages.end()) {
            shaderDesc.entryPoint = "PS";
        }
        
        fragmentShaderOut = renderSystem_->CreateShader(shaderDesc);
        if (!fragmentShaderOut) {
            std::cerr << "CreateShaders: Failed to create fragment shader" << std::endl;
            renderSystem_->Release(*vertexShaderOut);
            vertexShaderOut = nullptr;
            return false;
        }
        
        // Check for fragment shader compilation errors
        if (auto report = fragmentShaderOut->GetReport()) {
            if (report->HasErrors()) {
                std::cerr << "CreateShaders: Fragment shader compilation errors:\n" << report->GetText() << std::endl;
                renderSystem_->Release(*vertexShaderOut);
                renderSystem_->Release(*fragmentShaderOut);
                vertexShaderOut = nullptr;
                fragmentShaderOut = nullptr;
                return false;
            }
        }
        
        std::cout << "CreateShaders: Successfully created vertex and fragment shaders" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateShaders: Exception caught: " << e.what() << std::endl;
        if (vertexShaderOut) {
            renderSystem_->Release(*vertexShaderOut);
            vertexShaderOut = nullptr;
        }
        if (fragmentShaderOut) {
            renderSystem_->Release(*fragmentShaderOut);
            fragmentShaderOut = nullptr;
        }
        return false;
    }
}

LLGL::PipelineLayout* RenderingPlugin::CreatePipelineLayout() {
    if (!renderSystem_) {
        std::cerr << "CreatePipelineLayout: No render system available" << std::endl;
        return nullptr;
    }
    
    try {
        LLGL::PipelineLayoutDescriptor layoutDesc;
        
        // Add constant buffer binding for matrices
        LLGL::BindingDescriptor constantBufferBinding;
        constantBufferBinding.name = "Matrices";
        constantBufferBinding.type = LLGL::ResourceType::Buffer;
        constantBufferBinding.bindFlags = LLGL::BindFlags::ConstantBuffer;
        constantBufferBinding.stageFlags = LLGL::StageFlags::VertexStage;
        constantBufferBinding.slot.index = 0;
        layoutDesc.bindings.push_back(constantBufferBinding);
        
        // Add texture binding
        LLGL::BindingDescriptor textureBinding;
        textureBinding.name = "colorMap";
        textureBinding.type = LLGL::ResourceType::Texture;
        textureBinding.bindFlags = LLGL::BindFlags::Sampled;
        textureBinding.stageFlags = LLGL::StageFlags::FragmentStage;
        textureBinding.slot.index = 0;
        layoutDesc.bindings.push_back(textureBinding);
        
        // Add sampler binding
        LLGL::BindingDescriptor samplerBinding;
        samplerBinding.name = "colorMapSampler";
        samplerBinding.type = LLGL::ResourceType::Sampler;
        samplerBinding.stageFlags = LLGL::StageFlags::FragmentStage;
        samplerBinding.slot.index = 0;
        layoutDesc.bindings.push_back(samplerBinding);
        
        auto layout = renderSystem_->CreatePipelineLayout(layoutDesc);
        if (!layout) {
            std::cerr << "CreatePipelineLayout: Failed to create pipeline layout" << std::endl;
            return nullptr;
        }
        
        std::cout << "CreatePipelineLayout: Successfully created pipeline layout" << std::endl;
        return layout;
        
    } catch (const std::exception& e) {
        std::cerr << "CreatePipelineLayout: Exception caught: " << e.what() << std::endl;
        return nullptr;
    }
}

LLGL::ResourceHeap* RenderingPlugin::CreateResourceHeap(LLGL::PipelineLayout* layout, 
                                                        LLGL::Buffer* constantBuffer,
                                                        LLGL::Texture* texture, 
                                                        LLGL::Sampler* sampler) {
    if (!renderSystem_ || !layout) {
        std::cerr << "CreateResourceHeap: No render system or layout available" << std::endl;
        return nullptr;
    }
    
    try {
        LLGL::ResourceHeapDescriptor heapDesc;
        heapDesc.pipelineLayout = layout;
        
        // Add resources to heap descriptor
        std::vector<LLGL::Resource*> resources;
        if (constantBuffer) {
            resources.push_back(constantBuffer);
        }
        if (texture) {
            resources.push_back(texture);
        }
        if (sampler) {
            resources.push_back(sampler);
        }
        
        LLGL::ResourceHeap* heap = renderSystem_->CreateResourceHeap(heapDesc, resources.data());
        if (!heap) {
            std::cerr << "CreateResourceHeap: Failed to create resource heap" << std::endl;
            return nullptr;
        }
        
        std::cout << "CreateResourceHeap: Successfully created resource heap" << std::endl;
        return heap;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateResourceHeap: Exception caught: " << e.what() << std::endl;
        return nullptr;
    }
}

bool RenderingPlugin::CreatePipelineState(LLGL::PipelineState*& pipelineOut, LLGL::Shader* vertexShader, 
                                        LLGL::Shader* fragmentShader, LLGL::PipelineLayout* layout) {
    if (!renderSystem_ || !vertexShader || !fragmentShader || !layout) {
        std::cerr << "CreatePipelineState: Missing required parameters" << std::endl;
        return false;
    }
    
    try {
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        
        // Set shaders
        pipelineDesc.vertexShader = vertexShader;
        pipelineDesc.fragmentShader = fragmentShader;
        
        // Set pipeline layout
        pipelineDesc.pipelineLayout = layout;
        
        // Configure vertex input layout
        LLGL::VertexAttribute positionAttr;
        positionAttr.name = "position";
        positionAttr.format = LLGL::Format::RGB32Float;
        positionAttr.location = 0;
        positionAttr.offset = 0;
        
        LLGL::VertexAttribute normalAttr;
        normalAttr.name = "normal";
        normalAttr.format = LLGL::Format::RGB32Float;
        normalAttr.location = 1;
        normalAttr.offset = 12; // 3 * sizeof(float)
        
        LLGL::VertexAttribute texCoordAttr;
        texCoordAttr.name = "texCoord";
        texCoordAttr.format = LLGL::Format::RG32Float;
        texCoordAttr.location = 2;
        texCoordAttr.offset = 24; // 6 * sizeof(float)
        
        pipelineDesc.vertexShader.vertexAttributes = { positionAttr, normalAttr, texCoordAttr };
        
        // Configure rasterizer state
        pipelineDesc.rasterizer.cullMode = LLGL::CullMode::Back;
        pipelineDesc.rasterizer.fillMode = LLGL::FillMode::Solid;
        pipelineDesc.rasterizer.frontCCW = false;
        
        // Configure depth state
        pipelineDesc.depth.testEnabled = true;
        pipelineDesc.depth.writeEnabled = true;
        pipelineDesc.depth.compareOp = LLGL::CompareOp::Less;
        
        // Configure blend state
        pipelineDesc.blend.targets[0].colorMask = LLGL::ColorMaskFlags::All;
        
        // Configure multisampling
        pipelineDesc.rasterizer.multiSampleEnabled = (swapChain_ && swapChain_->GetSamples() > 1);
        
        pipelineOut = renderSystem_->CreatePipelineState(pipelineDesc);
        if (!pipelineOut) {
            std::cerr << "CreatePipelineState: Failed to create pipeline state" << std::endl;
            return false;
        }
        
        std::cout << "CreatePipelineState: Successfully created pipeline state" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "CreatePipelineState: Exception caught: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Vertex> RenderingPlugin::GenerateCubeVertices() {
    // Try using new modular architecture first
    if (geometryGenerator_) {
        return geometryGenerator_->GenerateCubeVertices();
    }
    
    // Fallback to legacy implementation
    std::vector<Vertex> vertices;
    
    // Define cube vertices with positions, normals, and texture coordinates
    // Front face
    vertices.push_back({{-1.0f, -1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ 1.0f, -1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ 1.0f,  1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-1.0f,  1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}});
    
    // Back face
    vertices.push_back({{ 1.0f, -1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{-1.0f, -1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-1.0f,  1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}});
    vertices.push_back({{ 1.0f,  1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}});
    
    // Left face
    vertices.push_back({{-1.0f, -1.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{-1.0f, -1.0f,  1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-1.0f,  1.0f,  1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-1.0f,  1.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}});
    
    // Right face
    vertices.push_back({{ 1.0f, -1.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ 1.0f, -1.0f, -1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ 1.0f,  1.0f, -1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{ 1.0f,  1.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}});
    
    // Top face
    vertices.push_back({{-1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ 1.0f,  1.0f, -1.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-1.0f,  1.0f, -1.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}});
    
    // Bottom face
    vertices.push_back({{-1.0f, -1.0f, -1.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ 1.0f, -1.0f, -1.0f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ 1.0f, -1.0f,  1.0f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-1.0f, -1.0f,  1.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}});
    
    return vertices;
}

std::vector<std::uint32_t> RenderingPlugin::GenerateCubeIndices() {
    // Try using new modular architecture first
    if (geometryGenerator_) {
        return geometryGenerator_->GenerateCubeIndices();
    }
    
    // Fallback to legacy implementation
    std::vector<std::uint32_t> indices;
    
    // Define indices for each face (2 triangles per face)
    // Front face
    indices.insert(indices.end(), {0, 1, 2, 0, 2, 3});
    // Back face
    indices.insert(indices.end(), {4, 5, 6, 4, 6, 7});
    // Left face
    indices.insert(indices.end(), {8, 9, 10, 8, 10, 11});
    // Right face
    indices.insert(indices.end(), {12, 13, 14, 12, 14, 15});
    // Top face
    indices.insert(indices.end(), {16, 17, 18, 16, 18, 19});
    // Bottom face
    indices.insert(indices.end(), {20, 21, 22, 20, 22, 23});
    
    return indices;
}

std::vector<Vertex> RenderingPlugin::GenerateTriangleVertices() {
    // Try using new modular architecture first
    if (geometryGenerator_) {
        return geometryGenerator_->GenerateTriangleVertices();
    }
    
    // Fallback to legacy implementation
    std::vector<Vertex> vertices;
    
    // Define triangle vertices with positions, normals, and texture coordinates
    vertices.push_back({{ 0.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}});
    vertices.push_back({{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}});
    
    return vertices;
}

std::vector<std::uint32_t> RenderingPlugin::GenerateTriangleIndices() {
    // Try using new modular architecture first
    if (geometryGenerator_) {
        return geometryGenerator_->GenerateTriangleIndices();
    }
    
    // Fallback to legacy implementation
    std::vector<std::uint32_t> indices;
    
    // Define indices for the triangle
    indices.insert(indices.end(), {0, 1, 2});
    
    return indices;
}

Gs::Matrix4f RenderingPlugin::BuildPerspectiveProjection(float fov, float aspectRatio, float nearPlane, float farPlane) {
    // Try using new modular architecture first
    if (geometryGenerator_) {
        return geometryGenerator_->BuildPerspectiveProjection(fov, aspectRatio, nearPlane, farPlane);
    }
    
    // Fallback to legacy implementation
    // Build perspective projection matrix using Gauss Math library
    return Gs::ProjectionMatrix4f::Perspective(fov, aspectRatio, nearPlane, farPlane).ToMatrix4();
}

bool RenderingPlugin::UpdateConstantBuffer(LLGL::Buffer* constantBuffer, const Matrices& matrices) {
    // Try using new modular architecture first
    if (resourceManager_) {
        return resourceManager_->UpdateConstantBuffer(constantBuffer, &matrices, sizeof(Matrices));
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_ || !constantBuffer) {
        std::cerr << "UpdateConstantBuffer: No render system or constant buffer available" << std::endl;
        return false;
    }
    
    try {
        // Map the constant buffer for writing
        if (auto mappedData = renderSystem_->MapBuffer(*constantBuffer, LLGL::CPUAccess::WriteOnly)) {
            // Copy matrices data to the mapped buffer
            std::memcpy(mappedData, &matrices, sizeof(Matrices));
            
            // Unmap the buffer
            renderSystem_->UnmapBuffer(*constantBuffer);
            
            return true;
        } else {
            std::cerr << "UpdateConstantBuffer: Failed to map constant buffer" << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "UpdateConstantBuffer: Exception caught: " << e.what() << std::endl;
        return false;
    }
}

bool RenderingPlugin::CreateRenderObject(RenderObject& renderObject, const std::vector<Vertex>& vertices, 
                                       const std::vector<std::uint32_t>& indices) {
    // Try using new modular architecture first
    if (resourceManager_) {
        return resourceManager_->CreateRenderObject(renderObject, vertices, indices);
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_) {
        std::cerr << "CreateRenderObject: No render system available" << std::endl;
        return false;
    }
    
    try {
        // Create vertex buffer
        if (!CreateVertexBuffer(renderObject.vertexBuffer, vertices.data(), vertices.size() * sizeof(Vertex))) {
            std::cerr << "CreateRenderObject: Failed to create vertex buffer" << std::endl;
            return false;
        }
        
        // Create index buffer
        if (!CreateIndexBuffer(renderObject.indexBuffer, indices.data(), indices.size() * sizeof(std::uint32_t))) {
            std::cerr << "CreateRenderObject: Failed to create index buffer" << std::endl;
            ReleaseRenderObject(renderObject);
            return false;
        }
        
        // Create constant buffer for matrices
        if (!CreateConstantBuffer(renderObject.constantBuffer, sizeof(Matrices))) {
            std::cerr << "CreateRenderObject: Failed to create constant buffer" << std::endl;
            ReleaseRenderObject(renderObject);
            return false;
        }
        
        // Create a simple white texture (1x1 pixel)
        std::uint32_t whitePixel = 0xFFFFFFFF;
        if (!CreateTexture(renderObject.texture, &whitePixel, 1, 1)) {
            std::cerr << "CreateRenderObject: Failed to create texture" << std::endl;
            ReleaseRenderObject(renderObject);
            return false;
        }
        
        // Create sampler
        if (!CreateSampler(renderObject.sampler)) {
            std::cerr << "CreateRenderObject: Failed to create sampler" << std::endl;
            ReleaseRenderObject(renderObject);
            return false;
        }
        
        // Create shaders
        if (!CreateShaders(renderObject.vertexShader, renderObject.fragmentShader)) {
            std::cerr << "CreateRenderObject: Failed to create shaders" << std::endl;
            ReleaseRenderObject(renderObject);
            return false;
        }
        
        // Create pipeline layout
        if (!CreatePipelineLayout(renderObject.pipelineLayout)) {
            std::cerr << "CreateRenderObject: Failed to create pipeline layout" << std::endl;
            ReleaseRenderObject(renderObject);
            return false;
        }
        
        // Create resource heap
        if (!CreateResourceHeap(renderObject.resourceHeap, renderObject.pipelineLayout, 
                               renderObject.constantBuffer, renderObject.texture, renderObject.sampler)) {
            std::cerr << "CreateRenderObject: Failed to create resource heap" << std::endl;
            ReleaseRenderObject(renderObject);
            return false;
        }
        
        // Create pipeline state
        if (!CreatePipelineState(renderObject.pipelineState, renderObject.vertexShader, 
                                renderObject.fragmentShader, renderObject.pipelineLayout)) {
            std::cerr << "CreateRenderObject: Failed to create pipeline state" << std::endl;
            ReleaseRenderObject(renderObject);
            return false;
        }
        
        // Store index count for rendering
        renderObject.indexCount = static_cast<std::uint32_t>(indices.size());
        
        std::cout << "CreateRenderObject: Successfully created render object with " 
                  << vertices.size() << " vertices and " << indices.size() << " indices" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "CreateRenderObject: Exception caught: " << e.what() << std::endl;
        ReleaseRenderObject(renderObject);
        return false;
    }
}

void RenderingPlugin::ReleaseRenderObject(RenderObject& renderObject) {
    // Try using new modular architecture first
    if (resourceManager_) {
        resourceManager_->ReleaseRenderObject(renderObject);
        return;
    }
    
    // Fallback to legacy implementation
    if (!renderSystem_) {
        return;
    }
    
    // Release all LLGL resources
    if (renderObject.vertexBuffer) {
        renderSystem_->Release(*renderObject.vertexBuffer);
        renderObject.vertexBuffer = nullptr;
    }
    
    if (renderObject.indexBuffer) {
        renderSystem_->Release(*renderObject.indexBuffer);
        renderObject.indexBuffer = nullptr;
    }
    
    if (renderObject.constantBuffer) {
        renderSystem_->Release(*renderObject.constantBuffer);
        renderObject.constantBuffer = nullptr;
    }
    
    if (renderObject.texture) {
        renderSystem_->Release(*renderObject.texture);
        renderObject.texture = nullptr;
    }
    
    if (renderObject.sampler) {
        renderSystem_->Release(*renderObject.sampler);
        renderObject.sampler = nullptr;
    }
    
    if (renderObject.vertexShader) {
        renderSystem_->Release(*renderObject.vertexShader);
        renderObject.vertexShader = nullptr;
    }
    
    if (renderObject.fragmentShader) {
        renderSystem_->Release(*renderObject.fragmentShader);
        renderObject.fragmentShader = nullptr;
    }
    
    if (renderObject.pipelineLayout) {
        renderSystem_->Release(*renderObject.pipelineLayout);
        renderObject.pipelineLayout = nullptr;
    }
    
    if (renderObject.resourceHeap) {
        renderSystem_->Release(*renderObject.resourceHeap);
        renderObject.resourceHeap = nullptr;
    }
    
    if (renderObject.pipelineState) {
        renderSystem_->Release(*renderObject.pipelineState);
        renderObject.pipelineState = nullptr;
    }
    
    renderObject.indexCount = 0;
}

bool RenderingPlugin::RenderObject(const RenderObject& renderObject, const Matrices& matrices) {
    // Try using new modular architecture first
    if (renderCommands_) {
        return renderCommands_->RenderObject(renderObject, matrices);
    }
    
    // Fallback to legacy implementation
    if (!commandBuffer_ || !renderObject.pipelineState || !renderObject.resourceHeap) {
        std::cerr << "RenderObject: Missing required rendering resources" << std::endl;
        return false;
    }
    
    try {
        // Update constant buffer with current matrices
        if (!UpdateConstantBuffer(renderObject.constantBuffer, matrices)) {
            std::cerr << "RenderObject: Failed to update constant buffer" << std::endl;
            return false;
        }
        
        // Set vertex buffer
        if (renderObject.vertexBuffer) {
            commandBuffer_->SetVertexBuffer(*renderObject.vertexBuffer);
        }
        
        // Set index buffer
        if (renderObject.indexBuffer) {
            commandBuffer_->SetIndexBuffer(*renderObject.indexBuffer);
        }
        
        // Set pipeline state
        commandBuffer_->SetPipelineState(*renderObject.pipelineState);
        
        // Set resource heap
        commandBuffer_->SetResourceHeap(*renderObject.resourceHeap);
        
        // Draw indexed triangles
        commandBuffer_->DrawIndexed(renderObject.indexCount, 0);
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "RenderObject: Exception caught: " << e.what() << std::endl;
        return false;
    }
}

bool RenderingPlugin::RenderDemo() {
    if (!initialized_ || !renderSystem_ || !commandBuffer_) {
        std::cerr << "RenderDemo: Plugin not properly initialized" << std::endl;
        return false;
    }
    
    static RenderObject cubeObject;
    static bool demoInitialized = false;
    static auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Initialize demo resources once
        if (!demoInitialized) {
            std::cout << "RenderDemo: Initializing demo resources..." << std::endl;
            
            // Generate cube geometry
            auto vertices = GenerateCubeVertices();
            auto indices = GenerateCubeIndices();
            
            // Create render object with cube geometry
            if (!CreateRenderObject(cubeObject, vertices, indices)) {
                std::cerr << "RenderDemo: Failed to create cube render object" << std::endl;
                return false;
            }
            
            demoInitialized = true;
            std::cout << "RenderDemo: Demo resources initialized successfully" << std::endl;
        }
        
        // Begin frame
        if (!BeginFrame()) {
            std::cerr << "RenderDemo: Failed to begin frame" << std::endl;
            return false;
        }
        
        // Calculate time-based rotation
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<float>(currentTime - startTime).count();
        float rotationAngle = elapsed * 0.5f; // Rotate at 0.5 radians per second
        
        // Get window dimensions for aspect ratio
        int windowWidth, windowHeight;
        if (!GetWindowSize(windowWidth, windowHeight)) {
            windowWidth = 800;
            windowHeight = 600;
        }
        float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
        
        // Build transformation matrices
        Matrices matrices;
        
        // World matrix (rotation around Y axis)
        matrices.world = Gs::Matrix4f::Identity();
        matrices.world *= Gs::RotationMatrix4f::RotateY(rotationAngle).ToMatrix4();
        
        // View matrix (camera looking at origin from distance)
        Gs::Vector3f cameraPos(0.0f, 0.0f, 3.0f);
        Gs::Vector3f target(0.0f, 0.0f, 0.0f);
        Gs::Vector3f up(0.0f, 1.0f, 0.0f);
        auto viewMatrix = Gs::LookAtMatrix4f(cameraPos, target, up).ToMatrix4();
        
        // Projection matrix
        auto projMatrix = BuildPerspectiveProjection(
            Gs::Deg2Rad(45.0f), // 45 degree field of view
            aspectRatio,
            0.1f,               // Near plane
            100.0f              // Far plane
        );
        
        // Combine world, view, and projection matrices
        matrices.worldViewProjection = matrices.world * viewMatrix * projMatrix;
        
        // Render the cube
        if (!RenderObject(cubeObject, matrices)) {
            std::cerr << "RenderDemo: Failed to render cube object" << std::endl;
            EndFrame(); // Still try to end frame
            return false;
        }
        
        // End frame
        if (!EndFrame()) {
            std::cerr << "RenderDemo: Failed to end frame" << std::endl;
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "RenderDemo: Exception caught: " << e.what() << std::endl;
        
        // Clean up demo resources on error
        if (demoInitialized) {
            ReleaseRenderObject(cubeObject);
            demoInitialized = false;
        }
        
        return false;
    }
}

// Plugin export functions implementation
extern "C" {
    RENDERING_PLUGIN_API IPlugin* CreatePlugin() {
        return new RenderingPlugin();
    }
    
    RENDERING_PLUGIN_API const PluginInfo* GetPluginInfo() {
        return &RenderingPlugin::GetPluginStaticInfo();
    }
}

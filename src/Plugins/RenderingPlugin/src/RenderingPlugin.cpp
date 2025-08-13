/**
 * @file RenderingPlugin.cpp
 * @brief Implementation of the RenderingPlugin class
 */

#include "RenderingPlugin.h"
#include "PluginExport.h"
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
    : renderSystem_(nullptr), swapChain_(nullptr), commandBuffer_(nullptr), surface_(nullptr),
      currentAPI_(RenderAPI::None), initialized_(false), currentMode_(RenderingMode::Hardware),
      softwareRenderingEnabled_(false), offscreenRenderTarget_(nullptr) {
    std::cerr << "RenderingPlugin constructor called, this = " << this << std::endl;
    std::cerr << "Before setting: instance_ = " << instance_ << std::endl;
    // Set singleton instance if not already set
    if (instance_ == nullptr) {
        instance_ = this;
        std::cerr << "Set instance_ to this = " << this << std::endl;
    } else {
        std::cerr << "instance_ already set to " << instance_ << std::endl;
    }
    std::cerr << "After setting: instance_ = " << instance_ << std::endl;
}

RenderingPlugin::~RenderingPlugin() {
    // Don't call Shutdown in destructor to avoid issues with LLGL cleanup
    // Shutdown should be called explicitly before destruction
    
    // Clear the singleton instance if it's this instance
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

bool RenderingPlugin::Initialize() {
    try {
        initialized_ = true;
        std::cout << "RenderingPlugin initialized successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize RenderingPlugin: " << e.what() << std::endl;
        initialized_ = false;
        return false;
    }
}

void RenderingPlugin::Shutdown() {
    // Prevent multiple shutdowns - check all critical resources
    if (!initialized_ && !renderSystem_ && !swapChain_) {
        return;
    }
    
    std::cout << "RenderingPlugin::Shutdown() - Starting shutdown process..." << std::endl;
    
    try {
        // Clear surface reference first
        surface_ = nullptr;
        std::cout << "RenderingPlugin::Shutdown() - Surface cleared" << std::endl;
        
        // Release command buffer first (before swap chain and render system)
        if (commandBuffer_ && renderSystem_) {
            std::cout << "RenderingPlugin::Shutdown() - Releasing command buffer..." << std::endl;
            renderSystem_->Release(*commandBuffer_);
            commandBuffer_ = nullptr;
            std::cout << "RenderingPlugin::Shutdown() - Command buffer released" << std::endl;
        }
        
        // Reset swap chain BEFORE render system cleanup to ensure proper destruction order
        if (swapChain_) {
            std::cout << "RenderingPlugin::Shutdown() - Resetting swap chain..." << std::endl;
            swapChain_.reset();
            swapChain_ = nullptr;  // Explicitly set to nullptr after reset
            std::cout << "RenderingPlugin::Shutdown() - Swap chain reset completed" << std::endl;
        }
        
        // Clean up render system AFTER swap chain to avoid dependency issues
        if (renderSystem_) {
            std::cout << "RenderingPlugin::Shutdown() - Deleting render system..." << std::endl;
            delete renderSystem_;
            renderSystem_ = nullptr;
            std::cout << "RenderingPlugin::Shutdown() - Render system deleted" << std::endl;
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
    if (!renderSystem_) {
        std::cout << "CreateWindow: No render system available" << std::endl;
        return false;
    }
    
    std::cout << "CreateWindow: Starting window creation..." << std::endl;
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
        
        // macOS-specific: Set window title in descriptor
        if (!desc.title.empty()) {
            // Note: LLGL may not directly support title in SwapChainDescriptor
            // but we'll store it for later use
        }
        
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
        
        // macOS-specific: Try to set window title if possible
        try {
            // Note: This might not work with all LLGL backends
            // surface_->SetTitle(desc.title.c_str());
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
        
#ifdef __APPLE__
        // macOS特定：强制窗口显示到前台
        try {
            if (surface_) {
                // 处理事件以确保窗口正确显示
                LLGL::Surface::ProcessEvents();
                
                // 短暂延迟让窗口系统处理
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                
                // 再次处理事件
                LLGL::Surface::ProcessEvents();
                
                std::cout << "CreateWindow: macOS window forced to front" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "CreateWindow: Warning - failed to force window to front: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "CreateWindow: Warning - unknown error forcing window to front" << std::endl;
        }
#endif
        
        // macOS-specific: Force window to front
        try {
            // Process events to ensure window is properly initialized
            LLGL::Surface::ProcessEvents();
            
            // Small delay to allow window system to process
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
    if (!initialized_ || !commandBuffer_) {
        return false;
    }
    
    try {
        commandBuffer_->Begin();
        commandBuffer_->BeginRenderPass(*swapChain_);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to begin frame: " << e.what() << std::endl;
        return false;
    }
}

bool RenderingPlugin::EndFrame() {
    if (!initialized_ || !commandBuffer_) {
        return false;
    }
    
    try {
        commandBuffer_->EndRenderPass();
        commandBuffer_->End();
        
        // Submit command buffer and present
        renderSystem_->GetCommandQueue()->Submit(*commandBuffer_);
        swapChain_->Present();
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to end frame: " << e.what() << std::endl;
        return false;
    }
}

void RenderingPlugin::Clear(const Color& color) {
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
    return initialized_;
}

RenderAPI RenderingPlugin::GetCurrentAPI() const {
    return currentAPI_;
}

bool RenderingPlugin::GetWindowSize(int& width, int& height) const {
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
    // For now, just return false. In a real implementation,
    // you would check if the window close button was pressed
    return false;
}

void RenderingPlugin::PollEvents() {
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

// Plugin export functions implementation
extern "C" {
    RENDERING_PLUGIN_API IPlugin* CreatePlugin() {
        return new RenderingPlugin();
    }
    
    RENDERING_PLUGIN_API const PluginInfo* GetPluginInfo() {
        return &RenderingPlugin::GetPluginStaticInfo();
    }
}

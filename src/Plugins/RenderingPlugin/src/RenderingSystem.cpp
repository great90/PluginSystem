/**
 * @file RenderingSystem.cpp
 * @brief Implementation of RenderingSystem class
 */

#include "../include/RenderingSystem.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#ifdef __APPLE__
// Metal headers should be included in .mm files only
// Forward declarations for Metal types
typedef void* MTLDevice;
extern "C" {
    MTLDevice MTLCreateSystemDefaultDevice();
}
#endif

namespace RenderingPlugin {

// === RenderingSystem Implementation ===

RenderingSystem::RenderingSystem()
    : renderSystem_(nullptr)
    , swapChain_(nullptr)
    , commandBuffer_(nullptr)
    , surface_(nullptr)
    , initialized_(false)
    , currentAPI_(RenderAPI::None)
    , currentMode_(RenderingMode::Hardware)
    , softwareRenderingEnabled_(false) {
    
    // Initialize window description with default values
    windowDesc_.title = "LLGL Rendering Window";
    windowDesc_.width = 800;
    windowDesc_.height = 600;
    windowDesc_.fullscreen = false;
    windowDesc_.resizable = true;
    // Window properties are set via WindowDesc structure
}

RenderingSystem::~RenderingSystem() {
    Shutdown();
}

bool RenderingSystem::Initialize(RenderAPI api) {
    if (initialized_) {
        std::cout << "RenderingSystem already initialized" << std::endl;
        return true;
    }
    
    // Set current mode to hardware by default
    
    // Check for environment variable overrides
    const char* forceAPI = std::getenv("LLGL_RENDER_API");
    const char* enableDebug = std::getenv("LLGL_DEBUG");
    const char* forceSoftware = std::getenv("LLGL_SOFTWARE_RENDERING");
    
    if (forceSoftware && std::string(forceSoftware) == "1") {
        softwareRenderingEnabled_ = true;
        std::cout << "Software rendering enabled via environment variable" << std::endl;
    }
    
    RenderAPI apiToTry = api;
    if (forceAPI) {
        std::string apiStr(forceAPI);
        if (apiStr == "OpenGL") apiToTry = RenderAPI::OpenGL;
        else if (apiStr == "Vulkan") apiToTry = RenderAPI::Vulkan;
        else if (apiStr == "Direct3D11") apiToTry = RenderAPI::Direct3D11;
        else if (apiStr == "Direct3D12") apiToTry = RenderAPI::Direct3D12;
        else if (apiStr == "Metal") apiToTry = RenderAPI::Metal;
        std::cout << "Forcing render API to: " << apiStr << std::endl;
    }
    
    // Try to initialize with the specified API
    if (apiToTry != RenderAPI::None) {
        std::string errorMessage;
        if (TryInitializeAPI(apiToTry, errorMessage)) {
            currentAPI_ = apiToTry;
            initialized_ = true;
            return true;
        }
    }
    
    // If specific API failed or Auto was requested, try fallback APIs
    return InitializeWithFallback(RenderAPI::None);
}

bool RenderingSystem::InitializeWithFallback(RenderAPI preferredAPI) {
    std::vector<RenderAPI> fallbackAPIs;
    
    // Platform-specific fallback order
#ifdef __APPLE__
    fallbackAPIs = { RenderAPI::Metal, RenderAPI::OpenGL };
#elif defined(_WIN32)
    fallbackAPIs = { RenderAPI::Direct3D11, RenderAPI::Direct3D12, RenderAPI::Vulkan, RenderAPI::OpenGL };
#else
    fallbackAPIs = { RenderAPI::Vulkan, RenderAPI::OpenGL };
#endif
    
    for (RenderAPI api : fallbackAPIs) {
        std::cout << "Trying to initialize " << GetAPIName(api) << "..." << std::endl;
        
        std::string errorMsg;
        if (TryInitializeAPI(api, errorMsg)) {
            currentAPI_ = api;
            initialized_ = true;
            std::cout << "Successfully initialized " << GetAPIName(api) << std::endl;
            return true;
        }
    }
    
    // All APIs failed, try software rendering if enabled
    if (softwareRenderingEnabled_) {
        return InitializeSoftwareRenderer();
    }
    
    // All initialization attempts failed
    std::cerr << "Failed to initialize any rendering API" << std::endl;
    std::cerr << "Possible solutions:" << std::endl;
    std::cerr << "1. Update your graphics drivers" << std::endl;
    std::cerr << "2. Check if your hardware supports the rendering APIs" << std::endl;
    std::cerr << "3. Set LLGL_SOFTWARE_RENDERING=1 to enable software rendering" << std::endl;
    
    return false;
}

bool RenderingSystem::TryInitializeAPI(RenderAPI api, std::string& errorMessage) {
    try {
        LLGL::RenderSystemDescriptor renderSystemDesc;
        renderSystemDesc.moduleName = GetModuleName(api);
        
        // Debug mode configuration
        // renderSystemDesc.debugger = true;
        // renderSystemDesc.flags = LLGL::RenderSystemFlags::DebugDevice;
        
        // Platform-specific validation
#ifdef __APPLE__
        if (api == RenderAPI::Metal) {
            // Check if Metal is available
            MTLDevice device = MTLCreateSystemDefaultDevice();
            if (!device) {
                std::cerr << "Metal device not available" << std::endl;
                return false;
            }
        }
#endif
        
        renderSystem_ = LLGL::RenderSystem::Load(renderSystemDesc);
        if (!renderSystem_) {
            std::cerr << "Failed to load " << GetAPIName(api) << " render system" << std::endl;
            return false;
        }
        
        // Get system graphics info
        const auto& info = renderSystem_->GetRendererInfo();
        systemInfo_.rendererName = info.rendererName;
        systemInfo_.deviceName = info.deviceName;
        systemInfo_.vendorName = info.vendorName;
        systemInfo_.shadingLanguageName = info.shadingLanguageName;
        
        std::cout << "Initialized " << GetAPIName(api) << " successfully" << std::endl;
        std::cout << "Renderer: " << systemInfo_.rendererName << std::endl;
        std::cout << "Device: " << systemInfo_.deviceName << std::endl;
        std::cout << "Vendor: " << systemInfo_.vendorName << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception while initializing " << GetAPIName(api) << ": " << e.what() << std::endl;
        return false;
    }
}

bool RenderingSystem::InitializeSoftwareRenderer() {
    std::cout << "Attempting to initialize software renderer..." << std::endl;
    
    // Software rendering is typically handled by OpenGL with software drivers
    std::string errorMsg;
    if (TryInitializeAPI(RenderAPI::OpenGL, errorMsg)) {
        softwareRenderingEnabled_ = true;
        std::cout << "Software rendering initialized successfully" << std::endl;
        return true;
    }
    
    std::cerr << "Failed to initialize software renderer" << std::endl;
    return false;
}

bool RenderingSystem::InitializeHeadlessRenderer() {
    currentMode_ = RenderingMode::Headless;
    
    // For headless rendering, we typically use OpenGL or Vulkan without a window
    std::vector<RenderAPI> headlessAPIs = { RenderAPI::OpenGL, RenderAPI::Vulkan };
    
    for (RenderAPI api : headlessAPIs) {
        std::string errorMessage;
        if (TryInitializeAPI(api, errorMessage)) {
            currentAPI_ = api;
            initialized_ = true;
            std::cout << "Headless renderer initialized with " << GetAPIName(api) << std::endl;
            return true;
        }
    }
    
    std::cerr << "Failed to initialize headless renderer" << std::endl;
    return false;
}

void RenderingSystem::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // Clean up in reverse order of creation
    if (commandBuffer_) {
        renderSystem_->Release(*commandBuffer_);
        commandBuffer_ = nullptr;
    }
    
    if (swapChain_) {
        renderSystem_->Release(*swapChain_);
        swapChain_ = nullptr;
    }
    
    // Surface and CommandBuffer are managed by RenderSystem, no explicit release needed
    surface_ = nullptr;
    
    if (renderSystem_) {
        LLGL::RenderSystem::Unload(std::move(renderSystem_));
        renderSystem_.reset();
    }
    
    initialized_ = false;
    currentAPI_ = RenderAPI::None;
    currentMode_ = RenderingMode::Hardware;
    softwareRenderingEnabled_ = false;
    
    std::cout << "RenderingSystem shut down successfully" << std::endl;
}

bool RenderingSystem::CreateWindow(const WindowDesc& desc) {
    if (!initialized_) {
        std::cerr << "RenderingSystem not initialized" << std::endl;
        return false;
    }
    
    if (currentMode_ == RenderingMode::Headless) {
        std::cout << "Skipping window creation for headless mode" << std::endl;
        return true;
    }
    
    windowDesc_ = desc;
    
    try {
        // Create surface
        // Create LLGL Window
        LLGL::WindowDescriptor windowDesc;
        windowDesc.title = windowDesc_.title;
        windowDesc.size = { static_cast<std::uint32_t>(windowDesc_.width),
                           static_cast<std::uint32_t>(windowDesc_.height) };
        windowDesc.flags = LLGL::WindowFlags::Visible;
        if (windowDesc_.resizable) {
            windowDesc.flags |= LLGL::WindowFlags::Resizable;
        }
        
        window_ = LLGL::Window::Create(windowDesc);
        surface_ = window_.get();
        if (!surface_) {
            std::cerr << "Failed to create surface" << std::endl;
            return false;
        }
        
        // Create swap chain
        LLGL::SwapChainDescriptor swapChainDesc;
        swapChainDesc.resolution = { static_cast<std::uint32_t>(windowDesc_.width),
                                    static_cast<std::uint32_t>(windowDesc_.height) };
        swapChainDesc.fullscreen = windowDesc_.fullscreen;
        swapChainDesc.resizable = windowDesc_.resizable;
        
        swapChain_.reset(renderSystem_->CreateSwapChain(swapChainDesc, std::shared_ptr<LLGL::Surface>(surface_, [](LLGL::Surface*){})));
        if (!swapChain_) {
            std::cerr << "Failed to create swap chain" << std::endl;
            return false;
        }
        
        // Create command buffer
        commandBuffer_ = renderSystem_->CreateCommandBuffer();
        if (!commandBuffer_) {
            std::cerr << "Failed to create command buffer" << std::endl;
            return false;
        }
        
        // Show window (LLGL windows are visible by default)
        // Window is already visible due to LLGL::WindowFlags::Visible
        
        std::cout << "Window created successfully: " << windowDesc_.width 
                  << "x" << windowDesc_.height << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception while creating window: " << e.what() << std::endl;
        return false;
    }
}

bool RenderingSystem::BeginFrame() {
    if (!commandBuffer_ || currentMode_ == RenderingMode::Headless) {
        return false;
    }
    
    commandBuffer_->Begin();
    
    if (swapChain_) {
        commandBuffer_->BeginRenderPass(*swapChain_);
        
        // Set viewport to window size
        LLGL::Viewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(windowDesc_.width);
        viewport.height = static_cast<float>(windowDesc_.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        commandBuffer_->SetViewport(viewport);
    }
    
    return true;
}

bool RenderingSystem::EndFrame() {
    if (!initialized_ || !swapChain_ || !commandBuffer_) {
        return false;
    }
    
    try {
        // End command recording
        commandBuffer_->End();
        
        // Submit commands through command queue
        auto* commandQueue = renderSystem_->GetCommandQueue();
        if (commandQueue) {
            commandQueue->Submit(*commandBuffer_);
        }
        
        // Present the frame
        swapChain_->Present();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error ending frame: " << e.what() << std::endl;
        return false;
    }
}

void RenderingSystem::Clear(const Color& color) {
    if (!initialized_ || !commandBuffer_) {
        return;
    }
    
    try {
        // Set clear color
        LLGL::ClearValue clearValue;
        clearValue.color[0] = color.r;
        clearValue.color[1] = color.g;
        clearValue.color[2] = color.b;
        clearValue.color[3] = color.a;
        clearValue.depth = 1.0f;
        clearValue.stencil = 0;
        
        // Clear attachments
        LLGL::AttachmentClear clearCmd;
        clearCmd.flags = LLGL::ClearFlags::Color;
        clearCmd.clearValue = clearValue;
        
        commandBuffer_->ClearAttachments(1, &clearCmd);
    } catch (const std::exception& e) {
        std::cerr << "Error clearing frame: " << e.what() << std::endl;
    }
}

void RenderingSystem::SetViewport(int x, int y, int width, int height) {
    if (!commandBuffer_) {
        return;
    }
    
    LLGL::Viewport viewport;
    viewport.x = static_cast<float>(x);
    viewport.y = static_cast<float>(y);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    commandBuffer_->SetViewport(viewport);
}

SystemGraphicsInfo RenderingSystem::DetectSystemGraphics() {
    SystemGraphicsInfo info;
    
    if (renderSystem_) {
        const auto& rendererInfo = renderSystem_->GetRendererInfo();
        info.rendererName = rendererInfo.rendererName;
        info.deviceName = rendererInfo.deviceName;
        info.vendorName = rendererInfo.vendorName;
        info.shadingLanguageName = rendererInfo.shadingLanguageName;
    }
    
#ifdef __APPLE__
    // Get additional macOS-specific graphics info
    std::string macInfo = GetMacOSGraphicsInfo();
    info.osVersion = macInfo;
    info.supportsMetal = true;
    
    // Set basic system information
    if (info.deviceName.empty()) {
        info.deviceName = "macOS Device";
    }
    if (info.vendorName.empty()) {
        info.vendorName = "Apple";
    }
#endif
    
    info.isHeadless = IsHeadlessEnvironment();
    
    return info;
}

bool RenderingSystem::IsHeadlessEnvironment() const {
    // Check for common headless environment indicators
    const char* display = std::getenv("DISPLAY");
    const char* sessionType = std::getenv("XDG_SESSION_TYPE");
    const char* term = std::getenv("TERM");
    
    // If no display is set (Linux/Unix)
    if (!display || std::string(display).empty()) {
        return true;
    }
    
    // Check for SSH session
    if (std::getenv("SSH_CLIENT") || std::getenv("SSH_TTY")) {
        return true;
    }
    
    // Check for CI/CD environments
    if (std::getenv("CI") || std::getenv("CONTINUOUS_INTEGRATION")) {
        return true;
    }
    
    return false;
}

#ifdef __APPLE__
std::string RenderingSystem::GetMacOSGraphicsInfo() const {
    return "macOS Graphics System";
}
#endif

std::string RenderingSystem::GetModuleName(RenderAPI api) const {
    switch (api) {
        case RenderAPI::OpenGL:     return "OpenGL";
        case RenderAPI::Vulkan:     return "Vulkan";
        case RenderAPI::Direct3D11: return "Direct3D11";
        case RenderAPI::Direct3D12: return "Direct3D12";
        case RenderAPI::Metal:      return "Metal";
        default:                    return "OpenGL"; // Default fallback
    }
}

std::string RenderingSystem::GetAPIName(RenderAPI api) const {
    switch (api) {
        // case RenderAPI::Auto:       return "Auto";
        case RenderAPI::OpenGL:     return "OpenGL";
        case RenderAPI::Vulkan:     return "Vulkan";
        case RenderAPI::Direct3D11: return "Direct3D 11";
        case RenderAPI::Direct3D12: return "Direct3D 12";
        case RenderAPI::Metal:      return "Metal";
        default:                    return "Unknown";
    }
}

// === Getters ===

bool RenderingSystem::IsInitialized() const {
    return initialized_;
}

RenderAPI RenderingSystem::GetCurrentAPI() const {
    return currentAPI_;
}

RenderingMode RenderingSystem::GetCurrentMode() const {
    return currentMode_;
}

bool RenderingSystem::GetWindowSize(int& width, int& height) const {
    if (!initialized_) {
        return false;
    }
    width = windowDesc_.width;
    height = windowDesc_.height;
    return true;
}





bool RenderingSystem::ShouldWindowClose() const {
    if (!surface_) {
        return false;
    }
    
    // LLGL surfaces don't have HasQuit method
    // Return false for now - window closing should be handled differently
    return false;
}

void RenderingSystem::PollEvents() {
    if (surface_) {
        surface_->ProcessEvents();
    }
}

LLGL::RenderSystem* RenderingSystem::GetRenderSystem() const {
    return renderSystem_.get();
}

LLGL::SwapChain* RenderingSystem::GetSwapChain() const {
    return swapChain_.get();
}

LLGL::CommandBuffer* RenderingSystem::GetCommandBuffer() const {
    return commandBuffer_;
}

LLGL::Surface* RenderingSystem::GetSurface() const {
    return surface_;
}

} // namespace RenderingPlugin
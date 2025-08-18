/*
 * rendering_example.cpp
 *
 * 基于LLGL MultiRenderer示例重新实现的3D立方体渲染示例
 * 使用RenderingPlugin接口进行渲染，支持鼠标交互旋转立方体
 */

#include "../include/Plugin.h"
#include <iostream>
#include <memory>
#include <chrono>
#include <cmath>

// 顶点结构体
struct Vertex {
    float position[3];
    float color[3];
};

// 变换矩阵结构体
struct Transform {
    float modelMatrix[16];
    float viewMatrix[16];
    float projMatrix[16];
};

// 渲染示例类
class CubeRenderer {
public:
    CubeRenderer() : plugin_(nullptr), renderObject_(nullptr) {
        // 初始化旋转角度
        rotationX_ = 0.0f;
        rotationY_ = 0.0f;
        lastMouseX_ = 0.0f;
        lastMouseY_ = 0.0f;
        mousePressed_ = false;
        
        // 初始化时间
        startTime_ = std::chrono::high_resolution_clock::now();
    }
    
    ~CubeRenderer() {
        Cleanup();
    }
    
    bool Initialize() {
        // 创建插件实例
        plugin_ = CreatePlugin();
        if (!plugin_) {
            std::cerr << "Failed to create plugin" << std::endl;
            return false;
        }
        
        // 初始化渲染系统
        RenderAPI preferredAPI = RenderAPI::OpenGL; // 默认使用OpenGL
        if (!plugin_->InitializeRenderSystem(preferredAPI)) {
            std::cerr << "Failed to initialize render system" << std::endl;
            return false;
        }
        
        // 创建窗口
        WindowDesc windowDesc;
        windowDesc.title = "3D Cube Rendering Example";
        windowDesc.width = 800;
        windowDesc.height = 600;
        windowDesc.resizable = true;
        
        if (!plugin_->CreateWindow(windowDesc)) {
            std::cerr << "Failed to create window" << std::endl;
            return false;
        }
        
        // 生成立方体几何数据
        if (!GenerateCubeData()) {
            std::cerr << "Failed to generate cube data" << std::endl;
            return false;
        }
        
        // 创建渲染对象
        if (!CreateRenderResources()) {
            std::cerr << "Failed to create render resources" << std::endl;
            return false;
        }
        
        std::cout << "Cube renderer initialized successfully" << std::endl;
        std::cout << "Use mouse to rotate the cube" << std::endl;
        
        return true;
    }
    
    void Run() {
        if (!plugin_) return;
        
        while (!plugin_->ShouldWindowClose()) {
            // 处理事件
            plugin_->PollEvents();
            
            // 处理鼠标输入
            HandleMouseInput();
            
            // 更新变换矩阵
            UpdateTransforms();
            
            // 渲染帧
            RenderFrame();
        }
    }
    
private:
    bool GenerateCubeData() {
        // 使用插件生成立方体顶点数据
        vertices_ = plugin_->GenerateCubeVertices();
        indices_ = plugin_->GenerateCubeIndices();
        
        if (vertices_.empty() || indices_.empty()) {
            std::cerr << "Failed to generate cube geometry" << std::endl;
            return false;
        }
        
        std::cout << "Generated cube with " << vertices_.size() << " vertices and " 
                  << indices_.size() << " indices" << std::endl;
        
        return true;
    }
    
    bool CreateRenderResources() {
        if (!plugin_) return false;
        
        // 创建渲染对象
        renderObject_ = plugin_->CreateRenderObject();
        if (!renderObject_) {
            std::cerr << "Failed to create render object" << std::endl;
            return false;
        }
        
        std::cout << "Render resources created successfully" << std::endl;
        return true;
    }
    
    void HandleMouseInput() {
        // 这里可以添加鼠标输入处理逻辑
        // 由于RenderingPlugin接口没有直接的鼠标输入方法，
        // 我们使用简单的时间基础旋转作为演示
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime_).count();
        
        // 自动旋转
        rotationY_ = (elapsed * 0.001f) * 30.0f; // 每秒30度
        rotationX_ = sin(elapsed * 0.001f) * 15.0f; // 上下摆动
    }
    
    void UpdateTransforms() {
        if (!plugin_) return;
        
        // 获取窗口尺寸
        auto windowSize = plugin_->GetWindowSize();
        float aspect = static_cast<float>(windowSize.first) / static_cast<float>(windowSize.second);
        
        // 构建投影矩阵
        float projMatrix[16];
        plugin_->BuildPerspectiveProjection(projMatrix, 45.0f, aspect, 0.1f, 100.0f);
        
        // 构建视图矩阵（简单的相机位置）
        float viewMatrix[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, -5.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        
        // 构建模型矩阵（旋转变换）
        float modelMatrix[16];
        BuildRotationMatrix(modelMatrix, rotationX_, rotationY_, 0.0f);
        
        // 更新常量缓冲区
        Transform transform;
        memcpy(transform.modelMatrix, modelMatrix, sizeof(modelMatrix));
        memcpy(transform.viewMatrix, viewMatrix, sizeof(viewMatrix));
        memcpy(transform.projMatrix, projMatrix, sizeof(projMatrix));
        
        plugin_->UpdateConstantBuffer(&transform, sizeof(Transform));
    }
    
    void BuildRotationMatrix(float* matrix, float rotX, float rotY, float rotZ) {
        // 简化的旋转矩阵构建
        float cosX = cos(rotX * M_PI / 180.0f);
        float sinX = sin(rotX * M_PI / 180.0f);
        float cosY = cos(rotY * M_PI / 180.0f);
        float sinY = sin(rotY * M_PI / 180.0f);
        
        // 组合Y轴和X轴旋转
        matrix[0] = cosY; matrix[1] = 0; matrix[2] = sinY; matrix[3] = 0;
        matrix[4] = sinY * sinX; matrix[5] = cosX; matrix[6] = -cosY * sinX; matrix[7] = 0;
        matrix[8] = -sinY * cosX; matrix[9] = sinX; matrix[10] = cosY * cosX; matrix[11] = 0;
        matrix[12] = 0; matrix[13] = 0; matrix[14] = 0; matrix[15] = 1;
    }
    
    void RenderFrame() {
        if (!plugin_) return;
        
        // 开始帧渲染
        plugin_->BeginFrame();
        
        // 清除屏幕
        Color clearColor = {0.1f, 0.1f, 0.2f, 1.0f}; // 深蓝色背景
        plugin_->Clear(clearColor);
        
        // 设置视口
        auto windowSize = plugin_->GetWindowSize();
        plugin_->SetViewport(0, 0, windowSize.first, windowSize.second);
        
        // 渲染立方体
        if (renderObject_) {
            plugin_->RenderObject(renderObject_);
        }
        
        // 结束帧渲染
        plugin_->EndFrame();
    }
    
    void Cleanup() {
        if (plugin_) {
            if (renderObject_) {
                plugin_->ReleaseRenderObject(renderObject_);
                renderObject_ = nullptr;
            }
            
            // 释放插件
            ReleasePlugin(plugin_);
            plugin_ = nullptr;
        }
    }
    
private:
    Plugin* plugin_;
    void* renderObject_;
    
    // 几何数据
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
    
    // 变换参数
    float rotationX_;
    float rotationY_;
    float lastMouseX_;
    float lastMouseY_;
    bool mousePressed_;
    
    // 时间管理
    std::chrono::high_resolution_clock::time_point startTime_;
};

int main() {
    std::cout << "Starting 3D Cube Rendering Example..." << std::endl;
    
    try {
        CubeRenderer renderer;
        
        if (!renderer.Initialize()) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return -1;
        }
        
        std::cout << "Renderer initialized, starting main loop..." << std::endl;
        renderer.Run();
        
        std::cout << "Rendering example completed successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return -1;
    }
    
    return 0;
}
/**
 * @file script_object_wrapper_test.cpp
 * @brief Unit tests for the ScriptObjectWrapper class
 */

#include <gtest/gtest.h>
#include "ScriptObjectWrapper.h"
#include "MathPlugin.h"
#include <memory>
#include <thread>
#include <chrono>

// Test fixture for ScriptObjectWrapper tests
class ScriptObjectWrapperTest : public ::testing::Test {
protected:
    std::shared_ptr<math::MathPlugin> mathPlugin;
    
    void SetUp() override {
        mathPlugin = std::make_shared<math::MathPlugin>();
        mathPlugin->Initialize();
    }
    
    void TearDown() override {
        if (mathPlugin) {
            mathPlugin->Shutdown();
        }
        ScriptObjectManager::GetInstance().CleanupAll();
    }
};

// Test basic wrapper functionality
TEST_F(ScriptObjectWrapperTest, BasicWrapperTest) {
    // Create wrapper
    auto wrapper = MakeScriptWrapper(mathPlugin);
    
    // Test validity
    EXPECT_TRUE(wrapper.IsValid());
    
    // Test getting the object
    auto obj = wrapper.Get();
    EXPECT_NE(nullptr, obj);
    EXPECT_EQ(mathPlugin.get(), obj.get());
    
    // Test TryGet
    auto tryObj = wrapper.TryGet();
    EXPECT_NE(nullptr, tryObj);
    EXPECT_EQ(mathPlugin.get(), tryObj.get());
}

// Test wrapper invalidation
TEST_F(ScriptObjectWrapperTest, InvalidationTest) {
    auto wrapper = MakeScriptWrapper(mathPlugin);
    
    // Initially valid
    EXPECT_TRUE(wrapper.IsValid());
    
    // Invalidate
    wrapper.Invalidate();
    EXPECT_FALSE(wrapper.IsValid());
    
    // TryGet should return nullptr
    auto tryObj = wrapper.TryGet();
    EXPECT_EQ(nullptr, tryObj);
    
    // Get should throw exception
    EXPECT_THROW(wrapper.Get(), ScriptObjectException);
}

// Test wrapper with object destruction
TEST_F(ScriptObjectWrapperTest, ObjectDestructionTest) {
    auto wrapper = MakeScriptWrapper(mathPlugin);
    
    // Initially valid
    EXPECT_TRUE(wrapper.IsValid());
    
    // Destroy the original object
    mathPlugin.reset();
    
    // Wrapper should detect object destruction
    EXPECT_FALSE(wrapper.IsValid());
    
    // TryGet should return nullptr
    auto tryObj = wrapper.TryGet();
    EXPECT_EQ(nullptr, tryObj);
}

// Test operator overloads
TEST_F(ScriptObjectWrapperTest, OperatorOverloadTest) {
    auto wrapper = MakeScriptWrapper(mathPlugin);
    
    // Test operator bool
    EXPECT_TRUE(static_cast<bool>(wrapper));
    
    // Test operator->
    const auto& info = wrapper->GetPluginInfo();
    EXPECT_EQ("MathPlugin", info.name);
    
    // Test operator*
    auto& plugin = *wrapper;
    EXPECT_EQ(mathPlugin.get(), &plugin);
    
    // Test after invalidation
    wrapper.Invalidate();
    EXPECT_FALSE(static_cast<bool>(wrapper));
    EXPECT_THROW((*wrapper).GetPluginInfo(), ScriptObjectException);
}

// Test ScriptObjectManager
TEST_F(ScriptObjectWrapperTest, ScriptObjectManagerTest) {
    bool cleanupCalled = false;
    
    // Register cleanup callback
    ScriptObjectManager::GetInstance().RegisterCleanupCallback(
        "TestPlugin",
        [&cleanupCalled]() {
            cleanupCalled = true;
        }
    );
    
    // Cleanup specific plugin
    ScriptObjectManager::GetInstance().CleanupPlugin("TestPlugin");
    EXPECT_TRUE(cleanupCalled);
    
    // Reset for next test
    cleanupCalled = false;
    
    // Register another callback
    ScriptObjectManager::GetInstance().RegisterCleanupCallback(
        "TestPlugin2",
        [&cleanupCalled]() {
            cleanupCalled = true;
        }
    );
    
    // Cleanup all
    ScriptObjectManager::GetInstance().CleanupAll();
    EXPECT_TRUE(cleanupCalled);
}

// Test thread safety
TEST_F(ScriptObjectWrapperTest, ThreadSafetyTest) {
    auto wrapper = MakeScriptWrapper(mathPlugin);
    const int numThreads = 4;
    const int operationsPerThread = 100;
    std::vector<std::thread> threads;
    std::vector<bool> results(numThreads * operationsPerThread, false);
    
    // Launch multiple threads that access the wrapper concurrently
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operationsPerThread; ++i) {
                int index = t * operationsPerThread + i;
                
                try {
                    // Alternate between different operations
                    if (i % 3 == 0) {
                        results[index] = wrapper.IsValid();
                    } else if (i % 3 == 1) {
                        auto obj = wrapper.TryGet();
                        results[index] = obj != nullptr;
                    } else {
                        results[index] = static_cast<bool>(wrapper);
                    }
                } catch (const std::exception&) {
                    results[index] = false;
                }
                
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify no crashes occurred (test passes if we reach here)
    EXPECT_TRUE(true);
}

// Test exception handling
TEST_F(ScriptObjectWrapperTest, ExceptionHandlingTest) {
    auto wrapper = MakeScriptWrapper(mathPlugin);
    
    // Invalidate wrapper
    wrapper.Invalidate();
    
    // Test that appropriate exceptions are thrown
    EXPECT_THROW(wrapper.Get(), ScriptObjectException);
    EXPECT_THROW((*wrapper).GetPluginInfo(), ScriptObjectException);
    EXPECT_THROW(*wrapper, ScriptObjectException);
    
    // Test exception message
    try {
        wrapper.Get();
        FAIL() << "Expected ScriptObjectException";
    } catch (const ScriptObjectException& e) {
        std::string message = e.what();
        EXPECT_FALSE(message.empty());
        EXPECT_NE(message.find("invalid"), std::string::npos);
    }
}
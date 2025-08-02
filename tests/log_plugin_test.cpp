/**
 * @file log_plugin_test.cpp
 * @brief Unit tests for the LogPlugin class
 */

#include <gtest/gtest.h>
#include "PluginManager.h"
#include "LogPlugin.h"
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

// Test fixture for LogPlugin tests
class LogPluginTest : public ::testing::Test {
protected:
    PluginManager pluginManager;
    LogPlugin* logPlugin;
    std::string testLogFile;
    
    void SetUp() override {
        // Set the plugin directory and load the LogPlugin
        pluginManager.SetPluginDirectory("plugins");
        pluginManager.LoadPlugin("plugins/LogPlugin" + std::string(PLUGIN_EXTENSION));
        
        // Get the LogPlugin instance
        logPlugin = pluginManager.GetPlugin<LogPlugin>("LogPlugin");
        ASSERT_NE(nullptr, logPlugin);
        
        // Create a test log file path
        testLogFile = "test_log.txt";
        
        // Remove any existing test log file
        if (std::filesystem::exists(testLogFile)) {
            std::filesystem::remove(testLogFile);
        }
    }
    
    void TearDown() override {
        // Unload all plugins to clean up
        pluginManager.UnloadAllPlugins();
        logPlugin = nullptr;
        
        // Clean up test log file
        if (std::filesystem::exists(testLogFile)) {
            std::filesystem::remove(testLogFile);
        }
    }
    
    // Helper function to check if a string exists in a file
    bool FileContainsString(const std::string& filePath, const std::string& searchString) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.find(searchString) != std::string::npos) {
                file.close();
                return true;
            }
        }
        
        file.close();
        return false;
    }
};

// Test the plugin info
TEST_F(LogPluginTest, PluginInfoTest) {
    const PluginInfo& info = logPlugin->GetPluginInfo();
    
    EXPECT_EQ("LogPlugin", info.name);
    EXPECT_EQ("Logging Plugin", info.displayName);
    EXPECT_EQ("Provides logging capabilities", info.description);
    EXPECT_EQ("Plugin System Team", info.author);
    
    // Check version
    EXPECT_EQ(1, info.version.major);
    EXPECT_EQ(0, info.version.minor);
    EXPECT_EQ(0, info.version.patch);
    EXPECT_EQ("1.0.0", info.version.ToString());
}

// Test log level setting and getting
TEST_F(LogPluginTest, LogLevelTest) {
    // Default level should be Info
    EXPECT_EQ(LogLevel::Info, logPlugin->GetLevel());
    
    // Set level to Debug and verify
    logPlugin->SetLevel(LogLevel::Debug);
    EXPECT_EQ(LogLevel::Debug, logPlugin->GetLevel());
    
    // Set level to Error and verify
    logPlugin->SetLevel(LogLevel::Error);
    EXPECT_EQ(LogLevel::Error, logPlugin->GetLevel());
}

// Test file logging
TEST_F(LogPluginTest, FileLoggingTest) {
    // Add a file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Set level to Trace to capture all messages
    logPlugin->SetLevel(LogLevel::Trace);
    
    // Log messages at different levels
    logPlugin->Trace("This is a trace message");
    logPlugin->Debug("This is a debug message");
    logPlugin->Info("This is an info message");
    logPlugin->Warning("This is a warning message");
    logPlugin->Error("This is an error message");
    logPlugin->Critical("This is a critical message");
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Give the file system a moment to write the file
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify messages were logged to the file
    EXPECT_TRUE(FileContainsString(testLogFile, "This is a trace message"));
    EXPECT_TRUE(FileContainsString(testLogFile, "This is a debug message"));
    EXPECT_TRUE(FileContainsString(testLogFile, "This is an info message"));
    EXPECT_TRUE(FileContainsString(testLogFile, "This is a warning message"));
    EXPECT_TRUE(FileContainsString(testLogFile, "This is an error message"));
    EXPECT_TRUE(FileContainsString(testLogFile, "This is a critical message"));
}

// Test log filtering by level
TEST_F(LogPluginTest, LogFilteringTest) {
    // Add a file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Set level to Warning to filter out less severe messages
    logPlugin->SetLevel(LogLevel::Warning);
    
    // Log messages at different levels
    logPlugin->Trace("This is a trace message");
    logPlugin->Debug("This is a debug message");
    logPlugin->Info("This is an info message");
    logPlugin->Warning("This is a warning message");
    logPlugin->Error("This is an error message");
    logPlugin->Critical("This is a critical message");
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Give the file system a moment to write the file
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify only messages at Warning level or higher were logged
    EXPECT_FALSE(FileContainsString(testLogFile, "This is a trace message"));
    EXPECT_FALSE(FileContainsString(testLogFile, "This is a debug message"));
    EXPECT_FALSE(FileContainsString(testLogFile, "This is an info message"));
    EXPECT_TRUE(FileContainsString(testLogFile, "This is a warning message"));
    EXPECT_TRUE(FileContainsString(testLogFile, "This is an error message"));
    EXPECT_TRUE(FileContainsString(testLogFile, "This is a critical message"));
}

// Test log pattern setting
TEST_F(LogPluginTest, LogPatternTest) {
    // Add a file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Set a custom pattern
    logPlugin->SetPattern("%^[%l]%$ %v");
    
    // Log a message
    logPlugin->Info("Custom pattern test");
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Give the file system a moment to write the file
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify the message was logged with the custom pattern
    EXPECT_TRUE(FileContainsString(testLogFile, "[info] Custom pattern test"));
}

// Test clearing sinks
TEST_F(LogPluginTest, ClearSinksTest) {
    // Add a file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Log a message
    logPlugin->Info("First message");
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Give the file system a moment to write the file
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify the message was logged
    EXPECT_TRUE(FileContainsString(testLogFile, "First message"));
    
    // Clear all sinks
    logPlugin->ClearSinks();
    
    // Remove the existing log file and create a new one
    if (std::filesystem::exists(testLogFile)) {
        std::filesystem::remove(testLogFile);
    }
    
    // Log another message
    logPlugin->Info("Second message");
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Give the file system a moment
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify the file doesn't exist or is empty since we cleared the sinks
    EXPECT_FALSE(std::filesystem::exists(testLogFile) && std::filesystem::file_size(testLogFile) > 0);
}
/**
 * @file LogPluginTests.cpp
 * @brief Unit tests for the LogPlugin
 */

#include <gtest/gtest.h>
#include "PluginManager.h"
#include "LogPlugin.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <regex>

class LogPluginTest : public ::testing::Test {
protected:
    PluginManager pluginManager;
    LogPlugin* logPlugin;
    std::string testLogFile;

    void SetUp() override {
        // Set plugin directory
        pluginManager.SetPluginDirectory("./plugins");
        
        // Load LogPlugin
        ASSERT_TRUE(pluginManager.LoadPlugin("LogPlugin"));
        
        // Get LogPlugin instance
        IPlugin* plugin = pluginManager.GetPlugin("LogPlugin");
        ASSERT_NE(plugin, nullptr);
        
        // Cast to LogPlugin
        logPlugin = dynamic_cast<LogPlugin*>(plugin);
        ASSERT_NE(logPlugin, nullptr);
        
        // Set up test log file
        testLogFile = "test_log.txt";
        
        // Clear any existing log file
        if (std::filesystem::exists(testLogFile)) {
            std::filesystem::remove(testLogFile);
        }
    }

    void TearDown() override {
        // Unload all plugins
        pluginManager.UnloadAllPlugins();
        logPlugin = nullptr;
        
        // Clean up test log file
        if (std::filesystem::exists(testLogFile)) {
            std::filesystem::remove(testLogFile);
        }
    }
    
    // Helper function to check if a file contains a specific string
    bool FileContainsString(const std::string& filePath, const std::string& searchString) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        
        return content.find(searchString) != std::string::npos;
    }
    
    // Helper function to check if a file contains a pattern using regex
    bool FileContainsPattern(const std::string& filePath, const std::string& pattern) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        
        std::regex regex(pattern);
        return std::regex_search(content, regex);
    }
};

// Test log level setting and getting
TEST_F(LogPluginTest, LogLevelSettings) {
    // Default level should be Info
    EXPECT_EQ(logPlugin->GetLevel(), LogLevel::Info);
    
    // Set level to Debug
    logPlugin->SetLevel(LogLevel::Debug);
    EXPECT_EQ(logPlugin->GetLevel(), LogLevel::Debug);
    
    // Set level to Error
    logPlugin->SetLevel(LogLevel::Error);
    EXPECT_EQ(logPlugin->GetLevel(), LogLevel::Error);
    
    // Set level to Trace
    logPlugin->SetLevel(LogLevel::Trace);
    EXPECT_EQ(logPlugin->GetLevel(), LogLevel::Trace);
}

// Test file logging
TEST_F(LogPluginTest, FileLogging) {
    // Add file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Set level to Trace to capture all messages
    logPlugin->SetLevel(LogLevel::Trace);
    
    // Log messages at different levels
    std::string traceMsg = "This is a trace message";
    std::string debugMsg = "This is a debug message";
    std::string infoMsg = "This is an info message";
    std::string warningMsg = "This is a warning message";
    std::string errorMsg = "This is an error message";
    std::string criticalMsg = "This is a critical message";
    
    logPlugin->Trace(traceMsg);
    logPlugin->Debug(debugMsg);
    logPlugin->Info(infoMsg);
    logPlugin->Warning(warningMsg);
    logPlugin->Error(errorMsg);
    logPlugin->Critical(criticalMsg);
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Check if all messages are in the log file
    EXPECT_TRUE(FileContainsString(testLogFile, traceMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, debugMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, infoMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, warningMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, errorMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, criticalMsg));
}

// Test log filtering by level
TEST_F(LogPluginTest, LogFiltering) {
    // Add file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Set level to Warning (should filter out Trace, Debug, Info)
    logPlugin->SetLevel(LogLevel::Warning);
    
    // Log messages at different levels
    std::string traceMsg = "This is a trace message for filtering";
    std::string debugMsg = "This is a debug message for filtering";
    std::string infoMsg = "This is an info message for filtering";
    std::string warningMsg = "This is a warning message for filtering";
    std::string errorMsg = "This is an error message for filtering";
    std::string criticalMsg = "This is a critical message for filtering";
    
    logPlugin->Trace(traceMsg);
    logPlugin->Debug(debugMsg);
    logPlugin->Info(infoMsg);
    logPlugin->Warning(warningMsg);
    logPlugin->Error(errorMsg);
    logPlugin->Critical(criticalMsg);
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Check filtering: Trace, Debug, Info should not be in the log file
    EXPECT_FALSE(FileContainsString(testLogFile, traceMsg));
    EXPECT_FALSE(FileContainsString(testLogFile, debugMsg));
    EXPECT_FALSE(FileContainsString(testLogFile, infoMsg));
    
    // Warning, Error, Critical should be in the log file
    EXPECT_TRUE(FileContainsString(testLogFile, warningMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, errorMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, criticalMsg));
}

// Test log pattern setting
TEST_F(LogPluginTest, LogPatternSetting) {
    // Add file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Set custom pattern
    std::string customPattern = "%^[%l]%$ %v";
    logPlugin->SetPattern(customPattern);
    
    // Log a message
    std::string testMessage = "Pattern test message";
    logPlugin->Info(testMessage);
    
    // Flush to ensure message is written
    logPlugin->Flush();
    
    // Check if the message follows the pattern
    std::string expectedPattern = "\\[INFO\\] Pattern test message";
    EXPECT_TRUE(FileContainsPattern(testLogFile, expectedPattern));
}

// Test rotating file sink
TEST_F(LogPluginTest, RotatingFileSink) {
    // Add rotating file sink (small max size to force rotation)
    std::string rotatingFile = "rotating_test.txt";
    logPlugin->AddRotatingFileSink(rotatingFile, 100, 2);
    
    // Set level to Info
    logPlugin->SetLevel(LogLevel::Info);
    
    // Log a large message multiple times to trigger rotation
    std::string largeMessage = "This is a large message that will be repeated to trigger file rotation. ";
    for (int i = 0; i < 10; ++i) {
        logPlugin->Info(largeMessage + std::to_string(i));
    }
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Check if the rotating file exists
    EXPECT_TRUE(std::filesystem::exists(rotatingFile));
    
    // Clean up rotating files
    if (std::filesystem::exists(rotatingFile)) {
        std::filesystem::remove(rotatingFile);
    }
    
    // Check for rotated files (up to max_files)
    for (int i = 1; i <= 2; ++i) {
        std::string rotatedFile = rotatingFile + "." + std::to_string(i);
        if (std::filesystem::exists(rotatedFile)) {
            std::filesystem::remove(rotatedFile);
        }
    }
}

// Test clearing sinks
TEST_F(LogPluginTest, ClearSinks) {
    // Add file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Log a message
    std::string beforeClearMsg = "Message before clearing sinks";
    logPlugin->Info(beforeClearMsg);
    
    // Flush to ensure message is written
    logPlugin->Flush();
    
    // Clear all sinks
    logPlugin->ClearSinks();
    
    // Add a new file sink
    std::string newLogFile = "new_test_log.txt";
    logPlugin->AddFileSink(newLogFile);
    
    // Log another message
    std::string afterClearMsg = "Message after clearing sinks";
    logPlugin->Info(afterClearMsg);
    
    // Flush to ensure message is written
    logPlugin->Flush();
    
    // Check if the first message is in the original log file
    EXPECT_TRUE(FileContainsString(testLogFile, beforeClearMsg));
    
    // Check if the second message is only in the new log file
    EXPECT_TRUE(FileContainsString(newLogFile, afterClearMsg));
    EXPECT_FALSE(FileContainsString(testLogFile, afterClearMsg));
    
    // Clean up new log file
    if (std::filesystem::exists(newLogFile)) {
        std::filesystem::remove(newLogFile);
    }
}

// Test generic Log method
TEST_F(LogPluginTest, GenericLogMethod) {
    // Add file sink
    logPlugin->AddFileSink(testLogFile);
    
    // Set level to Trace to capture all messages
    logPlugin->SetLevel(LogLevel::Trace);
    
    // Log messages using the generic Log method
    std::string traceMsg = "Generic trace message";
    std::string debugMsg = "Generic debug message";
    std::string infoMsg = "Generic info message";
    std::string warningMsg = "Generic warning message";
    std::string errorMsg = "Generic error message";
    std::string criticalMsg = "Generic critical message";
    
    logPlugin->Log(LogLevel::Trace, traceMsg);
    logPlugin->Log(LogLevel::Debug, debugMsg);
    logPlugin->Log(LogLevel::Info, infoMsg);
    logPlugin->Log(LogLevel::Warning, warningMsg);
    logPlugin->Log(LogLevel::Error, errorMsg);
    logPlugin->Log(LogLevel::Critical, criticalMsg);
    
    // Flush to ensure all messages are written
    logPlugin->Flush();
    
    // Check if all messages are in the log file
    EXPECT_TRUE(FileContainsString(testLogFile, traceMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, debugMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, infoMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, warningMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, errorMsg));
    EXPECT_TRUE(FileContainsString(testLogFile, criticalMsg));
}
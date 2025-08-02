/**
 * @file LogPlugin.cpp
 * @brief Implementation of the LogPlugin class
 */

#include "LogPlugin.h"
#include "PluginExport.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

// Initialize static members
LogPlugin* LogPlugin::instance_ = nullptr;

// Define plugin info
PluginInfo LogPlugin::pluginInfo_{
    "LogPlugin",                // name
    "Logging Plugin",           // displayName
    "Provides logging functionality", // description
    PluginInfo::Version(1, 0, 0),  // version
    "Plugin System Team"         // author
    // Dependencies can be added with AddDependency() method
};

LogPlugin::LogPlugin()
    : logger_(nullptr), currentLevel_(LogLevel::Info) {
    // Set the singleton instance
    if (instance_ == nullptr) {
        instance_ = this;
    }
}

LogPlugin::~LogPlugin() {
    Shutdown();
    
    // Clear the singleton instance if it's this instance
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

bool LogPlugin::Initialize() {
    try {
        // Create a logger with no sinks initially
        logger_ = std::make_shared<spdlog::logger>("plugin_logger");
        
        // Add a console sink by default
        AddConsoleSink();
        
        // Set the default log level
        SetLevel(LogLevel::Info);
        
        // Set the default pattern
        SetPattern("%Y-%m-%d %H:%M:%S.%e [%l] [%n] %v");
        
        // Set as default logger
        spdlog::set_default_logger(logger_);
        
        std::cout << "LogPlugin initialized successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize LogPlugin: " << e.what() << std::endl;
        return false;
    }
}

void LogPlugin::Shutdown() {
    if (logger_) {
        // Flush any pending log messages
        Flush();
        
        // Clear all sinks
        ClearSinks();
        
        // Reset the default logger
        spdlog::set_default_logger(nullptr);
        
        logger_.reset();
        std::cout << "LogPlugin shut down" << std::endl;
    }
}

const PluginInfo& LogPlugin::GetPluginInfo() const {
    return pluginInfo_;
}

std::string LogPlugin::Serialize() {
    // For now, we just serialize the current log level
    return std::to_string(static_cast<int>(currentLevel_));
}

bool LogPlugin::Deserialize(const std::string& data) {
    try {
        int level = std::stoi(data);
        currentLevel_ = static_cast<LogLevel>(level);
        if (logger_) {
            SetLevel(currentLevel_);
        }
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool LogPlugin::PrepareForHotReload() {
    // Flush any pending log messages before hot reload
    Flush();
    return true;
}

bool LogPlugin::CompleteHotReload() {
    // Re-apply the log level after hot reload
    if (logger_) {
        SetLevel(currentLevel_);
    }
    return true;
}

LogPlugin* LogPlugin::GetInstance() {
    return instance_;
}

void LogPlugin::Log(LogLevel level, const std::string& message) {
    if (!logger_) return;
    
    switch (level) {
        case LogLevel::Trace:
            logger_->trace(message);
            break;
        case LogLevel::Debug:
            logger_->debug(message);
            break;
        case LogLevel::Info:
            logger_->info(message);
            break;
        case LogLevel::Warning:
            logger_->warn(message);
            break;
        case LogLevel::Error:
            logger_->error(message);
            break;
        case LogLevel::Critical:
            logger_->critical(message);
            break;
    }
}

void LogPlugin::Trace(const std::string& message) {
    Log(LogLevel::Trace, message);
}

void LogPlugin::Debug(const std::string& message) {
    Log(LogLevel::Debug, message);
}

void LogPlugin::Info(const std::string& message) {
    Log(LogLevel::Info, message);
}

void LogPlugin::Warning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void LogPlugin::Error(const std::string& message) {
    Log(LogLevel::Error, message);
}

void LogPlugin::Critical(const std::string& message) {
    Log(LogLevel::Critical, message);
}

void LogPlugin::SetLevel(LogLevel level) {
    if (!logger_) return;
    
    currentLevel_ = level;
    logger_->set_level(ToSpdlogLevel(level));
}

LogLevel LogPlugin::GetLevel() const {
    return currentLevel_;
}

void LogPlugin::SetPattern(const std::string& pattern) {
    if (!logger_) return;
    
    logger_->set_pattern(pattern);
}

bool LogPlugin::AddFileSink(const std::string& filePath, bool truncate) {
    try {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, truncate);
        sinks_.push_back(sink);
        logger_->sinks().push_back(sink);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to add file sink: " << e.what() << std::endl;
        return false;
    }
}

bool LogPlugin::AddRotatingFileSink(const std::string& filePath, size_t maxFileSize, size_t maxFiles) {
    try {
        auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath, maxFileSize, maxFiles);
        sinks_.push_back(sink);
        logger_->sinks().push_back(sink);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to add rotating file sink: " << e.what() << std::endl;
        return false;
    }
}

bool LogPlugin::AddDailyFileSink(const std::string& filePath, int hour, int minute) {
    try {
        auto sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filePath, hour, minute);
        sinks_.push_back(sink);
        logger_->sinks().push_back(sink);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to add daily file sink: " << e.what() << std::endl;
        return false;
    }
}

bool LogPlugin::AddConsoleSink(bool colorize) {
    try {
        std::shared_ptr<spdlog::sinks::sink> sink;
        if (colorize) {
            sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        } else {
            sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            // Use color sink but with no colors (spdlog doesn't have a non-color stdout sink in newer versions)
            std::static_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(sink)->set_color_mode(spdlog::color_mode::never);
        }
        
        sinks_.push_back(sink);
        logger_->sinks().push_back(sink);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to add console sink: " << e.what() << std::endl;
        return false;
    }
}

void LogPlugin::ClearSinks() {
    if (!logger_) return;
    
    logger_->sinks().clear();
    sinks_.clear();
}

void LogPlugin::Flush() {
    if (logger_) {
        logger_->flush();
    }
}

spdlog::level_enum LogPlugin::ToSpdlogLevel(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace:    return spdlog::level::trace;
        case LogLevel::Debug:    return spdlog::level::debug;
        case LogLevel::Info:     return spdlog::level::info;
        case LogLevel::Warning:  return spdlog::level::warn;
        case LogLevel::Error:    return spdlog::level::err;
        case LogLevel::Critical: return spdlog::level::critical;
        default:                 return spdlog::level::info;
    }
}

LogLevel LogPlugin::FromSpdlogLevel(spdlog::level_enum spdlogLevel) const {
    // Use if-else instead of switch for enum class
    if (spdlogLevel == spdlog::level::trace)        return LogLevel::Trace;
    else if (spdlogLevel == spdlog::level::debug)   return LogLevel::Debug;
    else if (spdlogLevel == spdlog::level::info)    return LogLevel::Info;
    else if (spdlogLevel == spdlog::level::warn)    return LogLevel::Warning;
    else if (spdlogLevel == spdlog::level::err)     return LogLevel::Error;
    else if (spdlogLevel == spdlog::level::critical) return LogLevel::Critical;
    else                                            return LogLevel::Info;
}

// Register the plugin
REGISTER_PLUGIN(LogPlugin)
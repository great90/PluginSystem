/**
 * @file LogPlugin.h
 * @brief Defines the LogPlugin class for logging functionality
 */

#pragma once

#include "IPlugin.h"
#include "LogPluginExport.h"
#include <string>
#include <memory>
#include <vector>

// Forward declarations
namespace spdlog {
    class logger;
    
    // Forward declare the level namespace and level enum
    namespace level {
        enum level_enum : int;
    }
    
    // Alias for backward compatibility
    using level_enum = level::level_enum;
    
    namespace sinks {
        class sink;
    }
}

/**
 * @enum LogLevel
 * @brief Defines the severity levels for logging
 */
enum class LogLevel {
    Trace,      ///< Trace level for very detailed information
    Debug,      ///< Debug level for debugging information
    Info,       ///< Info level for general information
    Warning,    ///< Warning level for potential issues
    Error,      ///< Error level for errors that don't stop the application
    Critical    ///< Critical level for critical errors that might stop the application
};

/**
 * @class LogPlugin
 * @brief Plugin providing logging functionality
 * 
 * This plugin provides logging capabilities using the spdlog library.
 * It supports multiple log levels, custom sinks, and pattern-based formatting.
 */
class LOG_PLUGIN_API LogPlugin : public IPlugin {
public:
    /**
     * @brief Constructor
     */
    LogPlugin();
    
    /**
     * @brief Destructor
     */
    virtual ~LogPlugin();
    
    // IPlugin interface implementation
    bool Initialize() override;
    void Shutdown() override;
    const PluginInfo& GetPluginInfo() const override;
    std::string Serialize() override;
    bool Deserialize(const std::string& data) override;
    bool PrepareForHotReload() override;
    bool CompleteHotReload() override;
    
    /**
     * @brief Get the singleton instance of the LogPlugin
     * 
     * @return Pointer to the LogPlugin instance
     */
    static LogPlugin* GetInstance();
    
    /**
     * @brief Log a message with the specified level
     * 
     * @param level The severity level of the message
     * @param message The message to log
     */
    void Log(LogLevel level, const std::string& message);
    
    /**
     * @brief Log a trace message
     * 
     * @param message The message to log
     */
    void Trace(const std::string& message);
    
    /**
     * @brief Log a debug message
     * 
     * @param message The message to log
     */
    void Debug(const std::string& message);
    
    /**
     * @brief Log an info message
     * 
     * @param message The message to log
     */
    void Info(const std::string& message);
    
    /**
     * @brief Log a warning message
     * 
     * @param message The message to log
     */
    void Warning(const std::string& message);
    
    /**
     * @brief Log an error message
     * 
     * @param message The message to log
     */
    void Error(const std::string& message);
    
    /**
     * @brief Log a critical message
     * 
     * @param message The message to log
     */
    void Critical(const std::string& message);
    
    /**
     * @brief Set the minimum log level
     * 
     * Messages with a level lower than this will not be logged.
     * 
     * @param level The minimum log level
     */
    void SetLevel(LogLevel level);
    
    /**
     * @brief Get the current minimum log level
     * 
     * @return The current minimum log level
     */
    LogLevel GetLevel() const;
    
    /**
     * @brief Set the log pattern
     * 
     * @param pattern The pattern string (see spdlog documentation for format)
     */
    void SetPattern(const std::string& pattern);
    
    /**
     * @brief Add a file sink to the logger
     * 
     * @param filePath Path to the log file
     * @param truncate Whether to truncate the file if it exists
     * @return true if the sink was added successfully, false otherwise
     */
    bool AddFileSink(const std::string& filePath, bool truncate = false);
    
    /**
     * @brief Add a rotating file sink to the logger
     * 
     * @param filePath Path to the log file
     * @param maxFileSize Maximum size of each file in bytes
     * @param maxFiles Maximum number of files to keep
     * @return true if the sink was added successfully, false otherwise
     */
    bool AddRotatingFileSink(const std::string& filePath, size_t maxFileSize, size_t maxFiles);
    
    /**
     * @brief Add a daily file sink to the logger
     * 
     * @param filePath Path to the log file
     * @param hour Hour of the day to rotate the file (0-23)
     * @param minute Minute of the hour to rotate the file (0-59)
     * @return true if the sink was added successfully, false otherwise
     */
    bool AddDailyFileSink(const std::string& filePath, int hour = 0, int minute = 0);
    
    /**
     * @brief Add a console sink to the logger
     * 
     * @param colorize Whether to colorize the output
     * @return true if the sink was added successfully, false otherwise
     */
    bool AddConsoleSink(bool colorize = true);
    
    /**
     * @brief Remove all sinks from the logger
     */
    void ClearSinks();
    
    /**
     * @brief Flush the logger
     * 
     * Forces the logger to flush its buffers, ensuring all messages are written.
     */
    void Flush();

private:
    /**
     * @brief Convert LogLevel to spdlog level
     * 
     * @param level The LogLevel to convert
     * @return The corresponding spdlog level
     */
    spdlog::level_enum ToSpdlogLevel(LogLevel level) const;
    
    /**
     * @brief Convert spdlog level to LogLevel
     * 
     * @param spdlogLevel The spdlog level to convert
     * @return The corresponding LogLevel
     */
    LogLevel FromSpdlogLevel(spdlog::level_enum spdlogLevel) const;
    
    std::shared_ptr<spdlog::logger> logger_;  ///< The spdlog logger
    std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks_;  ///< The registered sinks
    LogLevel currentLevel_;  ///< The current minimum log level
    
    static LogPlugin* instance_;  ///< Singleton instance
    static PluginInfo pluginInfo_;  ///< Static plugin information
    
    /**
     * @brief Get static plugin information
     * 
     * @return Static plugin information
     */
    public: static const PluginInfo& GetPluginStaticInfo() {
        return pluginInfo_;
    }
};
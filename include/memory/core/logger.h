#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <mutex>

namespace memory::core {

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

class Logger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level);
    void setOutput(const std::string& filename);

    void log(LogLevel level, const std::string& message);
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

private:
    Logger() = default;
    std::string levelToString(LogLevel level) const;
    std::string getCurrentTime() const;

    LogLevel current_level_ = LogLevel::INFO;
    std::unique_ptr<std::ofstream> file_stream_;
    std::mutex mutex_;
};

// 便利宏
#define LOG_TRACE(msg) memory::core::Logger::getInstance().trace(msg)
#define LOG_DEBUG(msg) memory::core::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) memory::core::Logger::getInstance().info(msg)
#define LOG_WARN(msg) memory::core::Logger::getInstance().warn(msg)
#define LOG_ERROR(msg) memory::core::Logger::getInstance().error(msg)
#define LOG_FATAL(msg) memory::core::Logger::getInstance().fatal(msg)

} // namespace memory::core
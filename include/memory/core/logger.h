#pragma once

#include <string>
#include <string_view>
#include <fstream>
#include <memory>
#include <mutex>
#include <format>

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
    void setOutput(std::string_view filename);
    void closeFile();
    void flush();

    void log(LogLevel level, std::string_view message);
    void trace(std::string_view message);
    void debug(std::string_view message);
    void info(std::string_view message);
    void warn(std::string_view message);
    void error(std::string_view message);
    void fatal(std::string_view message);

    template<typename... Args>
    void log(LogLevel level, std::string_view format_str, Args&&... args);
    template<typename... Args>
    void trace(std::string_view format_str, Args&&... args);
    template<typename... Args>
    void debug(std::string_view format_str, Args&&... args);
    template<typename... Args>
    void info(std::string_view format_str, Args&&... args);
    template<typename... Args>
    void warn(std::string_view format_str, Args&&... args);
    template<typename... Args>
    void error(std::string_view format_str, Args&&... args);
    template<typename... Args>
    void fatal(std::string_view format_str, Args&&... args);

private:
    Logger() = default;
    std::string levelToString(LogLevel level) const;
    std::string getCurrentTime() const;

    LogLevel current_level_ = LogLevel::INFO;
    std::unique_ptr<std::ofstream> file_stream_;
    std::mutex mutex_;
};

// 便利宏
#define LOG_TRACE(msg, ...) memory::core::Logger::getInstance().trace(msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) memory::core::Logger::getInstance().debug(msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) memory::core::Logger::getInstance().info(msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...) memory::core::Logger::getInstance().warn(msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) memory::core::Logger::getInstance().error(msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) memory::core::Logger::getInstance().fatal(msg, ##__VA_ARGS__)

// Template implementations
template<typename... Args>
void Logger::log(LogLevel level, std::string_view format_str, Args&&... args) {
    log(level, std::format(format_str, std::forward<Args>(args)...));
}

template<typename... Args>
void Logger::trace(std::string_view format_str, Args&&... args) {
    log(LogLevel::TRACE, format_str, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::debug(std::string_view format_str, Args&&... args) {
    log(LogLevel::DEBUG, format_str, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::info(std::string_view format_str, Args&&... args) {
    log(LogLevel::INFO, format_str, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::warn(std::string_view format_str, Args&&... args) {
    log(LogLevel::WARN, format_str, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::error(std::string_view format_str, Args&&... args) {
    log(LogLevel::ERROR, format_str, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::fatal(std::string_view format_str, Args&&... args) {
    log(LogLevel::FATAL, format_str, std::forward<Args>(args)...);
}

} // namespace memory::core
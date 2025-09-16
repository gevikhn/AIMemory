#include "memory/core/logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace memory::core {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

void Logger::setOutput(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_stream_ = std::make_unique<std::ofstream>(filename, std::ios::app);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    std::string log_line = getCurrentTime() + " [" + levelToString(level) + "] " + message;

    if (file_stream_ && file_stream_->is_open()) {
        *file_stream_ << log_line << std::endl;
        file_stream_->flush();
    } else {
        std::cout << log_line << std::endl;
    }
}

void Logger::trace(const std::string& message) { log(LogLevel::TRACE, message); }
void Logger::debug(const std::string& message) { log(LogLevel::DEBUG, message); }
void Logger::info(const std::string& message) { log(LogLevel::INFO, message); }
void Logger::warn(const std::string& message) { log(LogLevel::WARN, message); }
void Logger::error(const std::string& message) { log(LogLevel::ERROR, message); }
void Logger::fatal(const std::string& message) { log(LogLevel::FATAL, message); }

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace memory::core
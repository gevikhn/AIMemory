#include "memory/core/logger.h"
#include <iostream>
#include <chrono>
#include <format>

namespace memory::core {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

void Logger::setOutput(std::string_view filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_stream_ = std::make_unique<std::ofstream>(std::string(filename), std::ios::app);
}

void Logger::closeFile() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_ && file_stream_->is_open()) {
        file_stream_->close();
        file_stream_.reset();
    }
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_ && file_stream_->is_open()) {
        file_stream_->flush();
    }
}

void Logger::log(LogLevel level, std::string_view message) {
    if (level < current_level_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    std::string log_line = std::format("{} [{}] {}", getCurrentTime(), levelToString(level), message);

    if (file_stream_ && file_stream_->is_open()) {
        *file_stream_ << log_line << std::endl;
        file_stream_->flush();
    } else {
        std::cout << log_line << std::endl;
    }
}

void Logger::trace(std::string_view message) { log(LogLevel::TRACE, message); }
void Logger::debug(std::string_view message) { log(LogLevel::DEBUG, message); }
void Logger::info(std::string_view message) { log(LogLevel::INFO, message); }
void Logger::warn(std::string_view message) { log(LogLevel::WARN, message); }
void Logger::error(std::string_view message) { log(LogLevel::ERROR, message); }
void Logger::fatal(std::string_view message) { log(LogLevel::FATAL, message); }

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

    return std::format("{:%Y-%m-%d %H:%M:%S}.{:03d}",
                      std::chrono::system_clock::from_time_t(time_t),
                      ms.count());
}

} // namespace memory::core
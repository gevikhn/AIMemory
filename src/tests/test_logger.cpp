#include <gtest/gtest.h>
#include "memory/core/logger.h"
#include <fstream>
#include <filesystem>

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = &memory::core::Logger::getInstance();
        log_file_ = "test_log.txt";
    }

    void TearDown() override {
        if (std::filesystem::exists(log_file_)) {
            std::filesystem::remove(log_file_);
        }
    }

    memory::core::Logger* logger_;
    std::string log_file_;
};

TEST_F(LoggerTest, SetLogLevel) {
    logger_->setLevel(memory::core::LogLevel::DEBUG);
    // 级别设置成功，无异常抛出
    SUCCEED();
}

TEST_F(LoggerTest, LogToFile) {
    logger_->setOutput(log_file_);
    logger_->info("Test message");

    // 检查文件是否存在且包含消息
    EXPECT_TRUE(std::filesystem::exists(log_file_));

    std::ifstream file(log_file_);
    std::string content;
    std::getline(file, content);

    EXPECT_TRUE(content.find("Test message") != std::string::npos);
    EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
}

TEST_F(LoggerTest, LogLevelFiltering) {
    logger_->setLevel(memory::core::LogLevel::WARN);
    logger_->setOutput(log_file_);

    logger_->debug("Debug message");  // 应该被过滤
    logger_->warn("Warning message"); // 应该被记录

    std::ifstream file(log_file_);
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    // 应该只有一行（warning消息）
    EXPECT_EQ(lines.size(), 1);
    EXPECT_TRUE(lines[0].find("Warning message") != std::string::npos);
    EXPECT_TRUE(lines[0].find("Debug message") == std::string::npos);
}

TEST_F(LoggerTest, AllLogLevels) {
    logger_->setLevel(memory::core::LogLevel::TRACE);
    logger_->setOutput(log_file_);

    logger_->trace("Trace message");
    logger_->debug("Debug message");
    logger_->info("Info message");
    logger_->warn("Warn message");
    logger_->error("Error message");
    logger_->fatal("Fatal message");

    std::ifstream file(log_file_);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    EXPECT_TRUE(content.find("[TRACE]") != std::string::npos);
    EXPECT_TRUE(content.find("[DEBUG]") != std::string::npos);
    EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
    EXPECT_TRUE(content.find("[WARN]") != std::string::npos);
    EXPECT_TRUE(content.find("[ERROR]") != std::string::npos);
    EXPECT_TRUE(content.find("[FATAL]") != std::string::npos);
}
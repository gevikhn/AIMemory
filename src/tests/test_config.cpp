#include <gtest/gtest.h>
#include "memory/core/config.h"
#include <fstream>
#include <filesystem>

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_ = &memory::core::Config::getInstance();
    }

    void TearDown() override {
        // 清理测试文件
        if (std::filesystem::exists("test_config.yaml")) {
            std::filesystem::remove("test_config.yaml");
        }
    }

    memory::core::Config* config_;
};

TEST_F(ConfigTest, SetAndGetString) {
    config_->set("test_key", "test_value");
    std::string value = config_->get<std::string>("test_key");
    EXPECT_EQ(value, "test_value");
}

TEST_F(ConfigTest, GetDefaultValue) {
    std::string value = config_->get<std::string>("nonexistent_key", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(ConfigTest, GetIntValue) {
    config_->set("int_key", "42");
    int value = config_->get<int>("int_key");
    EXPECT_EQ(value, 42);
}

TEST_F(ConfigTest, GetBoolValue) {
    config_->set("bool_key_true", "true");
    config_->set("bool_key_false", "false");

    EXPECT_TRUE(config_->get<bool>("bool_key_true"));
    EXPECT_FALSE(config_->get<bool>("bool_key_false"));
}

TEST_F(ConfigTest, LoadFromString) {
    std::string yaml_content = R"(
        host: localhost
        port: 8080
        debug: true
        timeout: 30
    )";

    config_->loadFromString(yaml_content);

    EXPECT_EQ(config_->get<std::string>("host"), "localhost");
    EXPECT_EQ(config_->get<int>("port"), 8080);
    EXPECT_TRUE(config_->get<bool>("debug"));
    EXPECT_EQ(config_->get<int>("timeout"), 30);
}

TEST_F(ConfigTest, LoadFromFile) {
    // 创建测试配置文件
    std::ofstream file("test_config.yaml");
    file << "database_url: sqlite://test.db\n";
    file << "log_level: DEBUG\n";
    file.close();

    config_->loadFromFile("test_config.yaml");

    EXPECT_EQ(config_->get<std::string>("database_url"), "sqlite://test.db");
    EXPECT_EQ(config_->get<std::string>("log_level"), "DEBUG");
}
#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace memory::core {

class Config {
public:
    static Config& getInstance();

    void loadFromFile(const std::string& filename);
    void loadFromString(const std::string& yaml_content);

    template<typename T>
    T get(const std::string& key, const T& default_value = T{}) const;

    void set(const std::string& key, const std::string& value);

private:
    Config() = default;
    std::unordered_map<std::string, std::string> config_map_;
};

} // namespace memory::core
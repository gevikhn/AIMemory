#include "memory/core/config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace memory::core {

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

void Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    loadFromString(buffer.str());
}

void Config::loadFromString(const std::string& yaml_content) {
    // Simple key-value pair parsing (MVP version)
    std::istringstream stream(yaml_content);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remove whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            config_map_[key] = value;
        }
    }
}

template<>
std::string Config::get<std::string>(const std::string& key, const std::string& default_value) const {
    auto it = config_map_.find(key);
    return (it != config_map_.end()) ? it->second : default_value;
}

template<>
int Config::get<int>(const std::string& key, const int& default_value) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return default_value;
        }
    }
    return default_value;
}

template<>
bool Config::get<bool>(const std::string& key, const bool& default_value) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        const std::string& value = it->second;
        return value == "true" || value == "1" || value == "yes";
    }
    return default_value;
}

void Config::set(const std::string& key, const std::string& value) {
    config_map_[key] = value;
}

} // namespace memory::core
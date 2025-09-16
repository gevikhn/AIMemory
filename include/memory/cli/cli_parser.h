#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace memory::cli {

struct CommandArgs {
    std::string command;
    std::string subcommand;
    std::unordered_map<std::string, std::string> options;
    std::vector<std::string> arguments;
};

class CliParser {
public:
    CommandArgs parse(int argc, char* argv[]);
    void printHelp() const;
    void printVersion() const;

private:
    bool isOption(const std::string& arg) const;
    std::string getOptionName(const std::string& arg) const;
};

} // namespace memory::cli
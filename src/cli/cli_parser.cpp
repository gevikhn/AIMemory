#include "memory/cli/cli_parser.h"
#include <iostream>

namespace memory::cli {

CommandArgs CliParser::parse(int argc, char* argv[]) {
    CommandArgs args;

    if (argc < 2) {
        return args;
    }

    args.command = argv[1];

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (isOption(arg)) {
            std::string option_name = getOptionName(arg);
            std::string option_value;

            // Check if has value
            if (i + 1 < argc && !isOption(argv[i + 1])) {
                option_value = argv[i + 1];
                ++i;
            } else {
                option_value = "true"; // Boolean option
            }

            args.options[option_name] = option_value;
        } else {
            if (args.subcommand.empty()) {
                args.subcommand = arg;
            } else {
                args.arguments.push_back(arg);
            }
        }
    }

    return args;
}

void CliParser::printHelp() const {
    std::cout << "AIMemory CLI Tool (memctl) v1.0.0\n\n";
    std::cout << "Usage: memctl <command> [subcommand] [options] [arguments]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  help, --help, -h     Show help information\n";
    std::cout << "  version, --version   Show version information\n";
    std::cout << "  config               Configuration management\n";
    std::cout << "  index                Index management\n";
    std::cout << "  graph                Graph database management\n";
    std::cout << "  recall               Memory recall\n";
    std::cout << "  metrics              View metrics\n\n";
    std::cout << "Run 'memctl <command> --help' for more information on a command.\n";
}

void CliParser::printVersion() const {
    std::cout << "memctl version 1.0.0\n";
    std::cout << "AI Memory System CLI Tool\n";
}

bool CliParser::isOption(const std::string& arg) const {
    return arg.length() > 1 && arg[0] == '-';
}

std::string CliParser::getOptionName(const std::string& arg) const {
    if (arg.length() > 2 && arg[0] == '-' && arg[1] == '-') {
        return arg.substr(2); // Remove "--"
    } else if (arg.length() > 1 && arg[0] == '-') {
        return arg.substr(1); // Remove "-"
    }
    return arg;
}

} // namespace memory::cli
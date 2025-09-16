#include "memory/cli/cli_parser.h"
#include "memory/cli/commands.h"
#include "memory/core/logger.h"
#include "memory/core/config.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // Initialize logging system
        auto& logger = memory::core::Logger::getInstance();
        logger.setLevel(memory::core::LogLevel::INFO);

        // Parse command line arguments
        memory::cli::CliParser parser;
        auto args = parser.parse(argc, argv);

        // Execute command
        return memory::cli::Commands::execute(args);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
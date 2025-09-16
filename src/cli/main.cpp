#include "memory/cli/cli_parser.h"
#include "memory/cli/commands.h"
#include "memory/core/logger.h"
#include "memory/core/config.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
    try {
#ifdef _WIN32
        // 设置控制台为UTF-8编码
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        // system("chcp 65001");
#endif
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
#include "memory/cli/commands.h"
#include "memory/core/logger.h"
#include "memory/core/config.h"
#include <iostream>

namespace memory::cli {

int Commands::execute(const CommandArgs& args) {
    if (args.command.empty() || args.command == "help" || args.command == "--help" || args.command == "-h") {
        CliParser parser;
        parser.printHelp();
        return 0;
    }

    if (args.command == "version" || args.command == "--version") {
        CliParser parser;
        parser.printVersion();
        return 0;
    }

    if (args.command == "config") {
        return executeConfig(args);
    } else if (args.command == "index") {
        return executeIndex(args);
    } else if (args.command == "graph") {
        return executeGraph(args);
    } else if (args.command == "recall") {
        return executeRecall(args);
    } else if (args.command == "metrics") {
        return executeMetrics(args);
    } else {
        std::cerr << "Unknown command: " << args.command << std::endl;
        std::cerr << "Run 'memctl --help' for usage information." << std::endl;
        return 1;
    }
}

int Commands::executeConfig(const CommandArgs& args) {
    if (args.options.count("help") || args.options.count("h")) {
        printConfigHelp();
        return 0;
    }

    if (args.subcommand == "set" && args.arguments.size() >= 2) {
        auto& config = memory::core::Config::getInstance();
        config.set(args.arguments[0], args.arguments[1]);
        std::cout << "配置已设置: " << args.arguments[0] << " = " << args.arguments[1] << std::endl;
        return 0;
    } else if (args.subcommand == "get" && args.arguments.size() >= 1) {
        auto& config = memory::core::Config::getInstance();
        std::string value = config.get<std::string>(args.arguments[0], "");
        std::cout << args.arguments[0] << " = " << value << std::endl;
        return 0;
    } else {
        printConfigHelp();
        return 1;
    }
}

int Commands::executeIndex(const CommandArgs& args) {
    if (args.options.count("help") || args.options.count("h")) {
        printIndexHelp();
        return 0;
    }

    LOG_INFO("索引操作: " + args.subcommand);
    std::cout << "索引功能正在开发中..." << std::endl;
    return 0;
}

int Commands::executeGraph(const CommandArgs& args) {
    if (args.options.count("help") || args.options.count("h")) {
        printGraphHelp();
        return 0;
    }

    LOG_INFO("图操作: " + args.subcommand);
    std::cout << "图数据库功能正在开发中..." << std::endl;
    return 0;
}

int Commands::executeRecall(const CommandArgs& args) {
    if (args.options.count("help") || args.options.count("h")) {
        printRecallHelp();
        return 0;
    }

    LOG_INFO("召回操作");
    std::cout << "记忆召回功能正在开发中..." << std::endl;
    return 0;
}

int Commands::executeMetrics(const CommandArgs& args) {
    if (args.options.count("help") || args.options.count("h")) {
        printMetricsHelp();
        return 0;
    }

    LOG_INFO("指标查询");
    std::cout << "指标功能正在开发中..." << std::endl;
    return 0;
}

void Commands::printConfigHelp() {
    std::cout << "配置管理\n\n";
    std::cout << "Usage: memctl config <subcommand> [arguments]\n\n";
    std::cout << "Subcommands:\n";
    std::cout << "  set <key> <value>    设置配置项\n";
    std::cout << "  get <key>            获取配置项值\n";
    std::cout << "  load <file>          从文件加载配置\n\n";
}

void Commands::printIndexHelp() {
    std::cout << "索引管理\n\n";
    std::cout << "Usage: memctl index <subcommand> [options]\n\n";
    std::cout << "Subcommands:\n";
    std::cout << "  add <file>           添加文档到索引\n";
    std::cout << "  search <query>       搜索文档\n";
    std::cout << "  rebuild              重建索引\n\n";
}

void Commands::printGraphHelp() {
    std::cout << "图数据库管理\n\n";
    std::cout << "Usage: memctl graph <subcommand> [options]\n\n";
    std::cout << "Subcommands:\n";
    std::cout << "  add-node <type>      添加节点\n";
    std::cout << "  add-edge <src> <dst> 添加边\n";
    std::cout << "  query <id>           查询节点信息\n";
    std::cout << "  neighbors <id>       查询邻居节点\n\n";
}

void Commands::printRecallHelp() {
    std::cout << "记忆召回\n\n";
    std::cout << "Usage: memctl recall [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --query <text>       查询文本\n";
    std::cout << "  --budget <tokens>    Token预算\n";
    std::cout << "  --k-hop <number>     图扩散跳数\n";
    std::cout << "  --trace              显示执行追踪\n\n";
}

void Commands::printMetricsHelp() {
    std::cout << "指标查看\n\n";
    std::cout << "Usage: memctl metrics [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --dump               导出所有指标\n";
    std::cout << "  --format <json|text> 输出格式\n\n";
}

} // namespace memory::cli
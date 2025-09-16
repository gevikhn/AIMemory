#pragma once

#include "memory/cli/cli_parser.h"

namespace memory::cli {

class Commands {
public:
    static int execute(const CommandArgs& args);

private:
    static int executeConfig(const CommandArgs& args);
    static int executeIndex(const CommandArgs& args);
    static int executeGraph(const CommandArgs& args);
    static int executeRecall(const CommandArgs& args);
    static int executeMetrics(const CommandArgs& args);

    static void printConfigHelp();
    static void printIndexHelp();
    static void printGraphHelp();
    static void printRecallHelp();
    static void printMetricsHelp();
};

} // namespace memory::cli
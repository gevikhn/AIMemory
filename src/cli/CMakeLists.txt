add_library(memory_cli
    cli_parser.cpp
    commands.cpp
)

target_include_directories(memory_cli PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(memory_cli
    memory_core
)
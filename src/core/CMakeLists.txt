add_library(memory_core
    config.cpp
    logger.cpp
    errors.cpp
    types.cpp
)

target_include_directories(memory_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(memory_core
    Threads::Threads
)
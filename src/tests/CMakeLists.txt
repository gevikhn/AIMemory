# 创建其他模块的空CMakeLists.txt以避免构建错误
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../search)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../graph)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../vector)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../pipeline)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../jobs)

# 为空模块创建基本CMakeLists.txt
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../search/CMakeLists.txt
"# SearchIndex模块 - 待实现\n# add_library(memory_search)\n")

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../graph/CMakeLists.txt
"# GraphStore模块 - 待实现\n# add_library(memory_graph)\n")

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../vector/CMakeLists.txt
"# VectorIndex模块 - 待实现\n# add_library(memory_vector)\n")

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../pipeline/CMakeLists.txt
"# Pipeline模块 - 待实现\n# add_library(memory_pipeline)\n")

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../jobs/CMakeLists.txt
"# Jobs模块 - 待实现\n# add_library(memory_jobs)\n")

# 单元测试
add_executable(test_config
    test_config.cpp
)

target_link_libraries(test_config
    memory_core
    gtest
    gtest_main
)

add_executable(test_logger
    test_logger.cpp
)

target_link_libraries(test_logger
    memory_core
    gtest
    gtest_main
)

add_executable(test_types
    test_types.cpp
)

target_link_libraries(test_types
    memory_core
    gtest
    gtest_main
)

# 添加测试到CTest
include(GoogleTest)
gtest_discover_tests(test_config)
gtest_discover_tests(test_logger)
gtest_discover_tests(test_types)
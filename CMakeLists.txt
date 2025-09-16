cmake_minimum_required(VERSION 3.20)
project(AIMemory VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Compiler flags
if(MSVC)
    add_compile_options(/utf-8 /W4 /permissive-  /Zc:wchar_t- /D_UNICODE /DUNICODE)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find packages
find_package(Threads REQUIRED)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src/core)
add_subdirectory(src/cli)
add_subdirectory(src/tests)

# 未来模块（暂时注释掉）
# add_subdirectory(src/search)
# add_subdirectory(src/graph)
# add_subdirectory(src/vector)
# add_subdirectory(src/pipeline)
# add_subdirectory(src/jobs)

# Main executable
add_executable(memctl
    src/cli/main.cpp
)

target_link_libraries(memctl
    memory_core
    memory_cli
    Threads::Threads
)

# Install targets
install(TARGETS memctl RUNTIME DESTINATION bin)
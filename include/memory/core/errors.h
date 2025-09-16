#pragma once

#include <stdexcept>
#include <string>

namespace memory::core {

// Base exception class
class MemoryException : public std::runtime_error {
public:
    explicit MemoryException(const std::string& message)
        : std::runtime_error(message) {}
};

// Configuration related exception
class ConfigException : public MemoryException {
public:
    explicit ConfigException(const std::string& message)
        : MemoryException("Config error: " + message) {}
};

// Storage related exception
class StorageException : public MemoryException {
public:
    explicit StorageException(const std::string& message)
        : MemoryException("Storage error: " + message) {}
};

// Index related exception
class IndexException : public MemoryException {
public:
    explicit IndexException(const std::string& message)
        : MemoryException("Index error: " + message) {}
};

// Query related exception
class QueryException : public MemoryException {
public:
    explicit QueryException(const std::string& message)
        : MemoryException("Query error: " + message) {}
};

} // namespace memory::core
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <cstdint>

namespace memory::core {

using NodeId = uint64_t;
using EdgeId = uint64_t;
using TenantId = std::string;
using Timestamp = std::chrono::system_clock::time_point;

// Node type enumeration
enum class NodeType {
    EPISODE,    // Conversation/experience/one-time event
    FACT,       // Declarative fact
    PREFERENCE, // User preference/habit/constraint
    CONCEPT,    // Abstract concept/topic/term
    ENTITY,     // Named entity
    TASK,       // Task/todo/goal state
    META        // System/process/policy node
};

// Edge type enumeration
enum class EdgeType {
    TEMPORAL_NEXT,  // Temporal order
    ABOUT,          // Topic/subject attribution
    MENTIONS,       // Mentions entity
    CAUSES,         // Causation
    CAUSED_BY,      // Caused by
    SUPPORTS,       // Support
    CONTRADICTS,    // Contradiction
    DERIVED_FROM,   // Summary/abstraction source
    REFERS_TO,      // Cross-session reference
    SIMILAR_TO,     // Similar/synonymous semantic
    VERSION_NEXT,   // Version chain
    ACTIVE_OF,      // Current active version
    SAME_AS         // Same entity/event
};

// Node data structure
struct Node {
    NodeId id;
    NodeType type;
    std::string title;
    std::string text;
    std::vector<std::string> keywords;
    std::vector<std::string> entities;
    float importance = 0.0f;
    float confidence = 1.0f;
    Timestamp recency;
    int frequency = 1;
    TenantId tenant_id;
    std::unordered_map<std::string, std::string> metadata;
};

// Edge data structure
struct Edge {
    EdgeId id;
    NodeId src;
    NodeId dst;
    EdgeType type;
    float weight = 1.0f;
    float confidence = 1.0f;
    TenantId tenant_id;
    std::unordered_map<std::string, std::string> metadata;
};

// Query structure
struct RecallQuery {
    std::string text;
    std::vector<std::string> keywords;
    size_t token_budget = 2000;
    int k_hop = 2;
    TenantId tenant_id;
    std::unordered_map<std::string, std::string> options;
};

// Scoring result
struct ScoredId {
    uint64_t id;
    float score;

    bool operator<(const ScoredId& other) const {
        return score > other.score; // Descending order
    }
};

// Utility functions
std::string nodeTypeToString(NodeType type);
std::string edgeTypeToString(EdgeType type);
NodeType stringToNodeType(const std::string& str);
EdgeType stringToEdgeType(const std::string& str);

} // namespace memory::core
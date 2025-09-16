#include "memory/core/types.h"
#include <stdexcept>

namespace memory::core {

std::string nodeTypeToString(NodeType type) {
    switch (type) {
        case NodeType::EPISODE: return "Episode";
        case NodeType::FACT: return "Fact";
        case NodeType::PREFERENCE: return "Preference";
        case NodeType::CONCEPT: return "Concept";
        case NodeType::ENTITY: return "Entity";
        case NodeType::TASK: return "Task";
        case NodeType::META: return "Meta";
        default: return "Unknown";
    }
}

std::string edgeTypeToString(EdgeType type) {
    switch (type) {
        case EdgeType::TEMPORAL_NEXT: return "TEMPORAL_NEXT";
        case EdgeType::ABOUT: return "ABOUT";
        case EdgeType::MENTIONS: return "MENTIONS";
        case EdgeType::CAUSES: return "CAUSES";
        case EdgeType::CAUSED_BY: return "CAUSED_BY";
        case EdgeType::SUPPORTS: return "SUPPORTS";
        case EdgeType::CONTRADICTS: return "CONTRADICTS";
        case EdgeType::DERIVED_FROM: return "DERIVED_FROM";
        case EdgeType::REFERS_TO: return "REFERS_TO";
        case EdgeType::SIMILAR_TO: return "SIMILAR_TO";
        case EdgeType::VERSION_NEXT: return "VERSION_NEXT";
        case EdgeType::ACTIVE_OF: return "ACTIVE_OF";
        case EdgeType::SAME_AS: return "SAME_AS";
        default: return "Unknown";
    }
}

NodeType stringToNodeType(const std::string& str) {
    if (str == "Episode") return NodeType::EPISODE;
    if (str == "Fact") return NodeType::FACT;
    if (str == "Preference") return NodeType::PREFERENCE;
    if (str == "Concept") return NodeType::CONCEPT;
    if (str == "Entity") return NodeType::ENTITY;
    if (str == "Task") return NodeType::TASK;
    if (str == "Meta") return NodeType::META;
    throw std::invalid_argument("Unknown node type: " + str);
}

EdgeType stringToEdgeType(const std::string& str) {
    if (str == "TEMPORAL_NEXT") return EdgeType::TEMPORAL_NEXT;
    if (str == "ABOUT") return EdgeType::ABOUT;
    if (str == "MENTIONS") return EdgeType::MENTIONS;
    if (str == "CAUSES") return EdgeType::CAUSES;
    if (str == "CAUSED_BY") return EdgeType::CAUSED_BY;
    if (str == "SUPPORTS") return EdgeType::SUPPORTS;
    if (str == "CONTRADICTS") return EdgeType::CONTRADICTS;
    if (str == "DERIVED_FROM") return EdgeType::DERIVED_FROM;
    if (str == "REFERS_TO") return EdgeType::REFERS_TO;
    if (str == "SIMILAR_TO") return EdgeType::SIMILAR_TO;
    if (str == "VERSION_NEXT") return EdgeType::VERSION_NEXT;
    if (str == "ACTIVE_OF") return EdgeType::ACTIVE_OF;
    if (str == "SAME_AS") return EdgeType::SAME_AS;
    throw std::invalid_argument("Unknown edge type: " + str);
}

} // namespace memory::core
#include <gtest/gtest.h>
#include "memory/core/types.h"

TEST(TypesTest, NodeTypeConversion) {
    EXPECT_EQ(memory::core::nodeTypeToString(memory::core::NodeType::EPISODE), "Episode");
    EXPECT_EQ(memory::core::nodeTypeToString(memory::core::NodeType::FACT), "Fact");
    EXPECT_EQ(memory::core::nodeTypeToString(memory::core::NodeType::CONCEPT), "Concept");

    EXPECT_EQ(memory::core::stringToNodeType("Episode"), memory::core::NodeType::EPISODE);
    EXPECT_EQ(memory::core::stringToNodeType("Fact"), memory::core::NodeType::FACT);
    EXPECT_EQ(memory::core::stringToNodeType("Concept"), memory::core::NodeType::CONCEPT);
}

TEST(TypesTest, EdgeTypeConversion) {
    EXPECT_EQ(memory::core::edgeTypeToString(memory::core::EdgeType::ABOUT), "ABOUT");
    EXPECT_EQ(memory::core::edgeTypeToString(memory::core::EdgeType::MENTIONS), "MENTIONS");
    EXPECT_EQ(memory::core::edgeTypeToString(memory::core::EdgeType::SUPPORTS), "SUPPORTS");

    EXPECT_EQ(memory::core::stringToEdgeType("ABOUT"), memory::core::EdgeType::ABOUT);
    EXPECT_EQ(memory::core::stringToEdgeType("MENTIONS"), memory::core::EdgeType::MENTIONS);
    EXPECT_EQ(memory::core::stringToEdgeType("SUPPORTS"), memory::core::EdgeType::SUPPORTS);
}

TEST(TypesTest, InvalidTypeConversion) {
    EXPECT_THROW(memory::core::stringToNodeType("InvalidType"), std::invalid_argument);
    EXPECT_THROW(memory::core::stringToEdgeType("InvalidType"), std::invalid_argument);
}

TEST(TypesTest, ScoredIdComparison) {
    memory::core::ScoredId id1{1, 0.8f};
    memory::core::ScoredId id2{2, 0.6f};

    EXPECT_TRUE(id1 < id2); // Higher scores come first (descending)
}

TEST(TypesTest, NodeStructure) {
    memory::core::Node node;
    node.id = 1;
    node.type = memory::core::NodeType::EPISODE;
    node.title = "Test Episode";
    node.text = "This is a test episode";
    node.importance = 0.7f;
    node.confidence = 0.9f;
    node.frequency = 3;
    node.tenant_id = "test_tenant";

    EXPECT_EQ(node.id, 1);
    EXPECT_EQ(node.type, memory::core::NodeType::EPISODE);
    EXPECT_EQ(node.title, "Test Episode");
    EXPECT_EQ(node.importance, 0.7f);
    EXPECT_EQ(node.tenant_id, "test_tenant");
}

TEST(TypesTest, EdgeStructure) {
    memory::core::Edge edge;
    edge.id = 100;
    edge.src = 1;
    edge.dst = 2;
    edge.type = memory::core::EdgeType::ABOUT;
    edge.weight = 0.8f;
    edge.confidence = 0.95f;
    edge.tenant_id = "test_tenant";

    EXPECT_EQ(edge.id, 100);
    EXPECT_EQ(edge.src, 1);
    EXPECT_EQ(edge.dst, 2);
    EXPECT_EQ(edge.type, memory::core::EdgeType::ABOUT);
    EXPECT_EQ(edge.weight, 0.8f);
}
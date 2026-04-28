#include <algorithm>
#include <gtest/gtest.h>

#include "LifeRule.hpp"

namespace gol {

TEST(LifeRuleTest, ValidConwayRule) {
    const auto rule = LifeRule::Make("B3/S23");
    ASSERT_TRUE(rule.has_value());

    // Default topology should be Plane
    EXPECT_EQ(rule->GetTopology(), TopologyKind::Plane);

    // Table should be populated for a real rule
    const auto& tbl = rule->Table();
    auto nonzero = std::count_if(tbl.begin(), tbl.end(),
                                 [](uint16_t v) { return v != 0; });
    EXPECT_GT(nonzero, 0u);

    // Empty pattern (all zeros) should map to 0 in the table
    EXPECT_EQ(tbl[0], 0u);
}

TEST(LifeRuleTest, TopologyAndBoundsExtraction) {
    // Torus with explicit dimensions
    const std::string ruleStr{"B36/S23:T10,20"};
    const auto topo = LifeRule::ExtractTopologyKind(ruleStr);
    ASSERT_TRUE(topo.has_value());
    EXPECT_EQ(*topo, TopologyKind::Torus);

    const auto dims = LifeRule::ExtractDimensions(ruleStr);
    ASSERT_TRUE(dims.has_value());
    EXPECT_EQ(dims->Width, 10);
    EXPECT_EQ(dims->Height, 20);

    const auto made = LifeRule::Make(ruleStr);
    ASSERT_TRUE(made.has_value());
    EXPECT_EQ(made->GetTopology(), TopologyKind::Torus);
    ASSERT_TRUE(made->Bounds().has_value());
    EXPECT_EQ(made->Bounds()->Size().Width, 10);
}

TEST(LifeRuleTest, InvalidRules) {
    // B0 is explicitly rejected
    const auto r1 = LifeRule::Make("B0/S23");
    EXPECT_FALSE(r1.has_value());

    // Completely malformed
    const auto r2 = LifeRule::Make("not-a-rule");
    EXPECT_FALSE(r2.has_value());

    // Invalid survive char
    const auto r3 = LifeRule::Make("B3/S9");
    EXPECT_FALSE(r3.has_value());
}

TEST(LifeRuleTest, IsValidRuleChecks) {
    const auto ok = LifeRule::IsValidRule("B3/S23");
    EXPECT_TRUE(ok.has_value());

    const auto bad = LifeRule::IsValidRule("B3X/S23");
    EXPECT_FALSE(bad.has_value());
}

} // namespace gol

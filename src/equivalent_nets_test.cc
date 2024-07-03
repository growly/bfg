#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utility>

#include "equivalent_nets.h"

namespace bfg {
namespace {

TEST(EquivalentNets, Empty) {
  EquivalentNets nets = EquivalentNets({});
  EXPECT_FALSE(nets.Contains("a"));
  EXPECT_FALSE(nets.Contains("b"));
  EXPECT_FALSE(nets.Contains("c"));
  EXPECT_EQ("", nets.primary());
}

TEST(EquivalentNets, General) {
  std::set<std::string> aliases = {"a", "b", "c"};
  std::string default_primary = *aliases.begin();
  EquivalentNets nets = EquivalentNets(aliases);

  EXPECT_TRUE(nets.Contains("a"));
  EXPECT_TRUE(nets.Contains("b"));
  EXPECT_TRUE(nets.Contains("c"));
  EXPECT_FALSE(nets.Contains("d"));
  EXPECT_TRUE(nets.Add("d"));
  EXPECT_FALSE(nets.Add("d"));
  EXPECT_EQ(default_primary, nets.primary());

  EXPECT_FALSE(nets.Contains("e"));
  nets.set_primary("e");
  EXPECT_TRUE(nets.Contains("e"));
  EXPECT_EQ("e", nets.primary());
}


}  // namespace
}  // namespace bfg

#include "equivalent_nets.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sstream>
#include <utility>

namespace bfg {
namespace {

TEST(EquivalentNets, Empty) {
  EquivalentNets nets = EquivalentNets(std::set<std::string>());
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

TEST(EquivalentNets, Sole) {
  EquivalentNets nets = EquivalentNets("q");
  EXPECT_TRUE(nets.Contains("q"));
  EXPECT_EQ("q", nets.primary());
}

TEST(EquivalentNets, ContainsAny) {
  EquivalentNets abc = EquivalentNets(std::set<std::string>{"a", "b", "c"});
  EquivalentNets cd = EquivalentNets(std::set<std::string>{"c", "d"});
  EquivalentNets de = EquivalentNets(std::set<std::string>{"d", "e"});
  EquivalentNets empty;

  EXPECT_TRUE(abc.ContainsAny(cd));
  EXPECT_TRUE(cd.ContainsAny(abc));
  EXPECT_FALSE(abc.ContainsAny(de));
  EXPECT_FALSE(abc.ContainsAny(empty));
  EXPECT_FALSE(empty.ContainsAny(abc));
  EXPECT_FALSE(empty.ContainsAny(empty));
}

TEST(EquivalentNets, AddEmptyNameIsIgnored) {
  EquivalentNets nets;
  EXPECT_FALSE(nets.Add(""));
  EXPECT_TRUE(nets.Empty());
}

TEST(EquivalentNets, AddOther) {
  EquivalentNets nets = EquivalentNets("a");
  EXPECT_TRUE(nets.Add(EquivalentNets(std::set<std::string>{"a", "b"})));
  EXPECT_TRUE(nets.Contains("b"));
  EXPECT_FALSE(nets.Add(EquivalentNets(std::set<std::string>{"a", "b"})));
  EXPECT_EQ("a", nets.primary());
}

TEST(EquivalentNets, PrimaryWithAliases) {
  EquivalentNets nets = EquivalentNets("z", {"a", "b"});
  EXPECT_EQ("z", nets.primary());
  EXPECT_TRUE(nets.Contains("z"));
  EXPECT_TRUE(nets.Contains("a"));
  EXPECT_EQ(3, nets.nets().size());
}

TEST(EquivalentNets, Delete) {
  EquivalentNets nets = EquivalentNets(std::set<std::string>{"a", "b"});
  EXPECT_TRUE(nets.Delete("b"));
  EXPECT_FALSE(nets.Contains("b"));
  EXPECT_FALSE(nets.Delete("b"));
}

// TODO(aryap): The class promises that the primary is always in the set, but
// Delete() does not clear or reassign primary_ when the primary is removed.
TEST(EquivalentNets, DISABLED_DeletePrimaryKeepsInvariant) {
  EquivalentNets nets = EquivalentNets("a", {"b"});
  EXPECT_TRUE(nets.Delete("a"));
  EXPECT_TRUE(nets.primary() == "" || nets.Contains(nets.primary()));
}

TEST(EquivalentNets, Describe) {
  EXPECT_EQ("", EquivalentNets().Describe());

  // The primary is listed first and marked; the rest follow in sorted order.
  EquivalentNets nets = EquivalentNets("c", {"a", "b"});
  EXPECT_EQ("c (P), a, b", nets.Describe());

  std::stringstream ss;
  ss << nets;
  EXPECT_EQ(nets.Describe(), ss.str());
}


}  // namespace
}  // namespace bfg

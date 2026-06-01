#include <gtest/gtest.h>

#include <sstream>

#include "signal.h"

namespace bfg {
namespace circuit {
namespace {

TEST(Signal, DefaultConstruct) {
  Signal s;
  EXPECT_EQ(s.name(), "");
  EXPECT_EQ(s.width(), 0u);
}

TEST(Signal, ConstructWithNameAndWidth) {
  Signal s("clk", 1);
  EXPECT_EQ(s.name(), "clk");
  EXPECT_EQ(s.width(), 1u);
}

TEST(Signal, Setters) {
  Signal s;
  s.set_name("data");
  s.set_width(8);
  EXPECT_EQ(s.name(), "data");
  EXPECT_EQ(s.width(), 8u);
}

TEST(Signal, EqualityOperator_SameNameAndWidth) {
  Signal a("clk", 1);
  Signal b("clk", 1);
  EXPECT_TRUE(a == b);
}

TEST(Signal, EqualityOperator_DifferentName) {
  Signal a("clk", 1);
  Signal b("data", 1);
  EXPECT_FALSE(a == b);
}

TEST(Signal, EqualityOperator_DifferentWidth) {
  Signal a("clk", 1);
  Signal b("clk", 4);
  EXPECT_FALSE(a == b);
}

TEST(Signal, StreamOperator) {
  Signal s("clk", 1);
  std::ostringstream os;
  os << s;
  EXPECT_EQ(os.str(), "clk");
}

TEST(Signal, ToVLSIRSignal) {
  Signal s("bus", 4);
  auto pb = s.ToVLSIRSignal();
  EXPECT_EQ(pb.name(), "bus");
  EXPECT_EQ(pb.width(), 4u);
}

}  // namespace
}  // namespace circuit
}  // namespace bfg

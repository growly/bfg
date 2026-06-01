#include <gtest/gtest.h>

#include "connection.h"
#include "instance.h"
#include "signal.h"
#include "slice.h"
#include "wire.h"
#include "../parameter.h"

namespace bfg {
namespace circuit {
namespace {

TEST(Instance, ConnectSignalAndGetConnection) {
  Signal clk("clk", 1);
  Instance inst;
  inst.set_name("u1");
  inst.Connect("CLK", clk);
  auto conn = inst.GetConnection("CLK");
  ASSERT_TRUE(conn.has_value());
  EXPECT_EQ(conn->connection_type(), Connection::SIGNAL);
  EXPECT_EQ(conn->signal(), &clk);
}

TEST(Instance, GetConnectionMissingPort) {
  Instance inst;
  EXPECT_FALSE(inst.GetConnection("CLK").has_value());
}

TEST(Instance, DisconnectConnectedPort) {
  Signal clk("clk", 1);
  Instance inst;
  inst.Connect("CLK", clk);
  EXPECT_TRUE(inst.Disconnect("CLK"));
  EXPECT_FALSE(inst.GetConnection("CLK").has_value());
}

TEST(Instance, DisconnectUnconnectedPort) {
  Instance inst;
  EXPECT_FALSE(inst.Disconnect("CLK"));
}

TEST(Instance, SetParameter) {
  Instance inst;
  Parameter p;
  p.name = "W";
  p.value = std::string("0.5");
  inst.SetParameter("W", p);
  const auto &params = inst.parameters();
  ASSERT_NE(params.find("W"), params.end());
  EXPECT_EQ(params.at("W").name, "W");
}

TEST(Instance, ConnectFullWidthSliceBecomesSignal) {
  Signal bus("bus", 4);
  Slice full_slice(bus, 0, 3);
  Instance inst;
  inst.Connect("DATA", full_slice);
  auto conn = inst.GetConnection("DATA");
  ASSERT_TRUE(conn.has_value());
  EXPECT_EQ(conn->connection_type(), Connection::SIGNAL);
}

TEST(Instance, ConnectPartialSliceRemainsSlice) {
  Signal bus("bus", 4);
  Slice partial(bus, 0, 1);
  Instance inst;
  inst.Connect("DATA", partial);
  auto conn = inst.GetConnection("DATA");
  ASSERT_TRUE(conn.has_value());
  EXPECT_EQ(conn->connection_type(), Connection::SLICE);
}

TEST(Instance, ConnectViaInitializerList) {
  Signal clk("clk", 1);
  Signal data("data", 1);
  Wire clk_wire(clk, 0);
  Wire data_wire(data, 0);
  Instance inst;
  inst.Connect({{"CLK", clk_wire}, {"DATA", data_wire}});
  EXPECT_TRUE(inst.GetConnection("CLK").has_value());
  EXPECT_TRUE(inst.GetConnection("DATA").has_value());
}

}  // namespace
}  // namespace circuit
}  // namespace bfg

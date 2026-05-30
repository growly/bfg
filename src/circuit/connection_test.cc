#include <gtest/gtest.h>

#include "connection.h"
#include "signal.h"
#include "slice.h"

namespace bfg {
namespace circuit {
namespace {

TEST(Connection, SetSignalStoresSignalType) {
  Signal s("clk", 1);
  Connection c;
  c.set_signal(&s);
  EXPECT_EQ(c.connection_type(), Connection::SIGNAL);
  EXPECT_EQ(c.signal(), &s);
}

TEST(Connection, SetSliceStoresSliceType) {
  Signal s("bus", 4);
  Slice slice(s, 1, 2);
  Connection c;
  c.set_slice(slice);
  EXPECT_EQ(c.connection_type(), Connection::SLICE);
  ASSERT_NE(c.slice(), nullptr);
  EXPECT_EQ(c.slice()->low_index(), 1u);
  EXPECT_EQ(c.slice()->high_index(), 2u);
}

TEST(Connection, GetSingleReferencedSignal_Signal) {
  Signal s("clk", 1);
  Connection c;
  c.set_signal(&s);
  auto result = c.GetSingleReferencedSignal();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, &s);
}

TEST(Connection, GetSingleReferencedSignal_Slice) {
  Signal s("bus", 4);
  Slice slice(s, 0, 1);
  Connection c;
  c.set_slice(slice);
  auto result = c.GetSingleReferencedSignal();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, &s);
}

TEST(Connection, CopyConstructor_Signal) {
  Signal s("clk", 1);
  Connection original;
  original.set_signal(&s);
  Connection copy(original);
  EXPECT_EQ(copy.connection_type(), Connection::SIGNAL);
  EXPECT_EQ(copy.signal(), &s);
}

TEST(Connection, CopyConstructor_SliceIsDeepCopied) {
  Signal s("bus", 4);
  Slice slice(s, 1, 2);
  Connection original;
  original.set_slice(slice);
  Connection copy(original);
  EXPECT_EQ(copy.connection_type(), Connection::SLICE);
  ASSERT_NE(copy.slice(), nullptr);
  EXPECT_NE(copy.slice(), original.slice());
  EXPECT_EQ(copy.slice()->low_index(), 1u);
}

TEST(Connection, ToVLSIRConnection_Signal) {
  Signal s("clk", 1);
  Connection c;
  c.set_signal(&s);
  auto pb = c.ToVLSIRConnection();
  EXPECT_EQ(pb.sig(), "clk");
}

TEST(Connection, ToVLSIRConnection_Slice) {
  Signal s("bus", 4);
  Slice slice(s, 0, 1);
  Connection c;
  c.set_slice(slice);
  auto pb = c.ToVLSIRConnection();
  EXPECT_EQ(pb.slice().signal(), "bus");
  EXPECT_EQ(pb.slice().bot(), 0u);
  EXPECT_EQ(pb.slice().top(), 1u);
}

}  // namespace
}  // namespace circuit
}  // namespace bfg

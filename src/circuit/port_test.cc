#include <gtest/gtest.h>

#include "port.h"
#include "signal.h"

namespace bfg {
namespace circuit {
namespace {

TEST(Port, FromVLSIRPortDirection_AllDirections) {
  EXPECT_EQ(Port::FromVLSIRPortDirection(::vlsir::circuit::Port::INPUT),
            Port::INPUT);
  EXPECT_EQ(Port::FromVLSIRPortDirection(::vlsir::circuit::Port::OUTPUT),
            Port::OUTPUT);
  EXPECT_EQ(Port::FromVLSIRPortDirection(::vlsir::circuit::Port::INOUT),
            Port::INOUT);
  EXPECT_EQ(Port::FromVLSIRPortDirection(::vlsir::circuit::Port::NONE),
            Port::NONE);
}

TEST(Port, ToVLSIRPortDirection_AllDirections) {
  EXPECT_EQ(Port::ToVLSIRPortDirection(Port::INPUT),
            ::vlsir::circuit::Port::INPUT);
  EXPECT_EQ(Port::ToVLSIRPortDirection(Port::OUTPUT),
            ::vlsir::circuit::Port::OUTPUT);
  EXPECT_EQ(Port::ToVLSIRPortDirection(Port::INOUT),
            ::vlsir::circuit::Port::INOUT);
  EXPECT_EQ(Port::ToVLSIRPortDirection(Port::NONE),
            ::vlsir::circuit::Port::NONE);
}

TEST(Port, ToVLSIRPort) {
  Signal s("out", 1);
  Port port(s, Port::OUTPUT);
  auto pb = port.ToVLSIRPort();
  EXPECT_EQ(pb.signal(), "out");
  EXPECT_EQ(pb.direction(), ::vlsir::circuit::Port::OUTPUT);
}

}  // namespace
}  // namespace circuit
}  // namespace bfg

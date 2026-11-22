#include "parameter.h"

#include <gtest/gtest.h>

#include <string>

#include "vlsir/utils.pb.h"

namespace bfg {
namespace {

TEST(ParameterTest, FromInteger) {
  Parameter parameter = Parameter::FromInteger(
      "width", 420, Parameter::SIUnitPrefix::NANO);
  EXPECT_EQ("width", parameter.name);
  EXPECT_EQ("", parameter.description);
  EXPECT_EQ(Parameter::SIUnitPrefix::NANO, parameter.unit_prefix);
  EXPECT_EQ(420, std::get<int64_t>(parameter.value));
}

TEST(ParameterTest, SIPrefixRoundTrip) {
  for (int i = Parameter::SIUnitPrefix::NONE;
       i < Parameter::SIUnitPrefix::MAX_VALUE;
       ++i) {
    Parameter::SIUnitPrefix prefix = static_cast<Parameter::SIUnitPrefix>(i);
    EXPECT_EQ(prefix, Parameter::FromVLSIRSIPrefix(
        Parameter::ToVLSIRSIPrefix(prefix)));
  }
}

TEST(ParameterTest, SIPrefixMapping) {
  EXPECT_EQ(vlsir::utils::SIPrefix::UNIT,
            Parameter::ToVLSIRSIPrefix(Parameter::SIUnitPrefix::NONE));
  EXPECT_EQ(vlsir::utils::SIPrefix::MICRO,
            Parameter::ToVLSIRSIPrefix(Parameter::SIUnitPrefix::MICRO));
  EXPECT_EQ(Parameter::SIUnitPrefix::KILO,
            Parameter::FromVLSIRSIPrefix(vlsir::utils::SIPrefix::KILO));
  EXPECT_EQ(Parameter::SIUnitPrefix::YOCTO,
            Parameter::FromVLSIRSIPrefix(vlsir::utils::SIPrefix::YOCTO));
}

TEST(ParameterTest, FromVLSIRParameter_Int64) {
  vlsir::utils::Param param_pb;
  param_pb.set_name("l");
  param_pb.set_desc("length");
  param_pb.mutable_value()->set_int64_value(150);

  Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
  EXPECT_EQ("l", parameter.name);
  EXPECT_EQ("length", parameter.description);
  EXPECT_EQ(150, std::get<int64_t>(parameter.value));
}

TEST(ParameterTest, FromVLSIRParameter_Double) {
  vlsir::utils::Param param_pb;
  param_pb.set_name("ratio");
  param_pb.mutable_value()->set_double_value(0.25);

  Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
  EXPECT_DOUBLE_EQ(0.25, std::get<double>(parameter.value));
}

TEST(ParameterTest, FromVLSIRParameter_String) {
  vlsir::utils::Param param_pb;
  param_pb.set_name("model");
  param_pb.mutable_value()->set_string_value("sky130_fd_pr__nfet_01v8");

  Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
  EXPECT_EQ("sky130_fd_pr__nfet_01v8", std::get<std::string>(parameter.value));
}

TEST(ParameterTest, FromVLSIRParameter_Literal) {
  vlsir::utils::Param param_pb;
  param_pb.set_name("expr");
  param_pb.mutable_value()->set_literal("w*2");

  Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
  EXPECT_EQ("w*2", std::get<std::string>(parameter.value));
}

TEST(ParameterTest, FromVLSIRParameter_PrefixedInt64) {
  vlsir::utils::Param param_pb;
  param_pb.set_name("w");
  param_pb.mutable_value()->mutable_prefixed()->set_prefix(
      vlsir::utils::SIPrefix::MICRO);
  param_pb.mutable_value()->mutable_prefixed()->set_int64_value(3);

  Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
  EXPECT_EQ(Parameter::SIUnitPrefix::MICRO, parameter.unit_prefix);
  EXPECT_EQ(3, std::get<int64_t>(parameter.value));
}

TEST(ParameterTest, FromVLSIRParameter_PrefixedDouble) {
  vlsir::utils::Param param_pb;
  param_pb.set_name("c");
  param_pb.mutable_value()->mutable_prefixed()->set_prefix(
      vlsir::utils::SIPrefix::FEMTO);
  param_pb.mutable_value()->mutable_prefixed()->set_double_value(1.5);

  Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
  EXPECT_EQ(Parameter::SIUnitPrefix::FEMTO, parameter.unit_prefix);
  EXPECT_DOUBLE_EQ(1.5, std::get<double>(parameter.value));
}

TEST(ParameterTest, FromVLSIRParameter_PrefixedString) {
  vlsir::utils::Param param_pb;
  param_pb.set_name("t");
  param_pb.mutable_value()->mutable_prefixed()->set_prefix(
      vlsir::utils::SIPrefix::PICO);
  param_pb.mutable_value()->mutable_prefixed()->set_string_value("10");

  Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
  EXPECT_EQ(Parameter::SIUnitPrefix::PICO, parameter.unit_prefix);
  EXPECT_EQ("10", std::get<std::string>(parameter.value));
}

TEST(ParameterTest, ToVLSIRParameter_Unprefixed) {
  Parameter parameter = Parameter::FromInteger(
      "nf", 2, Parameter::SIUnitPrefix::NONE);
  parameter.description = "fingers";

  vlsir::utils::Param param_pb = parameter.ToVLSIRParameter();
  EXPECT_EQ("nf", param_pb.name());
  EXPECT_EQ("fingers", param_pb.desc());
  ASSERT_EQ(vlsir::utils::ParamValue::ValueCase::kInt64Value,
            param_pb.value().value_case());
  EXPECT_EQ(2, param_pb.value().int64_value());
}

TEST(ParameterTest, ToVLSIRParameter_Prefixed) {
  Parameter parameter = Parameter::FromInteger(
      "w", 650, Parameter::SIUnitPrefix::NANO);

  vlsir::utils::Param param_pb = parameter.ToVLSIRParameter();
  ASSERT_EQ(vlsir::utils::ParamValue::ValueCase::kPrefixed,
            param_pb.value().value_case());
  EXPECT_EQ(vlsir::utils::SIPrefix::NANO, param_pb.value().prefixed().prefix());
  ASSERT_EQ(vlsir::utils::Prefixed::NumberCase::kInt64Value,
            param_pb.value().prefixed().number_case());
  EXPECT_EQ(650, param_pb.value().prefixed().int64_value());
}

TEST(ParameterTest, ToVLSIRParameter_PrefixedDoubleAndString) {
  Parameter parameter = Parameter::FromInteger(
      "x", 0, Parameter::SIUnitPrefix::MILLI);

  parameter.value = 2.5;
  vlsir::utils::Param param_pb = parameter.ToVLSIRParameter();
  EXPECT_DOUBLE_EQ(2.5, param_pb.value().prefixed().double_value());

  parameter.value = std::string("3");
  param_pb = parameter.ToVLSIRParameter();
  EXPECT_EQ("3", param_pb.value().prefixed().string_value());
}

TEST(ParameterTest, RoundTripPrefixed) {
  Parameter original = Parameter::FromInteger(
      "l", 150, Parameter::SIUnitPrefix::NANO);
  Parameter copy = Parameter::FromVLSIRParameter(original.ToVLSIRParameter());
  EXPECT_EQ(original.name, copy.name);
  EXPECT_EQ(original.unit_prefix, copy.unit_prefix);
  EXPECT_EQ(original.value, copy.value);
}

// TODO(aryap): FromVLSIRParameter leaves unit_prefix uninitialised when the
// value is not prefixed, so this reads an indeterminate value. Enable once
// unit_prefix defaults to NONE.
TEST(ParameterTest, DISABLED_FromVLSIRParameter_UnprefixedHasNoUnit) {
  vlsir::utils::Param param_pb;
  param_pb.set_name("nf");
  param_pb.mutable_value()->set_int64_value(2);

  Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
  EXPECT_EQ(Parameter::SIUnitPrefix::NONE, parameter.unit_prefix);
}

}  // namespace
}  // namespace bfg

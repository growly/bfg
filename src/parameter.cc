#include "parameter.h"

#include <string>

namespace bfg {

Parameter Parameter::FromInteger(
    const std::string &name, const int64_t value, const SIUnitPrefix unit) {
  return Parameter{
      .name = name,
      .description = "",
      .unit_prefix = unit,
      .value = value
  };
}

// Google style-guide says we're better off using an old-school array for
// static data so that it is trivially destructible. Since the data is small,
// a linear search is efficient enough.
//
// https://google.github.io/styleguide/cppguide.html#Static_and_Global_Variables
Parameter::SIUnitPrefix Parameter::FromVLSIRSIPrefix(
    const vlsir::utils::SIPrefix &prefix) {
  for (size_t i = 0; i < vlsir::utils::SIPrefix_ARRAYSIZE; ++i) {
    if (kToVLSIRPrefixMapping[i].second == prefix) {
      return kToVLSIRPrefixMapping[i].first;
    }
  }
  LOG(FATAL) << "Cannot convert from VLSIR SI Prefix: mapping not found for "
             << prefix << " (" << vlsir::utils::SIPrefix_Name(prefix) << ")";
  return NONE;
}

vlsir::utils::SIPrefix Parameter::ToVLSIRSIPrefix(
    const Parameter::SIUnitPrefix &prefix) {
  for (size_t i = 0; i < Parameter::SIUnitPrefix::MAX_VALUE; ++i) {
    if (kToVLSIRPrefixMapping[i].first == prefix) {
      return kToVLSIRPrefixMapping[i].second;
    }
  }
  LOG(FATAL) << "Cannot convert to VLSIR SI Prefix: mapping not found for "
             << prefix;
  return vlsir::utils::SIPrefix::DECI;
}

Parameter Parameter::FromVLSIRParameter(const vlsir::utils::Param &param_pb) {
  Parameter parameter;
  parameter.name = param_pb.name();
  parameter.description = param_pb.desc();
  const vlsir::utils::ParamValue &value = param_pb.value();
  switch (param_pb.value().value_case()) {
    case vlsir::utils::ParamValue::ValueCase::kInteger:
      parameter.value = value.integer();
      break;
    case vlsir::utils::ParamValue::ValueCase::kFloatingPoint:
      parameter.value = value.floating_point();
      break;
    case vlsir::utils::ParamValue::ValueCase::kString:
      parameter.value = value.string();
      break;
    case vlsir::utils::ParamValue::ValueCase::kLiteral:
      parameter.value = value.literal();
      break;
    case vlsir::utils::ParamValue::ValueCase::kPrefixed:
      const vlsir::utils::Prefixed &prefixed = param_pb.value().prefixed();
      parameter.unit_prefix = FromVLSIRSIPrefix(prefixed.prefix());
      switch (prefixed.number_case()) {
        case vlsir::utils::Prefixed::NumberCase::kInteger:
          parameter.value = prefixed.integer();
          break;
        case vlsir::utils::Prefixed::NumberCase::kFloatingPoint:
          parameter.value = prefixed.floating_point();
          break;
        case vlsir::utils::Prefixed::NumberCase::kString:
          parameter.value = prefixed.string();
          break;
      }
      break;
  }
  return parameter;
}

vlsir::utils::Param Parameter::ToVLSIRParameter() const {
  vlsir::utils::Param param_pb;
  param_pb.set_name(name);
  param_pb.set_desc(description);
  if (unit_prefix != SIUnitPrefix::NONE) {
    param_pb.mutable_value()->mutable_prefixed()->set_prefix(
        ToVLSIRSIPrefix(unit_prefix));
    switch (value.index()) {
      case 0:
        param_pb.mutable_value()->mutable_prefixed()->set_integer(
            std::get<int64_t>(value));
        break;
      case 1:
        param_pb.mutable_value()->mutable_prefixed()->set_floating_point(
            std::get<double>(value));
        break;
      case 2:
        param_pb.mutable_value()->mutable_prefixed()->set_string(
            std::get<std::string>(value));
        break;
      default:
        LOG(FATAL) << "Unexpected variant index " << value.index();
        break;
    }
    return param_pb;
  }
  switch (value.index()) {
    case 0:
      param_pb.mutable_value()->set_integer(std::get<int64_t>(value));
      break;
    case 1:
      param_pb.mutable_value()->set_floating_point(std::get<double>(value));
      break;
    case 2:
      param_pb.mutable_value()->set_string(std::get<std::string>(value));
      break;
    default:
      LOG(FATAL) << "Unexpected variant index " << value.index();
      break;
  }
  return param_pb;
}


}  // namespace bfg

#ifndef PARAMETER_H_
#define PARAMETER_H_

#include <string>
#include <variant>

#include <glog/logging.h>

#include "vlsir/utils.pb.h"

namespace bfg {

struct Parameter {
  enum SIUnitPrefix {
    NONE,    // E0
    YOCTO,   // E-24
    ZEPTO,   // E-21
    ATTO,    // E-18
    FEMTO,   // E-15
    PICO,    // E-12
    NANO,    // E-9
    MICRO,   // E-6
    MILLI,   // E-3
    CENTI,   // E-2
    DECI,    // E-1
    DECA,    // E1
    HECTO,   // E2
    KILO,    // E3
    MEGA,    // E6
    GIGA,    // E9
    TERA,    // E12
    PETA,    // E15
    EXA,     // E18
    ZETTA,   // E21
    YOTTA,   // E24
    MAX_VALUE
  };

  // The alternative to std::variant is to manually
  //    1) define the destructor for the union as doing nothing;
  //    2) call the string's deconstructor in the Parameter destructor if we
  //    know it's active.
  //
  // enum Type {
  //   INTEGER,
  //   FLOATING_POINT,
  //   LITERAL
  // };
  //
  // union Value {
  //   int64_t integer;
  //   double floating_point;
  //   std::string literal;
  //   ~Value() {}
  // };
  //
  // ~Parameter() {
  //   if (type == LITERAL) {
  //     value.literal.~string();
  //   }
  // }
  //
  // but std::variant does this all so who cares.

  static SIUnitPrefix FromVLSIRSIPrefix(const vlsir::utils::SIPrefix &prefix);
  static vlsir::utils::SIPrefix ToVLSIRSIPrefix(const SIUnitPrefix &prefix);

  static Parameter FromVLSIRParameter(const vlsir::utils::Param &param_pb);
  vlsir::utils::Param ToVLSIRParameter() const;

  static constexpr std::pair<Parameter::SIUnitPrefix, vlsir::utils::SIPrefix>
      kToVLSIRPrefixMapping[] = {
    {Parameter::SIUnitPrefix::YOCTO, vlsir::utils::SIPrefix::YOCTO},
    {Parameter::SIUnitPrefix::ZEPTO, vlsir::utils::SIPrefix::ZEPTO},
    {Parameter::SIUnitPrefix::ATTO,  vlsir::utils::SIPrefix::ATTO},
    {Parameter::SIUnitPrefix::FEMTO, vlsir::utils::SIPrefix::FEMTO},
    {Parameter::SIUnitPrefix::PICO,  vlsir::utils::SIPrefix::PICO},
    {Parameter::SIUnitPrefix::NANO,  vlsir::utils::SIPrefix::NANO},
    {Parameter::SIUnitPrefix::MICRO, vlsir::utils::SIPrefix::MICRO},
    {Parameter::SIUnitPrefix::MILLI, vlsir::utils::SIPrefix::MILLI},
    {Parameter::SIUnitPrefix::CENTI, vlsir::utils::SIPrefix::CENTI},
    {Parameter::SIUnitPrefix::DECI,  vlsir::utils::SIPrefix::DECI},
    {Parameter::SIUnitPrefix::DECA,  vlsir::utils::SIPrefix::DECA},
    {Parameter::SIUnitPrefix::HECTO, vlsir::utils::SIPrefix::HECTO},
    {Parameter::SIUnitPrefix::KILO,  vlsir::utils::SIPrefix::KILO},
    {Parameter::SIUnitPrefix::MEGA,  vlsir::utils::SIPrefix::MEGA},
    {Parameter::SIUnitPrefix::GIGA,  vlsir::utils::SIPrefix::GIGA},
    {Parameter::SIUnitPrefix::TERA,  vlsir::utils::SIPrefix::TERA},
    {Parameter::SIUnitPrefix::PETA,  vlsir::utils::SIPrefix::PETA},
    {Parameter::SIUnitPrefix::EXA,   vlsir::utils::SIPrefix::EXA},
    {Parameter::SIUnitPrefix::ZETTA, vlsir::utils::SIPrefix::ZETTA},
    {Parameter::SIUnitPrefix::YOTTA, vlsir::utils::SIPrefix::YOTTA}
  };

  std::string name;
  std::string description;

  SIUnitPrefix unit_prefix;
  std::variant<int64_t, double, std::string> value;
};

}  // namespace bfg

#endif  // PARAMETER_H_

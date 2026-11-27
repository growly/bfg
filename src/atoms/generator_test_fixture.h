#ifndef ATOMS_GENERATOR_TEST_FIXTURE_H_
#define ATOMS_GENERATOR_TEST_FIXTURE_H_

#include <gtest/gtest.h>

#include <fstream>
#include <set>
#include <string>

#include "../cell.h"
#include "../circuit.h"
#include "../design_database.h"
#include "../dev_pdk_setup.h"
#include "../geometry/rectangle.h"
#include "../layout.h"
#include "../physical_properties_database.h"
#include "vlsir/circuit.pb.h"

namespace bfg {

// Sets up a DesignDatabase the way main() does for Sky130 and checks the basic
// invariants every generated cell should satisfy.
class GeneratorTest : public testing::Test {
 protected:
  void SetUp() override {
    PhysicalPropertiesDatabase &physical_db = design_db_.physical_db();
    LoadPackage("test_data/sky130.primitives.pb");
    physical_db.LoadTechnologyFromFile("test_data/sky130.technology.pb");
    SetUpSky130(&physical_db);
    LoadPackage("test_data/sky130hd.pb");
  }

  void LoadPackage(const std::string &path) {
    vlsir::circuit::Package package_pb;
    std::fstream input(path, std::ios::in | std::ios::binary);
    ASSERT_TRUE(input) << "Could not open " << path;
    ASSERT_TRUE(package_pb.ParseFromIstream(&input)) << "Could not parse "
                                                     << path;
    design_db_.LoadPackage(package_pb);
  }

  template<typename Generator>
  Cell *Generate(const typename Generator::Parameters &parameters,
                 const std::string &name) {
    Generator generator(parameters, &design_db_);
    return generator.GenerateIntoDatabase(name);
  }

  // The port names of the cell's circuit.
  static std::set<std::string> CircuitPortNames(const Cell &cell) {
    std::set<std::string> names;
    vlsir::circuit::Module module_pb = cell.circuit()->ToVLSIRModule();
    for (const auto &port : module_pb.ports()) {
      names.insert(port.signal());
    }
    return names;
  }

  // Checks that the cell has a non-empty layout.
  static void ExpectLayout(const Cell *cell) {
    ASSERT_NE(nullptr, cell);
    ASSERT_NE(nullptr, cell->layout());
    geometry::Rectangle bounds = cell->layout()->GetBoundingBox();
    EXPECT_GT(bounds.Width(), 0);
    EXPECT_GT(bounds.Height(), 0);
  }

  // Checks that the cell has a non-empty layout and a circuit with ports, and
  // that every circuit port in `expected_pins` has a pin in the layout.
  static void ExpectWellFormed(const Cell *cell,
                               const std::set<std::string> &expected_pins) {
    ExpectLayout(cell);
    if (testing::Test::HasFatalFailure()) {
      return;
    }

    ASSERT_NE(nullptr, cell->circuit());
    std::set<std::string> ports = CircuitPortNames(*cell);
    EXPECT_FALSE(ports.empty());
    for (const std::string &pin : expected_pins) {
      EXPECT_EQ(1, ports.count(pin)) << "circuit has no port " << pin;
      EXPECT_TRUE(cell->layout()->HasPort(pin)) << "layout has no pin " << pin;
    }
  }

  DesignDatabase design_db_;
};

}  // namespace bfg

#endif  // ATOMS_GENERATOR_TEST_FIXTURE_H_

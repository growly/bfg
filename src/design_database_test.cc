#include "design_database.h"

#include <gtest/gtest.h>

#include <fstream>
#include <string>
#include <vector>

#include "cell.h"
#include "circuit.h"
#include "dev_pdk_setup.h"
#include "geometry/instance.h"
#include "layout.h"
#include "vlsir/circuit.pb.h"
#include "vlsir/layout/raw.pb.h"

namespace bfg {
namespace {

class DesignDatabaseTest : public testing::Test {
 protected:
  void SetUp() override {
    // Writing layouts needs real layer definitions.
    design_db_.physical_db().LoadTechnologyFromFile(
        "test_data/sky130.technology.pb");
    SetUpSky130(&design_db_.physical_db());
  }

  // A cell with an empty layout, owned by the database.
  Cell *MakeCell(const std::string &name) {
    Cell *cell = new Cell(name);
    cell->SetLayout(new Layout(design_db_.physical_db()));
    EXPECT_TRUE(design_db_.ConsumeCell(cell));
    return cell;
  }

  static void Instantiate(Cell *parent, Cell *child, const std::string &name) {
    geometry::Instance instance(child->layout(), {0, 0});
    instance.set_name(name);
    parent->layout()->AddInstance(instance);
  }

  DesignDatabase design_db_;
};

TEST_F(DesignDatabaseTest, ConsumeAndFindCell) {
  Cell *cell = MakeCell("a");
  EXPECT_EQ(cell, design_db_.FindCell("", "a"));
  EXPECT_EQ(cell, design_db_.FindCellOrDie("a"));
  EXPECT_EQ(nullptr, design_db_.FindCell("", "b"));
  EXPECT_EQ(nullptr, design_db_.FindCell("other_domain", "a"));

  // The (domain, name) pair is taken.
  Cell duplicate("a");
  EXPECT_FALSE(design_db_.ConsumeCell(&duplicate));
  EXPECT_EQ(cell, design_db_.FindCell("", "a"));
}

TEST_F(DesignDatabaseTest, Domains) {
  Cell *cell = new Cell("a");
  cell->set_domain("lib");
  ASSERT_TRUE(design_db_.ConsumeCell(cell));
  EXPECT_EQ(cell, design_db_.FindCellOrDie("lib", "a"));
  EXPECT_EQ(nullptr, design_db_.FindCell("", "a"));
  EXPECT_EQ(1, design_db_.cells().count("lib"));
}

TEST_F(DesignDatabaseTest, FindCellOrDie) {
  MakeCell("a");
  EXPECT_DEATH(design_db_.FindCellOrDie("missing"), "not found");
  EXPECT_DEATH(design_db_.FindCellOrDie("no_such_domain", "a"),
               "no entry for that domain");
}

TEST_F(DesignDatabaseTest, LoadPackage) {
  vlsir::circuit::Package package_pb;
  vlsir::circuit::Module *module_pb = package_pb.add_modules();
  module_pb->set_name("mod");
  module_pb->add_signals()->set_name("x");
  module_pb->add_ports()->set_signal("x");
  vlsir::circuit::ExternalModule *external_pb = package_pb.add_ext_modules();
  external_pb->mutable_name()->set_domain("lib");
  external_pb->mutable_name()->set_name("ext");

  design_db_.LoadPackage(package_pb);

  Cell *mod = design_db_.FindCell("", "mod");
  ASSERT_NE(nullptr, mod);
  ASSERT_NE(nullptr, mod->circuit());
  EXPECT_NE(nullptr, mod->circuit()->GetSignal("x"));
  EXPECT_FALSE(mod->is_external());

  Cell *ext = design_db_.FindCell("lib", "ext");
  ASSERT_NE(nullptr, ext);
  EXPECT_TRUE(ext->is_external());
}

TEST_F(DesignDatabaseTest, WriteTopOrdersDependenciesFirst) {
  // top -> mid -> leaf, and top -> leaf directly.
  Cell *leaf = MakeCell("leaf");
  Cell *mid = MakeCell("mid");
  Cell *top = MakeCell("top");
  MakeCell("unused");
  Instantiate(mid, leaf, "l0");
  Instantiate(top, mid, "m0");
  Instantiate(top, leaf, "l1");

  std::string library_path = testing::TempDir() + "/write_top.library.pb";
  std::string package_path = testing::TempDir() + "/write_top.package.pb";
  design_db_.WriteTop("top", library_path, package_path, false);

  vlsir::raw::Library library_pb;
  std::fstream input(library_path, std::ios::in | std::ios::binary);
  ASSERT_TRUE(input);
  ASSERT_TRUE(library_pb.ParseFromIstream(&input));

  std::vector<std::string> names;
  for (const auto &cell_pb : library_pb.cells()) {
    names.push_back(cell_pb.name());
  }
  std::vector<std::string> expected = {"leaf", "mid", "top"};
  EXPECT_EQ(expected, names);
}

TEST_F(DesignDatabaseTest, Describe) {
  MakeCell("a");
  EXPECT_NE(std::string::npos, design_db_.Describe().find("\ta"));
}

}  // namespace
}  // namespace bfg

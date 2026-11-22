#include "checkerboard_guide.h"

#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "cell.h"
#include "circuit.h"
#include "design_database.h"
#include "geometry/instance.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "layout.h"

namespace bfg {
namespace {

using geometry::Instance;
using geometry::Point;
using geometry::Rectangle;

class CheckerboardGuideTest : public testing::Test {
 protected:
  void SetUp() override {
    layout_.reset(new Layout(design_db_.physical_db()));
    small_ = MakeCell(100, 50);
    large_ = MakeCell(200, 80);
  }

  std::unique_ptr<Cell> MakeCell(int64_t width, int64_t height) {
    auto cell = std::make_unique<Cell>();
    Layout *layout = cell->SetLayout(new Layout(design_db_.physical_db()));
    layout->AddRectangle(Rectangle({0, 0}, {width, height}));
    cell->SetCircuit(new Circuit());
    return cell;
  }

  DesignDatabase design_db_;
  std::unique_ptr<Layout> layout_;
  std::unique_ptr<Cell> small_;
  std::unique_ptr<Cell> large_;
};

// The 2 x 2 arrangement LutB uses.
TEST_F(CheckerboardGuideTest, TwoByTwo) {
  std::vector<Cell*> templates = {small_.get()};
  CheckerboardGuide guide({0, 0}, "m", 2, 2, layout_.get(), nullptr,
                          &design_db_);
  guide.set_template_cells(&templates);

  const std::vector<Instance*> &instances = guide.InstantiateAll();
  ASSERT_EQ(2, instances.size());
  EXPECT_EQ(Point(0, 0), instances[0]->lower_left());
  EXPECT_EQ(Point(100, 50), instances[1]->lower_left());
  EXPECT_EQ("m_0", instances[0]->name());
  EXPECT_EQ("m_1", instances[1]->name());
  EXPECT_EQ(Rectangle({0, 0}, {200, 100}), *guide.GetBoundingBox());
}

TEST_F(CheckerboardGuideTest, FlipHorizontal) {
  std::vector<Cell*> templates = {small_.get()};
  CheckerboardGuide guide({0, 0}, "m", 2, 2, layout_.get(), nullptr,
                          &design_db_);
  guide.set_template_cells(&templates);
  guide.set_flip_horizontal(true);

  const std::vector<Instance*> &instances = guide.InstantiateAll();
  ASSERT_EQ(2, instances.size());
  EXPECT_EQ(Point(100, 0), instances[0]->lower_left());
  EXPECT_EQ(Point(0, 50), instances[1]->lower_left());
}

TEST_F(CheckerboardGuideTest, Overlaps) {
  std::vector<Cell*> templates = {small_.get()};
  CheckerboardGuide guide({10, 20}, "m", 2, 2, layout_.get(), nullptr,
                          &design_db_);
  guide.set_template_cells(&templates);
  // Negative overlaps space the cells apart.
  guide.set_horizontal_overlap(-30);
  guide.set_vertical_overlap(-5);

  const std::vector<Instance*> &instances = guide.InstantiateAll();
  ASSERT_EQ(2, instances.size());
  EXPECT_EQ(Point(10, 20), instances[0]->lower_left());
  EXPECT_EQ(Point(140, 75), instances[1]->lower_left());
}

TEST_F(CheckerboardGuideTest, TemplatesAlternate) {
  std::vector<Cell*> templates = {small_.get(), large_.get()};
  CheckerboardGuide guide({0, 0}, "m", 2, 2, layout_.get(), nullptr,
                          &design_db_);
  guide.set_template_cells(&templates);

  const std::vector<Instance*> &instances = guide.InstantiateAll();
  ASSERT_EQ(2, instances.size());
  EXPECT_EQ(small_->layout(), instances[0]->template_layout());
  EXPECT_EQ(large_->layout(), instances[1]->template_layout());
  // Columns are as wide as their widest cell.
  EXPECT_EQ(Point(100, 50), instances[1]->lower_left());
}

TEST_F(CheckerboardGuideTest, CircuitInstances) {
  Circuit circuit;
  std::vector<Cell*> templates = {small_.get()};
  CheckerboardGuide guide({0, 0}, "m", 2, 2, layout_.get(), &circuit,
                          &design_db_);
  guide.set_template_cells(&templates);
  const std::vector<Instance*> &instances = guide.InstantiateAll();
  ASSERT_EQ(2, circuit.instances().size());
  EXPECT_EQ(circuit.instances().front().get(),
            instances.front()->circuit_instance());
}

TEST_F(CheckerboardGuideTest, EmptyBeforeInstantiation) {
  CheckerboardGuide guide({0, 0}, "m", 2, 2, layout_.get(), nullptr,
                          &design_db_);
  EXPECT_FALSE(guide.GetBoundingBox());
}

// TODO(aryap): With an odd number of columns the row-to-row alternation of
// (i * num_columns + j) and the per-row flip cancel out, producing stripes
// instead of the checkerboard drawn in the class comment.
TEST_F(CheckerboardGuideTest, DISABLED_ThreeByThree) {
  std::vector<Cell*> templates = {small_.get()};
  CheckerboardGuide guide({0, 0}, "m", 3, 3, layout_.get(), nullptr,
                          &design_db_);
  guide.set_template_cells(&templates);

  std::set<std::pair<int64_t, int64_t>> cells;
  for (Instance *instance : guide.InstantiateAll()) {
    cells.insert({instance->lower_left().x() / 100,
                  instance->lower_left().y() / 50});
  }
  std::set<std::pair<int64_t, int64_t>> expected = {
      {0, 0}, {2, 0}, {1, 1}, {0, 2}, {2, 2}};
  EXPECT_EQ(expected, cells);
}

}  // namespace
}  // namespace bfg

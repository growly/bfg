#include "row_guide.h"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include <absl/strings/str_cat.h>

#include "cell.h"
#include "circuit.h"
#include "design_database.h"
#include "geometry/instance.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "layout.h"
#include "physical_properties_database.h"

namespace bfg {
namespace {

using geometry::Instance;
using geometry::Point;
using geometry::Rectangle;

class RowGuideTest : public testing::Test {
 protected:
  void SetUp() override {
    narrow_ = MakeTemplate(100, 200);
    wide_ = MakeTemplate(300, 200);
    tall_ = MakeTemplate(100, 400);
    tap_layout_ = MakeTemplate(50, 200);
    tap_cell_.SetLayout(tap_layout_.release());
    layout_.reset(new Layout(design_db_.physical_db()));
  }

  // A template whose tiling bounds are width x height from the origin, with a
  // rectangle filling them.
  std::unique_ptr<Layout> MakeTemplate(int64_t width, int64_t height) {
    auto layout = std::make_unique<Layout>(design_db_.physical_db());
    layout->AddRectangle(Rectangle({0, 0}, {width, height}));
    layout->SetTilingBounds(Rectangle({0, 0}, {width, height}));
    return layout;
  }

  RowGuide MakeRow(const Point &origin) {
    return RowGuide(origin, layout_.get(), nullptr, &design_db_);
  }

  DesignDatabase design_db_;
  std::unique_ptr<Layout> narrow_;
  std::unique_ptr<Layout> wide_;
  std::unique_ptr<Layout> tall_;
  std::unique_ptr<Layout> tap_layout_;
  Cell tap_cell_;
  std::unique_ptr<Layout> layout_;
};

TEST_F(RowGuideTest, Empty) {
  RowGuide row = MakeRow({10, 20});
  EXPECT_TRUE(row.empty());
  EXPECT_EQ(0, row.Width());
  EXPECT_EQ(0, row.Height());
  EXPECT_FALSE(row.GetBoundingBox());
  EXPECT_FALSE(row.GetTilingBounds());
  EXPECT_EQ(Point(10, 20), row.LowerLeft());
  EXPECT_EQ(Point(10, 20), row.LowerRight());
}

TEST_F(RowGuideTest, InstantiateBack) {
  RowGuide row = MakeRow({10, 20});
  Instance *a = row.InstantiateBack("a", narrow_.get());
  Instance *b = row.InstantiateBack("b", wide_.get());
  Instance *c = row.InstantiateBack("c", narrow_.get());

  EXPECT_EQ(Point(10, 20), a->TilingLowerLeft());
  EXPECT_EQ(Point(110, 20), b->TilingLowerLeft());
  EXPECT_EQ(Point(410, 20), c->TilingLowerLeft());
  EXPECT_EQ("b", b->name());
  EXPECT_EQ((std::vector<Instance*>{a, b, c}), row.instances());
  EXPECT_EQ(3, layout_->instances().size());

  EXPECT_EQ(500, row.Width());
  EXPECT_EQ(200, row.Height());
  EXPECT_EQ(Rectangle({10, 20}, {510, 220}), *row.GetTilingBounds());
  EXPECT_EQ(Point(10, 20), row.LowerLeft());
  EXPECT_EQ(Point(510, 20), row.LowerRight());
  EXPECT_EQ(Point(10, 220), row.UpperLeft());
  EXPECT_EQ(Point(510, 220), row.UpperRight());
}

TEST_F(RowGuideTest, InstantiateFront) {
  RowGuide row = MakeRow({1000, 0});
  Instance *a = row.InstantiateBack("a", narrow_.get());
  Instance *b = row.InstantiateFront("b", wide_.get());

  // Existing instances stay put; the new one goes to their left.
  EXPECT_EQ(Point(1000, 0), a->TilingLowerLeft());
  EXPECT_EQ(Point(700, 0), b->TilingLowerLeft());
  EXPECT_EQ((std::vector<Instance*>{b, a}), row.instances());
}

TEST_F(RowGuideTest, InstantiateAndInsertFront) {
  RowGuide row = MakeRow({0, 0});
  Instance *a = row.InstantiateBack("a", narrow_.get());
  Instance *b = row.InstantiateAndInsertFront("b", wide_.get());

  // The new instance takes the front position; the others move right.
  EXPECT_EQ(Point(0, 0), b->TilingLowerLeft());
  EXPECT_EQ(Point(300, 0), a->TilingLowerLeft());
  EXPECT_EQ((std::vector<Instance*>{b, a}), row.instances());
}

TEST_F(RowGuideTest, BlankSpace) {
  RowGuide row = MakeRow({0, 0});
  row.AddBlankSpaceBack(50);
  Instance *a = row.InstantiateBack("a", narrow_.get());
  row.AddBlankSpaceBack(25);
  Instance *b = row.InstantiateBack("b", narrow_.get());
  EXPECT_EQ(Point(50, 0), a->TilingLowerLeft());
  EXPECT_EQ(Point(175, 0), b->TilingLowerLeft());

  row.AddBlankSpaceFront(40);
  Instance *c = row.InstantiateFront("c", narrow_.get());
  EXPECT_EQ(Point(-90, 0), c->TilingLowerLeft());

  row.AddBlankSpaceBack(10);
  EXPECT_EQ(Point(285, 0), row.LowerRight());
}

TEST_F(RowGuideTest, AddBlankSpaceAndInsertFront) {
  RowGuide row = MakeRow({0, 0});
  Instance *a = row.InstantiateBack("a", narrow_.get());
  row.AddBlankSpaceAndInsertFront(60);
  EXPECT_EQ(Point(60, 0), a->TilingLowerLeft());
}

TEST_F(RowGuideTest, RowsCentreInstancesVertically) {
  RowGuide row = MakeRow({0, 0});
  row.InstantiateBack("tall", tall_.get());
  Instance *narrow = row.InstantiateBack("narrow", narrow_.get());
  // The narrow instance is 200 high next to one 400 high.
  EXPECT_EQ(Point(100, 100), narrow->TilingLowerLeft());
  EXPECT_EQ(400, row.Height());
  EXPECT_EQ(Rectangle({0, 0}, {200, 400}), *row.GetTilingBounds());
}

TEST_F(RowGuideTest, GetBoundingBox) {
  RowGuide row = MakeRow({0, 0});
  row.InstantiateBack("a", narrow_.get());
  row.InstantiateBack("b", wide_.get());
  EXPECT_EQ(Rectangle({0, 0}, {400, 200}), *row.GetBoundingBox());
}

// TODO(aryap): GetBoundingBox uses the lower left of the front instance and the
// upper right of the back instance, which is wrong when instances differ in
// height.
TEST_F(RowGuideTest, DISABLED_GetBoundingBox_DifferentHeights) {
  RowGuide row = MakeRow({0, 0});
  row.InstantiateBack("narrow", narrow_.get());
  row.InstantiateBack("tall", tall_.get());
  // The narrow instance is centred on the tall one, so the row spans the tall
  // one's full height.
  EXPECT_EQ(Rectangle({0, -100}, {200, 300}), *row.GetBoundingBox());
}

TEST_F(RowGuideTest, RotateInstances) {
  RowGuide row = MakeRow({0, 0});
  row.set_rotate_instances(true);
  EXPECT_EQ(180, row.RotationDegreesCCW());
  Instance *a = row.InstantiateBack("a", narrow_.get());
  Instance *b = row.InstantiateBack("b", wide_.get());
  EXPECT_EQ(180, a->rotation_degrees_ccw());
  // Rotation does not change where the instances' tiling bounds land.
  EXPECT_EQ(Rectangle({0, 0}, {100, 200}), a->GetTilingBounds());
  EXPECT_EQ(Rectangle({100, 0}, {400, 200}), b->GetTilingBounds());
}

TEST_F(RowGuideTest, MoveTo) {
  RowGuide row = MakeRow({0, 0});
  Instance *a = row.InstantiateBack("a", narrow_.get());
  Instance *b = row.InstantiateBack("b", narrow_.get());
  row.MoveTo({1000, 500});
  EXPECT_EQ(Point(1000, 500), row.origin());
  EXPECT_EQ(Point(1000, 500), a->TilingLowerLeft());
  EXPECT_EQ(Point(1100, 500), b->TilingLowerLeft());

  row.MoveLowerLeft({0, 0});
  EXPECT_EQ(Point(0, 0), row.LowerLeft());
  row.MoveLowerRight({0, 0});
  EXPECT_EQ(Point(0, 0), row.LowerRight());
  EXPECT_EQ(Point(-200, 0), a->TilingLowerLeft());
}

TEST_F(RowGuideTest, Taps) {
  RowGuide row = MakeRow({0, 0});
  row.set_tap_cell(tap_cell_);
  Instance *a = row.InstantiateBack("a", wide_.get());

  // Rows start with a tap.
  ASSERT_EQ(1, row.generated_taps().size());
  Instance *tap = row.generated_taps().front();
  EXPECT_EQ(Point(0, 0), tap->TilingLowerLeft());
  EXPECT_EQ(Point(50, 0), a->TilingLowerLeft());
  EXPECT_EQ((std::vector<Instance*>{tap, a}), row.instances());

  // Another tap is needed once the distance from the last one exceeds 15000.
  for (int i = 0; i < 60; ++i) {
    row.InstantiateBack(absl::StrCat("x", i), wide_.get());
  }
  EXPECT_EQ(2, row.generated_taps().size());
}

TEST_F(RowGuideTest, AvailableRightSpanUpTo) {
  RowGuide row = MakeRow({0, 0});
  row.set_tap_cell(tap_cell_);
  row.InstantiateBack("a", wide_.get());
  // 350 used so far (a tap and a). Nothing more needs a tap within 1000.
  EXPECT_EQ(650, row.AvailableRightSpanUpTo(1000));
  EXPECT_LT(row.AvailableRightSpanUpTo(100), 0);
  // Covering 20000 more needs another tap.
  EXPECT_EQ(20000 - 350 - 50, row.AvailableRightSpanUpTo(20000));
}

// TODO(aryap): AvailableRightSpanUpTo dereferences tap_cell_ without checking
// that one was set.
TEST_F(RowGuideTest, DISABLED_AvailableRightSpanUpTo_NoTapCell) {
  RowGuide row = MakeRow({0, 0});
  row.InstantiateBack("a", wide_.get());
  EXPECT_EQ(700, row.AvailableRightSpanUpTo(1000));
}

TEST_F(RowGuideTest, CircuitInstances) {
  Circuit circuit;
  RowGuide row(Point(0, 0), layout_.get(), &circuit, &design_db_);
  Cell master;
  master.SetLayout(MakeTemplate(100, 200).release());
  master.SetCircuit(new Circuit());

  Instance *a = row.InstantiateBack("a", &master);
  ASSERT_EQ(1, circuit.instances().size());
  EXPECT_EQ("a", circuit.instances().front()->name());
  EXPECT_EQ(circuit.instances().front().get(), a->circuit_instance());
}

}  // namespace
}  // namespace bfg

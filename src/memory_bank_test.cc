#include "memory_bank.h"

#include <gtest/gtest.h>

#include <memory>
#include <optional>

#include "cell.h"
#include "circuit.h"
#include "design_database.h"
#include "geometry/compass.h"
#include "geometry/instance.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "layout.h"

namespace bfg {
namespace {

using geometry::Compass;
using geometry::Instance;
using geometry::Point;
using geometry::Rectangle;

class MemoryBankTest : public testing::Test {
 protected:
  void SetUp() override {
    layout_.reset(new Layout(design_db_.physical_db()));
    narrow_ = MakeCell(100, 200);
    wide_ = MakeCell(300, 200);
    tall_ = MakeCell(100, 400);
  }

  // A cell whose tiling bounds are width x height from the origin.
  std::unique_ptr<Cell> MakeCell(int64_t width, int64_t height) {
    auto cell = std::make_unique<Cell>();
    Layout *layout = cell->SetLayout(new Layout(design_db_.physical_db()));
    layout->AddRectangle(Rectangle({0, 0}, {width, height}));
    layout->SetTilingBounds(Rectangle({0, 0}, {width, height}));
    return cell;
  }

  MemoryBank MakeBank(std::optional<Compass> alignment,
                      bool rotate_alternate_rows = false) {
    return MemoryBank(layout_.get(), nullptr, &design_db_, nullptr,
                      rotate_alternate_rows, false, alignment);
  }

  DesignDatabase design_db_;
  std::unique_ptr<Layout> layout_;
  std::unique_ptr<Cell> narrow_;
  std::unique_ptr<Cell> wide_;
  std::unique_ptr<Cell> tall_;
};

TEST_F(MemoryBankTest, Empty) {
  MemoryBank bank = MakeBank(std::nullopt);
  EXPECT_EQ(0, bank.NumRows());
  EXPECT_EQ(Point(0, 0), bank.Origin());
  EXPECT_FALSE(bank.GetBoundingBox());
  EXPECT_FALSE(bank.GetTilingBounds());
  bank.MoveTo({10, 10});  // No-op.
}

TEST_F(MemoryBankTest, RowCreatesIntermediateRows) {
  MemoryBank bank = MakeBank(std::nullopt);
  bank.Row(2);
  EXPECT_EQ(3, bank.NumRows());
  EXPECT_EQ(3, bank.instances().size());
  EXPECT_EQ(3, bank.instance_names().size());
  EXPECT_EQ(2, bank.LastRowIndex());
  EXPECT_EQ(&bank.rows().back(), &bank.LastRow());
}

TEST_F(MemoryBankTest, AlternateRowRotation) {
  MemoryBank bank = MakeBank(std::nullopt, true);
  bank.Row(3);
  EXPECT_FALSE(bank.Row(0).rotate_instances());
  EXPECT_TRUE(bank.Row(1).rotate_instances());
  EXPECT_FALSE(bank.Row(2).rotate_instances());
  EXPECT_TRUE(bank.Row(3).rotate_instances());
}

TEST_F(MemoryBankTest, RowsAbutVertically) {
  MemoryBank bank = MakeBank(std::nullopt);
  bank.InstantiateRight(0, "a", tall_.get());
  bank.InstantiateRight(1, "b", narrow_.get());
  bank.InstantiateRight(2, "c", narrow_.get());

  EXPECT_EQ(0, bank.Row(0).LowerLeft().y());
  EXPECT_EQ(400, bank.Row(1).LowerLeft().y());
  EXPECT_EQ(600, bank.Row(2).LowerLeft().y());
  EXPECT_EQ(Rectangle({0, 0}, {100, 800}), *bank.GetTilingBounds());
  EXPECT_EQ("b", bank.instance_names()[1].front());
}

TEST_F(MemoryBankTest, AlignRight) {
  MemoryBank bank = MakeBank(Compass::RIGHT);
  bank.InstantiateRight(0, "a", wide_.get());
  bank.InstantiateRight(1, "b", narrow_.get());
  EXPECT_EQ(bank.Row(0).LowerRight().x(), bank.Row(1).LowerRight().x());

  // Instantiating "inside" a right-aligned bank adds to the left.
  Instance *c = bank.InstantiateInside(1, "c", narrow_.get());
  EXPECT_EQ(c, bank.Row(1).instances().front());
  EXPECT_EQ(bank.Row(0).LowerRight().x(), bank.Row(1).LowerRight().x());
}

TEST_F(MemoryBankTest, AlignLeft) {
  MemoryBank bank = MakeBank(Compass::LEFT);
  bank.InstantiateRight(0, "a", wide_.get());
  bank.InstantiateLeft(1, "b", narrow_.get());
  EXPECT_EQ(bank.Row(0).LowerLeft().x(), bank.Row(1).LowerLeft().x());

  // Instantiating "inside" a left-aligned bank adds to the right.
  Instance *c = bank.InstantiateInside(0, "c", narrow_.get());
  EXPECT_EQ(c, bank.Row(0).instances().back());
}

TEST_F(MemoryBankTest, MoveTo) {
  MemoryBank bank = MakeBank(std::nullopt);
  bank.InstantiateRight(0, "a", narrow_.get());
  bank.InstantiateRight(1, "b", narrow_.get());
  bank.MoveTo({1000, 500});
  EXPECT_EQ(Point(1000, 500), bank.Origin());
  EXPECT_EQ(Point(1000, 700), bank.Row(1).LowerLeft());

  bank.AlignPointTo({1000, 500}, {0, 0});
  EXPECT_EQ(Point(0, 0), bank.Origin());
}

// TODO(aryap): AlignLeft and AlignRight start their search for the extreme x
// at 0 instead of at the first row, so an aligned bank that has been moved
// away from the origin snaps back to x = 0 the next time something is
// instantiated.
TEST_F(MemoryBankTest, DISABLED_AlignLeftAfterMove) {
  MemoryBank bank = MakeBank(Compass::LEFT);
  bank.InstantiateRight(0, "a", narrow_.get());
  bank.MoveTo({1000, 0});
  bank.InstantiateRight(1, "b", narrow_.get());
  EXPECT_EQ(1000, bank.Row(0).LowerLeft().x());
  EXPECT_EQ(1000, bank.Row(1).LowerLeft().x());
}

TEST_F(MemoryBankTest, InsertRowZero) {
  MemoryBank bank = MakeBank(std::nullopt, true);
  bank.InstantiateRight(0, "a", narrow_.get());
  ASSERT_FALSE(bank.Row(0).rotate_instances());

  RowGuide &zero = bank.InsertRowZero();
  EXPECT_EQ(2, bank.NumRows());
  EXPECT_EQ(&zero, &bank.Row(0));
  // The new row goes below, and existing rows keep their rotation.
  EXPECT_EQ(-200, zero.origin().y());
  EXPECT_FALSE(bank.Row(1).rotate_instances());
  EXPECT_TRUE(bank.Row(0).rotate_instances());
  EXPECT_EQ("a", bank.instance_names()[1].front());
}

TEST_F(MemoryBankTest, Taps) {
  std::unique_ptr<Cell> tap = MakeCell(50, 200);
  MemoryBank bank(layout_.get(), nullptr, &design_db_, tap.get(), false, false,
                  std::nullopt);
  bank.InstantiateRight(0, "a", narrow_.get());
  EXPECT_EQ(1, bank.Row(0).generated_taps().size());

  bank.DisableTapInsertionOnRow(1);
  bank.InstantiateRight(1, "b", narrow_.get());
  EXPECT_TRUE(bank.Row(1).generated_taps().empty());

  bank.EnableTapInsertionOnRow(2);
  bank.InstantiateRight(2, "c", narrow_.get());
  EXPECT_EQ(1, bank.Row(2).generated_taps().size());
}

TEST_F(MemoryBankTest, GetBoundingBox) {
  MemoryBank bank = MakeBank(std::nullopt);
  bank.InstantiateRight(0, "a", wide_.get());
  bank.InstantiateRight(1, "b", narrow_.get());
  EXPECT_EQ(Rectangle({0, 0}, {300, 400}), *bank.GetBoundingBox());
}

}  // namespace
}  // namespace bfg

#include <gtest/gtest.h>

#include "vector.h"
#include "radian.h"

namespace bfg {
namespace geometry {
namespace {

TEST(VectorTest, DotProduct) {
  Vector a = {2, 3};
  Vector b = {3, 4};
  EXPECT_EQ(18, a.DotProduct(b));
  EXPECT_EQ(18, b.DotProduct(a));
}

TEST(VectorTest, Project) {
  Vector a = {3, 5};
  Vector b = {2, 0};
  Vector c = {0, 4};
  EXPECT_EQ(Vector(3, 0), b.Project(a));
  EXPECT_EQ(Vector(0, 5), c.Project(a));

  Vector d = {1, 1};
  EXPECT_EQ(Vector(1, 1), d.Project(b));
}

TEST(VectorTest, UnitVector) {
  Vector x_axis = Vector::UnitVector(0);
  EXPECT_EQ(Point(1, 0), x_axis);

  Vector y_axis = Vector::UnitVector(Radian::kPi / 2);
  EXPECT_EQ(Point(0, 1), y_axis);

  Vector reverse_x_axis = Vector::UnitVector(Radian::kPi);
  EXPECT_EQ(Point(-1, 0), reverse_x_axis);

  Vector reverse_y_axis = Vector::UnitVector(3 * Radian::kPi / 2);
  EXPECT_EQ(Point(0, -1), reverse_y_axis);
}


}  // namespace
}  // namespace geometry
}  // namespace bfg

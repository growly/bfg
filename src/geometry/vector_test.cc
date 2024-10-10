#include <gtest/gtest.h>

#include "vector.h"

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

}  // namespace
}  // namespace geometry
}  // namespace bfg

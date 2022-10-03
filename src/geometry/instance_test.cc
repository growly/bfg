#include <gtest/gtest.h>

#include "instance.h"
#include "../layout.h"
#include "../physical_properties_database.h"

namespace bfg {
namespace geometry {
namespace {

TEST(Rectangle, BoundingBoxOnRotation) {
  bfg::Layout template_layout = bfg::Layout(PhysicalPropertiesDatabase());
  template_layout.AddRectangle(Rectangle(Point(1, 0), Point(2, 1)));

  Instance instance(&template_layout, Point(0, 0));
  instance.set_rotation_clockwise_degrees(90);

  Rectangle bounding_box = instance.GetBoundingBox();

  //        1
  //      +--+ (2, 1)
  //    1 |  |
  //  O   +--+
  //    (1, 0)
  //
  //      | rotation 90 clockwise:
  //   <_/
  //
  //  O
  //
  //  +--+ (1, -1)
  //  |  |
  //  +--+
  //  (0, -2)

  EXPECT_TRUE(Rectangle(Point(0, -2), Point(1, -1)) == bounding_box);
}

}  // namespace
}  // namespace geometry
}  // namespace bfg

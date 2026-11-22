#include "layout.h"

#include <gtest/gtest.h>

#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "geometry/shape_collection.h"
#include "physical_properties_database.h"

namespace bfg {
namespace {

using geometry::Point;
using geometry::Rectangle;

TEST(LayoutTest, GetBoundingBox_Empty) {
  PhysicalPropertiesDatabase physical_db;
  Layout layout(physical_db);
  EXPECT_EQ(Rectangle({0, 0}, {0, 0}), layout.GetBoundingBox());
}

TEST(LayoutTest, GetBoundingBox_IgnoresEmptyLayers) {
  PhysicalPropertiesDatabase physical_db;
  Layout layout(physical_db);

  layout.set_active_layer(1);
  layout.AddRectangle(Rectangle({10, 10}, {20, 30}));

  // Leave an empty shape collection behind on layer 2.
  layout.set_active_layer(2);
  layout.AddRectangle(Rectangle({-100, -100}, {-90, -90}));
  layout.GetShapeCollection(2)->rectangles().clear();

  EXPECT_EQ(Rectangle({10, 10}, {20, 30}), layout.GetBoundingBox());
}

}  // namespace
}  // namespace bfg

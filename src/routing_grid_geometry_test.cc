#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glog/logging.h>

#include "geometry/rectangle.h"
#include "geometry/point.h"
#include "geometry/layer.h"
#include "geometry/line.h"
#include "routing_layer_info.h"
#include "routing_grid_geometry.h"
#include "physical_properties_database.h"

namespace bfg {
namespace geometry {
namespace {

using ::testing::ContainerEq;

TEST(RoutingGridGeometry, EnvelopingVertexIndices_Point) {
  RoutingLayerInfo horizontal = RoutingLayerInfo {
    .layer = 0,
    .area = Rectangle({0, 0}, {200, 200}),
    .wire_width = 1,
    .offset = 10,
    .direction = RoutingTrackDirection::kTrackHorizontal,
    .pitch = 10,
  };

  RoutingLayerInfo vertical = RoutingLayerInfo {
    .layer = 1,
    .area = Rectangle({0, 0}, {200, 200}),
    .wire_width = 1,
    .offset = 10,
    .direction = RoutingTrackDirection::kTrackVertical,
    .pitch = 10,
  };

  RoutingGridGeometry grid_geometry;
  grid_geometry.ComputeForLayers(horizontal, vertical);

  std::set<std::pair<size_t, size_t>> nearest_vertices;

  std::set<std::pair<size_t, size_t>> expected = {
      {1, 0},
      {2, 0},
      {1, 1},
      {2, 1}
  };
  grid_geometry.EnvelopingVertexIndices({25, 15}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {0, 0}
  };
  grid_geometry.EnvelopingVertexIndices({5, 5}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {0, 0}
  };
  grid_geometry.EnvelopingVertexIndices({0, 0}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {3, 3},
      {3, 4},
      {3, 5},
      {4, 3},
      {4, 4},
      {4, 5},
      {5, 3},
      {5, 4},
      {5, 5}
  };
  grid_geometry.EnvelopingVertexIndices({50, 50}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {19, 19}
  };
  grid_geometry.EnvelopingVertexIndices({210, 210}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {  // { x,  y}
      {0, 0},   // {10, 10}
      {0, 1},   // {10, 20}
      {1, 0},   // {20, 10}
      {1, 1},   // {20, 20}
      {2, 0},   // {30, 10}
      {2, 1}    // {30, 20}
  };
  grid_geometry.EnvelopingVertexIndices({20, 15}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {  // { x,  y}
      {1, 1},   // {20, 20}
      {1, 2},   // {20, 30}
      {1, 3},   // {20, 40}
      {2, 1},   // {30, 20}
      {2, 2},   // {30, 30}
      {2, 3}    // {30, 40}
  };
  grid_geometry.EnvelopingVertexIndices({25, 30}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {0, 0},  // {10, 10}
      {1, 0},  // {20, 10}
      {2, 0},  // {30, 10}
  };
  grid_geometry.EnvelopingVertexIndices({20, 0}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {19, 8},
      {19, 9},
      {19, 10},
  };
  grid_geometry.EnvelopingVertexIndices({205, 100}, &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();
}

TEST(RoutingGridGeometry, EnvelopingVertexIndices_Point_2ConcentricLayers) {
  RoutingLayerInfo horizontal = RoutingLayerInfo {
    .layer = 0,
    .area = Rectangle({0, 0}, {200, 200}),
    .wire_width = 1,
    .offset = 10,
    .direction = RoutingTrackDirection::kTrackHorizontal,
    .pitch = 10,
  };

  RoutingLayerInfo vertical = RoutingLayerInfo {
    .layer = 1,
    .area = Rectangle({0, 0}, {200, 200}),
    .wire_width = 1,
    .offset = 10,
    .direction = RoutingTrackDirection::kTrackVertical,
    .pitch = 10,
  };

  RoutingGridGeometry grid_geometry;
  grid_geometry.ComputeForLayers(horizontal, vertical);

  std::set<std::pair<size_t, size_t>> nearest_vertices;

  std::set<std::pair<size_t, size_t>> expected = {
      {0, 0},
      {1, 0},
      {2, 0},
      {3, 0},
      {0, 1},
      {1, 1},
      {2, 1},
      {3, 1},
      {0, 2},
      {1, 2},
      {2, 2},
      {3, 2}
  };
  grid_geometry.EnvelopingVertexIndices(
      {25, 15},
      &nearest_vertices,
      0,    // No padding.
      2);   // 2 concentric layers, one more than the default of 1.
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {0, 0},
      {1, 0},
      {0, 1},
      {1, 1}
  };
  grid_geometry.EnvelopingVertexIndices({5, 5}, &nearest_vertices, 0, 2);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {0, 0},
      {1, 0},
      {0, 1},
      {1, 1}
  };
  grid_geometry.EnvelopingVertexIndices({0, 0}, &nearest_vertices, 0, 2);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {2, 2},
      {2, 3},
      {2, 4},
      {2, 5},
      {2, 6},
      {3, 2},
      {3, 3},
      {3, 4},
      {3, 5},
      {3, 6},
      {4, 2},
      {4, 3},
      {4, 4},
      {4, 5},
      {4, 6},
      {5, 2},
      {5, 3},
      {5, 4},
      {5, 5},
      {5, 6},
      {6, 2},
      {6, 3},
      {6, 4},
      {6, 5},
      {6, 6},
  };
  grid_geometry.EnvelopingVertexIndices({50, 50}, &nearest_vertices, 0, 2);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {18, 18},
      {18, 19},
      {19, 19},
      {19, 18}
  };
  grid_geometry.EnvelopingVertexIndices({210, 210}, &nearest_vertices, 0, 2);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {  // { x,  y}
      {0, 0},   // {10, 10}
      {0, 1},   // {10, 20}
      {0, 2},   // {10, 30}
      {1, 0},   // {20, 10}
      {1, 1},   // {20, 20}
      {1, 2},   // {20, 30}
      {2, 0},   // {30, 10}
      {2, 1},   // {30, 20}
      {2, 2},   // {30, 30}
      {3, 0},   // {40, 10}
      {3, 1},   // {40, 20}
      {3, 2}    // {40, 30}
  };
  grid_geometry.EnvelopingVertexIndices({20, 15}, &nearest_vertices, 0, 2);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {  // { x,  y}
      {0, 0},   // {10, 10}
      {0, 1},   // {10, 20}
      {0, 2},   // {10, 30}
      {0, 3},   // {10, 40}
      {0, 4},   // {10, 50}
      {1, 0},   // {20, 10}
      {1, 1},   // {20, 20}
      {1, 2},   // {20, 30}
      {1, 3},   // {20, 40}
      {1, 4},   // {20, 50}
      {2, 0},   // {30, 10}
      {2, 1},   // {30, 20}
      {2, 2},   // {30, 30}
      {2, 3},   // {30, 40}
      {2, 4},   // {30, 50}
      {3, 0},   // {40, 10}
      {3, 1},   // {40, 20}
      {3, 2},   // {40, 30}
      {3, 3},   // {40, 40}
      {3, 4}    // {40, 50}
  };
  grid_geometry.EnvelopingVertexIndices({25, 30}, &nearest_vertices, 0, 2);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {0, 0},  // {10, 10}
      {1, 0},  // {20, 10}
      {2, 0},  // {30, 10}
      {3, 0},  // {40, 10}
      {0, 1},  // {10, 20}
      {1, 1},  // {20, 20}
      {2, 1},  // {30, 20}
      {3, 1},  // {40, 20}
  };
  grid_geometry.EnvelopingVertexIndices({20, 0}, &nearest_vertices, 0, 2);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();

  expected = {
      {18, 7},
      {18, 8},
      {18, 9},
      {18, 10},
      {18, 11},
      {19, 7},
      {19, 8},
      {19, 9},
      {19, 10},
      {19, 11}
  };
  grid_geometry.EnvelopingVertexIndices({205, 100}, &nearest_vertices, 0, 2);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();
}

TEST(RoutingGridGeometry, EnvelopingVertexIndices_Rectangle) {
  RoutingLayerInfo horizontal = RoutingLayerInfo {
    .layer = 0,
    .area = Rectangle({0, 0}, {200, 200}),
    .wire_width = 1,
    .offset = 10,
    .direction = RoutingTrackDirection::kTrackHorizontal,
    .pitch = 10,
  };

  RoutingLayerInfo vertical = RoutingLayerInfo {
    .layer = 1,
    .area = Rectangle({0, 0}, {200, 200}),
    .wire_width = 1,
    .offset = 10,
    .direction = RoutingTrackDirection::kTrackVertical,
    .pitch = 10,
  };

  RoutingGridGeometry grid_geometry;
  grid_geometry.ComputeForLayers(horizontal, vertical);

  std::set<std::pair<size_t, size_t>> nearest_vertices;

  std::set<std::pair<size_t, size_t>> expected = {
      {1, 0},
      {2, 0},
      {3, 0},
      {4, 0},
      {1, 1},
      {2, 1},
      {3, 1},
      {4, 1},
      {1, 2},
      {2, 2},
      {3, 2},
      {4, 2},
      {1, 3},
      {2, 3},
      {3, 3},
      {4, 3},
      {1, 4},
      {2, 4},
      {3, 4},
      {4, 4},
      {1, 5},
      {2, 5},
      {3, 5},
      {4, 5},
  };
  grid_geometry.EnvelopingVertexIndices(
      Rectangle({25, 15}, {42, 53}), &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();
}

TEST(RoutingGridGeometry, EnvelopingVertexIndices_Rectangle_2) {
  RoutingLayerInfo horizontal = RoutingLayerInfo {
    .layer = 244,
    .area = Rectangle({0, -600}, {39840, 11600}),
    .wire_width = 140,
    .offset = 50,
    .direction = RoutingTrackDirection::kTrackHorizontal,
    .pitch = 340
  };

  RoutingLayerInfo vertical = RoutingLayerInfo {
    .layer = 220,
    .area = Rectangle({0, -600}, {39840, 11600}),
    .wire_width = 140,
    .offset = 50,
    .direction = RoutingTrackDirection::kTrackVertical,
    .pitch = 340
  };

  RoutingGridGeometry grid_geometry;
  grid_geometry.ComputeForLayers(horizontal, vertical);

  std::set<std::pair<size_t, size_t>> nearest_vertices;

  std::set<std::pair<size_t, size_t>> expected;
  for (size_t i = 0; i <= 18; ++i) {
    for (size_t j = 32; j <= 35; ++j) {
      expected.insert({i, j});
    }
  }

  grid_geometry.EnvelopingVertexIndices(
      Rectangle({0, 10640}, {6000, 11120}), &nearest_vertices);
  EXPECT_THAT(nearest_vertices, ContainerEq(expected));
  nearest_vertices.clear();
}

class RoutingGridGeometryTestFixture : public testing::Test {
 protected:
  void SetUp() override {
    RoutingLayerInfo horizontal = RoutingLayerInfo {
      .layer = 0,
      .area = Rectangle({0, 0}, {200, 200}),
      .wire_width = 1,
      .offset = 10,
      .direction = RoutingTrackDirection::kTrackHorizontal,
      .pitch = 10,
    };

    RoutingLayerInfo vertical = RoutingLayerInfo {
      .layer = 1,
      .area = Rectangle({0, 0}, {200, 200}),
      .wire_width = 1,
      .offset = 10,
      .direction = RoutingTrackDirection::kTrackVertical,
      .pitch = 10,
    };

    grid_geometry_.ComputeForLayers(horizontal, vertical);
  }

  RoutingGridGeometry grid_geometry_;
};

TEST_F(RoutingGridGeometryTestFixture, NearestTrackIndices_OnRowAndCol) {
  std::set<size_t> horizontal;
  std::set<size_t> vertical;

  std::set<size_t> expected = {0};
  grid_geometry_.NearestTrackIndices({10, 10}, &horizontal, &vertical);
  EXPECT_THAT(horizontal, ContainerEq(expected));
  EXPECT_THAT(vertical, ContainerEq(expected));
}

TEST_F(RoutingGridGeometryTestFixture, NearestTrackIndices_OnColumn) {
  std::set<size_t> horizontal;
  std::set<size_t> vertical;

  std::set<size_t> expected_horizontal = {0, 1};
  std::set<size_t> expected_vertical = {0};
  grid_geometry_.NearestTrackIndices({10, 15}, &horizontal, &vertical);
  EXPECT_THAT(horizontal, ContainerEq(expected_horizontal));
  EXPECT_THAT(vertical, ContainerEq(expected_vertical));
}

TEST_F(RoutingGridGeometryTestFixture, NearestTrackIndices_OnRow) {
  std::set<size_t> horizontal;
  std::set<size_t> vertical;

  std::set<size_t> expected_horizontal = {1};
  std::set<size_t> expected_vertical = {1, 2};
  grid_geometry_.NearestTrackIndices({25, 20}, &horizontal, &vertical);
  EXPECT_THAT(horizontal, ContainerEq(expected_horizontal));
  EXPECT_THAT(vertical, ContainerEq(expected_vertical));
}

TEST_F(RoutingGridGeometryTestFixture, NearestTrackIndices_OffGrid) {
  std::set<size_t> horizontal;
  std::set<size_t> vertical;

  std::set<size_t> expected_horizontal = {4, 5};
  std::set<size_t> expected_vertical = {3, 4};
  grid_geometry_.NearestTrackIndices({45, 55}, &horizontal, &vertical);
  EXPECT_THAT(horizontal, ContainerEq(expected_horizontal));
  EXPECT_THAT(vertical, ContainerEq(expected_vertical));
}

TEST_F(RoutingGridGeometryTestFixture, HorizontalLineThrough) {
  geometry::Line line = grid_geometry_.HorizontalLineThrough(5, 8);
  EXPECT_EQ(0, line.start().x());
  EXPECT_EQ(200, line.end().x());
  EXPECT_EQ(90, line.start().y());
  EXPECT_EQ(90, line.end().y());
}

TEST_F(RoutingGridGeometryTestFixture, VerticalLineThrough) {
  geometry::Line line = grid_geometry_.VerticalLineThrough(2, 3);
  EXPECT_EQ(0, line.start().y());
  EXPECT_EQ(200, line.end().y());
  EXPECT_EQ(30, line.start().x());
  EXPECT_EQ(30, line.end().x());
}

}  // namespace
}  // namespace geometry
}  // namespace bfg

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "routing_vertex_kd_tree.h"
#include "routing_vertex.h"
#include "geometry/point.h"

namespace bfg {
namespace {

using testing::IsEmpty;
using testing::UnorderedElementsAre;

TEST(RoutingVertexKDTreeTest, EmptyTree_SizeIsZero) {
  RoutingVertexKDTree tree;
  EXPECT_EQ(tree.Size(), 0);
}

TEST(RoutingVertexKDTreeTest, Add_IncreasesSize) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({0, 0});
  RoutingVertex v1({100, 100});

  tree.Add(&v0);
  EXPECT_EQ(tree.Size(), 1);

  tree.Add(&v1);
  EXPECT_EQ(tree.Size(), 2);
}

TEST(RoutingVertexKDTreeTest, Erase_DecreasesSize) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({0, 0});
  RoutingVertex v1({100, 100});

  tree.Add(&v0);
  tree.Add(&v1);
  EXPECT_EQ(tree.Size(), 2);

  tree.Erase(&v0);
  EXPECT_EQ(tree.Size(), 1);

  tree.Erase(&v1);
  EXPECT_EQ(tree.Size(), 0);
}

TEST(RoutingVertexKDTreeTest, FindNearby_EmptyTree_ReturnsEmpty) {
  RoutingVertexKDTree tree;
  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 1000);
  EXPECT_THAT(results, IsEmpty());
}

TEST(RoutingVertexKDTreeTest, FindNearby_SingleVertex_WithinRadius) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({50, 50});
  tree.Add(&v0);

  // Distance is 0 (same point), threshold = 1. 0 <= 1 -> found.
  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(50, 50), 1);
  EXPECT_THAT(results, UnorderedElementsAre(&v0));
}

TEST(RoutingVertexKDTreeTest, FindNearby_SingleVertex_OutsideRadius) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({500, 500});
  tree.Add(&v0);

  // find_within_range checks per-axis: |500 - 0| = 500 > 10 -> not found.
  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 10);
  EXPECT_THAT(results, IsEmpty());
}

TEST(RoutingVertexKDTreeTest, FindNearby_MultipleVertices_FiltersCorrectly) {
  RoutingVertexKDTree tree;
  // find_within_range uses per-axis bounding box with half-width radius.
  RoutingVertex v_close_0({10, 10});    // max per-axis dist from origin = 10
  RoutingVertex v_close_1({-10, -10});  // max per-axis dist from origin = 10
  RoutingVertex v_far({1000, 1000});    // max per-axis dist from origin = 1000

  tree.Add(&v_close_0);
  tree.Add(&v_close_1);
  tree.Add(&v_far);

  // radius=15. 10 <= 15 (close in), 1000 > 15 (far out).
  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 15);

  EXPECT_THAT(results, UnorderedElementsAre(&v_close_0, &v_close_1));
}

TEST(RoutingVertexKDTreeTest, FindNearby_ExactlyAtThreshold) {
  // Vertex at (25, 0): per-axis distance = 25. radius = 25.
  // 25 <= 25 -> found.
  RoutingVertexKDTree tree;
  RoutingVertex v0({25, 0});
  tree.Add(&v0);

  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 25);
  EXPECT_THAT(results, UnorderedElementsAre(&v0));
}

TEST(RoutingVertexKDTreeTest, FindNearby_JustOutsideThreshold) {
  // Vertex at (26, 0): per-axis distance = 26. radius = 25.
  // 26 > 25 -> not found.
  RoutingVertexKDTree tree;
  RoutingVertex v0({26, 0});
  tree.Add(&v0);

  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 25);
  EXPECT_THAT(results, IsEmpty());
}

TEST(RoutingVertexKDTreeTest, FindNearby_AfterErase_DoesNotReturnErased) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({10, 10});
  RoutingVertex v1({20, 20});

  tree.Add(&v0);
  tree.Add(&v1);
  tree.Erase(&v0);

  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 1000);
  EXPECT_THAT(results, UnorderedElementsAre(&v1));
}

TEST(RoutingVertexKDTreeTest, FindNearby_NonOriginReference) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({500, 500});  // per-axis dist from (505,505): max(5,5) = 5
  RoutingVertex v1({510, 510});  // per-axis dist from (505,505): max(5,5) = 5
  RoutingVertex v2({0, 0});      // per-axis dist from (505,505): max(505,505) = 505

  tree.Add(&v0);
  tree.Add(&v1);
  tree.Add(&v2);

  // radius=6. 5 <= 6 (nearby in), 505 > 6 (far out).
  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(505, 505), 6);
  EXPECT_THAT(results, UnorderedElementsAre(&v0, &v1));
}

TEST(RoutingVertexKDTreeTest, FindNearby_NegativeCoordinates) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({-100, -100});  // per-axis dist from (-105,-95): max(5,5) = 5
  RoutingVertex v1({-110, -90});   // per-axis dist from (-105,-95): max(5,5) = 5
  RoutingVertex v2({100, 100});    // per-axis dist from (-105,-95): max(205,195) = 205

  tree.Add(&v0);
  tree.Add(&v1);
  tree.Add(&v2);

  // radius=6. 5 <= 6 (nearby in), 205 > 6 (far out).
  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(-105, -95), 6);
  EXPECT_THAT(results, UnorderedElementsAre(&v0, &v1));
}

TEST(RoutingVertexKDTreeTest, FindNearby_ZeroRadius_OnlyExactMatch) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({100, 200});
  RoutingVertex v1({101, 200});

  tree.Add(&v0);
  tree.Add(&v1);

  // radius=0, threshold = 0. v0 distance = 0, v1 distance = 1. Only v0 found.
  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(100, 200), 0);
  EXPECT_THAT(results, UnorderedElementsAre(&v0));
}

TEST(RoutingVertexKDTreeTest, FindNearby_ManyVertices) {
  RoutingVertexKDTree tree;

  // Create a grid of vertices spaced 100 apart.
  std::vector<std::unique_ptr<RoutingVertex>> vertices;
  for (int x = 0; x < 1000; x += 100) {
    for (int y = 0; y < 1000; y += 100) {
      vertices.push_back(std::make_unique<RoutingVertex>(
          geometry::Point(x, y)));
      tree.Add(vertices.back().get());
    }
  }
  EXPECT_EQ(tree.Size(), 100);

  // radius=144.
  // find_within_range uses per-axis bounding box [ref - 144, ref + 144]:
  //   (0,0):     max(0,0) = 0     <= 144 -> in
  //   (100,0):   max(100,0) = 100 <= 144 -> in
  //   (0,100):   max(0,100) = 100 <= 144 -> in
  //   (100,100): max(100,100)=100 <= 144 -> in
  //   (200,0):   max(200,0) = 200 > 144  -> out
  //   (0,200):   max(0,200) = 200 > 144  -> out
  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 144);
  EXPECT_EQ(results.size(), 4);
}

TEST(RoutingVertexKDTreeTest, FindNearby_CoincidentVertices) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({42, 42});
  RoutingVertex v1({42, 42});

  tree.Add(&v0);
  tree.Add(&v1);

  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(42, 42), 1);
  EXPECT_THAT(results, UnorderedElementsAre(&v0, &v1));
}

TEST(RoutingVertexKDTreeTest, AddAfterFind_ReturnsNewVertex) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({10, 10});
  tree.Add(&v0);

  // Force an optimise by searching.
  tree.FindNearby(geometry::Point(0, 0), 100);

  // Add another and search again -- tree should re-optimise.
  RoutingVertex v1({20, 20});
  tree.Add(&v1);

  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 100);
  EXPECT_THAT(results, UnorderedElementsAre(&v0, &v1));
}

TEST(RoutingVertexKDTreeTest, EraseAfterFind_DoesNotReturnErased) {
  RoutingVertexKDTree tree;
  RoutingVertex v0({10, 10});
  RoutingVertex v1({20, 20});
  tree.Add(&v0);
  tree.Add(&v1);

  // Force an optimise by searching.
  tree.FindNearby(geometry::Point(0, 0), 100);

  // Erase and search again.
  tree.Erase(&v0);

  std::vector<RoutingVertex*> results =
      tree.FindNearby(geometry::Point(0, 0), 100);
  EXPECT_THAT(results, UnorderedElementsAre(&v1));
}

// RoutingVertexKDNode tests.

TEST(RoutingVertexKDNodeTest, IndexOperator_ReturnsCoordinates) {
  RoutingVertex v({123, 456});
  RoutingVertexKDNode node(&v);

  EXPECT_EQ(node[0], 123);
  EXPECT_EQ(node[1], 456);
}

TEST(RoutingVertexKDNodeTest, Vertex_ReturnsOriginalPointer) {
  RoutingVertex v({0, 0});
  RoutingVertexKDNode node(&v);
  EXPECT_EQ(node.vertex(), &v);
}

}  // namespace
}  // namespace bfg

#include "routing_path.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <set>
#include <vector>

#include "../equivalent_nets.h"
#include "../geometry/point.h"
#include "../geometry/port.h"
#include "routing_blockage_cache.h"
#include "routing_edge.h"
#include "routing_grid_test_fixture.h"
#include "routing_vertex.h"

namespace bfg {
namespace routing {
namespace {

// RoutingPaths are built from a real grid, so these reuse its fixture.
class RoutingPathTest : public RoutingGridTest {
 protected:
  // A path found but not installed in the grid. The caller owns it.
  std::unique_ptr<RoutingPath> FindPath(
      const geometry::Point &from, const geometry::Point &to,
      const std::string &net) {
    RoutingBlockageCache cache(*grid_);
    auto path = ShortestPath(NearestVertex(from), NearestVertex(to), cache);
    EXPECT_TRUE(path.ok()) << path.status();
    if (!path.ok()) {
      return nullptr;
    }
    std::unique_ptr<RoutingPath> owned(*path);
    owned->set_nets(EquivalentNets(net));
    return owned;
  }

  // Installs a route between ports near the given points.
  RoutingPath *InstallRoute(
      const geometry::Point &from, const geometry::Point &to,
      const std::string &net) {
    ports_.push_back(MakePort(from, net));
    geometry::Port *begin = ports_.back().get();
    ports_.push_back(MakePort(to, net));
    geometry::Port *end = ports_.back().get();
    RoutingBlockageCache cache(*grid_);
    auto path = grid_->AddRouteBetween(*begin, *end, cache, EquivalentNets(net));
    EXPECT_TRUE(path.ok()) << path.status();
    return path.ok() ? *path : nullptr;
  }

  std::vector<std::unique_ptr<geometry::Port>> ports_;
};

TEST_F(RoutingPathTest, ConstructorOrdersVertices) {
  std::unique_ptr<RoutingPath> path = FindPath({0, 0}, {kSize, kSize}, "a");
  ASSERT_NE(nullptr, path);
  ASSERT_EQ(path->edges().size() + 1, path->vertices().size());
  for (size_t i = 0; i < path->edges().size(); ++i) {
    RoutingEdge *edge = path->edges()[i];
    std::set<RoutingVertex*> ends = {edge->first(), edge->second()};
    EXPECT_EQ(1, ends.count(path->vertices()[i]));
    EXPECT_EQ(1, ends.count(path->vertices()[i + 1]));
  }
}

TEST_F(RoutingPathTest, Cost) {
  std::unique_ptr<RoutingPath> path = FindPath({0, 0}, {kSize, kSize}, "a");
  ASSERT_NE(nullptr, path);
  double expected = 0.0;
  for (RoutingVertex *vertex : path->vertices()) {
    expected += vertex->cost();
  }
  for (RoutingEdge *edge : path->edges()) {
    expected += edge->cost();
  }
  EXPECT_DOUBLE_EQ(expected, path->Cost());

  // Longer paths cost more.
  std::unique_ptr<RoutingPath> shorter = FindPath({0, 0}, {2000, 0}, "a");
  ASSERT_NE(nullptr, shorter);
  EXPECT_LT(shorter->Cost(), path->Cost());
}

TEST_F(RoutingPathTest, Empty) {
  RoutingPath path(NearestVertex({0, 0}), {}, grid_.get());
  EXPECT_TRUE(path.Empty());
  EXPECT_DOUBLE_EQ(NearestVertex({0, 0})->cost(), path.Cost());
}

TEST_F(RoutingPathTest, SpannedVertices) {
  std::unique_ptr<RoutingPath> path = FindPath({0, 0}, {kSize, kSize}, "a");
  ASSERT_NE(nullptr, path);

  std::set<RoutingVertex*> spanned = path->SpannedVertices();
  for (RoutingVertex *vertex : path->vertices()) {
    EXPECT_EQ(1, spanned.count(vertex));
  }
  // Edges between path vertices can pass over other vertices.
  EXPECT_GE(spanned.size(), path->vertices().size());

  // Nothing has been skipped, so every path vertex hosts a (potential) via.
  std::vector<RoutingVertex*> with_vias = path->SpannedVerticesWithVias();
  EXPECT_EQ(path->vertices(), with_vias);
}

TEST_F(RoutingPathTest, SpannedVerticesWithoutVias) {
  std::unique_ptr<RoutingPath> path = FindPath({0, 0}, {kSize, kSize}, "a");
  ASSERT_NE(nullptr, path);
  std::set<RoutingVertex*> spanned = path->SpannedVertices();
  std::vector<RoutingVertex*> with_vias = path->SpannedVerticesWithVias();

  std::set<RoutingVertex*> without_vias = path->SpannedVerticesWithoutVias();
  EXPECT_EQ(spanned.size() - std::set<RoutingVertex*>(
                with_vias.begin(), with_vias.end()).size(),
            without_vias.size());
  for (RoutingVertex *vertex : with_vias) {
    EXPECT_EQ(0, without_vias.count(vertex));
  }
}

TEST_F(RoutingPathTest, AddPortMidway) {
  std::unique_ptr<RoutingPath> path = FindPath({0, 0}, {kSize, 0}, "a");
  ASSERT_NE(nullptr, path);
  path->AddPortMidway("mid");

  std::vector<RoutingVertex*> hosts;
  for (RoutingVertex *vertex : path->vertices()) {
    if (vertex->hosts_port()) {
      hosts.push_back(vertex);
    }
  }
  ASSERT_EQ(1, hosts.size());
  EXPECT_EQ("mid", *hosts.front()->hosts_port());
}

TEST_F(RoutingPathTest, AddPortMidway_DefaultsToPrimaryNet) {
  std::unique_ptr<RoutingPath> path = FindPath({0, 0}, {kSize, 0}, "a");
  ASSERT_NE(nullptr, path);
  path->AddPortMidway(std::nullopt);
  bool found = false;
  for (RoutingVertex *vertex : path->vertices()) {
    if (vertex->hosts_port()) {
      EXPECT_EQ("a", *vertex->hosts_port());
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(RoutingPathTest, InstalledPathsAreLegalised) {
  RoutingPath *path = InstallRoute({500, 500}, {4000, 3000}, "a");
  ASSERT_NE(nullptr, path);
  EXPECT_TRUE(path->legalised());
  EXPECT_TRUE(path->StartAccessLayer());
  EXPECT_TRUE(path->EndAccessLayer());
  EXPECT_TRUE(path->CheckStillAvailable().ok());
}

TEST_F(RoutingPathTest, Legalise_MergesStraightRunsIntoOneEdge) {
  RoutingPath *path = InstallRoute({0, 2380}, {kSize, 2380}, "a");
  ASSERT_NE(nullptr, path);
  // A straight run along one track is merged into one edge.
  EXPECT_EQ(1, path->edges().size());
}

TEST_F(RoutingPathTest, ToPointsAndLayers) {
  RoutingPath *path = InstallRoute({0, 0}, {4000, 4000}, "a");
  ASSERT_NE(nullptr, path);

  std::vector<geometry::Point> points;
  std::vector<geometry::Layer> layers;
  path->ToPointsAndLayers(&points, &layers);
  // One layer per edge between consecutive points.
  ASSERT_EQ(points.size(), layers.size() + 1);
  ASSERT_GE(points.size(), 2);
  // Consecutive points are rectilinear.
  for (size_t i = 1; i < points.size(); ++i) {
    EXPECT_TRUE(points[i].x() == points[i - 1].x() ||
                points[i].y() == points[i - 1].y())
        << points[i - 1] << " to " << points[i];
  }
}

TEST_F(RoutingPathTest, CheckStillAvailable_CrossingOnOtherLayerIsFine) {
  // Find a horizontal (met1) path for net a, then install a vertical (met2)
  // route for net b across it.
  std::unique_ptr<RoutingPath> path = FindPath({0, 2380}, {kSize, 2380}, "a");
  ASSERT_NE(nullptr, path);
  ASSERT_NE(nullptr, InstallRoute({2380, 0}, {2380, kSize}, "b"));
  EXPECT_TRUE(path->CheckStillAvailable().ok())
      << path->CheckStillAvailable();
}

TEST_F(RoutingPathTest, CheckStillAvailable_SameLayerConflicts) {
  // Find a path for net a, then install net b on the same track before it.
  std::unique_ptr<RoutingPath> path = FindPath({0, 2380}, {kSize, 2380}, "a");
  ASSERT_NE(nullptr, path);
  ASSERT_NE(nullptr, InstallRoute({0, 2380}, {kSize, 2380}, "b"));

  absl::Status status = path->CheckStillAvailable();
  EXPECT_EQ(absl::StatusCode::kFailedPrecondition, status.code()) << status;
}

TEST_F(RoutingPathTest, CheckStillAvailable_SameNetIsFine) {
  std::unique_ptr<RoutingPath> path = FindPath({0, 2380}, {kSize, 2380}, "a");
  ASSERT_NE(nullptr, path);
  ASSERT_NE(nullptr, InstallRoute({0, 2380}, {kSize, 2380}, "a"));
  EXPECT_TRUE(path->CheckStillAvailable().ok())
      << path->CheckStillAvailable();
}

TEST_F(RoutingPathTest, Describe) {
  std::unique_ptr<RoutingPath> path = FindPath({0, 0}, {2000, 0}, "a");
  ASSERT_NE(nullptr, path);
  EXPECT_FALSE(path->Describe().empty());
}

}  // namespace
}  // namespace routing
}  // namespace bfg

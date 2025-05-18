#include <cmath>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <absl/strings/str_join.h>

#include "geometry/point.h"
#include "routing_vertex.h"
#include "routing_vertex_collector.h"

namespace bfg {
namespace {

TEST(RoutingVertexCollectorTest, OfferSameX) {
  RoutingVertex a = RoutingVertex(geometry::Point(1, 2));
  RoutingVertex b = RoutingVertex(geometry::Point(1, 3));
  RoutingVertex c = RoutingVertex(geometry::Point(1, 4));
  RoutingVertex d = RoutingVertex(geometry::Point(2, 5));
  RoutingVertex e = RoutingVertex(geometry::Point(2, 6));

  auto same_x = [](RoutingVertex *lhs, RoutingVertex *rhs) {
    return lhs->centre().x() == rhs->centre().x();
  };

  RoutingVertexCollector collector = RoutingVertexCollector(same_x);
  for (RoutingVertex *v : {&a, &b, &c, &d, &e}) {
    collector.Offer(v);
  }

  LOG(INFO) << collector.Describe();
  
  EXPECT_EQ(2, collector.groups().size());
  // I cannot remember gmock container matchers so I'm going to eyeball for now:
  // EXPECT_THAT(collector.groups().at(0),
  //     gmock::CONTAINER_EQ(
  //         {geometry::Point(1, 2),
  //         geometry::Point(1, 3),
  //         geometry::Point(1, 4)}));

  //for (const auto &entry : vias_too_close_together_per_layer) {
  //  const geometry::Layer &layer = entry.first;
  //  const auto &collections = entry.second;
  //  for (const auto &list : collections) {
  //    std::vector<std::string> vertex_centres;
  //    for (RoutingVertex *const vertex : list) {
  //      vertex_centres.push_back(vertex->centre().Describe());
  //    }
  //    LOG(INFO) << "layer " << layer
  //              << " has off-poly-line vias too close together: {"
  //              << absl::StrJoin(vertex_centres, ", ") << "}";
  //  }
  //}

}

TEST(RoutingVertexCollectorTest, OfferOffByOne) {
  RoutingVertex a = RoutingVertex(geometry::Point(1, 2));
  RoutingVertex b = RoutingVertex(geometry::Point(1, 3));
  RoutingVertex c = RoutingVertex(geometry::Point(1, 4));
  RoutingVertex d = RoutingVertex(geometry::Point(2, 5));
  RoutingVertex e = RoutingVertex(geometry::Point(2, 6));

  auto y_off_by_one = [](RoutingVertex *lhs, RoutingVertex *rhs) {
    return std::abs(lhs->centre().y() - rhs->centre().y()) == 1;
  };

  RoutingVertexCollector collector = RoutingVertexCollector(y_off_by_one);
  for (RoutingVertex *v : {&a, &b, &c, &d, &e}) {
    collector.Offer(v);
  }

  LOG(INFO) << collector.Describe();
}

TEST(RoutingVertexCollectorTest, OfferOrderBreaksGrouping) {
  RoutingVertex a = RoutingVertex(geometry::Point(1, 2));
  RoutingVertex b = RoutingVertex(geometry::Point(1, 3));
  RoutingVertex c = RoutingVertex(geometry::Point(1, 4));
  RoutingVertex d = RoutingVertex(geometry::Point(2, 5));
  RoutingVertex e = RoutingVertex(geometry::Point(2, 6));

  auto y_off_by_one = [](RoutingVertex *lhs, RoutingVertex *rhs) {
    return std::abs(lhs->centre().y() - rhs->centre().y()) == 1;
  };
  RoutingVertexCollector collector = RoutingVertexCollector(y_off_by_one);

  // Order matters! This should break the sequence:
  for (RoutingVertex *v : {&a, &b, &e, &c, &d}) {
    collector.Offer(v);
  }

  // Should be [(1, 2), (1, 3)]
  LOG(INFO) << collector.Describe();
}

TEST(LayeredRoutingVertexCollectors, OfferByLayers) {
  // Layer 0:
  RoutingVertex a = RoutingVertex(geometry::Point(1, 2));
  RoutingVertex b1 = RoutingVertex(geometry::Point(1, 3));
  RoutingVertex b2 = RoutingVertex(geometry::Point(1, 4));
  RoutingVertex b3 = RoutingVertex(geometry::Point(1, 5));
  RoutingVertex c = RoutingVertex(geometry::Point(2, 5));

  // Layer 1
  RoutingVertex d = RoutingVertex(geometry::Point(1, 6));
  RoutingVertex e = RoutingVertex(geometry::Point(2, 7));
  RoutingVertex f = RoutingVertex(geometry::Point(2, 6));

  auto same_x = [](
      const geometry::Layer &ignored_layer,
      RoutingVertex *lhs,
      RoutingVertex *rhs) {
    // Grouping within layer is not a function of itself:
    return lhs->centre().x() == rhs->centre().x();
  };

  LayeredRoutingVertexCollectors collectors(same_x);

  // Order only matters within a layer:
  for (RoutingVertex *v : {&a, &b1, &b2, &b3, &e}) {
    collectors.Offer(0, v);
  }
  for (RoutingVertex *v : {&d, &e, &f}) {
    collectors.Offer(1, v);
  }

  for (const auto &entry : collectors.collectors_by_layer()) {
    LOG(INFO) << "Layer " << entry.first << "; " << entry.second.Describe();
  }
}

TEST(LayeredRoutingVertexCollectors, GroupsByLayer) {
  // Layer 0:
  RoutingVertex a = RoutingVertex(geometry::Point(1, 2));
  RoutingVertex b1 = RoutingVertex(geometry::Point(1, 3));
  RoutingVertex b2 = RoutingVertex(geometry::Point(1, 4));
  RoutingVertex b3 = RoutingVertex(geometry::Point(1, 5));
  RoutingVertex c = RoutingVertex(geometry::Point(2, 5));

  // Layer 1
  RoutingVertex d = RoutingVertex(geometry::Point(1, 6));
  RoutingVertex e = RoutingVertex(geometry::Point(2, 7));
  RoutingVertex f = RoutingVertex(geometry::Point(2, 6));

  auto same_x = [](
      const geometry::Layer &ignored_layer,
      RoutingVertex *lhs,
      RoutingVertex *rhs) {
    return lhs->centre().x() == rhs->centre().x();
  };

  LayeredRoutingVertexCollectors collectors(same_x);

  // Order only matters within a layer:
  for (RoutingVertex *v : {&a, &b1, &b2, &b3, &e}) {
    collectors.Offer(0, v);
  }
  for (RoutingVertex *v : {&d, &e, &f}) {
    collectors.Offer(1, v);
  }

  for (const auto &entry : collectors.GroupsByLayer()) {
    size_t list_counter = 0;
    for (const auto &list : entry.second) {
      for (const RoutingVertex *const vertex : list) {
        LOG(INFO) << "Layer " << entry.first << " group "
                  << list_counter << " entry " << vertex->centre().Describe();
      }
      ++list_counter;
    }
  }
}

}   // namespace
}   // namespace bfg

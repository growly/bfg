#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utility>

#include "routing_vertex.h"
#include "geometry/point.h"

namespace bfg {
namespace {

using testing::ElementsAre;

TEST(RoutingVertex, Availabile_SetForcedBlocked_Permanent) {
  RoutingVertex test = RoutingVertex({10, 10});
  test.AddConnectedLayer(0);
  test.AddConnectedLayer(1);

  EXPECT_TRUE(test.Available());

  test.SetForcedBlocked(true, false);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(false, false);

  EXPECT_TRUE(test.Available());

  test.SetForcedBlocked(true, false, 0);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(true, false, 1);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(false, false, 0);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(false, false, 1);

  EXPECT_TRUE(test.Available());
}

TEST(RoutingVertex, Availabile_SetForcedBlocked_Temporary) {
  RoutingVertex test = RoutingVertex({10, 10});
  test.AddConnectedLayer(0);
  test.AddConnectedLayer(1);

  EXPECT_TRUE(test.Available());

  test.SetForcedBlocked(true, true);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(false, true);

  EXPECT_TRUE(test.Available());

  test.SetForcedBlocked(true, true, 0);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(true, true, 1);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(false, true, 0);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(false, true, 1);

  EXPECT_TRUE(test.Available());
}

TEST(RoutingVertex, Availabile_SetForcedBlocked_TemporaryOverPermanent) {
  RoutingVertex test = RoutingVertex({10, 10});
  test.AddConnectedLayer(0);
  test.AddConnectedLayer(1);

  EXPECT_TRUE(test.Available());

  test.SetForcedBlocked(true, false);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(false, true);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(true, true, 0);

  EXPECT_FALSE(test.Available());

  test.SetForcedBlocked(true, true, 1);

  EXPECT_FALSE(test.Available());

  test.ResetTemporaryStatus();

  EXPECT_FALSE(test.Available());
}

TEST(RoutingVertex, AvailabileFor) {
  RoutingVertex test = RoutingVertex({10, 10});
  test.AddConnectedLayer(0);
  test.AddConnectedLayer(1);

  EquivalentNets nets = {"net"};

  EXPECT_TRUE(test.Available());
  EXPECT_TRUE(test.AvailableForAll());
  EXPECT_TRUE(test.AvailableForAll(nets));
  EXPECT_TRUE(test.AvailableForAll(nets, 0));

  test.SetForcedBlocked(true, true, 0);

  EXPECT_FALSE(test.Available());
  EXPECT_FALSE(test.AvailableForAll());
  EXPECT_FALSE(test.AvailableForAll(nets));
  EXPECT_FALSE(test.AvailableForAll(nets, 0));

  EXPECT_TRUE(test.AvailableForAll(std::nullopt, 1));
  EXPECT_FALSE(test.AvailableForAll(std::nullopt, 0));

  test.ResetTemporaryStatus();

  EXPECT_TRUE(test.Available());
  EXPECT_TRUE(test.AvailableForAll());
  EXPECT_TRUE(test.AvailableForAll(nets));
  EXPECT_TRUE(test.AvailableForAll(nets, 0));
}

TEST(RoutingVertex, AvailabileFor_BlockingOne) {
  RoutingVertex test = RoutingVertex({10, 10});
  test.AddConnectedLayer(0);
  test.AddConnectedLayer(1);

  EquivalentNets nets = {"net"};

  EXPECT_TRUE(test.Available());
  EXPECT_TRUE(test.AvailableForAll());
  EXPECT_TRUE(test.AvailableForAll(nets));
  EXPECT_TRUE(test.AvailableForAll(nets, 0));

  EquivalentNets other(std::set<std::string>({"other_net", "yet_another"}));

  test.AddBlockingNet("other_net",
                      false,
                      0);

  EXPECT_FALSE(test.Available());
  EXPECT_FALSE(test.AvailableForAll());
  EXPECT_FALSE(test.AvailableForAll(nets));
  EXPECT_FALSE(test.AvailableForAll(nets, 0));

  // Vertex should be available for any net on the non-blocked layer:
  EXPECT_TRUE(test.AvailableForAll(std::nullopt, 1));
  EXPECT_FALSE(test.AvailableForAll(std::nullopt, 0));

  // And available for the blocking net on the blocked layer?
  EXPECT_TRUE(test.AvailableForAll(other, 0));

  EXPECT_TRUE(test.AvailableForAll(nets, 1));
}

TEST(RoutingVertex, AvailabileFor_BlockingOneUsingOther) {
  RoutingVertex test = RoutingVertex({10, 10});
  test.AddConnectedLayer(0);
  test.AddConnectedLayer(1);

  EquivalentNets nets = {"net"};

  EXPECT_TRUE(test.Available());
  EXPECT_TRUE(test.AvailableForAll());
  EXPECT_TRUE(test.AvailableForAll(nets));
  EXPECT_TRUE(test.AvailableForAll(nets, 0));

  EquivalentNets other(std::set<std::string>({"other_net", "yet_another"}));

  test.AddBlockingNet("other_net",
                      false,
                      0);
  test.AddUsingNet(nets.primary(),
                   false,
                   1);

  EXPECT_FALSE(test.Available());
  EXPECT_FALSE(test.AvailableForAll());
  EXPECT_FALSE(test.AvailableForAll(nets));
  EXPECT_FALSE(test.AvailableForAll(nets, 0));

  // Vertex should be UNavailable for "any" net on all layers:
  EXPECT_FALSE(test.AvailableForAll(std::nullopt, 1));
  EXPECT_FALSE(test.AvailableForAll(std::nullopt, 0));

  // And available for the blocking net on the blocked layer?
  EXPECT_TRUE(test.AvailableForAll(other, 0));

  EXPECT_TRUE(test.AvailableForAll(nets, 1));
}

TEST(RoutingVertex, AvailableForNetsOnAnyLayer) {
  RoutingVertex test = RoutingVertex({10, 10});
  test.AddConnectedLayer(0);
  test.AddConnectedLayer(1);

  EquivalentNets nets = {"net"};

  EXPECT_TRUE(test.AvailableForNetsOnAnyLayer(nets));

  test.AddBlockingNet("other", false, 0);

  // Should be available for net "net" on layer 1.
  EXPECT_TRUE(test.AvailableForNetsOnAnyLayer(nets));

  test.AddUsingNet("net", true, 1);

  // Still available on layer 1.
  EXPECT_TRUE(test.AvailableForNetsOnAnyLayer(nets));

  test.ResetTemporaryStatus();
  test.AddUsingNet("another", true, 1);

  EXPECT_FALSE(test.AvailableForNetsOnAnyLayer(nets));
}

}  // namespace
}  // namespace bfg

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <google/protobuf/util/message_differencer.h>

#include "utility.h"
#include "edge_list.h"
#include "proto/edge_list.pb.h"

namespace bfg {
namespace {

TEST(EdgeSpec, FromProto_1) {
  proto::EdgeSpec golden;
  bfg::ReadTextProtoOrDie("test_data/edge_spec.pb.txt", &golden);

  EdgeSpec spec;
  spec.FromProto(golden);

  EXPECT_EQ("SOME_MUX", spec.from().instance_name);
  EXPECT_EQ(2, spec.from().port_names.size());
  EXPECT_THAT(spec.from().port_names, testing::ElementsAre("Z0", "Z1"));

  EXPECT_EQ("SOME_OTHER_MUX", spec.to().instance_name);
  EXPECT_EQ(6, spec.to().port_names.size());
  EXPECT_THAT(spec.to().port_names,
              testing::ElementsAre("X0", "X1", "X2", "X3", "X4", "X5"));

}

TEST(EdgeSpec, ToProto_1) {
  EdgeSpec spec;
  spec.set_from("SOME_MUX", {"Z0", "Z1"});
  spec.set_to("SOME_OTHER_MUX", {"X0", "X1", "X2", "X3", "X4", "X5"});

  proto::EdgeSpec pb = spec.ToProto();

  EXPECT_EQ("SOME_MUX", pb.from().instance_name());
  EXPECT_EQ(2, pb.from().port_names_size());
  EXPECT_THAT(pb.from().port_names(), testing::ElementsAre("Z0", "Z1"));

  EXPECT_EQ("SOME_OTHER_MUX", pb.to().instance_name());
  EXPECT_EQ(6, pb.to().port_names_size());
  EXPECT_THAT(pb.to().port_names(),
              testing::ElementsAre("X0", "X1", "X2", "X3", "X4", "X5"));

}

TEST(EdgeList, FromProto_1) {
  proto::EdgeList golden;
  bfg::ReadTextProtoOrDie("test_data/edge_list.pb.txt", &golden);

  EdgeList list;
  list.FromProto(golden);

  EXPECT_EQ(4, list.edges().size());

  // Check the first entry thoroughly:
  const EdgeSpec &first = list.edges()[0];
  EXPECT_EQ("MUX_0", first.from().instance_name);
  EXPECT_EQ(1, first.from().port_names.size());
  EXPECT_THAT(first.from().port_names, testing::ElementsAre("Z0"));

  EXPECT_EQ("MUX_5", first.to().instance_name);
  EXPECT_EQ(5, first.to().port_names.size());
  EXPECT_THAT(first.to().port_names,
              testing::ElementsAre("X0", "X1", "X2", "X3", "X4"));

  // And the last entry:
  const EdgeSpec &last = list.edges()[3];
  EXPECT_EQ("MUX_3", last.from().instance_name);
  EXPECT_EQ(1, last.from().port_names.size());
  EXPECT_THAT(last.from().port_names, testing::ElementsAre("Z"));

  EXPECT_EQ("MUX_8", last.to().instance_name);
  EXPECT_EQ(5, last.to().port_names.size());
  EXPECT_THAT(last.to().port_names,
              testing::ElementsAre("X1", "X2", "X3", "X4", "X5"));
}

TEST(EdgeList, ToProto_1) {
  // Being lazy, we'll just test the round-trip:
  proto::EdgeList golden;
  bfg::ReadTextProtoOrDie("test_data/edge_list.pb.txt", &golden);
  EdgeList list;
  list.FromProto(golden);

  proto::EdgeList emitted = list.ToProto();
  EXPECT_TRUE(
      google::protobuf::util::MessageDifferencer::Equals(golden, emitted));
}

TEST(EdgeList, FromCSVOrDie_1) {
  EdgeList list;
  list.FromCSVOrDie("test_data/edge_list.csv");

  // Same data as in "test_data/edge_list.pb.txt".

  // Check the first entry thoroughly:
  const EdgeSpec &first = list.edges()[0];
  EXPECT_EQ("MUX_0", first.from().instance_name);
  EXPECT_EQ(1, first.from().port_names.size());
  EXPECT_THAT(first.from().port_names, testing::ElementsAre("Z0"));

  EXPECT_EQ("MUX_5", first.to().instance_name);
  EXPECT_EQ(5, first.to().port_names.size());
  EXPECT_THAT(first.to().port_names,
              testing::ElementsAre("X0", "X1", "X2", "X3", "X4"));

  // And the last entry:
  const EdgeSpec &last = list.edges()[3];
  EXPECT_EQ("MUX_3", last.from().instance_name);
  EXPECT_EQ(1, last.from().port_names.size());
  EXPECT_THAT(last.from().port_names, testing::ElementsAre("Z"));

  EXPECT_EQ("MUX_8", last.to().instance_name);
  EXPECT_EQ(5, last.to().port_names.size());
  EXPECT_THAT(last.to().port_names,
              testing::ElementsAre("X1", "X2", "X3", "X4", "X5"));
}

}  // namespace
}  // namespace bfg

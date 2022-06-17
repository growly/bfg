#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <absl/strings/str_join.h>

#include "physical_properties_database.h"
#include "cell.h"
#include "layout.h"
#include "atoms/sky130_buf.h"

#include "c_make_header.h"

DEFINE_string(example_flag, "default", "for later");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::string version =
      "BFG v" xstr(bfg_VERSION_MAJOR) "." xstr(bfg_VERSION_MINOR);
  std::cout << version << std::endl;
  LOG(INFO) << version << " start";

  // Some process "properties".
  bfg::RoutingLayerInfo layer_1;
  layer_1.layer = 4;
  layer_1.area = bfg::geometry::Rectangle(
      bfg::geometry::Point(0, 0), 1000, 1000);
  layer_1.wire_width = 50;
  layer_1.offset = 50;
  layer_1.pitch = 100;
  layer_1.direction = bfg::RoutingTrackDirection::kTrackVertical;

  bfg::RoutingLayerInfo layer_2;
  layer_2.layer = 5;
  layer_2.area = bfg::geometry::Rectangle(
      bfg::geometry::Point(0, 0), 1000, 1000);
  layer_2.wire_width = 50;
  layer_2.offset = 50;
  layer_2.pitch = 100;
  layer_2.direction = bfg::RoutingTrackDirection::kTrackHorizontal;

  bfg::ViaInfo layer_1_2;
  layer_1_2.layer = 6;
  layer_1_2.cost = 1.0;
  layer_1_2.width = 30;
  layer_1_2.height = 30;
  layer_1_2.overhang = 10;

  bfg::PhysicalPropertiesDatabase physical_db;
  physical_db.AddLayer(layer_1);
  physical_db.AddLayer(layer_2);
  physical_db.AddViaInfo(layer_1.layer, layer_2.layer, layer_1_2);

  bfg::atoms::Sky130Buf::Parameters buf_params = {
    .width = 1380,
    .height = 2720
  };
  bfg::atoms::Sky130Buf buf(physical_db, buf_params);
  std::unique_ptr<bfg::Cell> buf_cell(buf.Generate());
  std::cout << buf_cell->layout()->Describe();

  return EXIT_SUCCESS;
}

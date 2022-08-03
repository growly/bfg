#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
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
#include "atoms/sky130_dfxtp.h"

#include "vlsir/tech.pb.h"
#include "vlsir/layout/raw.pb.h"
#include <google/protobuf/text_format.h>

#include "c_make_header.h"

DEFINE_string(output_library, "library.pb", "Output Vlsir Library path");
DEFINE_bool(text_format, true, "Also write text format protobufs");

void WriteLibrary(const bfg::Cell &cell) {
  ::vlsir::raw::Library library;
  library.set_units(::vlsir::raw::Units::NANO);

  ::vlsir::raw::Cell *cell_pb = library.add_cells();
  *cell_pb->mutable_layout() = cell.layout()->ToVLSIRLayout();
  *cell_pb->mutable_module() = cell.circuit()->ToVLSIRCircuit();

  if (FLAGS_text_format) {
    std::string text_format;
    google::protobuf::TextFormat::PrintToString(library, &text_format);

    std::fstream text_format_output(
        FLAGS_output_library + ".txt",
        std::ios::out | std::ios::trunc | std::ios::binary);
    text_format_output << text_format;
    text_format_output.close();
  }

  std::fstream output_file(
      FLAGS_output_library, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!library.SerializeToOstream(&output_file)) {
    LOG(ERROR) << "Failed to write library";
  } else {
    LOG(INFO) << "Wrote library to " << FLAGS_output_library;
  }
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

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

  vlsir::tech::Technology sky130_pb;

  std::string pdk_file_name = "../sky130.technology.pb.txt";
  std::ifstream pdk_file(pdk_file_name);
  LOG_IF(FATAL, !pdk_file.is_open())
      << "Could not open PDK descriptor file: " << pdk_file_name;
  std::ostringstream ss;
  ss << pdk_file.rdbuf();
  google::protobuf::TextFormat::ParseFromString(ss.str(), &sky130_pb);

  bfg::PhysicalPropertiesDatabase physical_db;
  physical_db.LoadTechnology(sky130_pb);

  physical_db.AddRoutingLayerInfo(layer_1);
  physical_db.AddRoutingLayerInfo(layer_2);
  physical_db.AddViaInfo(layer_1.layer, layer_2.layer, layer_1_2);

  //bfg::atoms::Sky130Buf::Parameters buf_params = {
  //  .width_nm = 1380,
  //  .height_nm = 2720,
  //  .x0_width_nm = 520,
  //  .x1_width_nm = 790,
  //  .x2_width_nm = 520,
  //  .x3_width_nm = 790
  //};
  //bfg::atoms::Sky130Buf buf(physical_db, buf_params);
  //std::unique_ptr<bfg::Cell> buf_cell(buf.Generate());
  //WriteLibrary(*buf_cell);
  //std::cout << buf_cell->layout()->Describe();

  bfg::atoms::Sky130Dfxtp::Parameters params;
  bfg::atoms::Sky130Dfxtp generator(physical_db, params);
  std::unique_ptr<bfg::Cell> cell(generator.Generate());
  WriteLibrary(*cell);
  std::cout << cell->layout()->Describe();

  google::protobuf::ShutdownProtobufLibrary();

  return EXIT_SUCCESS;
}

#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <absl/strings/str_join.h>
#include <google/protobuf/text_format.h>

#include "design_database.h"
#include "physical_properties_database.h"
#include "cell.h"
#include "layout.h"
#include "atoms/sky130_mux.h"
#include "tiles/lut.h"

#include "vlsir/tech.pb.h"
#include "vlsir/layout/raw.pb.h"

#include "c_make_header.h"

DEFINE_string(technology, "technology.pb", "Path to binary technology proto");
DEFINE_string(external_circuits, "", "Path to binary circuits proto");
DEFINE_string(output_library, "library.pb", "Output Vlsir Library path");
//DEFINE_bool(read_text_format, true, "Expect input protobufs in text format");
DEFINE_bool(write_text_format, true, "Also write text format protobufs");

// Demo flags.
DEFINE_int32(k_lut, 4, "How many LUT inputs");
DEFINE_bool(s44, false, "Whether to make an S44 LUT (override K selection)");


int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  std::string version =
      "BFG v" xstr(bfg_VERSION_MAJOR) "." xstr(bfg_VERSION_MINOR);
  std::cout << version << std::endl;
  LOG(INFO) << version << " start";

  vlsir::tech::Technology tech_pb;
  // std::string pdk_file_name = "../sky130.technology.pb.txt";
  // std::ifstream pdk_file(pdk_file_name);
  // LOG_IF(FATAL, !pdk_file.is_open())
  //     << "Could not open PDK descriptor file: " << pdk_file_name;
  // std::ostringstream ss;
  // ss << pdk_file.rdbuf();
  // google::protobuf::TextFormat::ParseFromString(ss.str(), &tech_pb);

  std::fstream technology_input(
      FLAGS_technology, std::ios::in | std::ios::binary);
  LOG_IF(FATAL, !technology_input)
      << "Could not open technology protobuf, "
      << FLAGS_technology;
  if (!tech_pb.ParseFromIstream(&technology_input)) {
    LOG(FATAL) << "Could not parse technology protobuf, "
               << FLAGS_technology;
  }

  bfg::DesignDatabase design_db;
  bfg::PhysicalPropertiesDatabase &physical_db = design_db.physical_db();

  physical_db.LoadTechnology(tech_pb);

  bfg::IntraLayerConstraints intra_constraints = {
    .min_separation = 270,
  };
  physical_db.AddRules("diff.drawing", intra_constraints);

  intra_constraints = {
    .min_separation = 170,
    .min_width = 170,
  };
  physical_db.AddRules("li.drawing", intra_constraints);
  physical_db.AddRules("diff.drawing", intra_constraints);
  intra_constraints = {
    .min_width = 170,
    .via_width = 170,
  };
  physical_db.AddRules("licon.drawing", intra_constraints);
  physical_db.AddRules("li.pin", intra_constraints);
  physical_db.AddRules("via1.drawing", intra_constraints);
  physical_db.AddRules("mcon.drawing", intra_constraints);
  intra_constraints = {
    .min_separation = 200,
    .min_width = 170,
    .min_pitch = 500,
  };
  physical_db.AddRules("poly.drawing", intra_constraints);
  intra_constraints = {
    .min_separation = 200,
    .min_width = 140,
    .min_pitch = 340,
  };
  physical_db.AddRules("met1.drawing", intra_constraints);
  // Lazy:
  physical_db.AddRules("met2.drawing", intra_constraints);
  physical_db.AddRules("met3.drawing", intra_constraints);

  bfg::InterLayerConstraints inter_constraints = {
    .min_separation = 50,
    .via_overhang = 80,
    .via_overhang_wide = 50
  };
  physical_db.AddRules("poly.drawing", "licon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = 50,
    .via_overhang = 80,
    .via_overhang_wide = 35
  };
  physical_db.AddRules("li.drawing", "licon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = 40,
    .via_overhang = 40,
  };
  physical_db.AddRules("diff.drawing", "licon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = 50,
    .via_overhang = 60,
    .via_overhang_wide = 30
  };
  physical_db.AddRules("li.drawing", "mcon.drawing", inter_constraints);
  physical_db.AddRules("met1.drawing", "mcon.drawing", inter_constraints);
  physical_db.AddRules("met1.drawing", "via1.drawing", inter_constraints);
  physical_db.AddRules("met2.drawing", "via1.drawing", inter_constraints);
  // Lazy but doesn't make sense:
  physical_db.AddRules("met2.drawing", "mcon.drawing", inter_constraints);
  physical_db.AddRules("met3.drawing", "mcon.drawing", inter_constraints);
  // TODO(growly): Need to alias these layer names so that they apply to any
  // process.

  if (FLAGS_external_circuits != "") {
    vlsir::circuit::Package external_circuits_pb;
    std::fstream external_circuits_input(
        FLAGS_external_circuits, std::ios::in | std::ios::binary);
    LOG_IF(FATAL, !external_circuits_input)
        << "Could not open external circuits protobuf, "
        << FLAGS_external_circuits;
    if (!external_circuits_pb.ParseFromIstream(&external_circuits_input)) {
      LOG(FATAL) << "Could not parse external circuits protobuf, "
                 << FLAGS_external_circuits;
    }
    design_db.LoadPackage(external_circuits_pb);
  }

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

  // bfg::atoms::Sky130Dfxtp::Parameters params;
  // bfg::atoms::Sky130Dfxtp generator(design_db, params);
  // std::unique_ptr<bfg::Cell> cell(generator.Generate());
  // WriteLibrary(*cell);
  // std::cout << cell->layout()->Describe();

  std::string top_name = "lut";
  bfg::tiles::Lut generator(&design_db, FLAGS_k_lut);
  bfg::Cell *top = generator.GenerateIntoDatabase(top_name);
  //std::cout << top->layout()->Describe();

  //bfg::atoms::Sky130Mux::Parameters mux_params;
  //bfg::atoms::Sky130Mux mux(mux_params, &design_db);
  //top = mux.GenerateIntoDatabase(top_name);

  design_db.WriteTop(
      *top, FLAGS_output_library, FLAGS_write_text_format);

  google::protobuf::ShutdownProtobufLibrary();

  return EXIT_SUCCESS;
}

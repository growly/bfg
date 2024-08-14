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
#include "atoms/gf180mcu_mux.h"
#include "tiles/lut.h"
#include "tiles/lut_b.h"

#include "vlsir/tech.pb.h"
#include "vlsir/layout/raw.pb.h"

#include "dev_pdk_setup.h"

#include "c_make_header.h"

DEFINE_string(technology, "technology.pb", "Path to binary technology proto");
DEFINE_string(external_circuits, "", "Path to binary circuits proto");
DEFINE_string(output_library, "library.pb", "Output Vlsir Library path");
DEFINE_string(output_package, "package.pb", "Output Vlsir Package path");
//DEFINE_bool(read_text_format, true, "Expect input protobufs in text format");
// FIXME(aryap): Writing text format seems to cause a segfault with libprotoc
// 3.21.5.
DEFINE_bool(write_text_format, false, "Also write text format protobufs");

DEFINE_string(primitives, "primitives.pb", "Path to binary circuits proto");

// Demo flags.
DEFINE_int32(k_lut, 4, "How many LUT inputs");
DEFINE_bool(s44, false, "Whether to make an S44 LUT (override K selection)");

void Gf180McuMuxExperiment() {
  // GF180MCU 7T MUX experiment
  //
  bfg::DesignDatabase design_db;
  bfg::PhysicalPropertiesDatabase &physical_db = design_db.physical_db();

  bfg::SetUpGf180Mcu(&physical_db);

  std::string top_name = "gf180mcu_mux";
  bfg::atoms::Sky130Mux::Parameters params;
  bfg::atoms::Gf180McuMux generator(params, &design_db);
  bfg::Cell *top = generator.GenerateIntoDatabase(top_name);

  design_db.WriteTop(*top, "gf180_mux.pb", "gf180_mux.package.pb", true);
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  std::string version =
      "BFG v" xstr(bfg_VERSION_MAJOR) "." xstr(bfg_VERSION_MINOR);
  std::cout << version << std::endl;
  LOG(INFO) << version << " start";

  // The design database contains our design and all our dependencies.
  bfg::DesignDatabase design_db;

  // std::string pdk_file_name = "../sky130.technology.pb.txt";
  // std::ifstream pdk_file(pdk_file_name);
  // LOG_IF(FATAL, !pdk_file.is_open())
  //     << "Could not open PDK descriptor file: " << pdk_file_name;
  // std::ostringstream ss;
  // ss << pdk_file.rdbuf();
  // google::protobuf::TextFormat::ParseFromString(ss.str(), &tech_pb);

  // TODO(aryap): This is a workaround for not having the package in the
  // tech_pb. Want to do something like:
  if (FLAGS_primitives != "") {
    vlsir::circuit::Package package_pb;
    std::fstream primitives_input(
        FLAGS_primitives, std::ios::in | std::ios::binary);
    LOG_IF(FATAL, !primitives_input)
        << "Could not open primitives protobuf, "
        << FLAGS_primitives;
    if (!package_pb.ParseFromIstream(&primitives_input)) {
      LOG(FATAL) << "Could not parse primitives protobuf, "
                 << FLAGS_primitives;
    }
    design_db.LoadPackage(package_pb);
  }

  bfg::PhysicalPropertiesDatabase &physical_db = design_db.physical_db();
  physical_db.LoadTechnologyFromFile(FLAGS_technology);

  bfg::SetUpSky130(&physical_db);

  // TODO(aryap): Need to clarify what 'external circuits' are. See note in
  // DesignDatabase.
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

  std::string top_name = "lut";
  bfg::tiles::LutB generator(&design_db, FLAGS_k_lut);
  bfg::Cell *top = generator.GenerateIntoDatabase(top_name);

  // TODO(aryap): This is temporary, to make sense of one possible netlist.
  design_db.WriteTop("sky130_mux",
                     "sky130_mux.library.pb",
                     "sky130_mux.package.pb",
                     true);
  design_db.WriteTop("lut_dfxtp_tap_template",
                     "lut_dfxtp_tap_template.library.pb",
                     "lut_dfxtp_tap_template.package.pb",
                     true);

  design_db.WriteTop(*top,
                     FLAGS_output_library,
                     FLAGS_output_package,
                     FLAGS_write_text_format);

  //Gf180McuMuxExperiment();

  google::protobuf::ShutdownProtobufLibrary();

  return EXIT_SUCCESS;
}

#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include <absl/cleanup/cleanup.h>
#include <absl/strings/str_join.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <google/protobuf/text_format.h>

#include "design_database.h"
#include "physical_properties_database.h"
#include "cell.h"
#include "layout.h"
#include "atoms/sky130_switch_complex.h"
#include "atoms/sky130_interconnect_mux1.h"
#include "atoms/sky130_interconnect_mux2.h"
#include "atoms/sky130_transmission_gate.h"
#include "atoms/sky130_transmission_gate_stack.h"
#include "atoms/sky130_mux.h"
#include "atoms/sky130_decap.h"
#include "atoms/sky130_dfxtp.h"
#include "atoms/gf180mcu_mux.h"
#include "tiles/lut.h"
#include "tiles/lut_b.h"
#include "tiles/slice.h"
#include "tiles/reduced_slice.h"
#include "tiles/interconnect.h"
#include "tiles/interconnect_wire_block.h"
#include "utility.h"

#include "proto/parameters/interconnect.pb.h"
#include "proto/parameters/interconnect_wire_block.pb.h"
#include "proto/parameters/lut_b.pb.h"
#include "proto/parameters/slice.pb.h"
#include "proto/parameters/reduced_slice.pb.h"
#include "proto/parameters/sky130_decap.pb.h"
#include "proto/parameters/sky130_dfxtp.pb.h"
#include "proto/parameters/sky130_interconnect_mux1.pb.h"
#include "proto/parameters/sky130_transmission_gate.pb.h"
#include "proto/parameters/sky130_transmission_gate_stack.pb.h"

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

DEFINE_string(run_generator, "", "Name of a generator to run");
DEFINE_string(
    params,
    "", "Path to text proto containing parameters for the named generator");

DEFINE_string(primitives, "primitives.pb", "Path to binary circuits proto");

// Demo flags.
DEFINE_int32(k_lut, 4, "How many LUT inputs");
DEFINE_bool(s44, false, "Whether to make an S44 LUT (override K selection)");

DECLARE_int32(jobs);

namespace { 

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

template<typename ProtoParam, typename GeneratorParam, typename Generator>
bfg::Cell *ReadParamsAndGenerate(
    const std::string &generator_name,
    const std::string &parameter_pb_path,
    bfg::DesignDatabase *design_db) {
  ProtoParam params_pb;
  bfg::Utility::ReadTextProtoOrDie(parameter_pb_path, &params_pb);

  GeneratorParam params;
  params.FromProto(params_pb);

  Generator generator(params, design_db);
  return generator.GenerateIntoDatabase(generator_name);
}

int DispatchGenerator(
    const std::string &generator_name,
    const std::string &parameter_pb_path,
    const std::string &output_prefix,
    bfg::DesignDatabase *design_db) {
  bfg::Cell *cell;
  if (generator_name == "Sky130TransmissionGate") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::Sky130TransmissionGate,
        bfg::atoms::Sky130TransmissionGate::Parameters,
        bfg::atoms::Sky130TransmissionGate>(
            generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "Sky130TransmissionGateStack") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::Sky130TransmissionGateStack,
        bfg::atoms::Sky130TransmissionGateStack::Parameters,
        bfg::atoms::Sky130TransmissionGateStack>(
            generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "Sky130InterconnectMux1") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::Sky130InterconnectMux1,
        bfg::atoms::Sky130InterconnectMux1::Parameters,
        bfg::atoms::Sky130InterconnectMux1>(
            generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "Sky130InterconnectMux2") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::Sky130InterconnectMux1,
        bfg::atoms::Sky130InterconnectMux1::Parameters,
        bfg::atoms::Sky130InterconnectMux2>(
            generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "Sky130Decap") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::Sky130Decap,
        bfg::atoms::Sky130Decap::Parameters,
        bfg::atoms::Sky130Decap>(generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "Sky130Dfxtp") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::Sky130Dfxtp,
        bfg::atoms::Sky130Dfxtp::Parameters,
        bfg::atoms::Sky130Dfxtp>(generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "LutB") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::LutB,
        bfg::tiles::LutB::Parameters,
        bfg::tiles::LutB>(generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "Interconnect") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::Interconnect,
        bfg::tiles::Interconnect::Parameters,
        bfg::tiles::Interconnect>(generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "InterconnectWireBlock") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::InterconnectWireBlock,
        bfg::tiles::InterconnectWireBlock::Parameters,
        bfg::tiles::InterconnectWireBlock>(
            generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "Slice") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::Slice,
        bfg::tiles::Slice::Parameters,
        bfg::tiles::Slice>(generator_name, parameter_pb_path, design_db);
  } else if (generator_name == "ReducedSlice") {
    cell = ReadParamsAndGenerate<
        bfg::proto::parameters::ReducedSlice,
        bfg::tiles::ReducedSlice::Parameters,
        bfg::tiles::ReducedSlice>(generator_name, parameter_pb_path, design_db);
  } else {
    LOG(ERROR) << "Unrecognised generator name: " << generator_name;
    return EXIT_FAILURE;
  }

  design_db->WriteTop(*cell,
                      absl::StrCat(output_prefix, ".library.pb"),
                      absl::StrCat(output_prefix, ".package.pb"),
                      FLAGS_write_text_format);
  return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  absl::Cleanup on_shutdown = [&]() {
    google::protobuf::ShutdownProtobufLibrary();
  };

  std::string version =
      "BFG v" xstr(bfg_VERSION_MAJOR) "." xstr(bfg_VERSION_MINOR);
  std::string compiler_info =
      "built with " xstr(CXX_COMPILER_PATH) " " xstr(CXX_COMPILER_VERSION);
  std::string title =
    absl::StrCat(version, " ", compiler_info);
  std::cout << title << std::endl;
  LOG(INFO) << title;

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

  if (FLAGS_run_generator != "")  {
    return DispatchGenerator(FLAGS_run_generator,
                        FLAGS_params,
                        FLAGS_output_library,
                        &design_db);
  }

  //bfg::atoms::Sky130SwitchComplex::Parameters sc_params;
  //bfg::atoms::Sky130SwitchComplex sc_generator(sc_params, &design_db);
  //bfg::Cell *switch_complex = sc_generator.GenerateIntoDatabase(
  //    "switch_complex");

  //design_db.WriteTop(*switch_complex,
  //                   "sky130_switch_complex.library.pb",
  //                   "sky130_switch_complex.package.pb",
  //                   FLAGS_write_text_format);

  bfg::atoms::Sky130TransmissionGateStack::Parameters tg_params = {
  };
  bfg::atoms::Sky130TransmissionGateStack tg_generator(tg_params, &design_db);
  bfg::Cell *transmission_gate_stack = tg_generator.GenerateIntoDatabase(
      "transmission_gate_stack");

  design_db.WriteTop(*transmission_gate_stack,
                     "sky130_transmission_gate_stack.library.pb",
                     "sky130_transmission_gate_stack.package.pb",
                     FLAGS_write_text_format);

  Gf180McuMuxExperiment();

  //return EXIT_SUCCESS;

  std::string top_name = "lut";
  bfg::tiles::LutB::Parameters lut_b_params = {
    .lut_size = static_cast<uint32_t>(FLAGS_k_lut)
  };
  bfg::tiles::LutB generator(lut_b_params, &design_db);
  bfg::Cell *top = generator.GenerateIntoDatabase(top_name);

  // TODO(aryap): This is temporary, to make sense of one possible netlist.
  design_db.WriteTop("sky130_mux",
                     "sky130_mux.library.pb",
                     "sky130_mux.package.pb",
                     true);
  //design_db.WriteTop("lut_dfxtp_tap_template",
  //                   "lut_dfxtp_tap_template.library.pb",
  //                   "lut_dfxtp_tap_template.package.pb",
  //                   true);

  design_db.WriteTop(*top,
                     FLAGS_output_library,
                     FLAGS_output_package,
                     FLAGS_write_text_format);

  return EXIT_SUCCESS;
}

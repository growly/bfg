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

#include "vlsir/tech.pb.h"
#include "vlsir/layout/raw.pb.h"

#include "c_make_header.h"

DEFINE_string(technology, "technology.pb", "Path to binary technology proto");
DEFINE_string(external_circuits, "", "Path to binary circuits proto");
DEFINE_string(output_library, "library.pb", "Output Vlsir Library path");
//DEFINE_bool(read_text_format, true, "Expect input protobufs in text format");
DEFINE_bool(write_text_format, true, "Also write text format protobufs");

DEFINE_string(primitives, "primitives.pb", "Path to binary circuits proto");

// Demo flags.
DEFINE_int32(k_lut, 4, "How many LUT inputs");
DEFINE_bool(s44, false, "Whether to make an S44 LUT (override K selection)");

void SetUpSky130(bfg::PhysicalPropertiesDatabase *db) {
  // Virtual layers for n-type and p-type diffusion rules.
  bfg::LayerInfo pdiff = db->GetLayerInfo("diff.drawing");
  pdiff.name = "pdiff";
  pdiff.purpose = "drawing";
  db->AddLayerInfo(pdiff);
  bfg::LayerInfo ndiff = db->GetLayerInfo("diff.drawing");
  ndiff.name = "ndiff";
  ndiff.purpose = "drawing";
  db->AddLayerInfo(ndiff);

  bfg::LayerInfo ncon = db->GetLayerInfo("licon.drawing");
  ncon.name = "ncon";
  ncon.purpose = "drawing";
  db->AddLayerInfo(ncon);
  bfg::LayerInfo pcon = db->GetLayerInfo("licon.drawing");
  pcon.name = "pcon";
  pcon.purpose = "drawing";
  db->AddLayerInfo(pcon);
  bfg::LayerInfo polycon = db->GetLayerInfo("licon.drawing");
  polycon.name = "polycon";
  polycon.purpose = "drawing";
  db->AddLayerInfo(polycon);

  std::cout << "\n" << db->DescribeLayers();

  // The following constants are specified in nm, but the internal units are the
  // minimum manufacturing grid resolution of also 1 nm.
  db->set_internal_units_per_external(1); 

  bfg::IntraLayerConstraints intra_constraints = {
    .min_separation = db->ToInternalUnits(270),
    .min_width = db->ToInternalUnits(170),
    .min_pitch = db->ToInternalUnits(170 + 170 + 80),
  };
  db->AddRules("diff.drawing", intra_constraints);
  db->AddRules("ndiff.drawing", intra_constraints);
  db->AddRules("pdiff.drawing", intra_constraints);

  intra_constraints = {
    .min_separation = db->ToInternalUnits(170),
    .min_width = db->ToInternalUnits(170),
    .min_pitch = db->ToInternalUnits(170 + 170 + 80),
  };
  db->AddRules("li.drawing", intra_constraints);
  intra_constraints = {
    .min_width = db->ToInternalUnits(170),
    .via_width = db->ToInternalUnits(170),
  };
  db->AddRules("ncon.drawing", intra_constraints);
  db->AddRules("pcon.drawing", intra_constraints);
  db->AddRules("polycon.drawing", intra_constraints);
  db->AddRules("licon.drawing", intra_constraints);
  db->AddRules("li.pin", intra_constraints);

  intra_constraints = {
    .min_separation = db->ToInternalUnits(190),
    .min_width = db->ToInternalUnits(170),
    .via_width = db->ToInternalUnits(170),
  };
  db->AddRules("mcon.drawing", intra_constraints);

  intra_constraints = {
    .min_width = db->ToInternalUnits(170),
    .via_width = db->ToInternalUnits(150),
  };
  db->AddRules("via1.drawing", intra_constraints);
  db->AddRules("via2.drawing", intra_constraints);
  intra_constraints = {
    .min_separation = db->ToInternalUnits(210),
    .min_width = db->ToInternalUnits(170),
    .min_pitch = db->ToInternalUnits(500),
  };
  db->AddRules("poly.drawing", intra_constraints);
  intra_constraints = {
    .min_separation = db->ToInternalUnits(140),
    .min_width = db->ToInternalUnits(140),
    .min_pitch = db->ToInternalUnits(340),
  };
  db->AddRules("met1.drawing", intra_constraints);
  // Lazy:
  db->AddRules("met2.drawing", intra_constraints);
  db->AddRules("met3.drawing", intra_constraints);

  bfg::InterLayerConstraints inter_constraints = {
    .min_separation = db->ToInternalUnits(55),
    .via_overhang = db->ToInternalUnits(80),
    .via_overhang_wide = db->ToInternalUnits(50)
  };
  db->AddRules("poly.drawing", "pcon.drawing", inter_constraints);
  db->AddRules("poly.drawing", "ncon.drawing", inter_constraints);
  db->AddRules("poly.drawing", "polycon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(50),
    .via_overhang = db->ToInternalUnits(80),
    .via_overhang_wide = db->ToInternalUnits(0)
  };
  db->AddRules("li.drawing", "pcon.drawing", inter_constraints);
  db->AddRules("li.drawing", "ncon.drawing", inter_constraints);
  db->AddRules("li.drawing", "polycon.drawing", inter_constraints);
  db->AddRules("li.drawing", "licon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(40),
    // This is minimum enclosure in 1 direction?
    .min_enclosure = db->ToInternalUnits(60),
    .via_overhang = db->ToInternalUnits(40),
  };
  db->AddRules("ndiff.drawing", "pcon.drawing", inter_constraints);
  db->AddRules("ndiff.drawing", "ncon.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "pcon.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "ncon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(190),
    // TODO(aryap): I don't think this name captures what this rules is.
    // This is the min overhang of one layer by the other.
    .min_enclosure = db->ToInternalUnits(130),
    .via_overhang = db->ToInternalUnits(40),
  };
  db->AddRules("ndiff.drawing", "polycon.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "polycon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(50),
    .via_overhang = db->ToInternalUnits(60),
    .via_overhang_wide = db->ToInternalUnits(30)
  };
  db->AddRules("li.drawing", "mcon.drawing", inter_constraints);
  db->AddRules("met1.drawing", "mcon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(50),
    .via_overhang = db->ToInternalUnits(85),
    .via_overhang_wide = db->ToInternalUnits(55)
  };
  db->AddRules("met1.drawing", "via1.drawing", inter_constraints);
  db->AddRules("met2.drawing", "via1.drawing", inter_constraints);
  // Lazy but doesn't make sense:
  db->AddRules("met2.drawing", "via2.drawing", inter_constraints);
  db->AddRules("met3.drawing", "via2.drawing", inter_constraints);
  // TODO(growly): Need to alias these layer names so that they apply to any
  // process.
  inter_constraints = {
    .min_enclosure = db->ToInternalUnits(180),
  };
  db->AddRules("ndiff.drawing", "nsdm.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "psdm.drawing", inter_constraints);
  db->AddRules("ndiff.drawing", "nwell.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "nwell.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(340)
  };
  db->AddRules("nsdm.drawing", "nwell.drawing", inter_constraints);
}

void SetUpGf180Mcu(bfg::PhysicalPropertiesDatabase *db) {
  vlsir::tech::Technology tech_pb;
  std::string pdk_file_name = "gf180mcu.technology.pb.txt";
  std::ifstream pdk_file(pdk_file_name);
  LOG_IF(FATAL, !pdk_file.is_open())
      << "Could not open PDK descriptor file: " << pdk_file_name;
  std::ostringstream ss;
  ss << pdk_file.rdbuf();
  google::protobuf::TextFormat::ParseFromString(ss.str(), &tech_pb);

  db->LoadTechnology(tech_pb);

  // The manufacturing grid is 0.005 um, so our base unit should be some
  // multiple of that. If we just make 1 unit = 0.005 um, then that's
  //    1/0.005 = 200
  // internal units per um.
  //
  // If we instead decide that the external unit is 1 nm, then
  //    1 internal unit = 5 nm
  //    1/5 = 0.2
  db->set_internal_units_per_external(0.2); 

  // In sky130, the GDS output scale (according to magic) is 1 unit = 10 nm.
  //    scalefactor 10 nanometers
  // In gf180mcu, it's 1 unit = 50 nm.
  //    scalefactor 50 nanometers
  // The drc rules say "scalefactor 50" (no "nanometres") which I think means 1
  // unit = 500 nm = 0.5 um instead.
  //
  // By then by comparing with the klayout rule deck, it seems the rules are
  // still in nm.
  //
  // Does scalefactor refer to the magic internal grid or something?
  //
  // We'll keep the internal units in nanometres and convert accordingly.

  //std::fstream technology_input(
  //    FLAGS_technology, std::ios::in | std::ios::binary);
  //LOG_IF(FATAL, !technology_input)
  //    << "Could not open technology protobuf, "
  //    << FLAGS_technology;
  //if (!tech_pb.ParseFromIstream(&technology_input)) {
  //  LOG(FATAL) << "Could not parse technology protobuf, "
  //             << FLAGS_technology;
  //}

  // diff.drawing -> COMP 22/0
  // li.drawing -> Metal1 34/0
  // nsdm.drawing -> Nplus 32/0
  // psdm.drawing -> Pplus 31/0
  // poly.drawing -> Poly2 30/0
  // nwell.drawing -> Nwell 21/0
  // licon.drawing -> Contact 33/0
  db->AddLayerAlias("diff.drawing", "comp.comp");
  db->AddLayerAlias("nsdm.drawing", "nplus.nplus");
  db->AddLayerAlias("psdm.drawing", "pplus.pplus");
  db->AddLayerAlias("poly.drawing", "poly2.poly2");
  db->AddLayerAlias("nwell.drawing", "nwell.nwell");

  db->AddLayerAlias("li.drawing", "metal1.metal1");
  db->AddLayerAlias("met1.drawing", "metal2.metal2");
  db->AddLayerAlias("met2.drawing", "metal3.metal3");
  db->AddLayerAlias("met3.drawing", "metal4.metal4");

  db->AddLayerAlias("licon.drawing", "contact.contact");
  db->AddLayerAlias("mcon.drawing", "via1.via1");
  db->AddLayerAlias("via1.drawing", "via2.via2");
  db->AddLayerAlias("via2.drawing", "via3.via3");

  db->AddLayerAlias("areaid.standardrc", "pr_boundary.pr_boundary");

  // Virtual layers for n-type and p-type diffusion rules.
  bfg::LayerInfo pdiff = db->GetLayerInfo("diff.drawing");
  pdiff.name = "pdiff";
  pdiff.purpose = "drawing";
  db->AddLayerInfo(pdiff);
  bfg::LayerInfo ndiff = db->GetLayerInfo("diff.drawing");
  ndiff.name = "ndiff";
  ndiff.purpose = "drawing";
  db->AddLayerInfo(ndiff);

  // Virtual layers to separate diffusion contacts and poly contacts.
  bfg::LayerInfo ncon = db->GetLayerInfo("licon.drawing");
  ncon.name = "ncon";
  ncon.purpose = "drawing";
  db->AddLayerInfo(ncon);
  bfg::LayerInfo pcon = db->GetLayerInfo("licon.drawing");
  pcon.name = "pcon";
  pcon.purpose = "drawing";
  db->AddLayerInfo(pcon);
  bfg::LayerInfo polycon = db->GetLayerInfo("licon.drawing");
  polycon.name = "polycon";
  polycon.purpose = "drawing";
  db->AddLayerInfo(polycon);

  db->AddLayerAlias("li.pin", "text.text");

  LOG(INFO) << db->DescribeLayers();

  // We are targeting 5 V for this experiment.
  bfg::IntraLayerConstraints intra_constraints = {
    .min_separation = db->ToInternalUnits(280),
    .min_width = db->ToInternalUnits(300),
  };
  db->AddRules("diff.drawing", intra_constraints);
  db->AddRules("ndiff.drawing", intra_constraints);
  db->AddRules("pdiff.drawing", intra_constraints);

  intra_constraints = {
    .min_separation = db->ToInternalUnits(230),
    .min_width = db->ToInternalUnits(230),
    .min_pitch = db->ToInternalUnits(230 + 230 + 80),
  };
  db->AddRules("li.drawing", intra_constraints);
  intra_constraints = {
    .via_width = db->ToInternalUnits(220),
  };
  db->AddRules("ncon.drawing", intra_constraints);
  db->AddRules("pcon.drawing", intra_constraints);
  db->AddRules("polycon.drawing", intra_constraints);
  db->AddRules("licon.drawing", intra_constraints);
  //db->AddRules("li.pin", intra_constraints);
  db->AddRules("mcon.drawing", intra_constraints);
  intra_constraints = {
    .min_width = db->ToInternalUnits(170),
    .via_width = db->ToInternalUnits(150),
  };
  db->AddRules("via1.drawing", intra_constraints);
  db->AddRules("via2.drawing", intra_constraints);
  intra_constraints = {
    .min_separation = db->ToInternalUnits(240),
    .min_width = db->ToInternalUnits(180),
    .min_pitch = db->ToInternalUnits(500),
  };
  db->AddRules("poly.drawing", intra_constraints);

  intra_constraints = {
    .min_separation = db->ToInternalUnits(200),
    .min_width = db->ToInternalUnits(140),
    .min_pitch = db->ToInternalUnits(340),
  };
  db->AddRules("met1.drawing", intra_constraints);
  // Lazy:
  db->AddRules("met2.drawing", intra_constraints);
  db->AddRules("met3.drawing", intra_constraints);

  bfg::InterLayerConstraints inter_constraints = {
    // TODO(aryap): I'm using this as an 'overhang', essentially, but it
    // doesn't actually imply complete enclosure.
    .min_enclosure = db->ToInternalUnits(230),
  };
  db->AddRules("ndiff.drawing", "poly.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "poly.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(150),
    .via_overhang = db->ToInternalUnits(80),
    .via_overhang_wide = db->ToInternalUnits(50)
  };
  db->AddRules("poly.drawing", "pcon.drawing", inter_constraints);
  db->AddRules("poly.drawing", "ncon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(150),
    .min_enclosure = db->ToInternalUnits(70),
    .via_overhang = db->ToInternalUnits(80),
    .via_overhang_wide = db->ToInternalUnits(70)
  };
  db->AddRules("poly.drawing", "polycon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(150),
    .via_overhang = db->ToInternalUnits(80),
    .via_overhang_wide = db->ToInternalUnits(55)
  };
  db->AddRules("li.drawing", "pcon.drawing", inter_constraints);
  db->AddRules("li.drawing", "ncon.drawing", inter_constraints);
  db->AddRules("li.drawing", "polycon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(40),
    .max_separation = db->ToInternalUnits(190),
    // This is minimum enclosure in db->ToInternalUnits(1) direction?
    .min_enclosure = db->ToInternalUnits(70),
    .via_overhang = db->ToInternalUnits(40),
  };
  db->AddRules("ndiff.drawing", "ncon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(150),
    .max_separation = db->ToInternalUnits(170),
    .min_enclosure = db->ToInternalUnits(70),
    .via_overhang = db->ToInternalUnits(40),
  };
  db->AddRules("pdiff.drawing", "pcon.drawing", inter_constraints);
  db->AddRules("ndiff.drawing", "polycon.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "polycon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(50),
    .via_overhang = db->ToInternalUnits(60),
    .via_overhang_wide = db->ToInternalUnits(30)
  };
  db->AddRules("li.drawing", "mcon.drawing", inter_constraints);
  db->AddRules("met1.drawing", "mcon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(50),
    .via_overhang = db->ToInternalUnits(85),
    .via_overhang_wide = db->ToInternalUnits(55)
  };
  db->AddRules("met1.drawing", "via1.drawing", inter_constraints);
  db->AddRules("met2.drawing", "via1.drawing", inter_constraints);
  // Lazy but doesn't make sense:
  db->AddRules("met2.drawing", "via2.drawing", inter_constraints);
  db->AddRules("met3.drawing", "via2.drawing", inter_constraints);
  // TODO(growly): Need to alias these layer names so that they apply to any
  // process.
  inter_constraints = {
    .min_enclosure = db->ToInternalUnits(230),
  };
  db->AddRules("diff.drawing", "nsdm.drawing", inter_constraints);
  db->AddRules("ndiff.drawing", "nsdm.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "nsdm.drawing", inter_constraints);
  db->AddRules("diff.drawing", "psdm.drawing", inter_constraints);
  db->AddRules("ndiff.drawing", "psdm.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "psdm.drawing", inter_constraints);
  db->AddRules("diff.drawing", "nwell.drawing", inter_constraints);
  db->AddRules("ndiff.drawing", "nwell.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "nwell.drawing", inter_constraints);
  inter_constraints = {
    .min_enclosure = db->ToInternalUnits(180)
  };
  db->AddRules("psdm.drawing", "nwell.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(430),
  };
  // This is spacing to ndiff
  db->AddRules("ndiff.drawing", "nwell.drawing", inter_constraints);
  inter_constraints = {
    .min_enclosure = db->ToInternalUnits(430)
  };
  db->AddRules("pdiff.drawing", "nwell.drawing", inter_constraints);
}

void Gf180McuMuxExperiment() {
  // GF180MCU 7T MUX experiment
  //
  bfg::DesignDatabase design_db;
  bfg::PhysicalPropertiesDatabase &physical_db = design_db.physical_db();

  SetUpGf180Mcu(&physical_db);

  std::string top_name = "gf180mcu_mux";
  bfg::atoms::Sky130Mux::Parameters params;
  bfg::atoms::Gf180McuMux generator(params, &design_db);
  bfg::Cell *top = generator.GenerateIntoDatabase(top_name);

  design_db.WriteTop(*top, "gf180_mux.pb", "gf180_mux.pb.txt");
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
  physical_db.LoadTechnology(tech_pb);

  SetUpSky130(&physical_db);

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
  bfg::tiles::Lut generator(&design_db, FLAGS_k_lut);
  bfg::Cell *top = generator.GenerateIntoDatabase(top_name);

  design_db.WriteTop(
      *top, FLAGS_output_library, FLAGS_write_text_format);

  Gf180McuMuxExperiment();

  google::protobuf::ShutdownProtobufLibrary();

  return EXIT_SUCCESS;
}

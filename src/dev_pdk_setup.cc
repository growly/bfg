#include <fstream>

#include "dev_pdk_setup.h"

#include "physical_properties_database.h"

#include <google/protobuf/text_format.h>

namespace bfg {

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

  //std::cout << "\n" << db->DescribeLayers();

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
    // 0.0561 um^2 = 56100 nm^2.
    .min_area = db->ToInternalUnits(56100)
  };
  db->AddRules("li.drawing", intra_constraints);
  intra_constraints = {
    .min_separation = db->ToInternalUnits(170),
    .min_width = db->ToInternalUnits(170),
    .via_width = db->ToInternalUnits(170),
    .via_height = db->ToInternalUnits(170)
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
    .via_height = db->ToInternalUnits(170)
  };
  db->AddRules("mcon.drawing", intra_constraints);

  intra_constraints = {
    .min_width = db->ToInternalUnits(170),
    .via_width = db->ToInternalUnits(150),
    .via_height = db->ToInternalUnits(150)
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
    //  poly.4: (min) "Spacing of poly on field to diff (parallel edges only)"
    .min_separation = db->ToInternalUnits(75),
    //  poly.8: (min) "Extension of poly beyond diffusion (endcap)",
    .min_enclosure = db->ToInternalUnits(130),
    //  poly.7: (min) "Extension of diff beyond poly (min drain)"
    .min_extension = db->ToInternalUnits(250)
  };
  db->AddRules("poly.drawing", "pdiff.drawing", inter_constraints);
  db->AddRules("poly.drawing", "ndiff.drawing", inter_constraints);

  inter_constraints = {
    .min_enclosure = db->ToInternalUnits(100)
  };
  db->AddRules("npc.drawing", "polycon.drawing", inter_constraints);

  inter_constraints = {
    .min_separation = db->ToInternalUnits(50),
    .via_overhang = db->ToInternalUnits(80),
    .via_overhang_wide = db->ToInternalUnits(0)
  };
  db->AddRules("li.drawing", "pcon.drawing", inter_constraints);
  db->AddRules("li.drawing", "ncon.drawing", inter_constraints);
  db->AddRules("li.drawing", "polycon.drawing", inter_constraints);

  inter_constraints = {
    .min_separation = db->ToInternalUnits(50),
    .min_enclosure = db->ToInternalUnits(80),    // li.5.-
    .via_overhang = db->ToInternalUnits(80),
    .via_overhang_wide = db->ToInternalUnits(0)
  };
  db->AddRules("li.drawing", "licon.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(40),
    // licon.5c
    .min_enclosure = db->ToInternalUnits(60),
    // licon.5a
    .min_enclosure_alt = db->ToInternalUnits(40),
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
  // TODO(aryap): What is this? Is it for the tap? ncon, pcon and polycon are
  // all just licon (see above). Have to find where this is used to determine
  // what I meant.
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
  inter_constraints = {
    .via_overhang = db->ToInternalUnits(120),
    .via_overhang_wide = 0
  };
  db->AddRules("tap.drawing", "licon.drawing", inter_constraints);
  intra_constraints = {
    // 0.07011 um^2 = 70110 nm^2.
    .min_area = db->ToInternalUnits(70110)
  };
  db->AddRules("tap.drawing", intra_constraints);
  // TODO(growly): Need to alias these layer names so that they apply to any
  // process.
  inter_constraints = {
    .min_enclosure = db->ToInternalUnits(180),
  };
  db->AddRules("ndiff.drawing", "nsdm.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "psdm.drawing", inter_constraints);
  db->AddRules("pdiff.drawing", "nwell.drawing", inter_constraints);

  inter_constraints = {
    .min_separation = db->ToInternalUnits(340)
  };
  db->AddRules("ndiff.drawing", "nwell.drawing", inter_constraints);

  inter_constraints = {
    .min_separation = db->ToInternalUnits(130),
    .min_enclosure = db->ToInternalUnits(130)
  };
  db->AddRules("tap.drawing", "psdm.drawing", inter_constraints);
  db->AddRules("tap.drawing", "nsdm.drawing", inter_constraints);
  inter_constraints = {
    .min_separation = db->ToInternalUnits(340),
  };
  db->AddRules("nsdm.drawing", "nwell.drawing", inter_constraints);
  inter_constraints = {
    .min_enclosure = db->ToInternalUnits(180)   // TODO(growly): What is this
                                                // value?
  };
  db->AddRules("tap.drawing", "nwell.drawing", inter_constraints);
  intra_constraints = {
    .min_width = db->ToInternalUnits(840)
  };
  db->AddRules("nwell.drawing", intra_constraints);

  db->AddViaLayer("ndiff.drawing", "li.drawing", "licon.drawing");
  db->AddViaLayer("pdiff.drawing", "li.drawing", "licon.drawing");
  db->AddViaLayer("li.drawing", "met1.drawing", "mcon.drawing");
  db->AddViaLayer("met1.drawing", "met2.drawing", "via1.drawing");
  db->AddViaLayer("met2.drawing", "met3.drawing", "via2.drawing");
  db->AddViaLayer("capm.drawing", "met4.drawing", "via3.drawing");
  db->AddViaLayer("met3.drawing", "met4.drawing", "via3.drawing");
  db->AddViaLayer("cap2m.drawing", "met5.drawing", "via4.drawing");
  db->AddViaLayer("met4.drawing", "met5.drawing", "via4.drawing");
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

  //LOG(INFO) << db->DescribeLayers();

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
  db->AddRules("li.drawing", "licon.drawing", inter_constraints);

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
  db->AddRules("pdiff.drawing", "nwell.drawing", inter_constraints);
  db->AddRules("ndiff.drawing", "nwell.drawing", inter_constraints);

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

}   // namespace bfg

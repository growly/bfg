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
DEFINE_int32(port, 8222, "Listen port for the service");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  std::string version =
      "BFG Router Service v" xstr(bfg_VERSION_MAJOR) "." xstr(bfg_VERSION_MINOR);
  std::cout << version << std::endl;
  LOG(INFO) << version << " start";
}

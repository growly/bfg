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

#include "c_make_header.h"

DEFINE_string(example_flag, "default", "for later");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::string version = "BFG v" xstr(bfg_VERSION_MAJOR) "." xstr(bfg_VERSION_MINOR);
  std::cout << version << std::endl;
  LOG(INFO) << version << " entry";

  return EXIT_SUCCESS;
}

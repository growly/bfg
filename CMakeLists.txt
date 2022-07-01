cmake_minimum_required(VERSION 3.14)

project(bfg VERSION 0.1)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# For googletest on Windows.
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

find_package(gflags REQUIRED)
find_package(glog 0.5.0 REQUIRED)
find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

include_directories(${GTEST_INCLUDE_DIR})

set(PROTO_SRC
  ${PROJECT_SOURCE_DIR}/proto/inflator_rules.proto
)

set(VLSIR_SRC
  ${PROJECT_SOURCE_DIR}/vlsir/circuit.proto
  ${PROJECT_SOURCE_DIR}/vlsir/raw.proto
  ${PROJECT_SOURCE_DIR}/vlsir/utils.proto
  ${PROJECT_SOURCE_DIR}/vlsir/pdk.proto
)

set(SRC
  ${PROJECT_SOURCE_DIR}/src/circuit.cc
  ${PROJECT_SOURCE_DIR}/src/circuit_primitives_database.cc
  ${PROJECT_SOURCE_DIR}/src/layout.cc
  ${PROJECT_SOURCE_DIR}/src/physical_properties_database.cc
  ${PROJECT_SOURCE_DIR}/src/poly_line_cell.cc
  ${PROJECT_SOURCE_DIR}/src/poly_line_inflator.cc
  #${PROJECT_SOURCE_DIR}/src/routing_grid.cc
  ${PROJECT_SOURCE_DIR}/src/abstract_via.cc
)

#set(CIRCUIT_SRC
#  ${PROJECT_SOURCE_DIR}/src/circuit/port.cc
#  ${PROJECT_SOURCE_DIR}/src/circuit/signal.cc
#)

set(ATOMS_SRC
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_buf.cc
)

set(GEOMETRY_SRC 
  ${PROJECT_SOURCE_DIR}/src/geometry/instance.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/label.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/line.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/point.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/poly_line.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/polygon.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/rectangle.cc
)

set(CIRCUIT_SRC
  ${PROJECT_SOURCE_DIR}/src/circuit/instance.cc
)

# protobuf configuration
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# set(Protobuf_IMPORT_DIRS ...)
# set(Protobuf_DEBUG ...)
protobuf_generate_cpp(GENERATED_PROTO_SRCS
                      PROTO_HDRS
                      ${PROTO_SRC}
                      ${VLSIR_SRC})

# TODO(aryap): Make this a library that both the main and the test
# binaries can link against, instead of compiling twice:
set(EXEC_SRC
  ${SRC}
  ${ATOMS_SRC}
  ${GEOMETRY_SRC}
  ${CIRCUIT_SRC}
  ${GENERATED_PROTO_SRCS}
  ${PROTO_HDRS}
)

add_executable(bfg
  src/main.cc
  ${EXEC_SRC})

target_include_directories(bfg PUBLIC
                           "${PROJECT_BINARY_DIR}/src")

find_library(tcmalloc_lib NAMES tcmalloc REQUIRED)
message(STATUS ${tcmalloc_lib})

target_link_libraries(bfg PUBLIC ${tcmalloc_lib}
                                      gflags
                                      glog::glog
                                      absl::strings
                                      ${Protobuf_LIBRARIES})

configure_file(src/c_make_header.h.in src/c_make_header.h)

# Tests.

set(GEOMETRY_TEST_SRC
  ${PROJECT_SOURCE_DIR}/src/geometry/line_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/rectangle_test.cc
)

set(TEST_SRC
  ${GEOMETRY_TEST_SRC}
)

enable_testing()
add_executable(bfg_test ${TEST_SRC} ${EXEC_SRC})
target_link_libraries(bfg_test
  Threads::Threads
  gtest
  gtest_main
  ${tcmalloc_lib}
  gflags
  glog::glog
  absl::strings
  ${Protobuf_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(bfg_test)

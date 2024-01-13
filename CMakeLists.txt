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

set(ABSL_PROPAGATE_CXX_STD true)
set(ABSL_CXX_STANDARD 17)
set(BUILD_DEPS true)

include_directories(${GTEST_INCLUDE_DIR})

#set(PROTO_SRC_DIR ${PROJECT_SOURCE_DIR}/proto)
#set(PROTO_SRC
#  ${PROTO_SRC_DIR}/inflator_rules.proto
#)

set(VLSIR_SRC_DIR ${PROJECT_SOURCE_DIR}/vlsir/protos)
file(GLOB_RECURSE VLSIR_SRC "${VLSIR_SRC_DIR}/*.proto")
#set(VLSIR_SRC
#  ${VLSIR_SRC_DIR}/circuit.proto
#  ${VLSIR_SRC_DIR}/layout/raw.proto
#  ${VLSIR_SRC_DIR}/utils.proto
#  ${VLSIR_SRC_DIR}/tech.proto
#)

set(SRC
  ${PROJECT_SOURCE_DIR}/src/cell.cc
  ${PROJECT_SOURCE_DIR}/src/circuit.cc
  ${PROJECT_SOURCE_DIR}/src/circuit_primitives_database.cc
  ${PROJECT_SOURCE_DIR}/src/layout.cc
  ${PROJECT_SOURCE_DIR}/src/design_database.cc
  ${PROJECT_SOURCE_DIR}/src/physical_properties_database.cc
  ${PROJECT_SOURCE_DIR}/src/parameter.cc
  ${PROJECT_SOURCE_DIR}/src/poly_line_cell.cc
  ${PROJECT_SOURCE_DIR}/src/poly_line_inflator.cc
  ${PROJECT_SOURCE_DIR}/src/routing_edge.cc
  ${PROJECT_SOURCE_DIR}/src/routing_grid.cc
  ${PROJECT_SOURCE_DIR}/src/routing_grid_geometry.cc
  ${PROJECT_SOURCE_DIR}/src/routing_path.cc
  ${PROJECT_SOURCE_DIR}/src/routing_track.cc
  ${PROJECT_SOURCE_DIR}/src/routing_track_blockage.cc
  ${PROJECT_SOURCE_DIR}/src/routing_vertex.cc
  ${PROJECT_SOURCE_DIR}/src/abstract_via.cc
  ${PROJECT_SOURCE_DIR}/src/row_guide.cc
)

set(TILES_SRC
  ${PROJECT_SOURCE_DIR}/src/tiles/lut.cc
)

set(ATOMS_SRC
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_buf.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_dfxtp.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_mux.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_tap.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/gf180mcu_mux.cc
)

set(GEOMETRY_SRC 
  ${PROJECT_SOURCE_DIR}/src/geometry/instance.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/label.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/line.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/point.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/point_or_choice.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/poly_line.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/polygon.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/rectangle.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/shape_collection.cc
)

set(CIRCUIT_SRC
  ${PROJECT_SOURCE_DIR}/src/circuit/instance.cc
  ${PROJECT_SOURCE_DIR}/src/circuit/port.cc
  ${PROJECT_SOURCE_DIR}/src/circuit/signal.cc
  ${PROJECT_SOURCE_DIR}/src/circuit/slice.cc
  ${PROJECT_SOURCE_DIR}/src/circuit/connection.cc
)


# protobuf configuration
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# set(Protobuf_IMPORT_DIRS ...)
# set(Protobuf_DEBUG ...)

# This function that comes with the Protobuf CMake package annoyingly reports
# that all generated sources are in the root of the build directory, even if
# they come from nested paths. If you set 
#   set(PROTOBUF_GENERATE_CPP_APPEND_PATH OFF)
# then the include paths given to protoc don't work and it breaks. This seems to
# be a limitation of the Protobuf package for CMake, so we'll invoke protoc
# ourselves.
#protobuf_generate_cpp(GENERATED_PROTO_SRCS
#                      GENERATED_PROTO_HDRS
#                      ${PROTO_SRC}
#                      ${VLSIR_SRC})


# protoc will generate an output tree of files depending on the include path
# given, which should be the root of the proto files as reference in their
# respect `import`s.
#   https://github.com/protocolbuffers/protobuf/issues/723#issuecomment-132845240
#
# So we generate each proto tree (vlsir, proto) into its own subtree, not just
# the ${PROJECT_BINARY_DIR} root.
set(GENERATED_PROTO_SRCS)
set(GENERATED_PROTO_HDRS)
function(generate_protos proto_src_root proto_files out_dir)
  foreach(file ${proto_files})
    message(STATUS "Generating C++ proto from: ${file} -> ${out_dir}")
    # Get the relative path of the proto to the source dir root.
    file(RELATIVE_PATH relative_file ${proto_src_root} ${file})
    get_filename_component(proto_dir ${relative_file} DIRECTORY)
    get_filename_component(proto_name ${relative_file} NAME_WE)
    set(proto_hdr ${out_dir}/${proto_dir}/${proto_name}.pb.h)
    set(proto_src ${out_dir}/${proto_dir}/${proto_name}.pb.cc)
    message(STATUS \tfile=${file})
    message(STATUS \trelative_file=${relative_file})
    message(STATUS \tproto_dir=${proto_dir})
    message(STATUS \tproto_hdr=${proto_hdr})
    message(STATUS \tproto_src=${proto_src})
    add_custom_command(
      OUTPUT ${proto_hdr} ${proto_src}
      COMMAND protobuf::protoc
      "--proto_path=${proto_src_root}"
      "--cpp_out=${out_dir}"
      ${file}
      DEPENDS ${file} protobuf::protoc
      COMMENT "Generate C++ protocol buffer for ${file}"
      VERBATIM)
    list(APPEND GENERATED_PROTO_HDRS ${proto_hdr})
    list(APPEND GENERATED_PROTO_SRCS ${proto_src})
  endforeach()

  set(GENERATED_PROTO_HDRS ${GENERATED_PROTO_HDRS} PARENT_SCOPE)
  set(GENERATED_PROTO_SRCS ${GENERATED_PROTO_SRCS} PARENT_SCOPE)
endfunction()

# Vlsir
# -----
#
# Built as a library so that the proto import/include paths, which are relative
# to the root of the vlsir/protos directory, keep working. These are out of our
# control.
#
# Use this to generate the protobuf sources into the build directory at the same
# relative directory path as they were to the original source directory. e.g.,
# if they appear in the source tree at ${REPO_ROOT}/vlsir/protos, they will be
# generated into ${BUILD_ROOT}/vlsir/protos.
#   file(
#       RELATIVE_PATH vlsir_src_relative ${PROJECT_SOURCE_DIR} ${VLSIR_SRC_DIR})
#   set(VLSIR_OUT_DIR ${PROJECT_BINARY_DIR}/${vlsir_src_relative})
#
# This forces them into ${BUILD_ROOT}/vlsir, to simplify all the include paths
# throughout the code.
set(VLSIR_OUT_DIR ${PROJECT_BINARY_DIR}/vlsir)
file(MAKE_DIRECTORY ${VLSIR_OUT_DIR})
# Need quotes around "${VLSIR_SRC}" to ensure it gets passed as the
# semicolon-delimited list that it is (otherwise only the first argument gets
# in).
generate_protos(${VLSIR_SRC_DIR} "${VLSIR_SRC}" ${VLSIR_OUT_DIR})
set(VLSIR_GENERATED_HDRS ${GENERATED_PROTO_HDRS})
set(VLSIR_GENERATED_SRCS ${GENERATED_PROTO_SRCS})

add_library(vlsir_lib SHARED 
  ${VLSIR_GENERATED_SRCS}
  ${VLSIR_GENERATED_HDRS})
set_target_properties(vlsir_lib PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(vlsir_lib PRIVATE ${VLSIR_OUT_DIR})

# Internal protocol buffers
# -------------------------
#
# Included directly as code in our main library.

#file(RELATIVE_PATH proto_src_relative ${PROJECT_SOURCE_DIR} ${PROTO_SRC_DIR})
#set(PROTO_OUT_DIR ${PROJECT_BINARY_DIR}/${proto_src_relative})
#set(GENERATED_PROTO_SRCS)
#set(GENERATED_PROTO_HDRS)
#generate_protos(${PROTO_SRC_DIR} "${PROTO_SRC}" ${PROJECT_BINARY_DIR})
#set(PROTO_GENERATED_HDRS ${GENERATED_PROTO_HDRS})
#set(PROTO_GENERATED_SRCS ${GENERATED_PROTO_SRCS})

# bfg library
# -----------

set(BFG_LIB_SRC
  ${SRC}
  ${ATOMS_SRC}
  ${TILES_SRC}
  ${CIRCUIT_SRC}
  ${GEOMETRY_SRC}
  ${CIRCUIT_SRC}
  ${PROTO_GENERATED_SRCS}
)

add_library(bfg STATIC ${BFG_LIB_SRC})
target_include_directories(bfg PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           ${VLSIR_OUT_DIR})

find_library(tcmalloc_lib NAMES tcmalloc REQUIRED)
message(STATUS "found ${tcmalloc_lib}")
message(STATUS \t${Protobuf_LIBRARIES})

target_link_libraries(bfg PUBLIC ${tcmalloc_lib}
                                      gflags
                                      glog::glog
                                      absl::base
                                      absl::strings
                                      absl::str_format
                                      absl::status
                                      absl::statusor
                                      vlsir_lib
                                      ${Protobuf_LIBRARIES})

configure_file(src/c_make_header.h.in src/c_make_header.h)

# bfg executable
# --------------

add_executable(bfg_bin src/main.cc)
set_target_properties(bfg_bin PROPERTIES OUTPUT_NAME bfg)

target_include_directories(bfg_bin PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           ${VLSIR_OUT_DIR})

target_link_libraries(bfg_bin PUBLIC bfg
                                      ${tcmalloc_lib}
                                      gflags
                                      glog::glog
                                      absl::strings
                                      ${Protobuf_LIBRARIES})

# Tests
# -----

set(GEOMETRY_TEST_SRC
  ${PROJECT_SOURCE_DIR}/src/poly_line_inflator_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/instance_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/polygon_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/poly_line_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/line_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/rectangle_test.cc
)

set(TEST_SRC
  ${GEOMETRY_TEST_SRC}
  ${PROJECT_SOURCE_DIR}/src/routing_grid_geometry_test.cc
  ${PROJECT_SOURCE_DIR}/src/routing_edge_test.cc
)

enable_testing()
add_executable(bfg_test ${TEST_SRC})
target_link_libraries(bfg_test
  bfg
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

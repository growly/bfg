cmake_minimum_required(VERSION 3.14)

project(bfg VERSION 0.1)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Do this instead of 
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps")
#   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -save-temps")
#   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -save-temps")
add_compile_options(-save-temps)

# For googletest on Windows.
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

find_package(gflags REQUIRED)
find_package(glog 0.5.0 REQUIRED)
find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

configure_file(src/c_make_header.h.in src/c_make_header.h)

# Comment to disable tcmalloc.
find_library(tcmalloc_lib NAMES tcmalloc REQUIRED)
message(STATUS "Found tcmalloc: " ${tcmalloc_lib})
message(STATUS "protobuf libraries: " ${Protobuf_LIBRARIES})

set(ABSL_PROPAGATE_CXX_STD true)
set(ABSL_CXX_STANDARD 17)
set(BUILD_DEPS true)

include_directories(${GTEST_INCLUDE_DIR})

set(PROTO_SRC_DIR ${PROJECT_SOURCE_DIR}/proto)
file(GLOB_RECURSE PROTO_SRC "${PROTO_SRC_DIR}/*.proto")
#set(PROTO_SRC
#  ${PROTO_SRC_DIR}/router_service.proto
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
  ${PROJECT_SOURCE_DIR}/src/abstract_via.cc
  ${PROJECT_SOURCE_DIR}/src/checkerboard_guide.cc
  ${PROJECT_SOURCE_DIR}/src/cell.cc
  ${PROJECT_SOURCE_DIR}/src/circuit.cc
  ${PROJECT_SOURCE_DIR}/src/circuit_primitives_database.cc
  ${PROJECT_SOURCE_DIR}/src/design_database.cc
  ${PROJECT_SOURCE_DIR}/src/equivalent_nets.cc
  ${PROJECT_SOURCE_DIR}/src/layout.cc
  ${PROJECT_SOURCE_DIR}/src/scoped_layer.cc
  ${PROJECT_SOURCE_DIR}/src/memory_bank.cc
  ${PROJECT_SOURCE_DIR}/src/parameter.cc
  ${PROJECT_SOURCE_DIR}/src/physical_properties_database.cc
  ${PROJECT_SOURCE_DIR}/src/poly_line_cell.cc
  ${PROJECT_SOURCE_DIR}/src/poly_line_inflator.cc
  ${PROJECT_SOURCE_DIR}/src/routing_edge.cc
  ${PROJECT_SOURCE_DIR}/src/routing_grid.cc
  ${PROJECT_SOURCE_DIR}/src/routing_grid_geometry.cc
  ${PROJECT_SOURCE_DIR}/src/routing_path.cc
  ${PROJECT_SOURCE_DIR}/src/routing_track.cc
  ${PROJECT_SOURCE_DIR}/src/routing_track_blockage.cc
  ${PROJECT_SOURCE_DIR}/src/routing_track_direction.cc
  ${PROJECT_SOURCE_DIR}/src/routing_vertex.cc
  ${PROJECT_SOURCE_DIR}/src/routing_vertex_collector.cc
  ${PROJECT_SOURCE_DIR}/src/routing_via_info.cc
  ${PROJECT_SOURCE_DIR}/src/row_guide.cc
)

set(TILES_SRC
  ${PROJECT_SOURCE_DIR}/src/tiles/lut.cc
  ${PROJECT_SOURCE_DIR}/src/tiles/lut_b.cc
  ${PROJECT_SOURCE_DIR}/src/tiles/interconnect.cc
)

set(SKY130_ATOMS_SRC
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_buf.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_decap.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_dfxtp.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_hd_mux2_1.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_interconnect_mux6.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_mux.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_simple_transistor.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_switch_complex.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_tap.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_transmission_gate.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_transmission_gate_stack.cc
)

set(GF180_ATOMS_SRC
  ${PROJECT_SOURCE_DIR}/src/atoms/gf180mcu_mux.cc
)

set(ATOMS_SRC
  ${SKY130_ATOMS_SRC}
  ${GF180_ATOMS_SRC}
)

set(GEOMETRY_SRC 
  ${PROJECT_SOURCE_DIR}/src/geometry/compass.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/group.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/instance.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/label.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/line.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/point.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/point_or_choice.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/poly_line.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/polygon.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/port.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/radian.cc
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
# Internal protocol buffers are included directly as code in our main library.
# As a result, we must take care to ensure that the include paths used in the
# generated proto sources match the scheme we use for the rest of the bfg
# source. protoc, via our generate_protos macro, generates these based on the
# relative paths to protos from the proto root directory.
#
# So don't do this:
#file(RELATIVE_PATH proto_src_relative ${PROJECT_SOURCE_DIR} ${PROTO_SRC_DIR})
#set(PROTO_OUT_DIR ${PROJECT_BINARY_DIR}/${proto_src_relative})
set(PROTO_OUT_DIR ${PROJECT_BINARY_DIR})
set(GENERATED_PROTO_SRCS)
set(GENERATED_PROTO_HDRS)
generate_protos(${PROJECT_SOURCE_DIR} "${PROTO_SRC}" ${PROTO_OUT_DIR})
set(PROTO_GENERATED_HDRS ${GENERATED_PROTO_HDRS})
set(PROTO_GENERATED_SRCS ${GENERATED_PROTO_SRCS})

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

# bfg executable
# --------------

add_executable(bfg_bin
  src/main.cc
  src/dev_pdk_setup.cc)   # Temporary hack to set up PDK data.
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

# gRPC service generation via protobuf
# ------------------------------------

# Not entirely sure how this works but it should be in your PATH so maybe that's
# it:
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
message(STATUS "Found gRPC C++ plugin executable: "
  ${GRPC_CPP_PLUGIN_EXECUTABLE})

set(GENERATED_GRPC_HDRS)
set(GENERATED_GRPC_SRCS)
function(generate_grpc proto_src_root proto_files out_dir)
  foreach(file ${proto_files})
    message(STATUS "Generating gRPC C++ service from: ${file} -> ${out_dir}")
    file(RELATIVE_PATH relative_file ${proto_src_root} ${file})
    get_filename_component(proto_dir ${relative_file} DIRECTORY)
    get_filename_component(proto_name ${relative_file} NAME_WE)
    set(proto_hdr ${out_dir}/${proto_dir}/${proto_name}.pb.h)
    set(proto_src ${out_dir}/${proto_dir}/${proto_name}.pb.cc)
    set(grpc_hdr ${out_dir}/${proto_dir}/${proto_name}.grpc.pb.h)
    set(grpc_src ${out_dir}/${proto_dir}/${proto_name}.grpc.pb.cc)
    message(STATUS \tfile=${file})
    message(STATUS \trelative_file=${relative_file})
    message(STATUS \tproto_src_root=${proto_src_root})
    message(STATUS \tproto_dir=${proto_dir})
    message(STATUS \tproto_hdr=${proto_hdr})
    message(STATUS \tproto_src=${proto_src})
    message(STATUS \tgrpc_hdr=${grpc_hdr})
    message(STATUS \tgrpc_src=${grpc_src})
    # VERBATIM might break stuff here. Try quoting args and removing.
    add_custom_command(
      OUTPUT "${proto_hdr}" "${proto_src}" "${grpc_hdr}" "${grpc_src}"
      COMMAND protobuf::protoc
      ARGS --grpc_out=${out_dir}
           --cpp_out=${out_dir}
           --proto_path=${proto_src_root}
           --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
           "${file}"
      DEPENDS ${file} protobuf::protoc
      COMMENT "Generate gRPC C++ service/protocol buffers for ${file}"
      VERBATIM)
    list(APPEND GENERATED_GRPC_HDRS "${proto_hdr}")
    list(APPEND GENERATED_GRPC_SRCS "${proto_src}")
    list(APPEND GENERATED_GRPC_HDRS "${grpc_hdr}")
    list(APPEND GENERATED_GRPC_SRCS "${grpc_src}")
  endforeach()

  set(GENERATED_GRPC_HDRS ${GENERATED_GRPC_HDRS} PARENT_SCOPE)
  set(GENERATED_GRPC_SRCS ${GENERATED_GRPC_SRCS} PARENT_SCOPE)
endfunction()

# gRPC service(s)
# ---------------

# Could use a glob if protos were organised by name.
set(GRPC_SRC_DIR ${PROJECT_SOURCE_DIR}/services)
set(GRPC_SRC
  ${GRPC_SRC_DIR}/router_service.proto
)

set(GRPC_OUT_DIR ${PROJECT_BINARY_DIR}/services)
file(MAKE_DIRECTORY ${GRPC_OUT_DIR})

# Need quotes around "${GRPC_SRC}" to ensure it gets passed as the
# semicolon-delimited list that it is (otherwise only the first argument gets
# in).
generate_grpc(${GRPC_SRC_DIR} "${GRPC_SRC}" ${GRPC_OUT_DIR})
#set(GRPC_GENERATED_HDRS ${GENERATED_GRPC_HDRS})
#set(GRPC_GENERATED_SRCS ${GENERATED_GRPC_SRCS})

# router_service library
# ----------------------

set(ROUTER_SERVICE_LIB_SRC
  ${PROJECT_SOURCE_DIR}/src/router_session.cc
  ${PROJECT_SOURCE_DIR}/src/router_service_impl.cc
  ${GENERATED_GRPC_HDRS}
  ${GENERATED_GRPC_SRCS}
)
add_library(router_service STATIC ${ROUTER_SERVICE_LIB_SRC})
target_include_directories(router_service PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           ${GRPC_OUT_DIR})

target_link_libraries(router_service PUBLIC
                                      bfg
                                      vlsir_lib
                                      gflags
                                      ${tcmalloc_lib}
                                      glog::glog
                                      absl::base
                                      absl::strings
                                      absl::str_format
                                      absl::status
                                      absl::statusor
                                      gRPC::grpc++
                                      ${Protobuf_LIBRARIES})

# router_service executable
# -------------------------

add_executable(router_service_bin
  src/router_service_main.cc
  src/dev_pdk_setup.cc)   # Temporary hack to set up PDK data.
set_target_properties(router_service_bin PROPERTIES OUTPUT_NAME router_service)

target_include_directories(router_service_bin PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           ${VLSIR_OUT_DIR})

target_link_libraries(router_service_bin PUBLIC router_service
                                      ${tcmalloc_lib}
                                      gflags
                                      glog::glog
                                      absl::strings
                                      gRPC::grpc++
                                      gRPC::grpc++_reflection
                                      ${Protobuf_LIBRARIES})


# Tests
# -----
set(ATOMS_TEST_SRC
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_simple_transistor_test.cc
  ${PROJECT_SOURCE_DIR}/src/atoms/sky130_transmission_gate_test.cc
)

set(GEOMETRY_TEST_SRC
  ${PROJECT_SOURCE_DIR}/src/poly_line_inflator_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/instance_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/line_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/point_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/vector_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/poly_line_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/polygon_test.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/rectangle_test.cc
)

set(TEST_SRC
  ${ATOMS_TEST_SRC}
  ${GEOMETRY_TEST_SRC}
  ${PROJECT_SOURCE_DIR}/src/equivalent_nets_test.cc
  ${PROJECT_SOURCE_DIR}/src/routing_grid_geometry_test.cc
  ${PROJECT_SOURCE_DIR}/src/routing_edge_test.cc
  ${PROJECT_SOURCE_DIR}/src/routing_track_test.cc
  ${PROJECT_SOURCE_DIR}/src/routing_vertex_collector_test.cc
)

enable_testing()
add_executable(bfg_test
  ${TEST_SRC}
  src/dev_pdk_setup.cc)   # Temporary hack to set up PDK data.

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

# Test data
# ---------
# Since I like being able to run ./bfg_test directly, it's best to copy the test
# data into the binary output directory. The other option seems to be to specify
# WORKING_DIRECTORY to gtest_discovery_tests (or add_test), but I can't get that
# to work:
#   gtest_discover_tests(
#     bfg_test
#     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests")
#
set(PROJECT_TEST_DATA_DIR "${PROJECT_SOURCE_DIR}/test_data")
set(OUTPUT_TEST_DATA_DIR "${PROJECT_BINARY_DIR}/test_data")
message(STATUS "Configuring test data from ${PROJECT_TEST_DATA_DIR}")
file(REMOVE_RECURSE "${OUTPUT_TEST_DATA_DIR}/")
file(GLOB_RECURSE test_data_files "${PROJECT_TEST_DATA_DIR}/*")
foreach(data_file ${test_data_files})
  file(RELATIVE_PATH relative_file "${PROJECT_TEST_DATA_DIR}" "${data_file}")
  get_filename_component(sub_directory ${relative_file} DIRECTORY)
  get_filename_component(base_name ${relative_file} NAME)
  if(NOT "${sub_directory}" STREQUAL "")
    file(MAKE_DIRECTORY "${OUTPUT_TEST_DATA_DIR}/${sub_directory}")
    set(data_file_out "${OUTPUT_TEST_DATA_DIR}/${sub_directory}/${base_name}")
  else()
    set(data_file_out "${OUTPUT_TEST_DATA_DIR}/${base_name}")
  endif()
  configure_file("${data_file}" "${data_file_out}" COPYONLY)
  # Thanks I hate it.
endforeach(data_file)

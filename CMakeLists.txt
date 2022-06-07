cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Debug)

project(bfg VERSION 0.1)

find_package(gflags REQUIRED)
find_package(glog 0.5.0 REQUIRED)
find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)

set(PROTO_SRC
  ${PROJECT_SOURCE_DIR}/proto/inflator_rules.proto)

set(VLSIR_SRC
  ${PROJECT_SOURCE_DIR}/vlsir/raw.proto
  ${PROJECT_SOURCE_DIR}/vlsir/circuit.proto
  ${PROJECT_SOURCE_DIR}/vlsir/utils.proto)

set(SRC
  ${PROJECT_SOURCE_DIR}/src/instance.cc
  ${PROJECT_SOURCE_DIR}/src/cell.cc
  ${PROJECT_SOURCE_DIR}/src/physical_properties_database.cc
  ${PROJECT_SOURCE_DIR}/src/poly_line_inflator.cc
  ${PROJECT_SOURCE_DIR}/src/poly_line_cell.cc)

set(GEOMETRY_SRC 
  ${PROJECT_SOURCE_DIR}/src/geometry/line.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/point.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/poly_line.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/polygon.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/rectangle.cc
  ${PROJECT_SOURCE_DIR}/src/geometry/via.cc
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

add_executable(bfg
                src/main.cc
                ${SRC}
                ${GEOMETRY_SRC}
                ${GENERATED_PROTO_SRCS}
                ${PROTO_HDRS})

target_include_directories(bfg PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           ${Skia_BUILD_DIR}
                           ${Skia_BUILD_DIR}/include
                           )

find_library(tcmalloc_lib NAMES tcmalloc REQUIRED)
message(STATUS ${tcmalloc_lib})

target_link_libraries(bfg PUBLIC ${tcmalloc_lib}
                                      gflags
                                      glog::glog
                                      absl::strings
                                      ${Skia_LIBRARY}
                                      ${Protobuf_LIBRARIES})

configure_file(src/c_make_header.h.in src/c_make_header.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

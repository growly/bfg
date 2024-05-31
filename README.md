# BFG

An Open-Source Silicon Compiler for Reduced-Complexity Reconfigurable Fabrics

## Installation

### Prerequisites

Libraries

#### Debian
```
  sudo apt install -y build-essential cmake autoconf automake libtool curl make g++ unzip
  sudo apt install -y clang ninja-build python
  # for gRPC
  sudo apt install -y libre2-dev libc-ares-dev libssl-dev
```

#### Ubuntu
```
  sudo apt install -y build-essential cmake autoconf automake libtool curl \
    make g++ unzip clang ninja-build python libre2-dev libc-ares-dev libssl-dev
```

#### Red Hat
```
  sudo yum group install "Development Tools"
  sudo yum install cmake autoconf automake libtool curl make g++ unzip
  sudo yum install clang ninja-build python
  # for gRPC
  sudo yum install re2-devel c-ares-devel
```


[google/googletest](https://github.com/google/googletest)

  ```
  git clone git@github.com:google/googletest
  pushd googletest
  mkdir build && cd build
  cmake ../
  make -j $(nproc) && sudo make install
  popd
  ```

<!---
[google/tcmalloc](https://github.com/google/tcmalloc)
  ```
  git clone git@github.com:google/tcmalloc
  ?
  ```
  --->

[gperftools/gperftools](https://github.com/gperftools/gperftools)
  ```
  git clone git@github.com:gperftools/gperftools
  pushd gperftools
  ./autogen.sh
  ./configure
  make -j $(nproc) && sudo make install
  popd
  ```

[gflags/gflags](https://github.com/gflags/gflags/blob/master/INSTALL.md)
  ```
  git clone git@github.com:gflags/gflags.git
  pushd gflags
  mkdir build && cd build
  cmake .. -DBUILD_SHARED_LIBS=ON
  make -j $(nproc) && sudo make install
  popd
  ```

[glog/glog](https://github.com/google/glog)

  ```
  git clone https://github.com/google/glog.git
  pushd glog
  cmake -S . -B build -G "Unix Makefiles"
  cmake --build build
  sudo cmake --build build --target install
  popd
  ```

[abseil/abseil-cpp](https://abseil.io/docs/cpp/quickstart-cmake)

  ```
  git clone git@github.com:abseil/abseil-cpp.git
  pushd abseil-cpp
  mkdir build && cd build
  cmake -DABSL_RUN_TESTS=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=17 -DABSL_PROPAGATE_CXX_STD=ON ../
  make -j $(nproc)
  sudo make install
  popd
  ```

[protocolbuffers/protobuf](https://github.com/protocolbuffers/protobuf/tree/master/src)

  ```
  wget https://github.com/protocolbuffers/protobuf/releases/download/v21.5/protobuf-all-21.5.tar.gz
  tar xf protobuf-all-21.5.tar.gz
  pushd protobuf-21.5
  ./autogen.sh
  ./configure
  make -j $(nproc)
  sudo make install
  sudo ldconfig # refresh shared library cache.
  popd
  ```

Note: when I compile and build protocol buffers from [HEAD on
GitHub](https://github.com/protocolbuffers/protobuf), I get compilation errors
because the file `port_def.inc` doesn't get installed. Compiling and installing
from a release tarball, it seems fine.

[grpc/grpc](https://github.com/grpc/grpc)

We have to use an old version of gRPC because we use an old version of
protobuf.  (Feel free to update both!) There are actually quite a few
interesting gRPC build options which we do not explore because I need to
graduate, *including* how modern `cmake`s handle dependencies *and* the use of
a `.local` installation for the project instead of the entire system.

  ```
  wget https://github.com/grpc/grpc/archive/refs/tags/v1.48.1.tar.gz -O grpc-1.48.1.tar.gz
  tar xf grpc-1.48.1.tar.gz
  cd grpc-1.48.1
  mkdir -p cmake/build
  pushd cmake/build
  cmake \
    -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DgRPC_CARES_PROVIDER=package \
    -DgRPC_ABSL_PROVIDER=package \
    -DgRPC_PROTOBUF_PROVIDER=package \
    -DgRPC_RE2_PROVIDER=package \
    -DgRPC_SSL_PROVIDER=package \
    -DgRPC_ZLIB_PROVIDER=package \
    ../..
  make -j $(nproc)
  sudo make install
  popd
  ```

<!---
[skia](https://skia.org/user/build#quick)

(You need the [ninja build system](https://ninja-build.org/).) We include skia as a submodule because it was too hard to get to build as a library.

  ```
  # git clone https://skia.googlesource.com/skia.git
  git submodule update --init
  cd skia
  python2 tools/git-sync-deps
  bin/gn gen out/Shared --args='is_official_build=true is_component_build=true'
  bin/gn gen out/Static --args='is_official_build=true'
  tools/install_dependencies.sh
  ninja -C out/Shared
  ninja -C out/Static
  ```
--->

## Building

  ```
  git clone git@github.com:growly/bfg
  cd bfg
  git submodule update --init --recursive
  mkdir build && cd build
  cmake ../
  make
  ```

## Making tech protos

```
protoc --proto_path=vlsir/ --encode vlsir.tech.Technology vlsir/tech.proto < gf180mcu.technology.pb.txt > gf180mcu.technology.pb
```

## Testing the router service

Use [`grpcurl`](https://github.com/fullstorydev/grpcurl). `jq` makes the output
pretty (and is optional):

```
go install github.com/fullstorydev/grpcurl/cmd/grpcurl@latest
~/go/bin/grpcurl -plaintext localhost:8222 list
~/go/bin/grpcurl -plaintext localhost:8222 bfg.router_service.RouterService list
~/go/bin/grpcurl \
    -plaintext \
    -d '{ "predefined_technology": "TECHNOLOGY_SKY130", "grid_definition": { "layers": [{}, {}] } }' \
    localhost:8222 \
    bfg.router_service.RouterService/CreateRoutingGrid | jq
```

A full example of this is in the
[`router_service_poke.sh`](https://github.com/growly/bfg/blob/f52d23a74e4ed57b72fa4431ee85db1e2a1b450f/router_service_poke.sh)
script.

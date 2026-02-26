# BFG

An Open-Source Silicon Compiler for Reduced-Complexity Reconfigurable Fabrics

BFG makes FPGAs.

[Half a ReducedSlice](assets/img/reduced_slice_banner.png)

## Installation

### Via Docker

The easiest and safest way to build and run ```bfg``` is via Docker:

```
docker build -t bfg/router:latest -f docker/Dockerfile .

docker compose -f docker/docker-compose.yml up -d
```

> [!CAUTION]
> This docker command assumes you are running it from the root of the repo. The
> final argument is a path ("`.`") so that this directory gets copied into the
> container. It's used in an `ADD . .` command in the Dockerfile. Have a look.
> Anyway, don't run the command from another directory unless you know what
> you're doing.

### Prerequisites

Libraries

#### Debian
```
  sudo apt install -y build-essential cmake autoconf automake libtool curl make g++ unzip
  sudo apt install -y clang ninja-build python3 pkg-config zlib1g-dev
  # for kdtree++
  sudo apt install -y libkdtree++-dev
  # for gRPC
  sudo apt install -y libre2-dev libc-ares-dev libssl-dev
```

##### Profiling and performance optimisation
If you have an old Debian (12.11 is old as of this writing), you have to
manually [install an up-to-date version of `go`](https://go.dev/doc/install)
(the Go programming language) and then use that to install `pprof`, [the
profiler](https://github.com/google/pprof). You might also appreciate graphviz
and its tools, like `gv`.

```
  go install github.com/google/pprof@latest
  export PATH="${HOME}/go/bin:${PATH}"
  # pprof should now be available to you.

  sudo apt install -y graphviz gv
  # pprof -gv should now work.
```

#### Ubuntu
```
  sudo apt install -y build-essential cmake autoconf automake libtool curl \
    make g++ unzip clang ninja-build python libre2-dev libc-ares-dev libssl-dev
```

#### Red Hat
Red Hat 7 doesn't have a new enough `cmake`. Red Hat 9 doesn't have `ninja` in its `yum` repositories. Nor does it have re2, so you have to build it from source (below).

```
  sudo yum group install "Development Tools"
  sudo yum install cmake autoconf automake libtool curl make g++ unzip
  # there is no ninja package for yum :(
  sudo yum install clang python
  # for gRPC
  sudo yum install c-ares-devel
```

Also, *very importantly*, RHEL9 doesn't seem to include `/usr/local/lib` as a default linker path, so you have to add it (or set `LD_LIBRARY_PATH`). This breaks proto creation in particular.
```
cat <<EOF > /tmp/libc.conf
/usr/local/lib
EOF
sudo mv /tmp/libc.conf /etc/ld.so.conf.d/
sudo ldconfig
```

[google/googletest](https://github.com/google/googletest)

  ```
  git clone -b v1.16.0 git@github.com:google/googletest
  pushd googletest
  mkdir build && pushd build
  cmake ../
  make -j $(nproc) && sudo make install
  popd
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
  # On case-insensitive (insane) file systems (macOS) "build" conflicts
  # with the "BUILD" file that comes with the package.
  pushd gflags
  mkdir _build && pushd _build
  cmake .. -DBUILD_SHARED_LIBS=ON
  make -j $(nproc) && sudo make install
  popd; popd
  ```

[google/glog](https://github.com/google/glog)

  ```
  git clone https://github.com/google/glog.git
  pushd glog
  cmake -S . -B build -G "Unix Makefiles"
  cmake --build build
  sudo cmake --build build --target install
  popd
  ```

> [!CAUTION]
> When I compile and build newer protobuf, Abseil or gRPC libraries I end
> up in a hellish pit of compilation and/or linker and/or ABI errors that I have
> not had time to fix. These libraries should all be modernised at the same
> time(along with any fixes to this code). Contributions welcome!

[abseil/abseil-cpp](https://abseil.io/docs/cpp/quickstart-cmake)


  ```
  git clone git@github.com:abseil/abseil-cpp.git
  pushd abseil-cpp
  git checkout 1a31b81c0a467c1c8e229b9fc172a4eb0db5bd85
  mkdir build && pushd build
  cmake -DABSL_RUN_TESTS=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=17 -DABSL_PROPAGATE_CXX_STD=ON ../
  make -j $(nproc)
  sudo make install
  popd; popd
  ```

[google/re2](https://github.com/google/re2)

> [!WARNING]
> You must build RE2 against the Abseil library you just installed in the
> previous step. Otherwise the wrong version of Abseil might be installed by
> your package manager as a dependency of libre2, and you will have a nightmare
> trying to link `bfg` later.


```
git clone https://github.com/google/re2.git
pushd re2
mkdir build && pushd build
cmake ../
make -j $(nproc)
sudo make install
popd; popd
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

[grpc/grpc](https://github.com/grpc/grpc)

We have to use an old version of gRPC because we use an old version of
protobuf.  (Feel free to update both!) There are actually quite a few
interesting gRPC build options which we do not explore because I need to
graduate, *including* how modern `cmake`s handle dependencies *and* the use of
a `.local` installation for the project instead of the entire system.

  ```
  wget https://github.com/grpc/grpc/archive/refs/tags/v1.48.1.tar.gz -O grpc-1.48.1.tar.gz
  tar xf grpc-1.48.1.tar.gz
  pushd grpc-1.48.1
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
  popd; popd
  ```

[nvmd/libkdtree](https://github.com/nvmd/libkdtree)

This one is easier to install from your system's package manager
(`libkdtree++-dev` on Debian). If you can't find yours, try installing from
scratch:
  ```
  git clone https://github.com/nvmd/libkdtree.git
  pushd libkdtree
  ./configure
  sudo make install
  ```

(This is a header-only library.)

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

#### macOS

1. Install xcode.
2. Install xcode command line tools.
   ```
   sudo xcode-select --install
   ```
3. Install additional packages, e.g. using macports:
   ```
   sudo port selfupdate
   sudo port install cmake autoconf automake libtool curl gmake unzip m4 wget
   ```
4. Follow the instructions to install additional dependencies exactly as for
   Linux, above. (`ldconfig` will fail since it doesn't apply on macOS
   platforms, but that won't matter.)

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
protoc --proto_path=vlsir/protos/ --encode vlsir.tech.Technology vlsir/protos/tech.proto < sky130.technology.pb.txt > sky130.technology.pb
protoc --proto_path=vlsir/protos/ --encode vlsir.tech.Technology vlsir/protos/tech.proto < gf180mcu.technology.pb.txt > gf180mcu.technology.pb
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

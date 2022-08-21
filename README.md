# BFG

An Open-Source Silicon Compiler for Reduced-Complexity Reconfigurable Fabrics

## Installation

### Prerequisites

Libraries

```
  sudo apt install -y build-essential cmake autoconf automake libtool curl make g++ unzip
  sudo apt install -y clang ninja-build python
```

[google/googletest](https://github.com/google/googletest)

  ```
  git clone git@github.com:google/googletest
  cd googletest
  mkdir build && cd build
  cmake ../
  make -j $(nproc) && sudo make install
  ```

[gperftools/tcmalloc](https://github.com/gperftools/gperftools)
  ```
  git clone git@github.com:gperftools/gperftools
  cd gperftools
  ./autogen.sh
  ./configure
  make -j $(nproc) && sudo make install
  ```

[gflags/gflags](https://github.com/gflags/gflags/blob/master/INSTALL.md)
  ```
  git clone git@github.com:gflags/gflags.git
  cd gflags
  mkdir build && cd build
  cmake .. -DBUILD_SHARED_LIBS=ON
  make -j $(nproc) && sudo make install
  ```

[glog/glog](https://github.com/google/glog)

  ```
  git clone https://github.com/google/glog.git
  cd glog
  cmake -S . -B build -G "Unix Makefiles"
  cmake --build build
  sudo cmake --build build --target install
  ```

[abseil/abseil-cpp](https://abseil.io/docs/cpp/quickstart-cmake)

  ```
  git clone git@github.com:abseil/abseil-cpp.git
  cd abseil-cpp
  mkdir build && cd build
  cmake .. -DABSL_RUN_TESTS=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=17
  cmake --build . --target all
  sudo make install
  ```

[protocolbuffers/protobuf](https://github.com/protocolbuffers/protobuf/tree/master/src)

  ```
  wget https://github.com/protocolbuffers/protobuf/releases/download/v21.5/protobuf-all-21.5.tar.gz
  tar xf protobuf-all-21.5.tar.gz
  cd protobuf-all-21.5
  ./autogen.sh
  ./configure
  make -j $(nproc)
  sudo make install
  sudo ldconfig # refresh shared library cache.
  ```

Note: when I compile and build protocol buffers from [HEAD on
GitHub](https://github.com/protocolbuffers/protobuf), I get compilation errors
because the file `port_def.inc` doesn't get installed. Compiling and installing
from a release tarball, it seems fine.

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

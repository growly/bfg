# syntax=docker/dockerfile:1.7-labs
# Use ubuntu as base
# Base image for router code.
# This downloads and builds all the supporting modules.
# A separate base image is used to avoid redownloading and rebuilding
FROM ubuntu:latest AS base

# Add timezone data and certs
RUN apt update -y
RUN apt upgrade -y
RUN apt install -y libc-ares-dev
FROM base as builder
RUN apt install -y build-essential cmake autoconf automake libtool curl make g++ unzip clang
RUN apt install -y ninja-build python3 libre2-dev libssl-dev git wget zlib1g-dev

WORKDIR /src

RUN git clone -b v1.16.0 --depth 1 https://github.com/google/googletest.git
WORKDIR googletest/build
RUN cmake ../
RUN make -j $(nproc) && make install

WORKDIR /src
RUN git clone -b gperftools-2.16.90 --depth 1 https://github.com/gperftools/gperftools.git
WORKDIR gperftools
RUN ./autogen.sh
RUN ./configure
RUN make -j $(nproc) && make install

WORKDIR /src
RUN git clone -b v2.2.2 --depth 1 https://github.com/gflags/gflags.git
WORKDIR gflags/build
RUN cmake .. -DBUILD_SHARED_LIBS=ON
RUN make -j $(nproc) && make install

WORKDIR /src
RUN git clone -b v0.7.1 --depth 1 https://github.com/google/glog.git
WORKDIR glog
RUN cmake -S . -B build -G "Unix Makefiles"
RUN cmake --build build
RUN cmake --build build --target install

WORKDIR /src
RUN git clone https://github.com/abseil/abseil-cpp.git
WORKDIR abseil-cpp
RUN git checkout 1a31b81c0a467c1c8e229b9fc172a4eb0db5bd85
WORKDIR build
RUN cmake -DABSL_RUN_TESTS=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=17 -DABSL_PROPAGATE_CXX_STD=ON ../
RUN make -j $(nproc)
RUN make install

WORKDIR /src
RUN git clone -b 2024-07-02 --depth 1 https://github.com/google/re2.git
WORKDIR re2/build
RUN cmake ../
RUN make -j $(nproc)
RUN make install

WORKDIR /src
RUN wget -O - https://github.com/protocolbuffers/protobuf/releases/download/v21.5/protobuf-all-21.5.tar.gz | tar xfz -
WORKDIR protobuf-21.5
RUN ./autogen.sh
RUN ./configure
RUN make -j $(nproc)
RUN make install
# Refresh shared library cache
RUN ldconfig

WORKDIR /src
RUN wget -O - https://github.com/grpc/grpc/archive/refs/tags/v1.48.1.tar.gz | tar xfz -
WORKDIR grpc-1.48.1/cmake/build
RUN cmake \
  -DgRPC_INSTALL=ON \
  -DgRPC_BUILD_TESTS=OFF \
  -DgRPC_CARES_PROVIDER=package \
  -DgRPC_ABSL_PROVIDER=package \
  -DgRPC_PROTOBUF_PROVIDER=package \
  -DgRPC_RE2_PROVIDER=package \
  -DgRPC_SSL_PROVIDER=package \
  -DgRPC_ZLIB_PROVIDER=package \
  ../..
RUN make -j $(nproc)
RUN make install

WORKDIR /src/bfg
COPY . .
RUN git submodule update --init --recursive
WORKDIR build
RUN cmake ../
RUN make -j $(nproc)

RUN cp libvlsir_lib.so.0.1 /usr/local/lib
WORKDIR /usr/local/lib
RUN ln -s libvlsir_lib.so.0.1 libvlsir_lib.so

# Now install libraries and binaries into fresh image
FROM base
COPY --from=builder --exclude=*.a /usr/local/lib /usr/local/lib
WORKDIR /bfg
COPY --from=builder /src/bfg/gf180mcu.technology.pb .
COPY --from=builder /src/bfg/sky130.technology.pb .
WORKDIR bin
COPY --from=builder /src/bfg/build/router_service .
COPY --from=builder /src/bfg/build/bfg .
COPY --from=builder /src/bfg/build/bfg_test .
CMD ["/bfg/bin/router_service"]

#!/bin/bash

BUILD_DIR=watch_build
PID_FILE=klayout.pid
#rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}" && \
  cmake -S . -B "${BUILD_DIR}" && \
  cmake --build "${BUILD_DIR}" -j $(nproc) && \
  "${BUILD_DIR}"/bfg \
    --technology sky130.technology.pb \
    --external_circuits sky130hd.pb \
    --logtostderr --v 2 && \
  /home/aryap/src/Layout21/target/debug/proto2gds \
    -i library.pb \
    -t sky130.technology.pb \
    -o library.gds

#if [ $? -eq 0 ]; then
#  if [ -f "${PID_FILE}" ]; then
#    pkill -F "${PID_FILE}"
#  fi
#  klayout library.gds &
#  echo $! > "${PID_FILE}"
#fi

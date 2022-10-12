#!/bin/bash
# Example usage (from bfg root dir):
# sudo inotifywait -r -e modify -m src | while read dir action file; do ./watch_build.sh; done

BUILD_DIR=watch_build
PID_FILE=klayout.pid
PROTO2GDS_BIN="${HOME}/src/Layout21/target/debug/proto2gds"
TOP=

mkdir -p "${BUILD_DIR}" || exit 1

cat << EOF > ${BUILD_DIR}/view_script.py
import gdspy
lib = gdspy.GdsLibrary(infile='library.gds')
tops = lib.top_level()
for cell in tops:
  filename = f'library.{cell.name}.svg'
  cell.write_svg(filename)
  print(f'wrote {filename}')

EOF

cmake -S . -B "${BUILD_DIR}" || exit 2

cmake --build "${BUILD_DIR}" -j $(nproc) || exit 3

"${BUILD_DIR}"/bfg \
  --technology sky130.technology.pb \
  --external_circuits sky130hd.pb \
  --logtostderr --v 2 || exit 4

"${PROTO2GDS_BIN}" \
  -i library.pb \
  -t sky130.technology.pb \
  -o library.gds || exit 5

python3 "${BUILD_DIR}/view_script.py"

#if [ $? -eq 0 ]; then
#  if [ -f "${PID_FILE}" ]; then
#    pkill -F "${PID_FILE}"
#  fi
#  klayout library.gds &
#  echo $! > "${PID_FILE}"
#fi

#!/bin/bash
# Builds bfg_test with coverage instrumentation, runs it and prints line and
# function coverage for each file in src/, least covered first.
#
# Usage: scripts/coverage.sh [build_dir] [gtest args...]
#
# build_dir defaults to build_coverage, relative to the repository root.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="$(realpath -m "${1:-${root}/build_coverage}")"
shift || true

cmake -S "${root}" -B "${build_dir}" -DBFG_COVERAGE=ON > /dev/null
cmake --build "${build_dir}" --target bfg_test -j "$(nproc)"

# Stale counters from an earlier build make gcov complain.
find "${build_dir}" -name '*.gcda' -delete
(cd "${build_dir}" && ./bfg_test "$@")

gcov_dir="${build_dir}/gcov"
rm -rf "${gcov_dir}"
mkdir -p "${gcov_dir}"
# --preserve-paths keeps outputs for files with the same basename (e.g.
# geometry/instance.cc and circuit/instance.cc) from overwriting each other.
(cd "${gcov_dir}" && \
    find "${build_dir}/CMakeFiles/bfg.dir" "${build_dir}/CMakeFiles/bfg_test.dir" \
        -name '*.gcno' -print0 | \
    xargs -0 -P "$(nproc)" -n 8 \
        gcov --json-format --demangled-names --preserve-paths > /dev/null 2>&1)

python3 "${root}/scripts/coverage_summary.py" "${root}/src" "${gcov_dir}"

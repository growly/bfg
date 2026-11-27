# Scripts

## proto2cpp --layout

TODO: depend on vlsir pypi bindings

TODO: write this in rust to see what it's like

```
python3 proto2cpp.py --layout ../path/to/sky130_fd_sc_hd__dfxtp_1.pb --tech ../sky130.technology.pb
```

## coverage.sh

Builds `bfg_test` with gcov instrumentation (`-DBFG_COVERAGE=ON`), runs it,
and prints line and function coverage for each file in `src/`, least covered
first. Extra arguments are passed to `bfg_test`.

```
scripts/coverage.sh [build_dir] [gtest args...]
```

`build_dir` defaults to `build_coverage`. To list the unexecuted functions in
particular files, run the summary directly:

```
python3 scripts/coverage_summary.py src build_coverage/gcov --functions routing/routing_grid.cc
```

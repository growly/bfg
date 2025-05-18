#!/bin/bash

pushd ~/src/bigspicy

python3 bigspicy.py \
  --import \
  --spice_header /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_fd_sc_hd.spice \
  --spice_header /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__decap_12.spice \
  --spice_header /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__fakediode_2.spice \
  --spice_header /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__fill_12.spice \
  --spice_header /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__fill_4.spice \
  --spice_header /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__fill_8.spice \
  --verilog /home/arya/src/OpenLane/designs/s44/runs/RUN_2025.02.24_22.41.24/results/final/verilog/gl/lut.v \
  --spef /home/arya/src/OpenLane/designs/s44/runs/RUN_2025.02.24_22.41.24/results/final/spef/lut.spef \
  --show \
  --top lut \
  --dump_spice ~/src/bfg_lut_analysis_feb2025/lut.sp

popd

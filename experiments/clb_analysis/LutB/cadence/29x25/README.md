Made Spice file with BigSpicy:

(Need to run in conda environment with required packages and also from the
source folder since it relies on a kludge to get the python vlsir bindings on
the PYTHONPATH.)

```
./bigspicy.py --import --verilog ~/src/bfg/experiments/lut_analysis_2026/LutB/cadence/29x25/clb.lvs.v --spice_header /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_fd_sc_hd.spice --top clb --dump_spice clb.sp
```

BigSpicy is just `v2lvs` for FOSS.
```
 v2lvs -lsp /home/ff/eecs251b/sky130/sky130_cds/sky130_scl_9T_0.0.5/cdl/sky130_scl_9T.cdl -v clb.lvs.v -o clb.lvs.sp
```

I had a hell of a time getting this extracted with Calibre even though I already
had a working extraction flow for Sky130 and the _hd library.

LVS passed with the hierarchical netlist fine, pointed at the sky130_sc_fd_hd
master spice file as normal.

But for PEX to work I had to:
- flatten the netlist into a transistor-only circuit
    - there will still a couple of structures where i'd run into the
      'unexpected pin count' error (both cases where an inv2 had been
      flattened?)
    - so I replaced the sky130 transistor subckts with the primitive models and
      that went away. so:
- replace the 'pfet_01v8' etc models with the 'pshort', 'phighvt', 'nshort'
  primitives in the calibre rule deck, change the instance names to start with
  'm' instead 'x', etc.

Then to simulate with Spectre, I had to:
- rescale all transistor parameters to physical values (microns, nanometres):
  divide by 1E6
- replace the PEX models with the Calibre ones (nfet_01v8, pfet_01v8,
  pfet_01v8_hvt)

```
./bigspicy.py --import --spice_header ~/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_pr/spice/sky130_fd_pr__pfet_01v8_hvt.pm3.spice --spice_header ~/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_pr/spice/sky130_fd_pr__nfet_01v8.pm3.spice --spice_header ~/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_pr/spice/sky130_fd_pr__pfet_01v8.pm3.spice --spice /tmp/clb.spice --spice ~/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_fd_sc_hd.spice  --flatten --top clb --dump_spice clb.dump.sp
```

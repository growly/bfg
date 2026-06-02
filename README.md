# BFG

BFG is an open-source full-custom silicon compiler for high-performance FPGA fabrics. BFG makes FPGA IP. It works by hierarchically composing parameterised layout and circuit generators.

[![DOI](https://zenodo.org/badge/499658756.svg)](https://doi.org/10.5281/zenodo.20349466)

Here is a Configurable Logic Block based around a 4-LUT for Skywater 130nm, produced by the [LutB](src/tiles/lut_b.h) generator:

<img src="assets/img/LutB.20260602.png" alt="CLB" width="70%">

It has a bypass input and can register either the LUT output or the by pass. A combinational output pin also lets you select between the LUT output and the bypass, as in:

<img src="assets/img/bfg_clb.drawio.png" width="50%">'

The CLB itself is made up of generator for [flip-flops](src/atoms/sky130_dfxtp.h), a [hierarchical transmission-gate mux](src/atoms/sky130_mux.h), [two](src/atoms/sky130_buf.h) [buffer](src/atoms/sky130_split_buffer.h) topologies and active 2:1 muxes. Some of these are taken from the open-source [sky130_fd_sc_hd](https://sky130-unofficial.readthedocs.io/en/latest/contents/libraries/sky130_fd_sc_hd/README.html) library and then parameterised.

BFG can then [make](src/tiles/s44.h) an S-44 LUT based around this CLB and a [carry chain](src/atoms/sky130_carry1.h). Together with N:1 and N:2 (shared) multiplexer generators for interconnect wiring, and wire buses with configurable break-outs, this is enough to assemble a whole FPGA tile:

![ReducedSlice](assets/img/reduced_slice_banner.png)

# Status


# Usage

BFG relies on [VLSIR](https://github.com/Vlsir/Vlsir) for producing common formats like LEF/DEF, GDS and (the various) Spices. 

Once BFG and the prerequisites are [installed](INSTALL.md)) with:

```
./bfg --jobs 0  --technology ../sky130.technology.pb --primitives ../sky130.primitives.pb --external_circuits ../sky130hd.pb  --logtostderr --write_text_format --run_generator LutB --params LutB.params.pb.txt --output_library LutB
```



## Installation

See [INSTALL.md](INSTALL.md).

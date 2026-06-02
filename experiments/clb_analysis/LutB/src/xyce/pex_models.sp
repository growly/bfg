
* Calibre PEX as set up with the commercial Sky130 library available at BWRC
* will use shorthands for the transistors that we might want to map back to the
* original models in the sky130 library.

*.subckt nshort d g s b
*x0 d g s b sky130_fd_pr__nfet_01v8
*.ends
*
*.subckt pshort d g s b
*x0 d g s b sky130_fd_pr__pfet_01v8
*.ends
*
*.subckt phighvt d g s b
*x0 d g s b sky130_fd_pr__pfet_01v8_hvt
*.ends

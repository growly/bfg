; test harness for luts

.lib /home/arya/src/pdk-root/share/pdk/sky130A/libs.tech/ngspice/sky130.lib.spice tt

.include /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__decap_12.spice
.include /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__fakediode_2.spice
.include /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__fill_12.spice
.include /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__fill_4.spice
.include /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_ef_sc_hd__fill_8.spice
.include /home/arya/src/pdk-root/share/pdk/sky130A/libs.ref/sky130_fd_sc_hd/spice/sky130_fd_sc_hd.spice


; sky130 models seem to require mosfet params scaled by 1e6 ffs
; you can use (in vim) %s/\(\d\+\)\@<=n/m/g
;.include fake_lut.sp

.include final/spice/clb.spice

;.include lut.sp
;.include lut.spef.sp

.param num_bits=18
.param scan_clock_period=2n
.param test_start={19 * scan_clock_period + 50p}
.param read_clock=1.5n
.param half_vdd={1.8 / 2}

.subckt testbench
+ VSS

; for std-cell lut without spef use:
;+ VSS VPWR config_clk config_en config_in config_out out a0 a1 a2 a3
; for std-cell LUT with spef use:
;+ VSS VSS VPWR config_clk config_en config_in config_out out a0 a1 a2 a3
; for bfg lut use:
; a0 a1 a2 a3 out config_in config_out config_clk VPWR VPWR VSS VSS
; for fake_lut use:
; a0 a1 a2 a3 out config_in config_out config_clk VPWR VSS

xlut
+ VSS VPWR a0 a1 a2 a3 clk mux_out reg_out scan_clk scan_en scan_in scan_out bypass
+ clb
+ ; No parameters

cscan_load scan_out VSS 2f

vpower
+ VPWR VSS 
+ dc {1.8} 
+ ac {0} 
+ ; No parameters

*vclock
*+ config_clk VSS
*+ pulse ({0} {1.8} {scan_clock_period / 4} {50p} {50p} {scan_clock_period / 2} {scan_clock_period})

vconfig_clock
+ scan_clk VSS
+ pat ({1.8} {0} {0} {50p} {50p} {scan_clock_period / 2} b0101010101010101010101010101010101010 0)

*vconfig_data
*+ config_in VSS
*+ pulse ({0} {1.8} {0} {50p} {50p} {scan_clock_period} {2 * scan_clock_period})

* NOTE: The first bit has to be 0 so that the last register in the scan chain
* gets loaded with 0: this sets the combinational output selection mux to the
* internal mux output.
vscan_data
+ scan_in VSS
+ pat ({1.8} {0} {0} {50p} {50p} {scan_clock_period} b010101010101010100)

vbypass
+ bypass VSS 0

* ???
*vconfig_en
*+ config_en VSS
*+ pulse ({0} {1.8} {0} {50p} {50p} {15.5 * scan_clock_period} {10 * 16 * scan_clock_period})

* 
*va0_driver
*+ a0 VSS 
*+ pulse ({0} {1.8} {test_start} {50p} {50p} {read_clock / 2} {read_clock}) 
*+ ; No parameters
*
*va1_driver
*+ a1 VSS 
*+ pulse ({0} {1.8} {test_start} {50p} {50p} {2 * (read_clock / 2)} {2 * (read_clock)}) 
*+ ; No parameters
*
*va2_driver
*+ a2 VSS 
*+ pulse ({0} {1.8} {test_start} {50p} {50p} {4 * (read_clock / 2)} {4 * (read_clock)}) 
*+ ; No parameters
*
*va3_driver
*+ a3 VSS 
*+ pulse ({0} {1.8} {test_start} {50p} {50p} {8 * (read_clock / 2)} {8 * (read_clock)}) 
*+ ; No parameters
*_66_ _76_/CLK _66_/D VGND VGND VPWR VPWR _66_/Q sky130_fd_sc_hd__dfxtp_2
*_65_ _71_/CLK _65_/D VGND VGND VPWR VPWR _65_/Q sky130_fd_sc_hd__dfxtp_2
*_64_ _71_/CLK _64_/D VGND VGND VPWR VPWR _64_/Q sky130_fd_sc_hd__dfxtp_2
*_63_ _71_/CLK _63_/D VGND VGND VPWR VPWR _63_/Q sky130_fd_sc_hd__dfxtp_2
*_62_ _71_/CLK _62_/D VGND VGND VPWR VPWR _62_/Q sky130_fd_sc_hd__dfxtp_2
*_61_ _76_/CLK _61_/D VGND VGND VPWR VPWR _61_/Q sky130_fd_sc_hd__dfxtp_2
*_60_ _76_/CLK _60_/D VGND VGND VPWR VPWR _60_/Q sky130_fd_sc_hd__dfxtp_2
*_77_ input5/X _77_/D VGND VGND VPWR VPWR _77_/Q sky130_fd_sc_hd__dfxtp_2
*_76_ _76_/CLK _76_/D VGND VGND VPWR VPWR _76_/Q sky130_fd_sc_hd__dfxtp_2
*_59_ _76_/CLK _59_/D VGND VGND VPWR VPWR _59_/Q sky130_fd_sc_hd__dfxtp_2
*_75_ _76_/CLK _75_/D VGND VGND VPWR VPWR _75_/Q sky130_fd_sc_hd__dfxtp_2
*_74_ _76_/CLK _74_/D VGND VGND VPWR VPWR _74_/Q sky130_fd_sc_hd__dfxtp_2
*_73_ _76_/CLK _73_/D VGND VGND VPWR VPWR _73_/Q sky130_fd_sc_hd__dfxtp_2
*_72_ _76_/CLK _72_/D VGND VGND VPWR VPWR _72_/Q sky130_fd_sc_hd__dfxtp_2
*_71_ _71_/CLK _71_/D VGND VGND VPWR VPWR _71_/Q sky130_fd_sc_hd__dfxtp_2
*_70_ _71_/CLK _70_/D VGND VGND VPWR VPWR _70_/Q sky130_fd_sc_hd__dfxtp_2
*_69_ _71_/CLK _69_/D VGND VGND VPWR VPWR _69_/Q sky130_fd_sc_hd__dfxtp_2
*_68_ _71_/CLK _68_/D VGND VGND VPWR VPWR _68_/Q sky130_fd_sc_hd__dfxtp_2
*_67_ _71_/CLK _67_/D VGND VGND VPWR VPWR _67_/Q sky130_fd_sc_hd__dfxtp_2

*; mux order                  scan order
*;         0  _59_/Q          1
*;         1  _59_/Q          0
*;         2  _59_/Q          2
*;         3  _59_/Q          3
*;         4  _59_/Q         13
*;         5  _59_/Q         12
*;         6  _59_/Q         14
*;         7  _59_/Q         15
*;         8  _59_/Q         10
*;         9  _59_/Q         11
*;        10  _59_/Q          9
*;        11  _59_/Q          8
*;        12  _59_/Q          6
*;        13  _59_/Q          7
*;        14  _59_/Q          5
*;        15  _59_/Q          4
*; BUT THEY ARE numbered for the their place in the scan chain.
*;
*; SO TO READ In scan order, you have to read indices:
*; 1, 0, 2, 3, 15, 14, 12, 13, 11, 10, 8, 9, 5, 4, 6, 7
*VA3_DRIVER A3 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b000001111111100000 0)
*VA2_DRIVER A2 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b000001111000011110 0)
*VA1_DRIVER A1 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b000111100110000110 0)
*VA0_DRIVER A0 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b010011001100110010 0)

; READ ADDRESS into lut vertically:
VA3_DRIVER A3 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b00000000111111110 0)
VA2_DRIVER A2 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b00001111000011110 0)
VA1_DRIVER A1 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b00110011001100110 0)
VA0_DRIVER A0 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b01010101010101010 0)

.ENDS

.TRAN 1E-12 80E-9

XTOP 0 TESTBENCh

*.PRINT TRAN FOrmat=csv v(*) i(*)

* These are based on the input pattern (scan_data) and the order of connection
* from the scan chain to the muxes (mux order).
*
* TODO(aryap): the std-cell comparison model needs the bypass mux at the output
* (and before the output register).

* Have to count rises across MUX_OUT to figure this one:
*.param first=4

*.measure tran d0 trig v(xtop:a0)={half_vdd} fall=1 targ v(xtop:xlut:z)={half_vdd} fall={first}
*.measure tran d1 trig v(xtop:a0)={half_vdd} fall=2 targ v(xtop:xlut:z)={half_vdd} rise={first+1}
*.measure tran d2 trig v(xtop:a0)={half_vdd} rise=4 targ v(xtop:xlut:z)={half_vdd} fall={first+1}

*.measure tran  d0 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:mux_out)=0.9 rise={first+1}
*.measure tran  d1 trig v(xtop:a0)=0.9 fall=1 targ v(xtop:mux_out)=0.9 fall={first+1}
*.measure tran  d2 trig v(xtop:a0)=0.9 rise=2 targ v(xtop:mux_out)=0.9 rise={first+2}
*.measure tran  d3 trig v(xtop:a0)=0.9 fall=2 targ v(xtop:mux_out)=0.9 fall={first+2}
*.measure tran  d4 trig v(xtop:a0)=0.9 rise=3 targ v(xtop:mux_out)=0.9 rise={first+3}
*.measure tran  d5 trig v(xtop:a0)=0.9 fall=3 targ v(xtop:mux_out)=0.9 fall={first+3}
*.measure tran  d6 trig v(xtop:a0)=0.9 rise=4 targ v(xtop:mux_out)=0.9 rise={first+4}
*.measure tran  d7 trig v(xtop:a0)=0.9 fall=4 targ v(xtop:mux_out)=0.9 fall={first+4}
*.measure tran  d8 trig v(xtop:a0)=0.9 rise=5 targ v(xtop:mux_out)=0.9 rise={first+5}
*.measure tran  d9 trig v(xtop:a0)=0.9 fall=5 targ v(xtop:mux_out)=0.9 fall={first+5}
*.measure tran d10 trig v(xtop:a0)=0.9 rise=6 targ v(xtop:mux_out)=0.9 rise={first+6}
*.measure tran d11 trig v(xtop:a0)=0.9 fall=6 targ v(xtop:mux_out)=0.9 fall={first+6}
*.measure tran d12 trig v(xtop:a0)=0.9 rise=7 targ v(xtop:mux_out)=0.9 rise={first+7}
*.measure tran d13 trig v(xtop:a0)=0.9 fall=7 targ v(xtop:mux_out)=0.9 fall={first+7}
*.measure tran d14 trig v(xtop:a0)=0.9 rise=8 targ v(xtop:mux_out)=0.9 rise={first+8}
*.measure tran d15 trig v(xtop:a0)=0.9 fall=8 targ v(xtop:mux_out)=0.9 fall={first+8}

.end

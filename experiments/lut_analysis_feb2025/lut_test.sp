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
.include fake_lut.sp
;.include package.sp
;.include lut.sp
;.include lut.spef.sp

.param num_bits=16
.param config_clock_period=2n
.param test_start={17 * config_clock_period + 50p}
.param read_clock=750p

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
+ a0 a1 a2 a3 out config_in config_out config_clk VPWR VSS
+ lut
+ ; No parameters

vpower
+ VPWR VSS 
+ dc {1.8} 
+ ac {0} 
+ ; No parameters

*vclock
*+ config_clk VSS
*+ pulse ({0} {1.8} {config_clock_period / 4} {50p} {50p} {config_clock_period / 2} {config_clock_period})

vclock
+ config_clk VSS
+ pat ({1.8} {0} {0} {50p} {50p} {config_clock_period / 2} b010101010101010101010101010101010 0)

vclocki
+ config_clk_i VSS
+ pat ({1.8} {0} {50p} {50p} {50p} {config_clock_period / 2} b010101010101010101010101010101010 0)

vdata
+ config_in VSS
+ pulse ({0} {1.8} {0} {50p} {50p} {config_clock_period} {2 * config_clock_period})

vconfig_en
+ config_en VSS
+ pulse ({0} {1.8} {0} {50p} {50p} {15.5 * config_clock_period} {10 * 16 * config_clock_period})

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

; read address into lut vertically:
va3_driver a3 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b00000000111111110 0)
va2_driver a2 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b00001111000011110 0)
va1_driver a1 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b00110011001100110 0)
va0_driver a0 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b01010101010101010 0)

.ends

.tran 1e-12 60e-9

xtop 0 testbench

.print tran format=csv v(*) i(*)

.param first=8

.measure tran  d0 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:out)=0.9 rise={first+1}
.measure tran  d1 trig v(xtop:a0)=0.9 fall=1 targ v(xtop:out)=0.9 fall={first+1}
.measure tran  d2 trig v(xtop:a0)=0.9 rise=2 targ v(xtop:out)=0.9 rise={first+2}
.measure tran  d3 trig v(xtop:a0)=0.9 fall=2 targ v(xtop:out)=0.9 fall={first+2}
.measure tran  d4 trig v(xtop:a0)=0.9 rise=3 targ v(xtop:out)=0.9 rise={first+3}
.measure tran  d5 trig v(xtop:a0)=0.9 fall=3 targ v(xtop:out)=0.9 fall={first+3}
.measure tran  d6 trig v(xtop:a0)=0.9 rise=4 targ v(xtop:out)=0.9 rise={first+4}
.measure tran  d7 trig v(xtop:a0)=0.9 fall=4 targ v(xtop:out)=0.9 fall={first+4}
.measure tran  d8 trig v(xtop:a0)=0.9 rise=5 targ v(xtop:out)=0.9 rise={first+5}
.measure tran  d9 trig v(xtop:a0)=0.9 fall=5 targ v(xtop:out)=0.9 fall={first+5}
.measure tran d10 trig v(xtop:a0)=0.9 rise=6 targ v(xtop:out)=0.9 rise={first+6}
.measure tran d11 trig v(xtop:a0)=0.9 fall=6 targ v(xtop:out)=0.9 fall={first+6}
.measure tran d12 trig v(xtop:a0)=0.9 rise=7 targ v(xtop:out)=0.9 rise={first+7}
.measure tran d13 trig v(xtop:a0)=0.9 fall=7 targ v(xtop:out)=0.9 fall={first+7}
.measure tran d14 trig v(xtop:a0)=0.9 rise=8 targ v(xtop:out)=0.9 rise={first+8}
.measure tran d15 trig v(xtop:a0)=0.9 fall=8 targ v(xtop:out)=0.9 fall={first+8}

.end

* test harness for luts

* take generated clb.lvs.v
* remove fillers, FILL1, FILL4, etc, since we don't have spice definitions of those circuits
* convert with v2lvs:
*
* run this:

.lib '/home/ff/eecs251b/sky130/sky130_cds/sky130_release_0.0.4/models/sky130.lib.spice' tt

.include '/home/ff/eecs251b/sky130/sky130_cds/sky130_scl_9T_0.0.5/cdl/sky130_scl_9T.cdl'

.include '../build/bfg_clb-scl/par-rundir/clb.lvs.sp'
*.include '../build/bfg_clb-scl/pex-rundir/clb.post.sp'

.param num_bits=18
.param scan_clock_period=2n
.param test_start=19 * scan_clock_period + 50p
.param read_clock=3n
.param half_vdd=1.8 / 2

.subckt testbench
+ VSS

* post par, not pex
xlut
+ clk a3 a2 a1 a0 bypass mux_out reg_out scan_clk scan_en scan_in scan_out VPWR VSS
+ clb

* with pex
*xlut
*+ scan_in a0 scan_en a3 VPWR a2 VSS reg_out mux_out bypass scan_out scan_clk clk a1
*+ clb

vscan_en_driver scan_en VSS dc 1.8

cscan_load scan_out VSS 2f

cmux_load mux_out VSS 2f

* FIXME(aryap): Manually testing on the clock buffers required this modification
* for things to work (in clk_buf_0):
* xnfet_1
* + X P VGND VNB 
* + sky130_fd_pr__nfet_01v8
* + PARAMS: l={150m} w={580m} 
* 
* xpfet_1
* + X P VPWR VPB 
* + sky130_fd_pr__pfet_01v8_hvt
* + PARAMS: l={150m} w={840m} 

vpower VPWR VSS dc 1.8

*vclock
*+ config_clk VSS
*+ pulse ({0} {1.8} {scan_clock_period / 4} {50p} {50p} {scan_clock_period / 2} {scan_clock_period})

vclock
+ clk VSS
+ pulse (0 1.8 1.5n 50p 50p 1n 2n)

vconfig_clock
+ scan_clk VSS
+ pat (1.8 0 0 50p 50p scan_clock_period/2 B0101010101010101010101010101010101010 0)

*vconfig_data
*+ config_in VSS
*+ pulse ({0} {1.8} {0} {50p} {50p} {scan_clock_period} {2 * scan_clock_period})

* NOTE: The first bit has to be 0 so that the last register in the scan chain
* gets loaded with 0: this sets the combinational output selection mux to the
* internal mux output.
*
*vscan_data
*+ scan_in VSS
*+ pat ("1.8" "0" "0" "50p" "50p" "scan_clock_period" B010011001100110010)

vscan_data
+ scan_in VSS
*+ pat ("1.8" "0" "0" "50p" "50p" "scan_clock_period" B011000011110000110)
+ pat ("1.8" "0" "0" "50p" "50p" "scan_clock_period" B000111100001111000)

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

*; FIXME(aryap): This is outdated.
*; the BFG LUT flip-flops are connected to the muxes in this order:
*; mux order                  scan order
*;         0  mem[ 0]		10
*;         1  mem[ 1]		1
*;         2  mem[ 2]		11
*;         3  mem[ 3]		0
*;         4  mem[ 4]		2
*;         5  mem[ 5]		6
*;         6  mem[ 6]		3
*;         7  mem[ 7]		7
*;         8  mem[ 8]		12
*;         9  mem[ 9]		14
*;        10  mem[10]		13
*;        11  mem[11]		15
*;        12  mem[12]		4
*;        13  mem[13]		9
*;        14  mem[14]		5
*;        15  mem[15]		8
*; but they are numbered for the their place in the scan chain.
*;
*; So to read in scan order, you have to read indices:
*; 3, 1, 4, 6, 12, 14, 5, 7, 15, 13, 0, 2, 8, 10, 9, 11 (TODO):
*va3_driver a3 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b000001111111100000 0)
*va2_driver a2 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b000001111000011110 0)
*va1_driver a1 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b000111100110000110 0)
*va0_driver a0 VSS pat ({1.8} {0} {test_start} {50p} {50p} {read_clock / 2} b010011001100110010 0)

; read address into lut vertically:
va3_driver a3 VSS pat ("1.8" "0" 38050p "50p" "50p" 'read_clock/2' b00000000111111110 0)
va2_driver a2 VSS pat ("1.8" "0" 38050p "50p" "50p" 'read_clock/2' b00001111000011110 0)
va1_driver a1 VSS pat ("1.8" "0" 38050p "50p" "50p" 'read_clock/2' b00110011001100110 0)
va0_driver a0 VSS pat ("1.8" "0" 38050p "50p" "50p" 'read_clock/2' b01010101010101010 0)

.ends

.tran 1e-12 80e-9

xtop 0 testbench


*.end

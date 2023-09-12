.param high=1.8
.param low=0

xupper_left__fet0
+ upper_left__A0 S0_B X2 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={420m} l={150m} 

* fake inputs
v4 x0 VSS PULSE({low} {high} 50p 50p 50p 500p 1n)
v5 x1 VSS {low}
v6 x2 VSS {high}
v7 x3 VSS {low}
v8 x4 VSS {high}
v9 x5 VSS {low}
v10 x6 VSS {high}
v11 x7 VSS {low}

* load
cfake y VSS 2p

vsource dut_VPWR VSS DC 1.8

vs2 s2 VSS DC {low}
vs2_b s2_b VSS DC {high}

vs1 s1 VSS DC {low}
vs1_b s1_b VSS DC {high}

vs0 s0 VSS DC {low}
vs0_b s0_b VSS DC {high}

*vs0 s0 VSS PULSE({low} {high} 50p 50p 50p 2n 4n)
*vs0_b s0_b VSS PULSE({high} {low} 50p 50p 50p 2n 4n)

.ENDS

xtop 0 Tb ; Top-Level DUT 

.lib /home/arya/src/pdk-root/share/pdk/sky130A/libs.tech/ngspice/sky130.lib.spice tt
.tran 1e-12 4e-09 

.print tran format=csv v(*) i(*) 

.end 


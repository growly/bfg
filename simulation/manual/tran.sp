; `xyce` Sim Input for `__main__.Tb`
; Generated by `vlsirtools.XyceNetlister`
; 
; Anonymous `circuit.Package`
; Generated by `vlsirtools.XyceNetlister`
; 

* NOTES:
* (1) Because of (something about how Xyce parser interprets the 'scale'
* directive in the Sky130 ngspice models), all parameters to the PDK FET models
* have to be in multiples of 1E-6, or microns. That means that a 460nm width
* must be expressed as 360nm*1E6 = 480m.

.SUBCKT sky130_mux
+ S0 S0_B S1 S1_B S2 S2_B X0 X1 X2 X3 X4 X5 X6 X7 Y VPWR VGND 
; No parameters

xupper_left__fet0
+ upper_left__A0 S0_B X2 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xupper_left__fet1
+ upper_left__A1 S0_B X0 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xupper_left__fet2
+ X3 S0 upper_left__A0 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xupper_left__fet3
+ X1 S0 upper_left__A1 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xupper_left__fet4
+ A0 S1_B upper_left__A1 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={640m} l={170m} 

xupper_left__fet5
+ upper_left__A0 S1 A0 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xlower_left__fet0
+ lower_left__A0 S0_B X6 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xlower_left__fet1
+ lower_left__A1 S0_B X4 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xlower_left__fet2
+ X7 S0 lower_left__A0 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xlower_left__fet3
+ X5 S0 lower_left__A1 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xlower_left__fet4
+ A1 S1_B lower_left__A1 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xlower_left__fet5
+ lower_left__A0 S1 A1 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xupper_right__fet0
+ upper_right__A0 S0 X2 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xupper_right__fet1
+ upper_right__A1 S0 X0 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xupper_right__fet2
+ X3 S0_B upper_right__A0 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xupper_right__fet3
+ X1 S0_B upper_right__A1 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xupper_right__fet4
+ A2 S1 upper_right__A1 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xupper_right__fet5
+ upper_right__A0 S1_B A2 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xlower_right__fet0
+ lower_right__A0 S0 X6 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xlower_right__fet1
+ lower_right__A1 S0 X4 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xlower_right__fet2
+ X7 S0_B lower_right__A0 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xlower_right__fet3
+ X5 S0_B lower_right__A1 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xlower_right__fet4
+ A3 S1 lower_right__A1 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xlower_right__fet5
+ lower_right__A0 S1_B A3 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xoutput_mux2__nfet0
+ Y S2_B A0 VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xoutput_mux2__nfet1
+ A1 S2 Y VGND 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: w={480m} l={170m} 

xoutput_mux2__pfet0
+ Y S2 A2 VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

xoutput_mux2__pfet1
+ A3 S2_B Y VPWR 
+ sky130_fd_pr__pfet_01v8
+ PARAMS: w={640m} l={170m} 

.ENDS

.SUBCKT Tb
+ VSS 
; No parameters

xdut
+ s0 s0_b s1 s1_b s2 s2_b x0 x1 x2 x3 x4 x5 x6 x7 y dut_VPWR VSS 
+ sky130_mux
+ ; No parameters

.param high=1.8
.param low=0

* fake inputs
*v4 x0 VSS PULSE({low} {high} 50p 50p 50p 500p 1n)
v4 x0 VSS DC {high}
v5 x1 VSS DC {low}
v6 x2 VSS DC {high}
v7 x3 VSS DC {low}
v8 x4 VSS DC {high}
v9 x5 VSS DC {low}
v10 x6 VSS DC {high}
v11 x7 VSS DC {low}

* load
cfake y VSS 2f

vsource dut_VPWR VSS DC 1.8

*vs2 s2 VSS DC {low}
*vs2_b s2_b VSS DC {high}
*
*vs1 s1 VSS DC {low}
*vs1_b s1_b VSS DC {high}
*
*vs0 s0 VSS DC {low}
*vs0_b s0_b VSS DC {high}

vs2   s2    VSS  PULSE({low} {high} 50p 50p 50p 2n 4n)
vs2_b s2_b  VSS  PULSE({high} {low} 50p 50p 50p 2n 4n)

vs1   s1    VSS  PULSE({low} {high} 150p 50p 50p 1n 2n)
vs1_b s1_b  VSS  PULSE({high} {low} 150p 50p 50p 1n 2n)

vs0   s0    VSS  PULSE({low} {high} 100p 50p 50p 500p 1n)
vs0_b s0_b  VSS  PULSE({high} {low} 100p 50p 50p 500p 1n)

.ENDS

xtop 0 Tb ; Top-Level DUT 

.lib /home/arya/src/pdk-root/share/pdk/sky130A/libs.tech/ngspice/sky130.lib.spice tt
.tran 1e-12 4e-09 

.measure tran d 

.print tran format=csv v(*) i(*) 

.end 


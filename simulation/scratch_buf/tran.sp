
.SUBCKT LutB_clk_buf_0_template
+ X P A VPWR VGND VPB VNB 
; No parameters

xnfet_0
+ P A VGND VNB 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: l={150m} w={520m} 

xpfet_0
+ P A VPWR VPB 
+ sky130_fd_pr__pfet_01v8_hvt
+ PARAMS: l={150m} w={790m} 

xnfet_1
+ X P VGND VNB 
+ sky130_fd_pr__nfet_01v8
+ PARAMS: l={150m} w={520m} 

xpfet_1
+ X P VPWR VPB 
+ sky130_fd_pr__pfet_01v8_hvt
+ PARAMS: l={150m} w={790m} 

.ENDS

.SUBCKT Tb
+ VSS 
; No parameters

vpower
+ vpwr VSS 
+ dc {1.8} 
+ ac {0} 
+ ; No parameters

cloadx
+ x VSS 
+ 2f
+ ; No parameters

cloadp
+ p VSS 
+ 2f
+ ; No parameters

xdut
+ x p in vpwr VSS vpwr VSS
+ LutB_clk_buf_0_template
+ ; No parameters

vin_driver
+ in VSS 
+ pulse ({0} {1.8} {300p} {50p} {50p} {250p} {500p}) 
+ ; No parameters

.ENDS

xtop 0 Tb ; Top-Level DUT 

.lib /home/arya/src/pdk-root/share/pdk/sky130A/libs.tech/ngspice/sky130.lib.spice tt
.tran 1e-12 4e-09 

.print tran format=csv v(*) i(*) 

.end 


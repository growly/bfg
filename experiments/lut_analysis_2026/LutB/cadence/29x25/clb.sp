* THIS DESIGN, AS PRODUCED BY INNOVUS, HAS AN ERROR. THE CLOCK BUFFER sdclkp
* CELL WAS USED TO DRIVE THE MEM.x MEMORIES WHILE THE INPUT SCAN_CLK WAS USED TO
* DRIVE THE FIRST AND SECOND REGISTERS DIRECTLY, CAUSING A TIMING VIOLATION. I
* FIXED IT MANUALLY TO MEASURE THE CIRCUITS.
.SUBCKT clb
+ VGND VPWR clk addr.0 addr.1 addr.2 addr.3 x mux q scan_clk scan_en scan_in.0 scan_out.0 VSS VDD
XFE_OFC2_mux FE_OFN2_mux VSS VSS VDD VDD mux sky130_fd_sc_hd__buf_8 
XFE_OFC1_scan_out_0 FE_OFN1_scan_out_0 VSS VSS VDD VDD scan_out.0 sky130_fd_sc_hd__buf_8 
XFE_OFC0_q FE_OFN0_q VSS VSS VDD VDD q sky130_fd_sc_hd__buf_8 
XCLKGATE_RC_CG_HIER_INST0/RC_CGIC_INST scan_clk scan_en logic_0_1_net VSS VSS VDD VDD CLKGATE_rc_gclk sky130_fd_sc_hd__sdlclkp_1 
Xq_reg_reg clk n_20 VSS VSS VDD VDD FE_OFN0_q sky130_fd_sc_hd__dfxtp_1 
Xg475__2398 n_19 x FE_OFN1_scan_out_0 VSS VSS VDD VDD FE_OFN2_mux sky130_fd_sc_hd__mux2_2 
Xg476__5107 n_19 x q_control VSS VSS VDD VDD n_20 sky130_fd_sc_hd__mux2_2 
Xg477__6260 n_11 n_18 addr.3 VSS VSS VDD VDD n_19 sky130_fd_sc_hd__mux2i_1 
Xg478__4319 n_12 n_3 n_16 addr.0 n_17 VSS VSS VDD VDD n_18 sky130_fd_sc_hd__a221oi_1 
Xg479__8428 addr.0 n_15 VSS VSS VDD VDD n_17 sky130_fd_sc_hd__nor2_1 
Xg480__5526 mem.15 n_4 mem.13 n_2 n_14 VSS VSS VDD VDD n_16 sky130_fd_sc_hd__a221o_1 
Xg481__6783 mem.14 n_4 mem.12 n_2 VSS VSS VDD VDD n_15 sky130_fd_sc_hd__a22oi_1 
Xmem_reg[15] CLKGATE_rc_gclk mem.14 VSS VSS VDD VDD mem.15 sky130_fd_sc_hd__dfxtp_1 
Xmem_reg[14] CLKGATE_rc_gclk mem.13 VSS VSS VDD VDD mem.14 sky130_fd_sc_hd__dfxtp_1 
Xg484__3680 addr.2 n_13 VSS VSS VDD VDD n_14 sky130_fd_sc_hd__nor2_1 
Xmem_reg[13] CLKGATE_rc_gclk mem.12 VSS VSS VDD VDD mem.13 sky130_fd_sc_hd__dfxtp_1 
Xg486__1617 mem.9 mem.11 addr.1 VSS VSS VDD VDD n_13 sky130_fd_sc_hd__mux2i_1 
Xmem_reg[12] CLKGATE_rc_gclk mem.11 VSS VSS VDD VDD mem.12 sky130_fd_sc_hd__dfxtp_1 
Xg488__2802 mem.8 mem.10 addr.1 VSS VSS VDD VDD n_12 sky130_fd_sc_hd__mux2_1 
Xmem_reg[11] CLKGATE_rc_gclk mem.10 VSS VSS VDD VDD mem.11 sky130_fd_sc_hd__dfxtp_1 
Xg490__1705 n_5 n_3 n_9 addr.0 n_10 VSS VSS VDD VDD n_11 sky130_fd_sc_hd__a221oi_1 
Xmem_reg[10] CLKGATE_rc_gclk mem.9 VSS VSS VDD VDD mem.10 sky130_fd_sc_hd__dfxtp_1 
Xmem_reg[9] CLKGATE_rc_gclk mem.8 VSS VSS VDD VDD mem.9 sky130_fd_sc_hd__dfxtp_1 
Xg493__5122 addr.0 n_8 VSS VSS VDD VDD n_10 sky130_fd_sc_hd__nor2_1 
Xg494__8246 mem.7 n_4 mem.5 n_2 n_7 VSS VSS VDD VDD n_9 sky130_fd_sc_hd__a221o_1 
Xmem_reg[8] CLKGATE_rc_gclk mem.7 VSS VSS VDD VDD mem.8 sky130_fd_sc_hd__dfxtp_1 
Xg496__7098 mem.6 n_4 mem.4 n_2 VSS VSS VDD VDD n_8 sky130_fd_sc_hd__a22oi_1 
Xmem_reg[7] CLKGATE_rc_gclk mem.6 VSS VSS VDD VDD mem.7 sky130_fd_sc_hd__dfxtp_1 
Xmem_reg[6] CLKGATE_rc_gclk mem.5 VSS VSS VDD VDD mem.6 sky130_fd_sc_hd__dfxtp_1 
Xg499__6131 addr.2 n_6 VSS VSS VDD VDD n_7 sky130_fd_sc_hd__nor2_1 
Xmem_reg[5] CLKGATE_rc_gclk mem.4 VSS VSS VDD VDD mem.5 sky130_fd_sc_hd__dfxtp_1 
Xg501__1881 mem.1 mem.3 addr.1 VSS VSS VDD VDD n_6 sky130_fd_sc_hd__mux2i_1 
Xmem_reg[4] CLKGATE_rc_gclk mem.3 VSS VSS VDD VDD mem.4 sky130_fd_sc_hd__dfxtp_1 
Xg503__5115 mem.0 mem.2 addr.1 VSS VSS VDD VDD n_5 sky130_fd_sc_hd__mux2_1 
Xmem_reg[3] CLKGATE_rc_gclk mem.2 VSS VSS VDD VDD mem.3 sky130_fd_sc_hd__dfxtp_1 
Xmem_reg[2] CLKGATE_rc_gclk mem.1 VSS VSS VDD VDD mem.2 sky130_fd_sc_hd__dfxtp_1 
Xmem_reg[1] CLKGATE_rc_gclk mem.0 VSS VSS VDD VDD mem.1 sky130_fd_sc_hd__dfxtp_1 
Xmem_reg[0] CLKGATE_rc_gclk q_control VSS VSS VDD VDD mem.0 sky130_fd_sc_hd__dfxtp_1 
Xg508__7482 addr.2 addr.1 VSS VSS VDD VDD n_4 sky130_fd_sc_hd__and2_0 
Xg509__4733 addr.0 addr.2 VSS VSS VDD VDD n_3 sky130_fd_sc_hd__nor2_1 
Xg510__6161 addr.1 addr.2 VSS VSS VDD VDD n_2 sky130_fd_sc_hd__nor2b_1 
Xmux_control_reg CLKGATE_rc_gclk mem.15 scan_en VSS VSS VDD VDD FE_OFN1_scan_out_0 sky130_fd_sc_hd__edfxtp_1 
Xq_control_reg CLKGATE_rc_gclk scan_in.0 scan_en VSS VSS VDD VDD q_control sky130_fd_sc_hd__edfxtp_1 
Xtie_0_cell VSS VSS VDD VDD UNCONNECTED logic_0_1_net sky130_fd_sc_hd__conb_1 
XCLKGATE_RC_CG_HIER_INST0/FILLER_AO_T_0_1 VSS VSS VDD VDD sky130_fd_sc_hd__decap_6 
XCLKGATE_RC_CG_HIER_INST0/FILLER_AO_T_0_2 VSS VSS VDD VDD sky130_fd_sc_hd__decap_6 
XCLKGATE_RC_CG_HIER_INST0/FILLER_AO_T_0_3 VSS VSS VDD VDD sky130_fd_sc_hd__decap_3 
XCLKGATE_RC_CG_HIER_INST0/FILLER_AO_T_0_4 VSS VSS VDD VDD sky130_fd_sc_hd__decap_3 
XCLKGATE_RC_CG_HIER_INST0/FILLER_AO_T_0_5 VSS VSS VDD VDD sky130_fd_sc_hd__decap_3 
XCLKGATE_RC_CG_HIER_INST0/FILLER_AO_T_0_6 VSS VSS VDD VDD sky130_fd_sc_hd__decap_6 
.ENDS


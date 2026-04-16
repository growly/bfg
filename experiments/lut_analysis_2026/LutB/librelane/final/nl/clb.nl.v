module clb (clk,
    mux,
    q,
    scan_clk,
    scan_en,
    scan_in,
    scan_out,
    x,
    addr);
 input clk;
 output mux;
 output q;
 input scan_clk;
 input scan_en;
 input scan_in;
 output scan_out;
 input x;
 input [3:0] addr;

 wire _00_;
 wire _01_;
 wire _02_;
 wire _03_;
 wire _04_;
 wire _05_;
 wire _06_;
 wire _07_;
 wire _08_;
 wire _09_;
 wire _10_;
 wire _11_;
 wire _12_;
 wire _13_;
 wire _14_;
 wire _15_;
 wire _16_;
 wire _17_;
 wire _18_;
 wire _19_;
 wire _20_;
 wire _21_;
 wire _22_;
 wire _23_;
 wire _24_;
 wire _25_;
 wire _26_;
 wire _27_;
 wire _28_;
 wire net1;
 wire net2;
 wire net3;
 wire net4;
 wire net5;
 wire \mem[0] ;
 wire \mem[10] ;
 wire \mem[11] ;
 wire \mem[12] ;
 wire \mem[13] ;
 wire \mem[14] ;
 wire \mem[15] ;
 wire \mem[1] ;
 wire \mem[2] ;
 wire \mem[3] ;
 wire \mem[4] ;
 wire \mem[5] ;
 wire \mem[6] ;
 wire \mem[7] ;
 wire \mem[8] ;
 wire \mem[9] ;
 wire net9;
 wire net10;
 wire q_control;
 wire net6;
 wire net7;
 wire net11;
 wire net8;
 wire net12;
 wire net13;
 wire clknet_0_scan_clk;
 wire clknet_1_0__leaf_scan_clk;
 wire clknet_1_1__leaf_scan_clk;
 wire net14;
 wire net15;
 wire net16;
 wire net17;
 wire net18;
 wire net19;
 wire net20;
 wire net21;
 wire net22;
 wire net23;
 wire net24;
 wire net25;
 wire net26;
 wire net27;
 wire net28;
 wire net29;
 wire net30;
 wire net31;
 wire net32;
 wire net33;
 wire net34;
 wire net35;
 wire net36;
 wire net37;

 sky130_fd_sc_hd__fill_1 FILLER_0_55 ();
 sky130_fd_sc_hd__decap_3 FILLER_10_24 ();
 sky130_fd_sc_hd__fill_1 FILLER_10_27 ();
 sky130_fd_sc_hd__decap_3 FILLER_10_3 ();
 sky130_fd_sc_hd__decap_3 FILLER_10_61 ();
 sky130_fd_sc_hd__decap_3 FILLER_10_64 ();
 sky130_fd_sc_hd__decap_3 FILLER_11_19 ();
 sky130_fd_sc_hd__fill_1 FILLER_11_22 ();
 sky130_fd_sc_hd__decap_3 FILLER_11_31 ();
 sky130_fd_sc_hd__fill_1 FILLER_11_34 ();
 sky130_fd_sc_hd__decap_3 FILLER_11_43 ();
 sky130_fd_sc_hd__fill_1 FILLER_11_46 ();
 sky130_fd_sc_hd__fill_1 FILLER_11_55 ();
 sky130_fd_sc_hd__fill_2 FILLER_11_65 ();
 sky130_fd_sc_hd__fill_1 FILLER_12_14 ();
 sky130_fd_sc_hd__decap_3 FILLER_12_23 ();
 sky130_fd_sc_hd__fill_2 FILLER_12_26 ();
 sky130_fd_sc_hd__decap_3 FILLER_12_29 ();
 sky130_fd_sc_hd__decap_3 FILLER_12_3 ();
 sky130_fd_sc_hd__decap_3 FILLER_12_32 ();
 sky130_fd_sc_hd__decap_3 FILLER_12_51 ();
 sky130_fd_sc_hd__decap_3 FILLER_12_54 ();
 sky130_fd_sc_hd__fill_1 FILLER_12_57 ();
 sky130_fd_sc_hd__fill_1 FILLER_12_74 ();
 sky130_fd_sc_hd__fill_1 FILLER_13_27 ();
 sky130_fd_sc_hd__decap_3 FILLER_13_45 ();
 sky130_fd_sc_hd__decap_3 FILLER_13_48 ();
 sky130_fd_sc_hd__decap_3 FILLER_13_51 ();
 sky130_fd_sc_hd__fill_2 FILLER_13_54 ();
 sky130_fd_sc_hd__decap_3 FILLER_13_57 ();
 sky130_fd_sc_hd__decap_3 FILLER_13_60 ();
 sky130_fd_sc_hd__decap_3 FILLER_13_63 ();
 sky130_fd_sc_hd__fill_1 FILLER_13_66 ();
 sky130_fd_sc_hd__fill_2 FILLER_1_54 ();
 sky130_fd_sc_hd__fill_1 FILLER_1_82 ();
 sky130_fd_sc_hd__fill_1 FILLER_2_62 ();
 sky130_fd_sc_hd__decap_3 FILLER_2_80 ();
 sky130_fd_sc_hd__decap_3 FILLER_3_3 ();
 sky130_fd_sc_hd__fill_1 FILLER_3_35 ();
 sky130_fd_sc_hd__fill_2 FILLER_3_54 ();
 sky130_fd_sc_hd__fill_2 FILLER_3_6 ();
 sky130_fd_sc_hd__fill_1 FILLER_3_82 ();
 sky130_fd_sc_hd__fill_1 FILLER_4_22 ();
 sky130_fd_sc_hd__fill_1 FILLER_4_29 ();
 sky130_fd_sc_hd__fill_2 FILLER_4_3 ();
 sky130_fd_sc_hd__decap_3 FILLER_5_3 ();
 sky130_fd_sc_hd__fill_2 FILLER_5_54 ();
 sky130_fd_sc_hd__fill_2 FILLER_5_6 ();
 sky130_fd_sc_hd__decap_3 FILLER_5_77 ();
 sky130_fd_sc_hd__decap_3 FILLER_5_80 ();
 sky130_fd_sc_hd__fill_1 FILLER_6_27 ();
 sky130_fd_sc_hd__decap_3 FILLER_6_3 ();
 sky130_fd_sc_hd__decap_3 FILLER_6_38 ();
 sky130_fd_sc_hd__fill_1 FILLER_6_6 ();
 sky130_fd_sc_hd__decap_3 FILLER_6_78 ();
 sky130_fd_sc_hd__fill_2 FILLER_6_81 ();
 sky130_fd_sc_hd__decap_3 FILLER_7_3 ();
 sky130_fd_sc_hd__fill_2 FILLER_7_35 ();
 sky130_fd_sc_hd__fill_1 FILLER_7_55 ();
 sky130_fd_sc_hd__fill_2 FILLER_7_6 ();
 sky130_fd_sc_hd__decap_3 FILLER_7_60 ();
 sky130_fd_sc_hd__decap_3 FILLER_7_80 ();
 sky130_fd_sc_hd__fill_1 FILLER_8_55 ();
 sky130_fd_sc_hd__fill_1 FILLER_8_82 ();
 sky130_fd_sc_hd__fill_2 FILLER_9_54 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_0_Left_14 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_0_Right_0 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_10_Left_24 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_10_Right_10 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_11_Left_25 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_11_Right_11 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_12_Left_26 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_12_Right_12 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_13_Left_27 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_13_Right_13 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_1_Left_15 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_1_Right_1 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_2_Left_16 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_2_Right_2 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_3_Left_17 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_3_Right_3 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_4_Left_18 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_4_Right_4 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_5_Left_19 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_5_Right_5 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_6_Left_20 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_6_Right_6 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_7_Left_21 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_7_Right_7 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_8_Left_22 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_8_Right_8 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_9_Left_23 ();
 sky130_fd_sc_hd__decap_3 PHY_EDGE_ROW_9_Right_9 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_0_28 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_0_29 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_10_39 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_11_40 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_12_41 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_13_42 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_13_43 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_1_30 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_2_31 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_3_32 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_4_33 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_5_34 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_6_35 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_7_36 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_8_37 ();
 sky130_fd_sc_hd__tapvpwrvgnd_1 TAP_TAPCELL_ROW_9_38 ();
 sky130_fd_sc_hd__inv_2 _29_ (.A(net3),
    .Y(_19_));
 sky130_fd_sc_hd__inv_2 _30_ (.A(net4),
    .Y(_20_));
 sky130_fd_sc_hd__mux4_2 _31_ (.A0(\mem[8] ),
    .A1(\mem[9] ),
    .A2(\mem[10] ),
    .A3(\mem[11] ),
    .S0(net1),
    .S1(net2),
    .X(_21_));
 sky130_fd_sc_hd__or2_2 _32_ (.A(net3),
    .B(_21_),
    .X(_22_));
 sky130_fd_sc_hd__mux4_2 _33_ (.A0(\mem[12] ),
    .A1(\mem[13] ),
    .A2(\mem[14] ),
    .A3(\mem[15] ),
    .S0(net1),
    .S1(net2),
    .X(_23_));
 sky130_fd_sc_hd__o21a_2 _34_ (.A1(_19_),
    .A2(_23_),
    .B1(net4),
    .X(_24_));
 sky130_fd_sc_hd__mux4_2 _35_ (.A0(\mem[0] ),
    .A1(\mem[1] ),
    .A2(\mem[2] ),
    .A3(\mem[3] ),
    .S0(net1),
    .S1(net2),
    .X(_25_));
 sky130_fd_sc_hd__mux4_2 _36_ (.A0(\mem[4] ),
    .A1(\mem[5] ),
    .A2(\mem[6] ),
    .A3(\mem[7] ),
    .S0(net1),
    .S1(net2),
    .X(_26_));
 sky130_fd_sc_hd__mux2_1 _37_ (.A0(_25_),
    .A1(_26_),
    .S(net3),
    .X(_27_));
 sky130_fd_sc_hd__a22o_2 _38_ (.A1(_22_),
    .A2(_24_),
    .B1(_27_),
    .B2(_20_),
    .X(_28_));
 sky130_fd_sc_hd__mux2_1 _39_ (.A0(_28_),
    .A1(net8),
    .S(q_control),
    .X(_00_));
 sky130_fd_sc_hd__mux2_1 _40_ (.A0(_28_),
    .A1(net8),
    .S(net11),
    .X(net9));
 sky130_fd_sc_hd__mux2_1 _41_ (.A0(net22),
    .A1(net16),
    .S(net13),
    .X(_01_));
 sky130_fd_sc_hd__mux2_1 _42_ (.A0(net16),
    .A1(\mem[13] ),
    .S(net13),
    .X(_02_));
 sky130_fd_sc_hd__mux2_1 _43_ (.A0(net20),
    .A1(\mem[12] ),
    .S(net13),
    .X(_03_));
 sky130_fd_sc_hd__mux2_1 _44_ (.A0(net27),
    .A1(net23),
    .S(net12),
    .X(_04_));
 sky130_fd_sc_hd__mux2_1 _45_ (.A0(net23),
    .A1(net14),
    .S(net12),
    .X(_05_));
 sky130_fd_sc_hd__mux2_1 _46_ (.A0(net14),
    .A1(\mem[9] ),
    .S(net12),
    .X(_06_));
 sky130_fd_sc_hd__mux2_1 _47_ (.A0(net31),
    .A1(net29),
    .S(net12),
    .X(_07_));
 sky130_fd_sc_hd__mux2_1 _48_ (.A0(net29),
    .A1(net18),
    .S(net12),
    .X(_08_));
 sky130_fd_sc_hd__mux2_1 _49_ (.A0(net18),
    .A1(\mem[6] ),
    .S(net12),
    .X(_09_));
 sky130_fd_sc_hd__mux2_1 _50_ (.A0(net37),
    .A1(net35),
    .S(net12),
    .X(_10_));
 sky130_fd_sc_hd__mux2_1 _51_ (.A0(net35),
    .A1(net33),
    .S(net12),
    .X(_11_));
 sky130_fd_sc_hd__mux2_1 _52_ (.A0(net33),
    .A1(\mem[3] ),
    .S(net12),
    .X(_12_));
 sky130_fd_sc_hd__mux2_1 _53_ (.A0(net36),
    .A1(net30),
    .S(net12),
    .X(_13_));
 sky130_fd_sc_hd__mux2_1 _54_ (.A0(net30),
    .A1(net26),
    .S(net13),
    .X(_14_));
 sky130_fd_sc_hd__mux2_1 _55_ (.A0(net26),
    .A1(net24),
    .S(net13),
    .X(_15_));
 sky130_fd_sc_hd__mux2_1 _56_ (.A0(net32),
    .A1(net7),
    .S(net13),
    .X(_16_));
 sky130_fd_sc_hd__mux2_1 _57_ (.A0(net24),
    .A1(q_control),
    .S(net13),
    .X(_17_));
 sky130_fd_sc_hd__mux2_1 _58_ (.A0(net28),
    .A1(net22),
    .S(net13),
    .X(_18_));
 sky130_fd_sc_hd__dfxtp_2 _59_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(_01_),
    .Q(\mem[15] ));
 sky130_fd_sc_hd__dfxtp_2 _60_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(net17),
    .Q(\mem[14] ));
 sky130_fd_sc_hd__dfxtp_2 _61_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(net21),
    .Q(\mem[13] ));
 sky130_fd_sc_hd__dfxtp_2 _62_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(_04_),
    .Q(\mem[12] ));
 sky130_fd_sc_hd__dfxtp_2 _63_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(_05_),
    .Q(\mem[11] ));
 sky130_fd_sc_hd__dfxtp_2 _64_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(net15),
    .Q(\mem[10] ));
 sky130_fd_sc_hd__dfxtp_2 _65_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(_07_),
    .Q(\mem[9] ));
 sky130_fd_sc_hd__dfxtp_2 _66_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(_08_),
    .Q(\mem[8] ));
 sky130_fd_sc_hd__dfxtp_2 _67_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(net19),
    .Q(\mem[7] ));
 sky130_fd_sc_hd__dfxtp_2 _68_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(_10_),
    .Q(\mem[6] ));
 sky130_fd_sc_hd__dfxtp_2 _69_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(_11_),
    .Q(\mem[5] ));
 sky130_fd_sc_hd__dfxtp_2 _70_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(net34),
    .Q(\mem[4] ));
 sky130_fd_sc_hd__dfxtp_2 _71_ (.CLK(clknet_1_0__leaf_scan_clk),
    .D(_13_),
    .Q(\mem[3] ));
 sky130_fd_sc_hd__dfxtp_2 _72_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(_14_),
    .Q(\mem[2] ));
 sky130_fd_sc_hd__dfxtp_2 _73_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(_15_),
    .Q(\mem[1] ));
 sky130_fd_sc_hd__dfxtp_2 _74_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(_16_),
    .Q(q_control));
 sky130_fd_sc_hd__dfxtp_2 _75_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(net25),
    .Q(\mem[0] ));
 sky130_fd_sc_hd__dfxtp_2 _76_ (.CLK(clknet_1_1__leaf_scan_clk),
    .D(_18_),
    .Q(net11));
 sky130_fd_sc_hd__dfxtp_2 _77_ (.CLK(net5),
    .D(_00_),
    .Q(net10));
 sky130_fd_sc_hd__clkbuf_16 clkbuf_0_scan_clk (.A(scan_clk),
    .X(clknet_0_scan_clk));
 sky130_fd_sc_hd__clkbuf_16 clkbuf_1_0__f_scan_clk (.A(clknet_0_scan_clk),
    .X(clknet_1_0__leaf_scan_clk));
 sky130_fd_sc_hd__clkbuf_16 clkbuf_1_1__f_scan_clk (.A(clknet_0_scan_clk),
    .X(clknet_1_1__leaf_scan_clk));
 sky130_fd_sc_hd__clkdlybuf4s25_1 fanout12 (.A(net6),
    .X(net12));
 sky130_fd_sc_hd__clkdlybuf4s25_1 fanout13 (.A(net6),
    .X(net13));
 sky130_fd_sc_hd__dlygate4sd3_1 hold14 (.A(\mem[10] ),
    .X(net14));
 sky130_fd_sc_hd__dlygate4sd3_1 hold15 (.A(_06_),
    .X(net15));
 sky130_fd_sc_hd__dlygate4sd3_1 hold16 (.A(\mem[14] ),
    .X(net16));
 sky130_fd_sc_hd__dlygate4sd3_1 hold17 (.A(_02_),
    .X(net17));
 sky130_fd_sc_hd__dlygate4sd3_1 hold18 (.A(\mem[7] ),
    .X(net18));
 sky130_fd_sc_hd__dlygate4sd3_1 hold19 (.A(_09_),
    .X(net19));
 sky130_fd_sc_hd__dlygate4sd3_1 hold20 (.A(\mem[13] ),
    .X(net20));
 sky130_fd_sc_hd__dlygate4sd3_1 hold21 (.A(_03_),
    .X(net21));
 sky130_fd_sc_hd__dlygate4sd3_1 hold22 (.A(\mem[15] ),
    .X(net22));
 sky130_fd_sc_hd__dlygate4sd3_1 hold23 (.A(\mem[11] ),
    .X(net23));
 sky130_fd_sc_hd__dlygate4sd3_1 hold24 (.A(\mem[0] ),
    .X(net24));
 sky130_fd_sc_hd__dlygate4sd3_1 hold25 (.A(_17_),
    .X(net25));
 sky130_fd_sc_hd__dlygate4sd3_1 hold26 (.A(\mem[1] ),
    .X(net26));
 sky130_fd_sc_hd__dlygate4sd3_1 hold27 (.A(\mem[12] ),
    .X(net27));
 sky130_fd_sc_hd__dlygate4sd3_1 hold28 (.A(net11),
    .X(net28));
 sky130_fd_sc_hd__dlygate4sd3_1 hold29 (.A(\mem[8] ),
    .X(net29));
 sky130_fd_sc_hd__dlygate4sd3_1 hold30 (.A(\mem[2] ),
    .X(net30));
 sky130_fd_sc_hd__dlygate4sd3_1 hold31 (.A(\mem[9] ),
    .X(net31));
 sky130_fd_sc_hd__dlygate4sd3_1 hold32 (.A(q_control),
    .X(net32));
 sky130_fd_sc_hd__dlygate4sd3_1 hold33 (.A(\mem[4] ),
    .X(net33));
 sky130_fd_sc_hd__dlygate4sd3_1 hold34 (.A(_12_),
    .X(net34));
 sky130_fd_sc_hd__dlygate4sd3_1 hold35 (.A(\mem[5] ),
    .X(net35));
 sky130_fd_sc_hd__dlygate4sd3_1 hold36 (.A(\mem[3] ),
    .X(net36));
 sky130_fd_sc_hd__dlygate4sd3_1 hold37 (.A(\mem[6] ),
    .X(net37));
 sky130_fd_sc_hd__clkdlybuf4s25_1 input1 (.A(addr[0]),
    .X(net1));
 sky130_fd_sc_hd__clkdlybuf4s25_1 input2 (.A(addr[1]),
    .X(net2));
 sky130_fd_sc_hd__clkdlybuf4s25_1 input3 (.A(addr[2]),
    .X(net3));
 sky130_fd_sc_hd__clkdlybuf4s25_1 input4 (.A(addr[3]),
    .X(net4));
 sky130_fd_sc_hd__clkdlybuf4s25_1 input5 (.A(clk),
    .X(net5));
 sky130_fd_sc_hd__clkdlybuf4s25_1 input6 (.A(scan_en),
    .X(net6));
 sky130_fd_sc_hd__clkdlybuf4s25_1 input7 (.A(scan_in),
    .X(net7));
 sky130_fd_sc_hd__clkdlybuf4s25_1 input8 (.A(x),
    .X(net8));
 sky130_fd_sc_hd__clkdlybuf4s25_1 output10 (.A(net10),
    .X(q));
 sky130_fd_sc_hd__clkdlybuf4s25_1 output11 (.A(net11),
    .X(scan_out));
 sky130_fd_sc_hd__clkdlybuf4s25_1 output9 (.A(net9),
    .X(mux));
endmodule

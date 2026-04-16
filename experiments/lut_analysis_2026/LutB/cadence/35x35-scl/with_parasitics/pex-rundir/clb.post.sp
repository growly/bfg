*
*
*
*                       LINUX           Thu Apr  9 17:25:47 2026
*
*
*
*  PROGRAM  advgen
*
*  Name           : advgen - Quantus - (64-bit)
*  Version        : 22.1.1-p041
*  Build Date     : Mon Apr 17 07:36:05 PDT 2023
*
*  HSPICE LIBRARY
*
*  OPERATING_TEMPERATURE 25
*  QRC_TECH_DIR /home/ff/eecs251b/sky130/sky130_cds/sky130_release_0.0.4/quantus/extraction/typical 
*
*
*

*
.SUBCKT clb scan_in[0] addr[0] scan_en addr[3] VDD addr[2] VSS q mux x
+ scan_out[0] scan_clk clk addr[1]
*
*
*  caps2d version: 10
*
*
*       TRANSISTOR CARDS
*
*
Mg526/Mmn0	n_12#30	addr[2]#11	VSS#128	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg527/Mmn0	n_2#30	addr[1]#29	VSS#65	VSS	nfet_01v8
+ L=1.5e-07	W=7.6e-07
Mg525__6131/Mmn1	n_8#33	addr[1]#25	VSS#292	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg525__6131/Mmn0	n_8#33	addr[0]#17	VSS#294	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg524__7098/Mmn1	n_7#30	n_2#9	VSS#77	VSS	nfet_01v8
+ L=1.5e-07	W=7.6e-07
Mg524__7098/Mmn0	n_7#30	addr[0]#11	VSS#80	VSS	nfet_01v8
+ L=1.5e-07	W=7.6e-07
Mg523__8246/Mmn0	g523__8246/n0	addr[0]#19	VSS#64	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg523__8246/Mmn1	n_11#17	n_12#9	g523__8246/n0	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg510__3680/Mmn2	g510__3680/net98	n_7#15	VSS#269	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg510__3680/Mmn0	g510__3680/net102	n_8#15	VSS#271	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg510__3680/Mmn3	n_5#11	mem[10]#11	g510__3680/net98	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg510__3680/Mmn1	n_5#11	mem[8]#11	g510__3680/net102	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg509__6783/Mmn2	g509__6783/net98	n_7#13	VSS#376	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg509__6783/Mmn0	g509__6783/net102	n_8#13	VSS#379	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg509__6783/Mmn3	n_6#11	mem[2]#7	g509__6783/net98	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg509__6783/Mmn1	n_6#11	mem[0]#7	g509__6783/net102	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg522__2802/Mmn2	g522__2802/net98	addr[1]#21	VSS#70	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg522__2802/Mmn0	g522__2802/net102	n_2#11	VSS#73	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg522__2802/Mmn3	n_3#9	mem[11]#9	g522__2802/net98	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg522__2802/Mmn1	n_3#9	mem[9]#9	g522__2802/net102	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg521__1617/Mmn2	g521__1617/net98	addr[1]#17	VSS#78	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg521__1617/Mmn0	g521__1617/net102	n_2#7	VSS#83	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg521__1617/Mmn3	n_4#8	mem[3]#11	g521__1617/net98	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg521__1617/Mmn1	n_4#8	mem[1]#11	g521__1617/net102	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg497__8428/Mmn5	n_10#13	mem[6]#11	g497__8428/net155
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mg497__8428/Mmn4	g497__8428/net155	n_7#11	VSS#68	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg497__8428/Mmn2	g497__8428/net147	addr[0]#15	VSS#69	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg497__8428/Mmn3	n_10#8	n_0#3	g497__8428/net147	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg497__8428/Mmn0	g497__8428/net139	n_8#11	VSS#74	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg497__8428/Mmn1	n_10#8	mem[4]#11	g497__8428/net139	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg498__5526/Mmn5	n_9#13	mem[14]#11	g498__5526/net155	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg498__5526/Mmn4	g498__5526/net155	n_7#9	VSS#296	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg498__5526/Mmn2	g498__5526/net147	addr[0]#13	VSS#298
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mg498__5526/Mmn3	n_9#8	n_1#3	g498__5526/net147	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg498__5526/Mmn0	g498__5526/net139	n_8#9	VSS#300	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg498__5526/Mmn1	n_9#8	mem[12]#11	g498__5526/net139	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg495__6260/Mmn5	n_14#13	n_3#3	g495__6260/n0#3	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg495__6260/Mmn3	g495__6260/n0#3	n_9#3	g495__6260/n1#4
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mg495__6260/Mmn4	n_14#13	n_11#7	g495__6260/n0#5	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg495__6260/Mmn0	g495__6260/n1#2	addr[2]#9	VSS#61	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg495__6260/Mmn2	g495__6260/n0	n_12#11	g495__6260/n1#4
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mg495__6260/Mmn1	g495__6260/n1#2	n_5#3	VSS#60	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg496__4319/Mmn5	n_13#13	n_4#3	g496__4319/n0#3	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg496__4319/Mmn3	g496__4319/n0#3	n_10#3	g496__4319/n1#4
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mg496__4319/Mmn4	n_13#13	n_11#5	g496__4319/n0#5	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg496__4319/Mmn0	g496__4319/n1#2	addr[2]#7	VSS#291
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mg496__4319/Mmn2	g496__4319/n0	n_12#7	g496__4319/n1#4	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg496__4319/Mmn1	g496__4319/n1#2	n_6#3	VSS#289	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg494__5107/Mmn2	g494__5107/n0#11	g494__5107/S0b#3
+ g494__5107/n3	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg494__5107/Mmn1	g494__5107/n3	n_13#3	VSS#272	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mg494__5107/Mmn4	g494__5107/n0#11	addr[3]#7	g494__5107/n5
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg494__5107/Mmn3	g494__5107/n5	n_14#3	VSS#270	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mg494__5107/Mmn5	n_15#21	g494__5107/n0#3	VSS#270
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mg494__5107/Mmn0	g494__5107/S0b#11	addr[3]#5	VSS#272
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg519__1705/Mmn2	g519__1705/n0#9	g519__1705/S0b#3
+ g519__1705/n3	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg519__1705/Mmn1	g519__1705/n3	mem[13]#9	VSS#383	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mg519__1705/Mmn4	g519__1705/n0#9	addr[1]#31	g519__1705/n5
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg519__1705/Mmn3	g519__1705/n5	mem[15]#9	VSS#381	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mg519__1705/Mmn5	n_1#11	g519__1705/n0#3	VSS#381	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg519__1705/Mmn0	g519__1705/S0b#9	addr[1]#27	VSS#383
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg520__5122/Mmn2	g520__5122/n0#9	g520__5122/S0b#3
+ g520__5122/n3	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg520__5122/Mmn1	g520__5122/n3	mem[5]#11	VSS#387	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mg520__5122/Mmn4	g520__5122/n0#9	addr[1]#23	g520__5122/n5
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg520__5122/Mmn3	g520__5122/n5	mem[7]#9	VSS#385	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mg520__5122/Mmn5	n_0#11	g520__5122/n0#3	VSS#385	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg520__5122/Mmn0	g520__5122/S0b#9	addr[1]#19	VSS#387
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mg493__2398/Mmn2	g493__2398/n0#31	g493__2398/S0b#3
+ g493__2398/n3	VSS	nfet_01v8	L=1.5e-07	W=6e-07
Mg493__2398/Mmn5	mux#2	g493__2398/n0#9	VSS#127	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg493__2398/Mmn5@1	mux#2	g493__2398/n0#11	VSS#126	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg493__2398/Mmn5@2	mux#6	g493__2398/n0#13	VSS#126	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg493__2398/Mmn5@3	mux#6	g493__2398/n0#15	VSS#108	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
Mg493__2398/Mmn4	g493__2398/n0#31	FE_OFN1_scan_out_0#27
+ g493__2398/n5	VSS	nfet_01v8	L=1.5e-07	W=6e-07
Mg493__2398/Mmn1	g493__2398/n3	n_15#7	VSS#129	VSS
+ nfet_01v8	L=1.5e-07	W=6e-07
Mg493__2398/Mmn3	g493__2398/n5	x#7	VSS#127	VSS
+ nfet_01v8	L=1.5e-07	W=6e-07
Mg493__2398/Mmn0	g493__2398/S0b#9	FE_OFN1_scan_out_0#25	VSS#129
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn50	\mem_reg[10]/net436	\mem_reg[10]/qbint#7	VSS#45
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn12	\mem_reg[10]/Db#4	\mem_reg[10]/SEb#3
+ \mem_reg[10]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn11	\mem_reg[10]/net449	mem[10]#9	VSS#56	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn51	\mem_reg[10]/n30#11	\mem_reg[10]/CKb#11
+ \mem_reg[10]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn36	\mem_reg[10]/n20#11	\mem_reg[10]/CKbb#5
+ \mem_reg[10]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn25	\mem_reg[10]/n20#11	\mem_reg[10]/CKb#7
+ \mem_reg[10]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn35	\mem_reg[10]/net429	\mem_reg[10]/mout#3	VSS#50
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn13	\mem_reg[10]/net461	mem[8]#9	VSS#54	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn14	\mem_reg[10]/Db#4	scan_en#143
+ \mem_reg[10]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn40	\mem_reg[10]/n30#11	\mem_reg[10]/CKbb#7
+ \mem_reg[10]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn30	\mem_reg[10]/mout#11	\mem_reg[10]/n20#3	VSS#48
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn56	\mem_reg[10]/net367#11	\mem_reg[10]/qbint#9	VSS#43
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn21	\mem_reg[10]/CKbb#20	\mem_reg[10]/CKb#9	VSS#50
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn20	\mem_reg[10]/CKb#25	scan_clk#71	VSS#54	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn57	UNCONNECTED15#4	\mem_reg[10]/net367#3	VSS#40
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[10]/Mmn45	\mem_reg[10]/qbint#29	\mem_reg[10]/n30#3	VSS#43
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn10	\mem_reg[10]/SEb#12	scan_en#141	VSS#56	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[10]/Mmn55	mem[10]#31	\mem_reg[10]/qbint#11	VSS#40	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[0]/Mmn50	\mem_reg[0]/net436	\mem_reg[0]/qbint#7	VSS#367
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn12	\mem_reg[0]/Db#4	\mem_reg[0]/SEb#3
+ \mem_reg[0]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn11	\mem_reg[0]/net449	mem[0]#9	VSS#375
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn51	\mem_reg[0]/n30#11	\mem_reg[0]/CKb#11
+ \mem_reg[0]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn36	\mem_reg[0]/n20#11	\mem_reg[0]/CKbb#5
+ \mem_reg[0]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn25	\mem_reg[0]/n20#11	\mem_reg[0]/CKb#7
+ \mem_reg[0]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn35	\mem_reg[0]/net429	\mem_reg[0]/mout#3	VSS#371
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn13	\mem_reg[0]/net461	mem[13]#11	VSS#373
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn14	\mem_reg[0]/Db#4	scan_en#137
+ \mem_reg[0]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn40	\mem_reg[0]/n30#11	\mem_reg[0]/CKbb#7
+ \mem_reg[0]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn30	\mem_reg[0]/mout#11	\mem_reg[0]/n20#3	VSS#369
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn56	\mem_reg[0]/net367#11	\mem_reg[0]/qbint#9	VSS#365
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn21	\mem_reg[0]/CKbb#20	\mem_reg[0]/CKb#9	VSS#371
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn20	\mem_reg[0]/CKb#25	scan_clk#67	VSS#373
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn57	UNCONNECTED9#4	\mem_reg[0]/net367#3	VSS#363
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[0]/Mmn45	\mem_reg[0]/qbint#29	\mem_reg[0]/n30#3	VSS#365
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn10	\mem_reg[0]/SEb#12	scan_en#133	VSS#375
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[0]/Mmn55	mem[0]#31	\mem_reg[0]/qbint#11	VSS#363
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[2]/Mmn50	\mem_reg[2]/net436	\mem_reg[2]/qbint#7	VSS#366
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn12	\mem_reg[2]/Db	\mem_reg[2]/SEb#3
+ \mem_reg[2]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn11	\mem_reg[2]/net449	mem[2]#11	VSS#374
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn51	\mem_reg[2]/n30#8	\mem_reg[2]/CKb#11
+ \mem_reg[2]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn36	\mem_reg[2]/n20#8	\mem_reg[2]/CKbb#5
+ \mem_reg[2]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn25	\mem_reg[2]/n20#8	\mem_reg[2]/CKb#7
+ \mem_reg[2]/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn35	\mem_reg[2]/net429	\mem_reg[2]/mout#3	VSS#370
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn13	\mem_reg[2]/net461	mem[0]#11	VSS#372
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn14	\mem_reg[2]/Db	scan_en#139	\mem_reg[2]/net461
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn40	\mem_reg[2]/n30#8	\mem_reg[2]/CKbb#7
+ \mem_reg[2]/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn30	\mem_reg[2]/mout#8	\mem_reg[2]/n20#3	VSS#368
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn56	\mem_reg[2]/net367#8	\mem_reg[2]/qbint#9	VSS#364
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn21	\mem_reg[2]/CKbb#17	\mem_reg[2]/CKb#9	VSS#370
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn20	\mem_reg[2]/CKb#22	scan_clk#69	VSS#372
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn57	UNCONNECTED1	\mem_reg[2]/net367#3	VSS#362
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[2]/Mmn45	\mem_reg[2]/qbint#29	\mem_reg[2]/n30#3	VSS#364
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn10	\mem_reg[2]/SEb#9	scan_en#135	VSS#374
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[2]/Mmn55	mem[2]#28	\mem_reg[2]/qbint#11	VSS#362
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[15]/Mmn50	\mem_reg[15]/net436	\mem_reg[15]/qbint#7	VSS#277
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn12	\mem_reg[15]/Db	\mem_reg[15]/SEb#3
+ \mem_reg[15]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn11	\mem_reg[15]/net449	mem[15]#11	VSS#285
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn51	\mem_reg[15]/n30#8	\mem_reg[15]/CKb#11
+ \mem_reg[15]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn36	\mem_reg[15]/n20#8	\mem_reg[15]/CKbb#5
+ \mem_reg[15]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn25	\mem_reg[15]/n20#8	\mem_reg[15]/CKb#7
+ \mem_reg[15]/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn35	\mem_reg[15]/net429	\mem_reg[15]/mout#3	VSS#281
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn13	\mem_reg[15]/net461	mem[7]#11	VSS#283
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn14	\mem_reg[15]/Db	scan_en#129
+ \mem_reg[15]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn40	\mem_reg[15]/n30#8	\mem_reg[15]/CKbb#7
+ \mem_reg[15]/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn30	\mem_reg[15]/mout#8	\mem_reg[15]/n20#3	VSS#279
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn56	\mem_reg[15]/net367#8	\mem_reg[15]/qbint#9	VSS#275
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn21	\mem_reg[15]/CKbb#17	\mem_reg[15]/CKb#9	VSS#281
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn20	\mem_reg[15]/CKb#22	scan_clk#63	VSS#283
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn57	UNCONNECTED7	\mem_reg[15]/net367#3	VSS#273
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[15]/Mmn45	\mem_reg[15]/qbint#29	\mem_reg[15]/n30#3	VSS#275
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn10	\mem_reg[15]/SEb#9	scan_en#123	VSS#285
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[15]/Mmn55	mem[15]#28	\mem_reg[15]/qbint#11	VSS#273
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
mux_control_reg/Mmn50	mux_control_reg/net436	mux_control_reg/qbint#7
+ VSS#133	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn12	mux_control_reg/Db#4	mux_control_reg/SEb#3
+ mux_control_reg/net449	VSS	nfet_01v8	L=1.5e-07
+ W=4.25e-07
mux_control_reg/Mmn11	mux_control_reg/net449	FE_OFN1_scan_out_0#23	VSS#147
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn51	mux_control_reg/n30#11	mux_control_reg/CKb#11
+ mux_control_reg/net436	VSS	nfet_01v8	L=1.5e-07
+ W=4.25e-07
mux_control_reg/Mmn36	mux_control_reg/n20#11	mux_control_reg/CKbb#5
+ mux_control_reg/net429	VSS	nfet_01v8	L=1.5e-07
+ W=4.25e-07
mux_control_reg/Mmn25	mux_control_reg/n20#11	mux_control_reg/CKb#7
+ mux_control_reg/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn35	mux_control_reg/net429	mux_control_reg/mout#3	VSS#137
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn13	mux_control_reg/net461	mem[11]#11	VSS#143
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn14	mux_control_reg/Db#4	scan_en#131
+ mux_control_reg/net461	VSS	nfet_01v8	L=1.5e-07
+ W=4.25e-07
mux_control_reg/Mmn40	mux_control_reg/n30#11	mux_control_reg/CKbb#7
+ mux_control_reg/mout#11	VSS	nfet_01v8	L=1.5e-07
+ W=4.25e-07
mux_control_reg/Mmn30	mux_control_reg/mout#11	mux_control_reg/n20#3
+ VSS#136	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn56	mux_control_reg/net367#11	mux_control_reg/qbint#9
+ VSS#132	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn21	mux_control_reg/CKbb#20	mux_control_reg/CKb#9
+ VSS#137	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn20	mux_control_reg/CKb#25	scan_clk#65	VSS#143
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn57	UNCONNECTED2#4	mux_control_reg/net367#3	VSS#130
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
mux_control_reg/Mmn45	mux_control_reg/qbint#29	mux_control_reg/n30#3
+ VSS#132	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn10	mux_control_reg/SEb#12	scan_en#125	VSS#147
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
mux_control_reg/Mmn55	FE_OFN1_scan_out_0#58	mux_control_reg/qbint#11
+ VSS#130	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[8]/Mmn50	\mem_reg[8]/net436	\mem_reg[8]/qbint#7	VSS#278
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn12	\mem_reg[8]/Db#4	\mem_reg[8]/SEb#3
+ \mem_reg[8]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn11	\mem_reg[8]/net449	mem[8]#7	VSS#286
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn51	\mem_reg[8]/n30#11	\mem_reg[8]/CKb#11
+ \mem_reg[8]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn36	\mem_reg[8]/n20#11	\mem_reg[8]/CKbb#5
+ \mem_reg[8]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn25	\mem_reg[8]/n20#11	\mem_reg[8]/CKb#7
+ \mem_reg[8]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn35	\mem_reg[8]/net429	\mem_reg[8]/mout#3	VSS#282
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn13	\mem_reg[8]/net461	mem[2]#9	VSS#284
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn14	\mem_reg[8]/Db#4	scan_en#127
+ \mem_reg[8]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn40	\mem_reg[8]/n30#11	\mem_reg[8]/CKbb#7
+ \mem_reg[8]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn30	\mem_reg[8]/mout#11	\mem_reg[8]/n20#3	VSS#280
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn56	\mem_reg[8]/net367#11	\mem_reg[8]/qbint#9	VSS#276
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn21	\mem_reg[8]/CKbb#20	\mem_reg[8]/CKb#9	VSS#282
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn20	\mem_reg[8]/CKb#25	scan_clk#61	VSS#284
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn57	UNCONNECTED14#4	\mem_reg[8]/net367#3	VSS#274
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[8]/Mmn45	\mem_reg[8]/qbint#29	\mem_reg[8]/n30#3	VSS#276
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn10	\mem_reg[8]/SEb#12	scan_en#121	VSS#286
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[8]/Mmn55	mem[8]#29	\mem_reg[8]/qbint#11	VSS#274
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mq_reg_reg/Mmn50	q_reg_reg/net436	q_reg_reg/qbint#7	VSS#135
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn12	q_reg_reg/Db	q_reg_reg/SEb#3
+ q_reg_reg/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn11	q_reg_reg/net449	n_15#5	VSS#146	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn51	q_reg_reg/n30#8	q_reg_reg/CKb#11
+ q_reg_reg/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn36	q_reg_reg/n20#8	q_reg_reg/CKbb#5
+ q_reg_reg/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn25	q_reg_reg/n20#8	q_reg_reg/CKb#7
+ q_reg_reg/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn35	q_reg_reg/net429	q_reg_reg/mout#3	VSS#139
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn13	q_reg_reg/net461	x#5	VSS#142	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn14	q_reg_reg/Db	q_control#15	q_reg_reg/net461
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn40	q_reg_reg/n30#8	q_reg_reg/CKbb#7
+ q_reg_reg/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn30	q_reg_reg/mout#8	q_reg_reg/n20#3	VSS#138
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn56	q_reg_reg/net367#8	q_reg_reg/qbint#9	VSS#134
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn21	q_reg_reg/CKbb#17	q_reg_reg/CKb#9	VSS#139
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn20	q_reg_reg/CKb#22	clk#3	VSS#142	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn57	UNCONNECTED	q_reg_reg/net367#3	VSS#131
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mq_reg_reg/Mmn45	q_reg_reg/qbint#29	q_reg_reg/n30#3	VSS#134
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn10	q_reg_reg/SEb#9	q_control#13	VSS#146
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_reg_reg/Mmn55	FE_OFN0_q#32	q_reg_reg/qbint#11	VSS#131
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[9]/Mmn50	\mem_reg[9]/net436	\mem_reg[9]/qbint#7	VSS#59
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn12	\mem_reg[9]/Db	\mem_reg[9]/SEb#3
+ \mem_reg[9]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn11	\mem_reg[9]/net449	mem[9]#11	VSS#67	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn51	\mem_reg[9]/n30#8	\mem_reg[9]/CKb#11
+ \mem_reg[9]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn36	\mem_reg[9]/n20#8	\mem_reg[9]/CKbb#5
+ \mem_reg[9]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn25	\mem_reg[9]/n20#8	\mem_reg[9]/CKb#7
+ \mem_reg[9]/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn35	\mem_reg[9]/net429	\mem_reg[9]/mout#3	VSS#63
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn13	\mem_reg[9]/net461	mem[10]#7	VSS#66	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn14	\mem_reg[9]/Db	scan_en#119	\mem_reg[9]/net461
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn40	\mem_reg[9]/n30#8	\mem_reg[9]/CKbb#7
+ \mem_reg[9]/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn30	\mem_reg[9]/mout#8	\mem_reg[9]/n20#3	VSS#62
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn56	\mem_reg[9]/net367#8	\mem_reg[9]/qbint#9	VSS#58
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn21	\mem_reg[9]/CKbb#17	\mem_reg[9]/CKb#9	VSS#63
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn20	\mem_reg[9]/CKb#22	scan_clk#59	VSS#66	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn57	UNCONNECTED3	\mem_reg[9]/net367#3	VSS#57	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[9]/Mmn45	\mem_reg[9]/qbint#29	\mem_reg[9]/n30#3	VSS#58
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn10	\mem_reg[9]/SEb#9	scan_en#117	VSS#67	VSS
+ nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[9]/Mmn55	mem[9]#28	\mem_reg[9]/qbint#11	VSS#57	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[13]/Mmn50	\mem_reg[13]/net436	\mem_reg[13]/qbint#7	VSS#380
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn12	\mem_reg[13]/Db#4	\mem_reg[13]/SEb#3
+ \mem_reg[13]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn11	\mem_reg[13]/net449	mem[13]#7	VSS#388
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn51	\mem_reg[13]/n30#11	\mem_reg[13]/CKb#11
+ \mem_reg[13]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn36	\mem_reg[13]/n20#11	\mem_reg[13]/CKbb#5
+ \mem_reg[13]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn25	\mem_reg[13]/n20#11	\mem_reg[13]/CKb#7
+ \mem_reg[13]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn35	\mem_reg[13]/net429	\mem_reg[13]/mout#3	VSS#384
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn13	\mem_reg[13]/net461	mem[15]#7	VSS#386
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn14	\mem_reg[13]/Db#4	scan_en#115
+ \mem_reg[13]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn40	\mem_reg[13]/n30#11	\mem_reg[13]/CKbb#7
+ \mem_reg[13]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn30	\mem_reg[13]/mout#11	\mem_reg[13]/n20#3	VSS#382
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn56	\mem_reg[13]/net367#11	\mem_reg[13]/qbint#9	VSS#378
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn21	\mem_reg[13]/CKbb#20	\mem_reg[13]/CKb#9	VSS#384
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn20	\mem_reg[13]/CKb#25	scan_clk#57	VSS#386
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn57	UNCONNECTED5#4	\mem_reg[13]/net367#3	VSS#377
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[13]/Mmn45	\mem_reg[13]/qbint#29	\mem_reg[13]/n30#3	VSS#378
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn10	\mem_reg[13]/SEb#12	scan_en#113	VSS#388
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[13]/Mmn55	mem[13]#29	\mem_reg[13]/qbint#11	VSS#377
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[6]/Mmn50	\mem_reg[6]/net436	\mem_reg[6]/qbint#7	VSS#290
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn12	\mem_reg[6]/Db	\mem_reg[6]/SEb#3
+ \mem_reg[6]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn11	\mem_reg[6]/net449	mem[6]#9	VSS#299
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn51	\mem_reg[6]/n30#8	\mem_reg[6]/CKb#11
+ \mem_reg[6]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn36	\mem_reg[6]/n20#8	\mem_reg[6]/CKbb#5
+ \mem_reg[6]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn25	\mem_reg[6]/n20#8	\mem_reg[6]/CKb#7
+ \mem_reg[6]/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn35	\mem_reg[6]/net429	\mem_reg[6]/mout#3	VSS#295
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn13	\mem_reg[6]/net461	mem[4]#9	VSS#297
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn14	\mem_reg[6]/Db	scan_en#109	\mem_reg[6]/net461
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn40	\mem_reg[6]/n30#8	\mem_reg[6]/CKbb#7
+ \mem_reg[6]/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn30	\mem_reg[6]/mout#8	\mem_reg[6]/n20#3	VSS#293
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn56	\mem_reg[6]/net367#8	\mem_reg[6]/qbint#9	VSS#288
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn21	\mem_reg[6]/CKbb#17	\mem_reg[6]/CKb#9	VSS#295
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn20	\mem_reg[6]/CKb#22	scan_clk#53	VSS#297
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn57	UNCONNECTED12	\mem_reg[6]/net367#3	VSS#287
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[6]/Mmn45	\mem_reg[6]/qbint#29	\mem_reg[6]/n30#3	VSS#288
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn10	\mem_reg[6]/SEb#9	scan_en#105	VSS#299
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[6]/Mmn55	mem[6]#28	\mem_reg[6]/qbint#11	VSS#287
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[11]/Mmn50	\mem_reg[11]/net436	\mem_reg[11]/qbint#7	VSS#157
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn12	\mem_reg[11]/Db#4	\mem_reg[11]/SEb#3
+ \mem_reg[11]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn11	\mem_reg[11]/net449	mem[11]#7	VSS#177
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn51	\mem_reg[11]/n30#11	\mem_reg[11]/CKb#11
+ \mem_reg[11]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn36	\mem_reg[11]/n20#11	\mem_reg[11]/CKbb#5
+ \mem_reg[11]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn25	\mem_reg[11]/n20#11	\mem_reg[11]/CKb#7
+ \mem_reg[11]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn35	\mem_reg[11]/net429	\mem_reg[11]/mout#3	VSS#165
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn13	\mem_reg[11]/net461	mem[9]#7	VSS#171
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn14	\mem_reg[11]/Db#4	scan_en#111
+ \mem_reg[11]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn40	\mem_reg[11]/n30#11	\mem_reg[11]/CKbb#7
+ \mem_reg[11]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn30	\mem_reg[11]/mout#11	\mem_reg[11]/n20#3	VSS#164
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn56	\mem_reg[11]/net367#11	\mem_reg[11]/qbint#9	VSS#156
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn21	\mem_reg[11]/CKbb#20	\mem_reg[11]/CKb#9	VSS#165
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn20	\mem_reg[11]/CKb#25	scan_clk#55	VSS#171
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn57	UNCONNECTED16#4	\mem_reg[11]/net367#3	VSS#151
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[11]/Mmn45	\mem_reg[11]/qbint#29	\mem_reg[11]/n30#3	VSS#156
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn10	\mem_reg[11]/SEb#12	scan_en#107	VSS#177
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[11]/Mmn55	mem[11]#29	\mem_reg[11]/qbint#11	VSS#151
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mq_control_reg/Mmn50	q_control_reg/net436	q_control_reg/qbint#7	VSS#159
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn12	q_control_reg/Db	q_control_reg/SEb#3
+ q_control_reg/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn11	q_control_reg/net449	q_control#11	VSS#176
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn51	q_control_reg/n30#8	q_control_reg/CKb#11
+ q_control_reg/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn36	q_control_reg/n20#8	q_control_reg/CKbb#5
+ q_control_reg/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn25	q_control_reg/n20#8	q_control_reg/CKb#7
+ q_control_reg/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn35	q_control_reg/net429	q_control_reg/mout#3	VSS#167
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn13	q_control_reg/net461	scan_in[0]#3	VSS#170
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn14	q_control_reg/Db	scan_en#103
+ q_control_reg/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn40	q_control_reg/n30#8	q_control_reg/CKbb#7
+ q_control_reg/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn30	q_control_reg/mout#8	q_control_reg/n20#3	VSS#163
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn56	q_control_reg/net367#8	q_control_reg/qbint#9	VSS#155
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn21	q_control_reg/CKbb#17	q_control_reg/CKb#9	VSS#167
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn20	q_control_reg/CKb#22	scan_clk#51	VSS#170
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn57	UNCONNECTED11	q_control_reg/net367#3	VSS#150
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mq_control_reg/Mmn45	q_control_reg/qbint#29	q_control_reg/n30#3	VSS#155
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn10	q_control_reg/SEb#9	scan_en#101	VSS#176
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
Mq_control_reg/Mmn55	q_control#33	q_control_reg/qbint#11	VSS#150
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[5]/Mmn50	\mem_reg[5]/net436	\mem_reg[5]/qbint#7	VSS#393
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn12	\mem_reg[5]/Db	\mem_reg[5]/SEb#3
+ \mem_reg[5]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn11	\mem_reg[5]/net449	mem[5]#7	VSS#401
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn51	\mem_reg[5]/n30#8	\mem_reg[5]/CKb#11
+ \mem_reg[5]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn36	\mem_reg[5]/n20#8	\mem_reg[5]/CKbb#5
+ \mem_reg[5]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn25	\mem_reg[5]/n20#8	\mem_reg[5]/CKb#7
+ \mem_reg[5]/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn35	\mem_reg[5]/net429	\mem_reg[5]/mout#3	VSS#397
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn13	\mem_reg[5]/net461	mem[14]#9	VSS#399
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn14	\mem_reg[5]/Db	scan_en#99	\mem_reg[5]/net461
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn40	\mem_reg[5]/n30#8	\mem_reg[5]/CKbb#7
+ \mem_reg[5]/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn30	\mem_reg[5]/mout#8	\mem_reg[5]/n20#3	VSS#395
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn56	\mem_reg[5]/net367#8	\mem_reg[5]/qbint#9	VSS#391
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn21	\mem_reg[5]/CKbb#17	\mem_reg[5]/CKb#9	VSS#397
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn20	\mem_reg[5]/CKb#22	scan_clk#49	VSS#399
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn57	UNCONNECTED0	\mem_reg[5]/net367#3	VSS#389
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[5]/Mmn45	\mem_reg[5]/qbint#29	\mem_reg[5]/n30#3	VSS#391
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn10	\mem_reg[5]/SEb#9	scan_en#95	VSS#401
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[5]/Mmn55	mem[5]#26	\mem_reg[5]/qbint#11	VSS#389
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[7]/Mmn50	\mem_reg[7]/net436	\mem_reg[7]/qbint#7	VSS#394
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn12	\mem_reg[7]/Db#4	\mem_reg[7]/SEb#3
+ \mem_reg[7]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn11	\mem_reg[7]/net449	mem[7]#7	VSS#402
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn51	\mem_reg[7]/n30#11	\mem_reg[7]/CKb#11
+ \mem_reg[7]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn36	\mem_reg[7]/n20#11	\mem_reg[7]/CKbb#5
+ \mem_reg[7]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn25	\mem_reg[7]/n20#11	\mem_reg[7]/CKb#7
+ \mem_reg[7]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn35	\mem_reg[7]/net429	\mem_reg[7]/mout#3	VSS#398
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn13	\mem_reg[7]/net461	mem[5]#9	VSS#400
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn14	\mem_reg[7]/Db#4	scan_en#97
+ \mem_reg[7]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn40	\mem_reg[7]/n30#11	\mem_reg[7]/CKbb#7
+ \mem_reg[7]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn30	\mem_reg[7]/mout#11	\mem_reg[7]/n20#3	VSS#396
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn56	\mem_reg[7]/net367#11	\mem_reg[7]/qbint#9	VSS#392
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn21	\mem_reg[7]/CKbb#20	\mem_reg[7]/CKb#9	VSS#398
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn20	\mem_reg[7]/CKb#25	scan_clk#47	VSS#400
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn57	UNCONNECTED13#4	\mem_reg[7]/net367#3	VSS#390
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[7]/Mmn45	\mem_reg[7]/qbint#29	\mem_reg[7]/n30#3	VSS#392
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn10	\mem_reg[7]/SEb#12	scan_en#93	VSS#402
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[7]/Mmn55	mem[7]#27	\mem_reg[7]/qbint#11	VSS#390
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[14]/Mmn50	\mem_reg[14]/net436	\mem_reg[14]/qbint#7	VSS#305
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn12	\mem_reg[14]/Db#4	\mem_reg[14]/SEb#3
+ \mem_reg[14]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn11	\mem_reg[14]/net449	mem[14]#7	VSS#313
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn51	\mem_reg[14]/n30#11	\mem_reg[14]/CKb#11
+ \mem_reg[14]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn36	\mem_reg[14]/n20#11	\mem_reg[14]/CKbb#5
+ \mem_reg[14]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn25	\mem_reg[14]/n20#11	\mem_reg[14]/CKb#7
+ \mem_reg[14]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn35	\mem_reg[14]/net429	\mem_reg[14]/mout#3	VSS#309
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn13	\mem_reg[14]/net461	mem[12]#9	VSS#311
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn14	\mem_reg[14]/Db#4	scan_en#91
+ \mem_reg[14]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn40	\mem_reg[14]/n30#11	\mem_reg[14]/CKbb#7
+ \mem_reg[14]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn30	\mem_reg[14]/mout#11	\mem_reg[14]/n20#3	VSS#307
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn56	\mem_reg[14]/net367#11	\mem_reg[14]/qbint#9	VSS#303
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn21	\mem_reg[14]/CKbb#20	\mem_reg[14]/CKb#9	VSS#309
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn20	\mem_reg[14]/CKb#25	scan_clk#45	VSS#311
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn57	UNCONNECTED6#4	\mem_reg[14]/net367#3	VSS#301
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[14]/Mmn45	\mem_reg[14]/qbint#29	\mem_reg[14]/n30#3	VSS#303
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn10	\mem_reg[14]/SEb#12	scan_en#81	VSS#313
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[14]/Mmn55	mem[14]#29	\mem_reg[14]/qbint#11	VSS#301
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[12]/Mmn50	\mem_reg[12]/net436	\mem_reg[12]/qbint#7	VSS#306
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn12	\mem_reg[12]/Db	\mem_reg[12]/SEb#3
+ \mem_reg[12]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn11	\mem_reg[12]/net449	mem[12]#7	VSS#314
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn51	\mem_reg[12]/n30#8	\mem_reg[12]/CKb#11
+ \mem_reg[12]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn36	\mem_reg[12]/n20#8	\mem_reg[12]/CKbb#5
+ \mem_reg[12]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn25	\mem_reg[12]/n20#8	\mem_reg[12]/CKb#7
+ \mem_reg[12]/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn35	\mem_reg[12]/net429	\mem_reg[12]/mout#3	VSS#310
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn13	\mem_reg[12]/net461	mem[6]#7	VSS#312
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn14	\mem_reg[12]/Db	scan_en#83
+ \mem_reg[12]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn40	\mem_reg[12]/n30#8	\mem_reg[12]/CKbb#7
+ \mem_reg[12]/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn30	\mem_reg[12]/mout#8	\mem_reg[12]/n20#3	VSS#308
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn56	\mem_reg[12]/net367#8	\mem_reg[12]/qbint#9	VSS#304
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn21	\mem_reg[12]/CKbb#17	\mem_reg[12]/CKb#9	VSS#310
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn20	\mem_reg[12]/CKb#22	scan_clk#37	VSS#312
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn57	UNCONNECTED4	\mem_reg[12]/net367#3	VSS#302
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[12]/Mmn45	\mem_reg[12]/qbint#29	\mem_reg[12]/n30#3	VSS#304
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn10	\mem_reg[12]/SEb#9	scan_en#73	VSS#314
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[12]/Mmn55	mem[12]#26	\mem_reg[12]/qbint#11	VSS#302
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[4]/Mmn50	\mem_reg[4]/net436	\mem_reg[4]/qbint#7	VSS#94
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn12	\mem_reg[4]/Db#4	\mem_reg[4]/SEb#3
+ \mem_reg[4]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn11	\mem_reg[4]/net449	mem[4]#7	VSS#107
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn51	\mem_reg[4]/n30#11	\mem_reg[4]/CKb#11
+ \mem_reg[4]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn36	\mem_reg[4]/n20#11	\mem_reg[4]/CKbb#5
+ \mem_reg[4]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn25	\mem_reg[4]/n20#11	\mem_reg[4]/CKb#7
+ \mem_reg[4]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn35	\mem_reg[4]/net429	\mem_reg[4]/mout#3	VSS#102
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn13	\mem_reg[4]/net461	mem[1]#9	VSS#105
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn14	\mem_reg[4]/Db#4	scan_en#85
+ \mem_reg[4]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn40	\mem_reg[4]/n30#11	\mem_reg[4]/CKbb#7
+ \mem_reg[4]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn30	\mem_reg[4]/mout#11	\mem_reg[4]/n20#3	VSS#97
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn56	\mem_reg[4]/net367#11	\mem_reg[4]/qbint#9	VSS#91
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn21	\mem_reg[4]/CKbb#20	\mem_reg[4]/CKb#9	VSS#102
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn20	\mem_reg[4]/CKb#25	scan_clk#39	VSS#105
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn57	UNCONNECTED17#4	\mem_reg[4]/net367#3	VSS#88
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[4]/Mmn45	\mem_reg[4]/qbint#29	\mem_reg[4]/n30#3	VSS#91
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn10	\mem_reg[4]/SEb#12	scan_en#75	VSS#107
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[4]/Mmn55	mem[4]#27	\mem_reg[4]/qbint#11	VSS#88	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[1]/Mmn50	\mem_reg[1]/net436	\mem_reg[1]/qbint#7	VSS#90
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn12	\mem_reg[1]/Db	\mem_reg[1]/SEb#3
+ \mem_reg[1]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn11	\mem_reg[1]/net449	mem[1]#7	VSS#104
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn51	\mem_reg[1]/n30#8	\mem_reg[1]/CKb#11
+ \mem_reg[1]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn36	\mem_reg[1]/n20#8	\mem_reg[1]/CKbb#5
+ \mem_reg[1]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn25	\mem_reg[1]/n20#8	\mem_reg[1]/CKb#7
+ \mem_reg[1]/Db#5	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn35	\mem_reg[1]/net429	\mem_reg[1]/mout#3	VSS#96
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn13	\mem_reg[1]/net461	mem[3]#9	VSS#101
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn14	\mem_reg[1]/Db	scan_en#87	\mem_reg[1]/net461
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn40	\mem_reg[1]/n30#8	\mem_reg[1]/CKbb#7
+ \mem_reg[1]/mout#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn30	\mem_reg[1]/mout#8	\mem_reg[1]/n20#3	VSS#93
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn56	\mem_reg[1]/net367#8	\mem_reg[1]/qbint#9	VSS#87
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn21	\mem_reg[1]/CKbb#17	\mem_reg[1]/CKb#9	VSS#96
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn20	\mem_reg[1]/CKb#22	scan_clk#41	VSS#101
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn57	UNCONNECTED10	\mem_reg[1]/net367#3	VSS#82	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[1]/Mmn45	\mem_reg[1]/qbint#29	\mem_reg[1]/n30#3	VSS#87
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn10	\mem_reg[1]/SEb#9	scan_en#77	VSS#104
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[1]/Mmn55	mem[1]#26	\mem_reg[1]/qbint#11	VSS#82	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[3]/Mmn50	\mem_reg[3]/net436	\mem_reg[3]/qbint#7	VSS#189
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn12	\mem_reg[3]/Db#4	\mem_reg[3]/SEb#3
+ \mem_reg[3]/net449	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn11	\mem_reg[3]/net449	mem[3]#7	VSS#213
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn51	\mem_reg[3]/n30#11	\mem_reg[3]/CKb#11
+ \mem_reg[3]/net436	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn36	\mem_reg[3]/n20#11	\mem_reg[3]/CKbb#5
+ \mem_reg[3]/net429	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn25	\mem_reg[3]/n20#11	\mem_reg[3]/CKb#7
+ \mem_reg[3]/Db#8	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn35	\mem_reg[3]/net429	\mem_reg[3]/mout#3	VSS#199
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn13	\mem_reg[3]/net461	q_control#9	VSS#208
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn14	\mem_reg[3]/Db#4	scan_en#89
+ \mem_reg[3]/net461	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn40	\mem_reg[3]/n30#11	\mem_reg[3]/CKbb#7
+ \mem_reg[3]/mout#11	VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn30	\mem_reg[3]/mout#11	\mem_reg[3]/n20#3	VSS#195
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn56	\mem_reg[3]/net367#11	\mem_reg[3]/qbint#9	VSS#185
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn21	\mem_reg[3]/CKbb#20	\mem_reg[3]/CKb#9	VSS#199
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn20	\mem_reg[3]/CKb#25	scan_clk#43	VSS#208
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn57	UNCONNECTED8#4	\mem_reg[3]/net367#3	VSS#181
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
M\mem_reg[3]/Mmn45	\mem_reg[3]/qbint#29	\mem_reg[3]/n30#3	VSS#185
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn10	\mem_reg[3]/SEb#12	scan_en#79	VSS#213
+ VSS	nfet_01v8	L=1.5e-07	W=4.25e-07
M\mem_reg[3]/Mmn55	mem[3]#29	\mem_reg[3]/qbint#11	VSS#181
+ VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn0@3	VSS#55	FE_OFN0_q#9	FE_OFC0_q/n0#114	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn0	VSS#53	FE_OFN0_q#11	FE_OFC0_q/n0#114	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn0@2	VSS#53	FE_OFN0_q#13	FE_OFC0_q/n0#118	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn0@1	VSS#52	FE_OFN0_q#15	FE_OFC0_q/n0#118	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@1	VSS#52	FE_OFC0_q/n0#33	q#1	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@3	VSS#51	FE_OFC0_q/n0#35	q#1	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@4	VSS#51	FE_OFC0_q/n0#37	q#5	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@15	VSS#49	FE_OFC0_q/n0#39	q#5	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@9	VSS#49	FE_OFC0_q/n0#41	q#9	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@6	VSS#47	FE_OFC0_q/n0#43	q#9	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@8	VSS#47	FE_OFC0_q/n0#45	q#13	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@5	VSS#46	FE_OFC0_q/n0#47	q#13	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@2	VSS#46	FE_OFC0_q/n0#49	q#17	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@7	VSS#44	FE_OFC0_q/n0#51	q#17	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@11	VSS#44	FE_OFC0_q/n0#53	q#21	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1	VSS#42	FE_OFC0_q/n0#55	q#21	VSS	nfet_01v8
+ L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@10	VSS#42	FE_OFC0_q/n0#57	q#25	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@14	VSS#41	FE_OFC0_q/n0#59	q#25	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@12	VSS#41	FE_OFC0_q/n0#61	q#29	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC0_q/Mmn1@13	VSS#1	FE_OFC0_q/n0#63	q#29	VSS
+ nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn0@3	VSS#212	FE_OFN1_scan_out_0#15
+ FE_OFC1_scan_out_0/n0#114	VSS	nfet_01v8	L=1.5e-07
+ W=7.6e-07
MFE_OFC1_scan_out_0/Mmn0	VSS#210	FE_OFN1_scan_out_0#17
+ FE_OFC1_scan_out_0/n0#114	VSS	nfet_01v8	L=1.5e-07
+ W=7.6e-07
MFE_OFC1_scan_out_0/Mmn0@2	VSS#210	FE_OFN1_scan_out_0#19
+ FE_OFC1_scan_out_0/n0#118	VSS	nfet_01v8	L=1.5e-07
+ W=7.6e-07
MFE_OFC1_scan_out_0/Mmn0@1	VSS#207	FE_OFN1_scan_out_0#21
+ FE_OFC1_scan_out_0/n0#118	VSS	nfet_01v8	L=1.5e-07
+ W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@1	VSS#207	FE_OFC1_scan_out_0/n0#33
+ scan_out[0]#1	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@3	VSS#204	FE_OFC1_scan_out_0/n0#35
+ scan_out[0]#1	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@4	VSS#204	FE_OFC1_scan_out_0/n0#37
+ scan_out[0]#5	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@15	VSS#202	FE_OFC1_scan_out_0/n0#39
+ scan_out[0]#5	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@9	VSS#202	FE_OFC1_scan_out_0/n0#41
+ scan_out[0]#9	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@6	VSS#198	FE_OFC1_scan_out_0/n0#43
+ scan_out[0]#9	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@8	VSS#198	FE_OFC1_scan_out_0/n0#45
+ scan_out[0]#13	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@5	VSS#194	FE_OFC1_scan_out_0/n0#47
+ scan_out[0]#13	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@2	VSS#194	FE_OFC1_scan_out_0/n0#49
+ scan_out[0]#17	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@7	VSS#191	FE_OFC1_scan_out_0/n0#51
+ scan_out[0]#17	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@11	VSS#191	FE_OFC1_scan_out_0/n0#53
+ scan_out[0]#21	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1	VSS#188	FE_OFC1_scan_out_0/n0#55
+ scan_out[0]#21	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@10	VSS#188	FE_OFC1_scan_out_0/n0#57
+ scan_out[0]#25	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@14	VSS#184	FE_OFC1_scan_out_0/n0#59
+ scan_out[0]#25	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@12	VSS#184	FE_OFC1_scan_out_0/n0#61
+ scan_out[0]#29	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
MFE_OFC1_scan_out_0/Mmn1@13	VSS#180	FE_OFC1_scan_out_0/n0#63
+ scan_out[0]#29	VSS	nfet_01v8	L=1.5e-07	W=7.6e-07
Mg526/Mmp0	n_12#29	addr[2]#5	VDD#57	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
Mg527/Mmp0	n_2#29	addr[1]#15	VDD#245	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
Mg525__6131/Mmp0	g525__6131/net41	addr[0]#7	VDD#314
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg525__6131/Mmp1	n_8#36	addr[1]#9	g525__6131/net41	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg524__7098/Mmp0	g524__7098/net41	addr[0]#1	VDD#88	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg524__7098/Mmp1	n_7#31	n_2#3	g524__7098/net41	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg523__8246/Mmp1	n_11#16	n_12#3	VDD#241	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg523__8246/Mmp0	n_11#16	addr[0]#9	VDD#242	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg510__3680/Mmp1	g510__3680/net89#3	mem[8]#5	VDD#224
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg510__3680/Mmp2	n_5#13	n_7#7	g510__3680/net89#5	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg510__3680/Mmp3	n_5#13	mem[10]#5	g510__3680/net89#3	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg510__3680/Mmp0	g510__3680/net89	n_8#7	VDD#224	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg509__6783/Mmp1	g509__6783/net89#3	mem[0]	VDD#304	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg509__6783/Mmp2	n_6#13	n_7#5	g509__6783/net89#5	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg509__6783/Mmp3	n_6#13	mem[2]	g509__6783/net89#3	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg509__6783/Mmp0	g509__6783/net89	n_8#5	VDD#304	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg522__2802/Mmp1	g522__2802/net89#3	mem[9]#3	VDD#85	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg522__2802/Mmp2	n_3#11	addr[1]#5	g522__2802/net89#5	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg522__2802/Mmp3	n_3#11	mem[11]#3	g522__2802/net89#3	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg522__2802/Mmp0	g522__2802/net89	n_2#5	VDD#85	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg521__1617/Mmp1	g521__1617/net89#4	mem[1]#5	VDD#250
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg521__1617/Mmp2	n_4#10	addr[1]#1	g521__1617/net89#6	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg521__1617/Mmp3	n_4#10	mem[3]#5	g521__1617/net89#4	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg521__1617/Mmp0	g521__1617/net89#2	n_2	VDD#250	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg497__8428/Mmp3	g497__8428/net118#2	n_0	g497__8428/net122#4
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg497__8428/Mmp5	n_10#10	mem[6]#5	g497__8428/net118#6
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg497__8428/Mmp1	g497__8428/net122#4	mem[4]#5	VDD#248
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg497__8428/Mmp4	n_10#10	n_7#3	g497__8428/net118#4	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg497__8428/Mmp2	g497__8428/net118#2	addr[0]#5
+ g497__8428/net122#6	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg497__8428/Mmp0	g497__8428/net122#2	n_8#3	VDD#248	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg498__5526/Mmp3	g498__5526/net118#2	n_1	g498__5526/net122#4
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg498__5526/Mmp5	n_9#10	mem[14]#5	g498__5526/net118#6	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg498__5526/Mmp1	g498__5526/net122#4	mem[12]#5	VDD#320
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg498__5526/Mmp4	n_9#10	n_7	g498__5526/net118#4	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg498__5526/Mmp2	g498__5526/net118#2	addr[0]#3
+ g498__5526/net122#6	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mg498__5526/Mmp0	g498__5526/net122#2	n_8	VDD#320	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg495__6260/Mmp0	g495__6260/n2	addr[2]#3	VDD#239	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg495__6260/Mmp3	n_14#11	n_9	g495__6260/n3	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg495__6260/Mmp5	n_14#11	n_3	g495__6260/n4	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg495__6260/Mmp4	g495__6260/n4	n_11#3	VDD#235	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg495__6260/Mmp2	g495__6260/n3	n_12#5	VDD#237	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg495__6260/Mmp1	n_14#9	n_5	g495__6260/n2	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
Mg496__4319/Mmp0	g496__4319/n2	addr[2]#1	VDD#312	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg496__4319/Mmp3	n_13#11	n_10	g496__4319/n3	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg496__4319/Mmp5	n_13#11	n_4	g496__4319/n4	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg496__4319/Mmp4	g496__4319/n4	n_11	VDD#303	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg496__4319/Mmp2	g496__4319/n3	n_12	VDD#307	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg496__4319/Mmp1	n_13#9	n_6	g496__4319/n2	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
Mg494__5107/Mmp4	g494__5107/n0#9	g494__5107/S0b	g494__5107/n6
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg494__5107/Mmp3	g494__5107/n6	n_14	VDD#273	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mg494__5107/Mmp5	n_15#20	g494__5107/n0	VDD#273	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg494__5107/Mmp1	g494__5107/n4	n_13	VDD#275	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mg494__5107/Mmp2	g494__5107/n0#9	addr[3]#3	g494__5107/n4
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg494__5107/Mmp0	g494__5107/S0b#9	addr[3]#1	VDD#275
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg519__1705/Mmp4	g519__1705/n0#11	g519__1705/S0b	g519__1705/n6
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg519__1705/Mmp3	g519__1705/n6	mem[15]#3	VDD#308	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mg519__1705/Mmp5	n_1#12	g519__1705/n0	VDD#308	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg519__1705/Mmp1	g519__1705/n4	mem[13]#3	VDD#315	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mg519__1705/Mmp2	g519__1705/n0#11	addr[1]#13	g519__1705/n4
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg519__1705/Mmp0	g519__1705/S0b#11	addr[1]#11	VDD#315
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg520__5122/Mmp4	g520__5122/n0#11	g520__5122/S0b	g520__5122/n6
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg520__5122/Mmp3	g520__5122/n6	mem[7]#3	VDD#321	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mg520__5122/Mmp5	n_0#12	g520__5122/n0	VDD#321	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg520__5122/Mmp1	g520__5122/n4	mem[5]#5	VDD#322	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mg520__5122/Mmp2	g520__5122/n0#11	addr[1]#7	g520__5122/n4
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg520__5122/Mmp0	g520__5122/S0b#11	addr[1]#3	VDD#322
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg493__2398/Mmp5	mux#3	g493__2398/n0	VDD#136	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg493__2398/Mmp5@1	mux#3	g493__2398/n0#3	VDD#135	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg493__2398/Mmp5@2	mux#7	g493__2398/n0#5	VDD#135	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg493__2398/Mmp5@3	mux#7	g493__2398/n0#7	VDD#103	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mg493__2398/Mmp1	g493__2398/n4	n_15#3	VDD#137	VDD
+ pfet_01v8	L=1.5e-07	W=9.05e-07
Mg493__2398/Mmp4	g493__2398/n0#32	g493__2398/S0b	g493__2398/n6
+ VDD	pfet_01v8	L=1.5e-07	W=9.05e-07
Mg493__2398/Mmp0	g493__2398/S0b#12	FE_OFN1_scan_out_0#11	VDD#137
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mg493__2398/Mmp2	g493__2398/n0#32	FE_OFN1_scan_out_0#13
+ g493__2398/n4	VDD	pfet_01v8	L=1.5e-07	W=9.05e-07
Mg493__2398/Mmp3	g493__2398/n6	x#3	VDD#136	VDD
+ pfet_01v8	L=1.5e-07	W=9.05e-07
M\mem_reg[10]/Mmp50	\mem_reg[10]/net376	\mem_reg[10]/qbint	VDD#225
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp30	\mem_reg[10]/mout#8	\mem_reg[10]/n20	VDD#227
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp14	\mem_reg[10]/Db	\mem_reg[10]/SEb
+ \mem_reg[10]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp51	\mem_reg[10]/n30#8	\mem_reg[10]/CKbb#3
+ \mem_reg[10]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp36	\mem_reg[10]/n20#8	\mem_reg[10]/CKb
+ \mem_reg[10]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp12	\mem_reg[10]/Db	scan_en#71
+ \mem_reg[10]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp11	\mem_reg[10]/net384	mem[10]#3	VDD#234
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp35	\mem_reg[10]/net356	\mem_reg[10]/mout	VDD#229
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp13	\mem_reg[10]/net392	mem[8]#3	VDD#231
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp40	\mem_reg[10]/n30#8	\mem_reg[10]/CKb#5
+ \mem_reg[10]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp45	\mem_reg[10]/qbint#22	\mem_reg[10]/n30	VDD#223
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp10	\mem_reg[10]/SEb#9	scan_en#69	VDD#234
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp56	\mem_reg[10]/net367#8	\mem_reg[10]/qbint#3	VDD#223
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp21	\mem_reg[10]/CKbb#17	\mem_reg[10]/CKb#3	VDD#229
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp20	\mem_reg[10]/CKb#22	scan_clk#35	VDD#231
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp57	UNCONNECTED15	\mem_reg[10]/net367	VDD#222
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[10]/Mmp25	\mem_reg[10]/n20#8	\mem_reg[10]/CKbb
+ \mem_reg[10]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[10]/Mmp55	mem[10]#28	\mem_reg[10]/qbint#5	VDD#222
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[0]/Mmp50	\mem_reg[0]/net376	\mem_reg[0]/qbint	VDD#392
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp30	\mem_reg[0]/mout#8	\mem_reg[0]/n20	VDD#393
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp14	\mem_reg[0]/Db	\mem_reg[0]/SEb
+ \mem_reg[0]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp51	\mem_reg[0]/n30#8	\mem_reg[0]/CKbb#3
+ \mem_reg[0]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp36	\mem_reg[0]/n20#8	\mem_reg[0]/CKb
+ \mem_reg[0]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp12	\mem_reg[0]/Db	scan_en#65	\mem_reg[0]/net384
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp11	\mem_reg[0]/net384	mem[0]#3	VDD#396
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp35	\mem_reg[0]/net356	\mem_reg[0]/mout	VDD#394
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp13	\mem_reg[0]/net392	mem[13]#5	VDD#395
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp40	\mem_reg[0]/n30#8	\mem_reg[0]/CKb#5
+ \mem_reg[0]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp45	\mem_reg[0]/qbint#22	\mem_reg[0]/n30	VDD#391
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp10	\mem_reg[0]/SEb#9	scan_en#61	VDD#396
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp56	\mem_reg[0]/net367#8	\mem_reg[0]/qbint#3	VDD#391
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp21	\mem_reg[0]/CKbb#17	\mem_reg[0]/CKb#3	VDD#394
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp20	\mem_reg[0]/CKb#22	scan_clk#31	VDD#395
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp57	UNCONNECTED9	\mem_reg[0]/net367	VDD#390
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[0]/Mmp25	\mem_reg[0]/n20#8	\mem_reg[0]/CKbb
+ \mem_reg[0]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[0]/Mmp55	mem[0]#28	\mem_reg[0]/qbint#5	VDD#390
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[2]/Mmp50	\mem_reg[2]/net376	\mem_reg[2]/qbint	VDD#281
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp30	\mem_reg[2]/mout#11	\mem_reg[2]/n20	VDD#285
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp14	\mem_reg[2]/Db#4	\mem_reg[2]/SEb
+ \mem_reg[2]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp51	\mem_reg[2]/n30#11	\mem_reg[2]/CKbb#3
+ \mem_reg[2]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp36	\mem_reg[2]/n20#11	\mem_reg[2]/CKb
+ \mem_reg[2]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp12	\mem_reg[2]/Db#4	scan_en#67
+ \mem_reg[2]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp11	\mem_reg[2]/net384	mem[2]#5	VDD#298
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp35	\mem_reg[2]/net356	\mem_reg[2]/mout	VDD#289
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp13	\mem_reg[2]/net392	mem[0]#5	VDD#297
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp40	\mem_reg[2]/n30#11	\mem_reg[2]/CKb#5
+ \mem_reg[2]/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp45	\mem_reg[2]/qbint#23	\mem_reg[2]/n30	VDD#278
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp10	\mem_reg[2]/SEb#12	scan_en#63	VDD#298
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp56	\mem_reg[2]/net367#11	\mem_reg[2]/qbint#3	VDD#278
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp21	\mem_reg[2]/CKbb#20	\mem_reg[2]/CKb#3	VDD#289
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp20	\mem_reg[2]/CKb#25	scan_clk#33	VDD#297
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp57	UNCONNECTED1#4	\mem_reg[2]/net367	VDD#274
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[2]/Mmp25	\mem_reg[2]/n20#11	\mem_reg[2]/CKbb
+ \mem_reg[2]/Db#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[2]/Mmp55	mem[2]#31	\mem_reg[2]/qbint#5	VDD#274
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[15]/Mmp50	\mem_reg[15]/net376	\mem_reg[15]/qbint	VDD#230
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp30	\mem_reg[15]/mout#11	\mem_reg[15]/n20	VDD#232
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp14	\mem_reg[15]/Db#4	\mem_reg[15]/SEb
+ \mem_reg[15]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp51	\mem_reg[15]/n30#11	\mem_reg[15]/CKbb#3
+ \mem_reg[15]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp36	\mem_reg[15]/n20#11	\mem_reg[15]/CKb
+ \mem_reg[15]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp12	\mem_reg[15]/Db#4	scan_en#57
+ \mem_reg[15]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp11	\mem_reg[15]/net384	mem[15]#5	VDD#238
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp35	\mem_reg[15]/net356	\mem_reg[15]/mout	VDD#233
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp13	\mem_reg[15]/net392	mem[7]#5	VDD#236
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp40	\mem_reg[15]/n30#11	\mem_reg[15]/CKb#5
+ \mem_reg[15]/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp45	\mem_reg[15]/qbint#23	\mem_reg[15]/n30	VDD#228
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp10	\mem_reg[15]/SEb#12	scan_en#51	VDD#238
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp56	\mem_reg[15]/net367#11	\mem_reg[15]/qbint#3	VDD#228
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp21	\mem_reg[15]/CKbb#20	\mem_reg[15]/CKb#3	VDD#233
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp20	\mem_reg[15]/CKb#25	scan_clk#27	VDD#236
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp57	UNCONNECTED7#4	\mem_reg[15]/net367	VDD#226
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[15]/Mmp25	\mem_reg[15]/n20#11	\mem_reg[15]/CKbb
+ \mem_reg[15]/Db#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[15]/Mmp55	mem[15]#31	\mem_reg[15]/qbint#5	VDD#226
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
mux_control_reg/Mmp50	mux_control_reg/net376	mux_control_reg/qbint	VDD#65
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp30	mux_control_reg/mout#8	mux_control_reg/n20	VDD#68
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp14	mux_control_reg/Db	mux_control_reg/SEb
+ mux_control_reg/net392	VDD	pfet_01v8	L=1.5e-07
+ W=6.25e-07
mux_control_reg/Mmp51	mux_control_reg/n30#8	mux_control_reg/CKbb#3
+ mux_control_reg/net376	VDD	pfet_01v8	L=1.5e-07
+ W=6.25e-07
mux_control_reg/Mmp36	mux_control_reg/n20#8	mux_control_reg/CKb
+ mux_control_reg/net356	VDD	pfet_01v8	L=1.5e-07
+ W=6.25e-07
mux_control_reg/Mmp12	mux_control_reg/Db	scan_en#59
+ mux_control_reg/net384	VDD	pfet_01v8	L=1.5e-07
+ W=6.25e-07
mux_control_reg/Mmp11	mux_control_reg/net384	FE_OFN1_scan_out_0#9	VDD#75
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp35	mux_control_reg/net356	mux_control_reg/mout	VDD#70
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp13	mux_control_reg/net392	mem[11]#5	VDD#72	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp40	mux_control_reg/n30#8	mux_control_reg/CKb#5
+ mux_control_reg/mout#8	VDD	pfet_01v8	L=1.5e-07
+ W=6.25e-07
mux_control_reg/Mmp45	mux_control_reg/qbint#22	mux_control_reg/n30
+ VDD#62	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp10	mux_control_reg/SEb#9	scan_en#53	VDD#75	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp56	mux_control_reg/net367#8	mux_control_reg/qbint#3
+ VDD#62	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp21	mux_control_reg/CKbb#17	mux_control_reg/CKb#3
+ VDD#70	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp20	mux_control_reg/CKb#22	scan_clk#29	VDD#72	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp57	UNCONNECTED2	mux_control_reg/net367	VDD#60	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
mux_control_reg/Mmp25	mux_control_reg/n20#8	mux_control_reg/CKbb
+ mux_control_reg/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
mux_control_reg/Mmp55	FE_OFN1_scan_out_0#55	mux_control_reg/qbint#5
+ VDD#60	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[8]/Mmp50	\mem_reg[8]/net376	\mem_reg[8]/qbint	VDD#284
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp30	\mem_reg[8]/mout#8	\mem_reg[8]/n20	VDD#288
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp14	\mem_reg[8]/Db	\mem_reg[8]/SEb
+ \mem_reg[8]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp51	\mem_reg[8]/n30#8	\mem_reg[8]/CKbb#3
+ \mem_reg[8]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp36	\mem_reg[8]/n20#8	\mem_reg[8]/CKb
+ \mem_reg[8]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp12	\mem_reg[8]/Db	scan_en#55	\mem_reg[8]/net384
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp11	\mem_reg[8]/net384	mem[8]	VDD#300	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp35	\mem_reg[8]/net356	\mem_reg[8]/mout	VDD#291
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp13	\mem_reg[8]/net392	mem[2]#3	VDD#296
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp40	\mem_reg[8]/n30#8	\mem_reg[8]/CKb#5
+ \mem_reg[8]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp45	\mem_reg[8]/qbint#22	\mem_reg[8]/n30	VDD#280
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp10	\mem_reg[8]/SEb#9	scan_en#49	VDD#300
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp56	\mem_reg[8]/net367#8	\mem_reg[8]/qbint#3	VDD#280
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp21	\mem_reg[8]/CKbb#17	\mem_reg[8]/CKb#3	VDD#291
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp20	\mem_reg[8]/CKb#22	scan_clk#25	VDD#296
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp57	UNCONNECTED14	\mem_reg[8]/net367	VDD#277
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[8]/Mmp25	\mem_reg[8]/n20#8	\mem_reg[8]/CKbb
+ \mem_reg[8]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[8]/Mmp55	mem[8]#26	\mem_reg[8]/qbint#5	VDD#277
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mq_reg_reg/Mmp50	q_reg_reg/net376	q_reg_reg/qbint	VDD#140
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp30	q_reg_reg/mout#11	q_reg_reg/n20	VDD#141
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp14	q_reg_reg/Db#4	q_reg_reg/SEb	q_reg_reg/net392
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp51	q_reg_reg/n30#11	q_reg_reg/CKbb#3
+ q_reg_reg/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp36	q_reg_reg/n20#11	q_reg_reg/CKb
+ q_reg_reg/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp12	q_reg_reg/Db#4	q_control#7	q_reg_reg/net384
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp11	q_reg_reg/net384	n_15	VDD#144	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp35	q_reg_reg/net356	q_reg_reg/mout	VDD#142
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp13	q_reg_reg/net392	x#1	VDD#143	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp40	q_reg_reg/n30#11	q_reg_reg/CKb#5
+ q_reg_reg/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp45	q_reg_reg/qbint#23	q_reg_reg/n30	VDD#139
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp10	q_reg_reg/SEb#12	q_control#5	VDD#144
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp56	q_reg_reg/net367#11	q_reg_reg/qbint#3	VDD#139
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp21	q_reg_reg/CKbb#20	q_reg_reg/CKb#3	VDD#142
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp20	q_reg_reg/CKb#25	clk#1	VDD#143	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp57	UNCONNECTED#4	q_reg_reg/net367	VDD#138
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mq_reg_reg/Mmp25	q_reg_reg/n20#11	q_reg_reg/CKbb	q_reg_reg/Db#8
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_reg_reg/Mmp55	FE_OFN0_q#35	q_reg_reg/qbint#5	VDD#138
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[9]/Mmp50	\mem_reg[9]/net376	\mem_reg[9]/qbint	VDD#74
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp30	\mem_reg[9]/mout#11	\mem_reg[9]/n20	VDD#76
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp14	\mem_reg[9]/Db#4	\mem_reg[9]/SEb
+ \mem_reg[9]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp51	\mem_reg[9]/n30#11	\mem_reg[9]/CKbb#3
+ \mem_reg[9]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp36	\mem_reg[9]/n20#11	\mem_reg[9]/CKb
+ \mem_reg[9]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp12	\mem_reg[9]/Db#4	scan_en#47
+ \mem_reg[9]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp11	\mem_reg[9]/net384	mem[9]#5	VDD#83	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp35	\mem_reg[9]/net356	\mem_reg[9]/mout	VDD#78
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp13	\mem_reg[9]/net392	mem[10]	VDD#81	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp40	\mem_reg[9]/n30#11	\mem_reg[9]/CKb#5
+ \mem_reg[9]/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp45	\mem_reg[9]/qbint#23	\mem_reg[9]/n30	VDD#73
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp10	\mem_reg[9]/SEb#12	scan_en#45	VDD#83	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp56	\mem_reg[9]/net367#11	\mem_reg[9]/qbint#3	VDD#73
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp21	\mem_reg[9]/CKbb#20	\mem_reg[9]/CKb#3	VDD#78
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp20	\mem_reg[9]/CKb#25	scan_clk#23	VDD#81	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp57	UNCONNECTED3#4	\mem_reg[9]/net367	VDD#71	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[9]/Mmp25	\mem_reg[9]/n20#11	\mem_reg[9]/CKbb
+ \mem_reg[9]/Db#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[9]/Mmp55	mem[9]#31	\mem_reg[9]/qbint#5	VDD#71	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[13]/Mmp50	\mem_reg[13]/net376	\mem_reg[13]/qbint	VDD#399
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp30	\mem_reg[13]/mout#8	\mem_reg[13]/n20	VDD#400
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp14	\mem_reg[13]/Db	\mem_reg[13]/SEb
+ \mem_reg[13]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp51	\mem_reg[13]/n30#8	\mem_reg[13]/CKbb#3
+ \mem_reg[13]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp36	\mem_reg[13]/n20#8	\mem_reg[13]/CKb
+ \mem_reg[13]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp12	\mem_reg[13]/Db	scan_en#43
+ \mem_reg[13]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp11	\mem_reg[13]/net384	mem[13]	VDD#403
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp35	\mem_reg[13]/net356	\mem_reg[13]/mout	VDD#401
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp13	\mem_reg[13]/net392	mem[15]	VDD#402
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp40	\mem_reg[13]/n30#8	\mem_reg[13]/CKb#5
+ \mem_reg[13]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp45	\mem_reg[13]/qbint#22	\mem_reg[13]/n30	VDD#398
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp10	\mem_reg[13]/SEb#9	scan_en#41	VDD#403
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp56	\mem_reg[13]/net367#8	\mem_reg[13]/qbint#3	VDD#398
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp21	\mem_reg[13]/CKbb#17	\mem_reg[13]/CKb#3	VDD#401
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp20	\mem_reg[13]/CKb#22	scan_clk#21	VDD#402
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp57	UNCONNECTED5	\mem_reg[13]/net367	VDD#397
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[13]/Mmp25	\mem_reg[13]/n20#8	\mem_reg[13]/CKbb
+ \mem_reg[13]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[13]/Mmp55	mem[13]#26	\mem_reg[13]/qbint#5	VDD#397
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[6]/Mmp50	\mem_reg[6]/net376	\mem_reg[6]/qbint	VDD#244
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp30	\mem_reg[6]/mout#11	\mem_reg[6]/n20	VDD#246
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp14	\mem_reg[6]/Db#4	\mem_reg[6]/SEb
+ \mem_reg[6]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp51	\mem_reg[6]/n30#11	\mem_reg[6]/CKbb#3
+ \mem_reg[6]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp36	\mem_reg[6]/n20#11	\mem_reg[6]/CKb
+ \mem_reg[6]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp12	\mem_reg[6]/Db#4	scan_en#37
+ \mem_reg[6]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp11	\mem_reg[6]/net384	mem[6]#3	VDD#251
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp35	\mem_reg[6]/net356	\mem_reg[6]/mout	VDD#247
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp13	\mem_reg[6]/net392	mem[4]#3	VDD#249
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp40	\mem_reg[6]/n30#11	\mem_reg[6]/CKb#5
+ \mem_reg[6]/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp45	\mem_reg[6]/qbint#23	\mem_reg[6]/n30	VDD#243
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp10	\mem_reg[6]/SEb#12	scan_en#33	VDD#251
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp56	\mem_reg[6]/net367#11	\mem_reg[6]/qbint#3	VDD#243
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp21	\mem_reg[6]/CKbb#20	\mem_reg[6]/CKb#3	VDD#247
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp20	\mem_reg[6]/CKb#25	scan_clk#17	VDD#249
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp57	UNCONNECTED12#4	\mem_reg[6]/net367	VDD#240
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[6]/Mmp25	\mem_reg[6]/n20#11	\mem_reg[6]/CKbb
+ \mem_reg[6]/Db#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[6]/Mmp55	mem[6]#31	\mem_reg[6]/qbint#5	VDD#240
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[11]/Mmp50	\mem_reg[11]/net376	\mem_reg[11]/qbint	VDD#80
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp30	\mem_reg[11]/mout#8	\mem_reg[11]/n20	VDD#82
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp14	\mem_reg[11]/Db	\mem_reg[11]/SEb
+ \mem_reg[11]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp51	\mem_reg[11]/n30#8	\mem_reg[11]/CKbb#3
+ \mem_reg[11]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp36	\mem_reg[11]/n20#8	\mem_reg[11]/CKb
+ \mem_reg[11]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp12	\mem_reg[11]/Db	scan_en#39
+ \mem_reg[11]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp11	\mem_reg[11]/net384	mem[11]	VDD#87	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp35	\mem_reg[11]/net356	\mem_reg[11]/mout	VDD#84
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp13	\mem_reg[11]/net392	mem[9]	VDD#86	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp40	\mem_reg[11]/n30#8	\mem_reg[11]/CKb#5
+ \mem_reg[11]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp45	\mem_reg[11]/qbint#22	\mem_reg[11]/n30	VDD#79
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp10	\mem_reg[11]/SEb#9	scan_en#35	VDD#87	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp56	\mem_reg[11]/net367#8	\mem_reg[11]/qbint#3	VDD#79
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp21	\mem_reg[11]/CKbb#17	\mem_reg[11]/CKb#3	VDD#84
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp20	\mem_reg[11]/CKb#22	scan_clk#19	VDD#86	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp57	UNCONNECTED16	\mem_reg[11]/net367	VDD#77	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[11]/Mmp25	\mem_reg[11]/n20#8	\mem_reg[11]/CKbb
+ \mem_reg[11]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[11]/Mmp55	mem[11]#26	\mem_reg[11]/qbint#5	VDD#77	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
Mq_control_reg/Mmp50	q_control_reg/net376	q_control_reg/qbint	VDD#147
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp30	q_control_reg/mout#11	q_control_reg/n20	VDD#148
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp14	q_control_reg/Db#4	q_control_reg/SEb
+ q_control_reg/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp51	q_control_reg/n30#11	q_control_reg/CKbb#3
+ q_control_reg/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp36	q_control_reg/n20#11	q_control_reg/CKb
+ q_control_reg/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp12	q_control_reg/Db#4	scan_en#31
+ q_control_reg/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp11	q_control_reg/net384	q_control#3	VDD#151
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp35	q_control_reg/net356	q_control_reg/mout	VDD#149
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp13	q_control_reg/net392	scan_in[0]#1	VDD#150
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp40	q_control_reg/n30#11	q_control_reg/CKb#5
+ q_control_reg/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp45	q_control_reg/qbint#23	q_control_reg/n30	VDD#146
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp10	q_control_reg/SEb#12	scan_en#29	VDD#151
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp56	q_control_reg/net367#11	q_control_reg/qbint#3
+ VDD#146	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp21	q_control_reg/CKbb#20	q_control_reg/CKb#3	VDD#149
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp20	q_control_reg/CKb#25	scan_clk#15	VDD#150
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp57	UNCONNECTED11#4	q_control_reg/net367	VDD#145
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
Mq_control_reg/Mmp25	q_control_reg/n20#11	q_control_reg/CKbb
+ q_control_reg/Db#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
Mq_control_reg/Mmp55	q_control#36	q_control_reg/qbint#5	VDD#145
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[5]/Mmp50	\mem_reg[5]/net376	\mem_reg[5]/qbint	VDD#332
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp30	\mem_reg[5]/mout#11	\mem_reg[5]/n20	VDD#336
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp14	\mem_reg[5]/Db#4	\mem_reg[5]/SEb
+ \mem_reg[5]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp51	\mem_reg[5]/n30#11	\mem_reg[5]/CKbb#3
+ \mem_reg[5]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp36	\mem_reg[5]/n20#11	\mem_reg[5]/CKb
+ \mem_reg[5]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp12	\mem_reg[5]/Db#4	scan_en#27
+ \mem_reg[5]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp11	\mem_reg[5]/net384	mem[5]	VDD#350	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp35	\mem_reg[5]/net356	\mem_reg[5]/mout	VDD#340
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp13	\mem_reg[5]/net392	mem[14]#3	VDD#349
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp40	\mem_reg[5]/n30#11	\mem_reg[5]/CKb#5
+ \mem_reg[5]/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp45	\mem_reg[5]/qbint#23	\mem_reg[5]/n30	VDD#328
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp10	\mem_reg[5]/SEb#12	scan_en#23	VDD#350
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp56	\mem_reg[5]/net367#11	\mem_reg[5]/qbint#3	VDD#328
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp21	\mem_reg[5]/CKbb#20	\mem_reg[5]/CKb#3	VDD#340
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp20	\mem_reg[5]/CKb#25	scan_clk#13	VDD#349
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp57	UNCONNECTED0#4	\mem_reg[5]/net367	VDD#323
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[5]/Mmp25	\mem_reg[5]/n20#11	\mem_reg[5]/CKbb
+ \mem_reg[5]/Db#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[5]/Mmp55	mem[5]#29	\mem_reg[5]/qbint#5	VDD#323
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[7]/Mmp50	\mem_reg[7]/net376	\mem_reg[7]/qbint	VDD#406
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp30	\mem_reg[7]/mout#8	\mem_reg[7]/n20	VDD#407
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp14	\mem_reg[7]/Db	\mem_reg[7]/SEb
+ \mem_reg[7]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp51	\mem_reg[7]/n30#8	\mem_reg[7]/CKbb#3
+ \mem_reg[7]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp36	\mem_reg[7]/n20#8	\mem_reg[7]/CKb
+ \mem_reg[7]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp12	\mem_reg[7]/Db	scan_en#25	\mem_reg[7]/net384
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp11	\mem_reg[7]/net384	mem[7]	VDD#410	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp35	\mem_reg[7]/net356	\mem_reg[7]/mout	VDD#408
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp13	\mem_reg[7]/net392	mem[5]#3	VDD#409
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp40	\mem_reg[7]/n30#8	\mem_reg[7]/CKb#5
+ \mem_reg[7]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp45	\mem_reg[7]/qbint#22	\mem_reg[7]/n30	VDD#405
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp10	\mem_reg[7]/SEb#9	scan_en#21	VDD#410
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp56	\mem_reg[7]/net367#8	\mem_reg[7]/qbint#3	VDD#405
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp21	\mem_reg[7]/CKbb#17	\mem_reg[7]/CKb#3	VDD#408
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp20	\mem_reg[7]/CKb#22	scan_clk#11	VDD#409
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp57	UNCONNECTED13	\mem_reg[7]/net367	VDD#404
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[7]/Mmp25	\mem_reg[7]/n20#8	\mem_reg[7]/CKbb
+ \mem_reg[7]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[7]/Mmp55	mem[7]#24	\mem_reg[7]/qbint#5	VDD#404
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[14]/Mmp50	\mem_reg[14]/net376	\mem_reg[14]/qbint	VDD#335
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp30	\mem_reg[14]/mout#8	\mem_reg[14]/n20	VDD#339
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp14	\mem_reg[14]/Db	\mem_reg[14]/SEb
+ \mem_reg[14]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp51	\mem_reg[14]/n30#8	\mem_reg[14]/CKbb#3
+ \mem_reg[14]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp36	\mem_reg[14]/n20#8	\mem_reg[14]/CKb
+ \mem_reg[14]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp12	\mem_reg[14]/Db	scan_en#19
+ \mem_reg[14]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp11	\mem_reg[14]/net384	mem[14]	VDD#352
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp35	\mem_reg[14]/net356	\mem_reg[14]/mout	VDD#342
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp13	\mem_reg[14]/net392	mem[12]#3	VDD#348
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp40	\mem_reg[14]/n30#8	\mem_reg[14]/CKb#5
+ \mem_reg[14]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp45	\mem_reg[14]/qbint#22	\mem_reg[14]/n30	VDD#331
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp10	\mem_reg[14]/SEb#9	scan_en#9	VDD#352
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp56	\mem_reg[14]/net367#8	\mem_reg[14]/qbint#3	VDD#331
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp21	\mem_reg[14]/CKbb#17	\mem_reg[14]/CKb#3	VDD#342
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp20	\mem_reg[14]/CKb#22	scan_clk#9	VDD#348
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp57	UNCONNECTED6	\mem_reg[14]/net367	VDD#327
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[14]/Mmp25	\mem_reg[14]/n20#8	\mem_reg[14]/CKbb
+ \mem_reg[14]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[14]/Mmp55	mem[14]#26	\mem_reg[14]/qbint#5	VDD#327
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[12]/Mmp50	\mem_reg[12]/net376	\mem_reg[12]/qbint	VDD#257
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp30	\mem_reg[12]/mout#11	\mem_reg[12]/n20	VDD#259
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp14	\mem_reg[12]/Db#4	\mem_reg[12]/SEb
+ \mem_reg[12]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp51	\mem_reg[12]/n30#11	\mem_reg[12]/CKbb#3
+ \mem_reg[12]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp36	\mem_reg[12]/n20#11	\mem_reg[12]/CKb
+ \mem_reg[12]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp12	\mem_reg[12]/Db#4	scan_en#11
+ \mem_reg[12]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp11	\mem_reg[12]/net384	mem[12]	VDD#265
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp35	\mem_reg[12]/net356	\mem_reg[12]/mout	VDD#261
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp13	\mem_reg[12]/net392	mem[6]	VDD#263	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp40	\mem_reg[12]/n30#11	\mem_reg[12]/CKb#5
+ \mem_reg[12]/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp45	\mem_reg[12]/qbint#23	\mem_reg[12]/n30	VDD#255
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp10	\mem_reg[12]/SEb#12	scan_en#1	VDD#265
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp56	\mem_reg[12]/net367#11	\mem_reg[12]/qbint#3	VDD#255
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp21	\mem_reg[12]/CKbb#20	\mem_reg[12]/CKb#3	VDD#261
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp20	\mem_reg[12]/CKb#25	scan_clk#1	VDD#263
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp57	UNCONNECTED4#4	\mem_reg[12]/net367	VDD#253
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[12]/Mmp25	\mem_reg[12]/n20#11	\mem_reg[12]/CKbb
+ \mem_reg[12]/Db#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[12]/Mmp55	mem[12]#29	\mem_reg[12]/qbint#5	VDD#253
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[4]/Mmp50	\mem_reg[4]/net376	\mem_reg[4]/qbint	VDD#256
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp30	\mem_reg[4]/mout#8	\mem_reg[4]/n20	VDD#258
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp14	\mem_reg[4]/Db	\mem_reg[4]/SEb
+ \mem_reg[4]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp51	\mem_reg[4]/n30#8	\mem_reg[4]/CKbb#3
+ \mem_reg[4]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp36	\mem_reg[4]/n20#8	\mem_reg[4]/CKb
+ \mem_reg[4]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp12	\mem_reg[4]/Db	scan_en#13	\mem_reg[4]/net384
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp11	\mem_reg[4]/net384	mem[4]	VDD#264	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp35	\mem_reg[4]/net356	\mem_reg[4]/mout	VDD#260
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp13	\mem_reg[4]/net392	mem[1]#3	VDD#262
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp40	\mem_reg[4]/n30#8	\mem_reg[4]/CKb#5
+ \mem_reg[4]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp45	\mem_reg[4]/qbint#22	\mem_reg[4]/n30	VDD#254
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp10	\mem_reg[4]/SEb#9	scan_en#3	VDD#264
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp56	\mem_reg[4]/net367#8	\mem_reg[4]/qbint#3	VDD#254
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp21	\mem_reg[4]/CKbb#17	\mem_reg[4]/CKb#3	VDD#260
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp20	\mem_reg[4]/CKb#22	scan_clk#3	VDD#262
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp57	UNCONNECTED17	\mem_reg[4]/net367	VDD#252
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[4]/Mmp25	\mem_reg[4]/n20#8	\mem_reg[4]/CKbb
+ \mem_reg[4]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[4]/Mmp55	mem[4]#24	\mem_reg[4]/qbint#5	VDD#252
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[1]/Mmp50	\mem_reg[1]/net376	\mem_reg[1]/qbint	VDD#94
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp30	\mem_reg[1]/mout#11	\mem_reg[1]/n20	VDD#96
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp14	\mem_reg[1]/Db#4	\mem_reg[1]/SEb
+ \mem_reg[1]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp51	\mem_reg[1]/n30#11	\mem_reg[1]/CKbb#3
+ \mem_reg[1]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp36	\mem_reg[1]/n20#11	\mem_reg[1]/CKb
+ \mem_reg[1]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp12	\mem_reg[1]/Db#4	scan_en#15
+ \mem_reg[1]/net384	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp11	\mem_reg[1]/net384	mem[1]	VDD#102	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp35	\mem_reg[1]/net356	\mem_reg[1]/mout	VDD#98
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp13	\mem_reg[1]/net392	mem[3]#3	VDD#100
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp40	\mem_reg[1]/n30#11	\mem_reg[1]/CKb#5
+ \mem_reg[1]/mout#11	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp45	\mem_reg[1]/qbint#23	\mem_reg[1]/n30	VDD#92
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp10	\mem_reg[1]/SEb#12	scan_en#5	VDD#102
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp56	\mem_reg[1]/net367#11	\mem_reg[1]/qbint#3	VDD#92
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp21	\mem_reg[1]/CKbb#20	\mem_reg[1]/CKb#3	VDD#98
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp20	\mem_reg[1]/CKb#25	scan_clk#5	VDD#100
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp57	UNCONNECTED10#4	\mem_reg[1]/net367	VDD#90
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[1]/Mmp25	\mem_reg[1]/n20#11	\mem_reg[1]/CKbb
+ \mem_reg[1]/Db#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[1]/Mmp55	mem[1]#29	\mem_reg[1]/qbint#5	VDD#90	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[3]/Mmp50	\mem_reg[3]/net376	\mem_reg[3]/qbint	VDD#93
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp30	\mem_reg[3]/mout#8	\mem_reg[3]/n20	VDD#95
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp14	\mem_reg[3]/Db	\mem_reg[3]/SEb
+ \mem_reg[3]/net392	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp51	\mem_reg[3]/n30#8	\mem_reg[3]/CKbb#3
+ \mem_reg[3]/net376	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp36	\mem_reg[3]/n20#8	\mem_reg[3]/CKb
+ \mem_reg[3]/net356	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp12	\mem_reg[3]/Db	scan_en#17	\mem_reg[3]/net384
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp11	\mem_reg[3]/net384	mem[3]	VDD#101	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp35	\mem_reg[3]/net356	\mem_reg[3]/mout	VDD#97
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp13	\mem_reg[3]/net392	q_control	VDD#99	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp40	\mem_reg[3]/n30#8	\mem_reg[3]/CKb#5
+ \mem_reg[3]/mout#8	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp45	\mem_reg[3]/qbint#22	\mem_reg[3]/n30	VDD#91
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp10	\mem_reg[3]/SEb#9	scan_en#7	VDD#101
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp56	\mem_reg[3]/net367#8	\mem_reg[3]/qbint#3	VDD#91
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp21	\mem_reg[3]/CKbb#17	\mem_reg[3]/CKb#3	VDD#97
+ VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp20	\mem_reg[3]/CKb#22	scan_clk#7	VDD#99	VDD
+ pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp57	UNCONNECTED8	\mem_reg[3]/net367	VDD#89	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
M\mem_reg[3]/Mmp25	\mem_reg[3]/n20#8	\mem_reg[3]/CKbb
+ \mem_reg[3]/Db#5	VDD	pfet_01v8	L=1.5e-07	W=6.25e-07
M\mem_reg[3]/Mmp55	mem[3]#26	\mem_reg[3]/qbint#5	VDD#89	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp0@2	FE_OFC0_q/n0#116	FE_OFN0_q	VDD#69	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp0	FE_OFC0_q/n0#116	FE_OFN0_q#3	VDD#67	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp0@3	FE_OFC0_q/n0#120	FE_OFN0_q#5	VDD#67	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp0@1	FE_OFC0_q/n0#120	FE_OFN0_q#7	VDD#66	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@2	q#3	FE_OFC0_q/n0	VDD#66	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@3	q#3	FE_OFC0_q/n0#3	VDD#64	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@1	q#7	FE_OFC0_q/n0#5	VDD#64	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@4	q#7	FE_OFC0_q/n0#7	VDD#63	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@5	q#11	FE_OFC0_q/n0#9	VDD#63	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@7	q#11	FE_OFC0_q/n0#11	VDD#61	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@8	q#15	FE_OFC0_q/n0#13	VDD#61	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@6	q#15	FE_OFC0_q/n0#15	VDD#59	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@9	q#19	FE_OFC0_q/n0#17	VDD#59	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@10	q#19	FE_OFC0_q/n0#19	VDD#58	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@12	q#23	FE_OFC0_q/n0#21	VDD#58	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1	q#23	FE_OFC0_q/n0#23	VDD#56	VDD	pfet_01v8
+ L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@11	q#27	FE_OFC0_q/n0#25	VDD#56	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@13	q#27	FE_OFC0_q/n0#27	VDD#55	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@14	q#31	FE_OFC0_q/n0#29	VDD#55	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC0_q/Mmp1@15	q#31	FE_OFC0_q/n0#31	VDD#1	VDD
+ pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp0@2	FE_OFC1_scan_out_0/n0#116
+ FE_OFN1_scan_out_0	VDD#162	VDD	pfet_01v8	L=1.5e-07
+ W=1.13e-06
MFE_OFC1_scan_out_0/Mmp0	FE_OFC1_scan_out_0/n0#116
+ FE_OFN1_scan_out_0#3	VDD#171	VDD	pfet_01v8	L=1.5e-07
+ W=1.13e-06
MFE_OFC1_scan_out_0/Mmp0@3	FE_OFC1_scan_out_0/n0#120
+ FE_OFN1_scan_out_0#5	VDD#171	VDD	pfet_01v8	L=1.5e-07
+ W=1.13e-06
MFE_OFC1_scan_out_0/Mmp0@1	FE_OFC1_scan_out_0/n0#120
+ FE_OFN1_scan_out_0#7	VDD#170	VDD	pfet_01v8	L=1.5e-07
+ W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@2	scan_out[0]#3	FE_OFC1_scan_out_0/n0	VDD#170
+ VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@3	scan_out[0]#3	FE_OFC1_scan_out_0/n0#3
+ VDD#169	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@1	scan_out[0]#7	FE_OFC1_scan_out_0/n0#5
+ VDD#169	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@4	scan_out[0]#7	FE_OFC1_scan_out_0/n0#7
+ VDD#168	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@5	scan_out[0]#11	FE_OFC1_scan_out_0/n0#9
+ VDD#168	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@7	scan_out[0]#11	FE_OFC1_scan_out_0/n0#11
+ VDD#167	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@8	scan_out[0]#15	FE_OFC1_scan_out_0/n0#13
+ VDD#167	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@6	scan_out[0]#15	FE_OFC1_scan_out_0/n0#15
+ VDD#166	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@9	scan_out[0]#19	FE_OFC1_scan_out_0/n0#17
+ VDD#166	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@10	scan_out[0]#19	FE_OFC1_scan_out_0/n0#19
+ VDD#165	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@12	scan_out[0]#23	FE_OFC1_scan_out_0/n0#21
+ VDD#165	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1	scan_out[0]#23	FE_OFC1_scan_out_0/n0#23
+ VDD#164	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@11	scan_out[0]#27	FE_OFC1_scan_out_0/n0#25
+ VDD#164	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@13	scan_out[0]#27	FE_OFC1_scan_out_0/n0#27
+ VDD#163	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@14	scan_out[0]#31	FE_OFC1_scan_out_0/n0#29
+ VDD#163	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
MFE_OFC1_scan_out_0/Mmp1@15	scan_out[0]#31	FE_OFC1_scan_out_0/n0#31
+ VDD#152	VDD	pfet_01v8	L=1.5e-07	W=1.13e-06
*
*
*       RESISTOR AND CAP/DIODE CARDS
*
Rg2	scan_en#73	scan_en#217	290.345947	$poly
Rg3	scan_en#217	scan_en#1	463.865936	$poly
Rg6	scan_en#3	scan_en#218	463.865906	$poly
Rg7	scan_en#218	scan_en#75	290.345947	$poly
Rg10	scan_en#77	scan_en#219	290.345947	$poly
Rg11	scan_en#219	scan_en#5	463.865936	$poly
Rg14	scan_en#7	scan_en#220	463.865906	$poly
Rg15	scan_en#220	scan_en#79	290.345947	$poly
Rg18	scan_en#9	scan_en#221	463.865906	$poly
Rg19	scan_en#221	scan_en#81	290.345947	$poly
Rg22	mem[12]#7	mem[12]#19	465.569092	$poly
Rg23	mem[12]#19	mem[12]	279.195740	$poly
Rg26	mem[4]	mem[4]#19	279.195740	$poly
Rg27	mem[4]#19	mem[4]#7	465.569092	$poly
Rg30	mem[1]#7	mem[1]#19	465.569092	$poly
Rg31	mem[1]#19	mem[1]	279.195740	$poly
Rg34	mem[3]	mem[3]#19	279.195740	$poly
Rg35	mem[3]#19	mem[3]#7	465.569092	$poly
Rg38	\mem_reg[12]/SEb#3	\mem_reg[12]/SEb#7	137.809082	$poly
Rg40	\mem_reg[4]/SEb#3	\mem_reg[4]/SEb#7	137.809082	$poly
Rg42	\mem_reg[1]/SEb#3	\mem_reg[1]/SEb#7	137.809082	$poly
Rg44	\mem_reg[3]/SEb#3	\mem_reg[3]/SEb#7	137.809082	$poly
Rg46	mem[14]	mem[14]#19	279.195740	$poly
Rg47	mem[14]#19	mem[14]#7	465.569092	$poly
Rg50	scan_en#83	scan_en#222	400.944214	$poly
Rg51	scan_en#222	scan_en#11	407.290009	$poly
Rg54	scan_en#85	scan_en#223	400.944214	$poly
Rg55	scan_en#223	scan_en#13	407.290009	$poly
Rg58	scan_en#87	scan_en#224	400.944214	$poly
Rg59	scan_en#224	scan_en#15	407.290009	$poly
Rg62	scan_en#89	scan_en#225	400.944214	$poly
Rg63	scan_en#225	scan_en#17	407.290009	$poly
Rg66	FE_OFN1_scan_out_0#21	FE_OFN1_scan_out_0#43	206.496658	$poly
Rg67	FE_OFN1_scan_out_0#43	FE_OFN1_scan_out_0#44	62.011234	$poly
Rg68	FE_OFN1_scan_out_0#44	FE_OFN1_scan_out_0#45	36.929379	$poly
Rg69	FE_OFN1_scan_out_0#45	FE_OFN1_scan_out_0#46	36.929379	$poly
Rg70	FE_OFN1_scan_out_0#46	FE_OFN1_scan_out_0#47	62.011227	$poly
Rg71	FE_OFN1_scan_out_0#47	FE_OFN1_scan_out_0	433.036652	$poly
Rg73	FE_OFN1_scan_out_0#43	FE_OFN1_scan_out_0#7	433.036652	$poly
Rg75	FE_OFN1_scan_out_0#44	FE_OFN1_scan_out_0#5	430.479736	$poly
Rg77	FE_OFN1_scan_out_0#44	FE_OFN1_scan_out_0#19	203.939743	$poly
Rg79	FE_OFN1_scan_out_0#46	FE_OFN1_scan_out_0#3	430.479736	$poly
Rg81	FE_OFN1_scan_out_0#46	FE_OFN1_scan_out_0#17	203.939743	$poly
Rg83	FE_OFN1_scan_out_0#47	FE_OFN1_scan_out_0#15	206.496658	$poly
Rg86	\mem_reg[12]/SEb	\mem_reg[12]/SEb#8	169.942413	$poly
Rg88	\mem_reg[4]/SEb	\mem_reg[4]/SEb#8	169.942413	$poly
Rg90	\mem_reg[1]/SEb	\mem_reg[1]/SEb#8	169.942413	$poly
Rg92	\mem_reg[3]/SEb	\mem_reg[3]/SEb#8	169.942413	$poly
Rg94	\mem_reg[14]/SEb#3	\mem_reg[14]/SEb#7	137.809082	$poly
Rg96	scan_en#91	scan_en#226	400.944214	$poly
Rg97	scan_en#226	scan_en#19	407.290009	$poly
Rg100	mem[6]#7	mem[6]#19	356.315765	$poly
Rg101	mem[6]#19	mem[6]	388.449066	$poly
Rg104	mem[1]#3	mem[1]#20	388.449097	$poly
Rg105	mem[1]#20	mem[1]#9	356.315735	$poly
Rg108	mem[3]#9	mem[3]#20	356.315765	$poly
Rg109	mem[3]#20	mem[3]#3	388.449066	$poly
Rg112	q_control	q_control#25	388.449097	$poly
Rg113	q_control#25	q_control#9	356.315735	$poly
Rg116	\mem_reg[14]/SEb	\mem_reg[14]/SEb#8	169.942413	$poly
Rg118	scan_en#21	scan_en#227	463.865906	$poly
Rg119	scan_en#227	scan_en#93	290.345947	$poly
Rg122	scan_clk#37	scan_clk#109	137.809082	$poly
Rg123	scan_clk#109	scan_clk#1	606.955750	$poly
Rg126	scan_clk#3	scan_clk#110	606.955750	$poly
Rg127	scan_clk#110	scan_clk#39	137.809082	$poly
Rg130	scan_clk#41	scan_clk#111	137.809082	$poly
Rg131	scan_clk#111	scan_clk#5	606.955750	$poly
Rg134	scan_clk#7	scan_clk#112	606.955750	$poly
Rg135	scan_clk#112	scan_clk#43	137.809082	$poly
Rg138	mem[12]#3	mem[12]#20	388.449097	$poly
Rg139	mem[12]#20	mem[12]#9	356.315735	$poly
Rg142	scan_en#95	scan_en#228	290.345947	$poly
Rg143	scan_en#228	scan_en#23	463.865936	$poly
Rg146	mem[7]	mem[7]#19	279.195740	$poly
Rg147	mem[7]#19	mem[7]#7	465.569092	$poly
Rg150	scan_clk#9	scan_clk#113	606.955750	$poly
Rg151	scan_clk#113	scan_clk#45	137.809082	$poly
Rg154	\mem_reg[7]/SEb#3	\mem_reg[7]/SEb#7	137.809082	$poly
Rg156	mem[5]#7	mem[5]#19	465.569092	$poly
Rg157	mem[5]#19	mem[5]	279.195740	$poly
Rg160	scan_en#97	scan_en#229	400.944214	$poly
Rg161	scan_en#229	scan_en#25	407.290009	$poly
Rg164	\mem_reg[12]/CKbb	\mem_reg[12]/CKbb#13	169.942413	$poly
Rg166	\mem_reg[4]/CKbb	\mem_reg[4]/CKbb#13	169.942413	$poly
Rg168	\mem_reg[1]/CKbb	\mem_reg[1]/CKbb#13	169.942413	$poly
Rg170	\mem_reg[3]/CKbb	\mem_reg[3]/CKbb#13	169.942413	$poly
Rg172	\mem_reg[7]/SEb	\mem_reg[7]/SEb#8	169.942413	$poly
Rg174	\mem_reg[5]/SEb#3	\mem_reg[5]/SEb#7	137.809082	$poly
Rg176	\mem_reg[12]/CKb	\mem_reg[12]/CKb#19	418.796082	$poly
Rg177	\mem_reg[12]/CKb#19	\mem_reg[12]/CKb#7	386.662750	$poly
Rg180	\mem_reg[4]/CKb	\mem_reg[4]/CKb#19	418.796082	$poly
Rg181	\mem_reg[4]/CKb#19	\mem_reg[4]/CKb#7	386.662750	$poly
Rg184	\mem_reg[1]/CKb	\mem_reg[1]/CKb#19	418.796082	$poly
Rg185	\mem_reg[1]/CKb#19	\mem_reg[1]/CKb#7	386.662750	$poly
Rg188	\mem_reg[3]/CKb	\mem_reg[3]/CKb#19	418.796082	$poly
Rg189	\mem_reg[3]/CKb#19	\mem_reg[3]/CKb#7	386.662750	$poly
Rg192	\mem_reg[12]/CKbb#5	\mem_reg[12]/CKbb#14	137.809082	$poly
Rg194	\mem_reg[4]/CKbb#5	\mem_reg[4]/CKbb#14	137.809082	$poly
Rg196	\mem_reg[1]/CKbb#5	\mem_reg[1]/CKbb#14	137.809082	$poly
Rg198	\mem_reg[3]/CKbb#5	\mem_reg[3]/CKbb#14	137.809082	$poly
Rg200	scan_en#99	scan_en#230	400.944214	$poly
Rg201	scan_en#230	scan_en#27	407.290009	$poly
Rg204	mem[5]#3	mem[5]#20	388.449097	$poly
Rg205	mem[5]#20	mem[5]#9	356.315735	$poly
Rg208	\mem_reg[14]/CKbb	\mem_reg[14]/CKbb#13	169.942413	$poly
Rg210	\mem_reg[5]/SEb	\mem_reg[5]/SEb#8	169.942413	$poly
Rg212	\mem_reg[14]/CKb	\mem_reg[14]/CKb#19	418.796082	$poly
Rg213	\mem_reg[14]/CKb#19	\mem_reg[14]/CKb#7	386.662750	$poly
Rg216	\mem_reg[12]/mout#3	\mem_reg[12]/mout#7	465.569092	$poly
Rg217	\mem_reg[12]/mout#7	\mem_reg[12]/mout	279.195740	$poly
Rg220	\mem_reg[4]/mout	\mem_reg[4]/mout#7	279.195740	$poly
Rg221	\mem_reg[4]/mout#7	\mem_reg[4]/mout#3	465.569092	$poly
Rg224	\mem_reg[1]/mout#3	\mem_reg[1]/mout#7	465.569092	$poly
Rg225	\mem_reg[1]/mout#7	\mem_reg[1]/mout	279.195740	$poly
Rg228	\mem_reg[3]/mout	\mem_reg[3]/mout#7	279.195740	$poly
Rg229	\mem_reg[3]/mout#7	\mem_reg[3]/mout#3	465.569092	$poly
Rg232	scan_clk#11	scan_clk#114	606.955750	$poly
Rg233	scan_clk#114	scan_clk#47	137.809082	$poly
Rg236	\mem_reg[14]/CKbb#5	\mem_reg[14]/CKbb#14	137.809082	$poly
Rg238	mem[14]#9	mem[14]#20	356.315765	$poly
Rg239	mem[14]#20	mem[14]#3	388.449066	$poly
Rg242	\mem_reg[14]/mout	\mem_reg[14]/mout#7	279.195740	$poly
Rg243	\mem_reg[14]/mout#7	\mem_reg[14]/mout#3	465.569092	$poly
Rg246	scan_clk#49	scan_clk#115	137.809082	$poly
Rg247	scan_clk#115	scan_clk#13	606.955750	$poly
Rg250	\mem_reg[12]/CKb#9	\mem_reg[12]/CKb#20	361.039276	$poly
Rg251	\mem_reg[12]/CKb#20	\mem_reg[12]/CKb#3	393.172607	$poly
Rg254	\mem_reg[4]/CKb#3	\mem_reg[4]/CKb#20	393.172607	$poly
Rg255	\mem_reg[4]/CKb#20	\mem_reg[4]/CKb#9	361.039276	$poly
Rg258	\mem_reg[1]/CKb#9	\mem_reg[1]/CKb#20	361.039276	$poly
Rg259	\mem_reg[1]/CKb#20	\mem_reg[1]/CKb#3	393.172607	$poly
Rg262	\mem_reg[3]/CKb#3	\mem_reg[3]/CKb#20	393.172607	$poly
Rg263	\mem_reg[3]/CKb#20	\mem_reg[3]/CKb#9	361.039276	$poly
Rg266	\mem_reg[14]/CKb#3	\mem_reg[14]/CKb#20	393.172607	$poly
Rg267	\mem_reg[14]/CKb#20	\mem_reg[14]/CKb#9	361.039276	$poly
Rg270	\mem_reg[7]/CKbb	\mem_reg[7]/CKbb#13	169.942413	$poly
Rg272	\mem_reg[7]/CKb	\mem_reg[7]/CKb#19	418.796082	$poly
Rg273	\mem_reg[7]/CKb#19	\mem_reg[7]/CKb#7	386.662750	$poly
Rg276	\mem_reg[12]/n20#3	\mem_reg[12]/n20#7	574.822449	$poly
Rg277	\mem_reg[12]/n20#7	\mem_reg[12]/n20	169.942413	$poly
Rg280	\mem_reg[4]/n20	\mem_reg[4]/n20#7	169.942413	$poly
Rg281	\mem_reg[4]/n20#7	\mem_reg[4]/n20#3	574.822449	$poly
Rg284	\mem_reg[1]/n20#3	\mem_reg[1]/n20#7	574.822449	$poly
Rg285	\mem_reg[1]/n20#7	\mem_reg[1]/n20	169.942413	$poly
Rg288	\mem_reg[3]/n20	\mem_reg[3]/n20#7	169.942413	$poly
Rg289	\mem_reg[3]/n20#7	\mem_reg[3]/n20#3	574.822449	$poly
Rg292	\mem_reg[7]/CKbb#5	\mem_reg[7]/CKbb#14	137.809082	$poly
Rg294	\mem_reg[5]/CKbb	\mem_reg[5]/CKbb#13	169.942413	$poly
Rg296	\mem_reg[12]/CKbb#7	\mem_reg[12]/CKbb#15	152.809082	$poly
Rg298	\mem_reg[4]/CKbb#7	\mem_reg[4]/CKbb#15	152.809082	$poly
Rg300	\mem_reg[1]/CKbb#7	\mem_reg[1]/CKbb#15	152.809082	$poly
Rg302	\mem_reg[3]/CKbb#7	\mem_reg[3]/CKbb#15	152.809082	$poly
Rg304	\mem_reg[5]/CKb	\mem_reg[5]/CKb#19	418.796082	$poly
Rg305	\mem_reg[5]/CKb#19	\mem_reg[5]/CKb#7	386.662750	$poly
Rg308	\mem_reg[7]/mout	\mem_reg[7]/mout#7	279.195740	$poly
Rg309	\mem_reg[7]/mout#7	\mem_reg[7]/mout#3	465.569092	$poly
Rg312	\mem_reg[14]/n20	\mem_reg[14]/n20#7	169.942413	$poly
Rg313	\mem_reg[14]/n20#7	\mem_reg[14]/n20#3	574.822449	$poly
Rg316	\mem_reg[5]/CKbb#5	\mem_reg[5]/CKbb#14	137.809082	$poly
Rg318	\mem_reg[12]/CKb#11	\mem_reg[12]/CKb#21	386.662750	$poly
Rg319	\mem_reg[12]/CKb#21	\mem_reg[12]/CKb#5	418.796082	$poly
Rg322	\mem_reg[4]/CKb#11	\mem_reg[4]/CKb#21	386.662750	$poly
Rg323	\mem_reg[4]/CKb#21	\mem_reg[4]/CKb#5	418.796082	$poly
Rg326	\mem_reg[1]/CKb#11	\mem_reg[1]/CKb#21	386.662750	$poly
Rg327	\mem_reg[1]/CKb#21	\mem_reg[1]/CKb#5	418.796082	$poly
Rg330	\mem_reg[3]/CKb#11	\mem_reg[3]/CKb#21	386.662750	$poly
Rg331	\mem_reg[3]/CKb#21	\mem_reg[3]/CKb#5	418.796082	$poly
Rg334	\mem_reg[12]/CKbb#3	\mem_reg[12]/CKbb#16	169.942413	$poly
Rg336	\mem_reg[4]/CKbb#3	\mem_reg[4]/CKbb#16	169.942413	$poly
Rg338	\mem_reg[1]/CKbb#3	\mem_reg[1]/CKbb#16	169.942413	$poly
Rg340	\mem_reg[3]/CKbb#3	\mem_reg[3]/CKbb#16	169.942413	$poly
Rg342	\mem_reg[14]/CKbb#7	\mem_reg[14]/CKbb#15	152.809082	$poly
Rg344	\mem_reg[5]/mout#3	\mem_reg[5]/mout#7	465.569092	$poly
Rg345	\mem_reg[5]/mout#7	\mem_reg[5]/mout	279.195740	$poly
Rg348	\mem_reg[7]/CKb#3	\mem_reg[7]/CKb#20	393.172607	$poly
Rg349	\mem_reg[7]/CKb#20	\mem_reg[7]/CKb#9	361.039276	$poly
Rg352	\mem_reg[14]/CKb#11	\mem_reg[14]/CKb#21	386.662750	$poly
Rg353	\mem_reg[14]/CKb#21	\mem_reg[14]/CKb#5	418.796082	$poly
Rg356	\mem_reg[12]/qbint#7	\mem_reg[12]/qbint#19	247.062408	$poly
Rg357	\mem_reg[12]/qbint#19	\mem_reg[12]/qbint	497.702423	$poly
Rg360	\mem_reg[4]/qbint	\mem_reg[4]/qbint#19	497.702423	$poly
Rg361	\mem_reg[4]/qbint#19	\mem_reg[4]/qbint#7	247.062408	$poly
Rg364	\mem_reg[1]/qbint#7	\mem_reg[1]/qbint#19	247.062408	$poly
Rg365	\mem_reg[1]/qbint#19	\mem_reg[1]/qbint	497.702423	$poly
Rg368	\mem_reg[3]/qbint	\mem_reg[3]/qbint#19	497.702423	$poly
Rg369	\mem_reg[3]/qbint#19	\mem_reg[3]/qbint#7	247.062408	$poly
Rg372	\mem_reg[14]/CKbb#3	\mem_reg[14]/CKbb#16	169.942413	$poly
Rg374	\mem_reg[5]/CKb#9	\mem_reg[5]/CKb#20	361.039276	$poly
Rg375	\mem_reg[5]/CKb#20	\mem_reg[5]/CKb#3	393.172607	$poly
Rg378	\mem_reg[14]/qbint	\mem_reg[14]/qbint#19	497.702423	$poly
Rg379	\mem_reg[14]/qbint#19	\mem_reg[14]/qbint#7	247.062408	$poly
Rg382	\mem_reg[7]/n20	\mem_reg[7]/n20#7	169.942413	$poly
Rg383	\mem_reg[7]/n20#7	\mem_reg[7]/n20#3	574.822449	$poly
Rg386	\mem_reg[12]/qbint#9	\mem_reg[12]/qbint#20	247.062408	$poly
Rg387	\mem_reg[12]/qbint#20	\mem_reg[12]/qbint#3	497.702423	$poly
Rg390	\mem_reg[4]/qbint#3	\mem_reg[4]/qbint#20	497.702423	$poly
Rg391	\mem_reg[4]/qbint#20	\mem_reg[4]/qbint#9	247.062408	$poly
Rg394	\mem_reg[1]/qbint#9	\mem_reg[1]/qbint#20	247.062408	$poly
Rg395	\mem_reg[1]/qbint#20	\mem_reg[1]/qbint#3	497.702423	$poly
Rg398	\mem_reg[3]/qbint#3	\mem_reg[3]/qbint#20	497.702423	$poly
Rg399	\mem_reg[3]/qbint#20	\mem_reg[3]/qbint#9	247.062408	$poly
Rg402	\mem_reg[7]/CKbb#7	\mem_reg[7]/CKbb#15	152.809082	$poly
Rg404	\mem_reg[5]/n20#3	\mem_reg[5]/n20#7	574.822449	$poly
Rg405	\mem_reg[5]/n20#7	\mem_reg[5]/n20	169.942413	$poly
Rg408	\mem_reg[7]/CKb#11	\mem_reg[7]/CKb#21	386.662750	$poly
Rg409	\mem_reg[7]/CKb#21	\mem_reg[7]/CKb#5	418.796082	$poly
Rg412	\mem_reg[7]/CKbb#3	\mem_reg[7]/CKbb#16	169.942413	$poly
Rg414	\mem_reg[14]/qbint#3	\mem_reg[14]/qbint#20	497.702423	$poly
Rg415	\mem_reg[14]/qbint#20	\mem_reg[14]/qbint#9	247.062408	$poly
Rg418	\mem_reg[5]/CKbb#7	\mem_reg[5]/CKbb#15	152.809082	$poly
Rg420	FE_OFC1_scan_out_0/n0#63	FE_OFC1_scan_out_0/n0#97
+ 315.750000	$poly
Rg421	FE_OFC1_scan_out_0/n0#97	FE_OFC1_scan_out_0/n0#98
+ 62.011234	$poly
Rg422	FE_OFC1_scan_out_0/n0#98	FE_OFC1_scan_out_0/n0#99
+ 73.858757	$poly
Rg423	FE_OFC1_scan_out_0/n0#99	FE_OFC1_scan_out_0/n0#100
+ 73.858757	$poly
Rg424	FE_OFC1_scan_out_0/n0#100	FE_OFC1_scan_out_0/n0#101
+ 73.858757	$poly
Rg425	FE_OFC1_scan_out_0/n0#101	FE_OFC1_scan_out_0/n0#102
+ 73.858757	$poly
Rg426	FE_OFC1_scan_out_0/n0#102	FE_OFC1_scan_out_0/n0#103
+ 73.858757	$poly
Rg427	FE_OFC1_scan_out_0/n0#103	FE_OFC1_scan_out_0/n0#104
+ 73.858757	$poly
Rg428	FE_OFC1_scan_out_0/n0#104	FE_OFC1_scan_out_0/n0#105
+ 36.929379	$poly
Rg429	FE_OFC1_scan_out_0/n0#105	FE_OFC1_scan_out_0/n0#106
+ 36.929379	$poly
Rg430	FE_OFC1_scan_out_0/n0#106	FE_OFC1_scan_out_0/n0#107
+ 73.858757	$poly
Rg431	FE_OFC1_scan_out_0/n0#107	FE_OFC1_scan_out_0/n0#108
+ 73.858757	$poly
Rg432	FE_OFC1_scan_out_0/n0#108	FE_OFC1_scan_out_0/n0#109
+ 73.858757	$poly
Rg433	FE_OFC1_scan_out_0/n0#109	FE_OFC1_scan_out_0/n0#110
+ 73.858757	$poly
Rg434	FE_OFC1_scan_out_0/n0#110	FE_OFC1_scan_out_0/n0#111
+ 73.858757	$poly
Rg435	FE_OFC1_scan_out_0/n0#111	FE_OFC1_scan_out_0/n0#112
+ 73.858757	$poly
Rg436	FE_OFC1_scan_out_0/n0#112	FE_OFC1_scan_out_0/n0#113
+ 62.011227	$poly
Rg437	FE_OFC1_scan_out_0/n0#113	FE_OFC1_scan_out_0/n0	323.783325
+ $poly
Rg439	FE_OFC1_scan_out_0/n0#97	FE_OFC1_scan_out_0/n0#31
+ 323.783325	$poly
Rg441	FE_OFC1_scan_out_0/n0#98	FE_OFC1_scan_out_0/n0#29
+ 321.226410	$poly
Rg443	FE_OFC1_scan_out_0/n0#98	FE_OFC1_scan_out_0/n0#61
+ 313.193085	$poly
Rg445	FE_OFC1_scan_out_0/n0#99	FE_OFC1_scan_out_0/n0#27
+ 321.226410	$poly
Rg447	FE_OFC1_scan_out_0/n0#99	FE_OFC1_scan_out_0/n0#59
+ 313.193085	$poly
Rg449	FE_OFC1_scan_out_0/n0#100	FE_OFC1_scan_out_0/n0#25
+ 321.226410	$poly
Rg451	FE_OFC1_scan_out_0/n0#100	FE_OFC1_scan_out_0/n0#57
+ 313.193085	$poly
Rg453	FE_OFC1_scan_out_0/n0#101	FE_OFC1_scan_out_0/n0#23
+ 321.226410	$poly
Rg455	FE_OFC1_scan_out_0/n0#101	FE_OFC1_scan_out_0/n0#55
+ 313.193085	$poly
Rg457	FE_OFC1_scan_out_0/n0#102	FE_OFC1_scan_out_0/n0#21
+ 321.226410	$poly
Rg459	FE_OFC1_scan_out_0/n0#102	FE_OFC1_scan_out_0/n0#53
+ 313.193085	$poly
Rg461	FE_OFC1_scan_out_0/n0#103	FE_OFC1_scan_out_0/n0#19
+ 321.226410	$poly
Rg463	FE_OFC1_scan_out_0/n0#103	FE_OFC1_scan_out_0/n0#51
+ 313.193085	$poly
Rg465	FE_OFC1_scan_out_0/n0#104	FE_OFC1_scan_out_0/n0#17
+ 321.226410	$poly
Rg467	FE_OFC1_scan_out_0/n0#104	FE_OFC1_scan_out_0/n0#49
+ 313.193085	$poly
Rg469	FE_OFC1_scan_out_0/n0#106	FE_OFC1_scan_out_0/n0#15
+ 321.226410	$poly
Rg471	FE_OFC1_scan_out_0/n0#106	FE_OFC1_scan_out_0/n0#47
+ 313.193085	$poly
Rg473	FE_OFC1_scan_out_0/n0#107	FE_OFC1_scan_out_0/n0#13
+ 321.226410	$poly
Rg475	FE_OFC1_scan_out_0/n0#107	FE_OFC1_scan_out_0/n0#45
+ 313.193085	$poly
Rg477	FE_OFC1_scan_out_0/n0#108	FE_OFC1_scan_out_0/n0#11
+ 321.226410	$poly
Rg479	FE_OFC1_scan_out_0/n0#108	FE_OFC1_scan_out_0/n0#43
+ 313.193085	$poly
Rg481	FE_OFC1_scan_out_0/n0#109	FE_OFC1_scan_out_0/n0#9
+ 321.226410	$poly
Rg483	FE_OFC1_scan_out_0/n0#109	FE_OFC1_scan_out_0/n0#41
+ 313.193085	$poly
Rg485	FE_OFC1_scan_out_0/n0#110	FE_OFC1_scan_out_0/n0#7
+ 321.226410	$poly
Rg487	FE_OFC1_scan_out_0/n0#110	FE_OFC1_scan_out_0/n0#39
+ 313.193085	$poly
Rg489	FE_OFC1_scan_out_0/n0#111	FE_OFC1_scan_out_0/n0#5
+ 321.226410	$poly
Rg491	FE_OFC1_scan_out_0/n0#111	FE_OFC1_scan_out_0/n0#37
+ 313.193085	$poly
Rg493	FE_OFC1_scan_out_0/n0#112	FE_OFC1_scan_out_0/n0#3
+ 321.226410	$poly
Rg495	FE_OFC1_scan_out_0/n0#112	FE_OFC1_scan_out_0/n0#35
+ 313.193085	$poly
Rg497	FE_OFC1_scan_out_0/n0#113	FE_OFC1_scan_out_0/n0#33
+ 315.750000	$poly
Rg500	\mem_reg[12]/n30#3	\mem_reg[12]/n30#7	515.374878	$poly
Rg501	\mem_reg[12]/n30#7	\mem_reg[12]/n30	219.748169	$poly
Rg504	\mem_reg[4]/n30	\mem_reg[4]/n30#7	219.748169	$poly
Rg505	\mem_reg[4]/n30#7	\mem_reg[4]/n30#3	515.374878	$poly
Rg508	\mem_reg[1]/n30#3	\mem_reg[1]/n30#7	515.374878	$poly
Rg509	\mem_reg[1]/n30#7	\mem_reg[1]/n30	219.748169	$poly
Rg512	\mem_reg[3]/n30	\mem_reg[3]/n30#7	219.748169	$poly
Rg513	\mem_reg[3]/n30#7	\mem_reg[3]/n30#3	515.374878	$poly
Rg516	\mem_reg[5]/CKb#11	\mem_reg[5]/CKb#21	386.662750	$poly
Rg517	\mem_reg[5]/CKb#21	\mem_reg[5]/CKb#5	418.796082	$poly
Rg520	\mem_reg[7]/qbint	\mem_reg[7]/qbint#19	497.702423	$poly
Rg521	\mem_reg[7]/qbint#19	\mem_reg[7]/qbint#7	247.062408	$poly
Rg524	\mem_reg[5]/CKbb#3	\mem_reg[5]/CKbb#16	169.942413	$poly
Rg526	\mem_reg[14]/n30	\mem_reg[14]/n30#7	219.748169	$poly
Rg527	\mem_reg[14]/n30#7	\mem_reg[14]/n30#3	515.374878	$poly
Rg530	\mem_reg[12]/qbint#11	\mem_reg[12]/qbint#21	188.418182	$poly
Rg531	\mem_reg[12]/qbint#21	\mem_reg[12]/qbint#5	411.744843	$poly
Rg534	\mem_reg[4]/qbint#5	\mem_reg[4]/qbint#21	411.744843	$poly
Rg535	\mem_reg[4]/qbint#21	\mem_reg[4]/qbint#11	188.418182	$poly
Rg538	\mem_reg[1]/qbint#11	\mem_reg[1]/qbint#21	188.418182	$poly
Rg539	\mem_reg[1]/qbint#21	\mem_reg[1]/qbint#5	411.744843	$poly
Rg542	\mem_reg[3]/qbint#5	\mem_reg[3]/qbint#21	411.744843	$poly
Rg543	\mem_reg[3]/qbint#21	\mem_reg[3]/qbint#11	188.418182	$poly
Rg546	\mem_reg[5]/qbint#7	\mem_reg[5]/qbint#19	247.062408	$poly
Rg547	\mem_reg[5]/qbint#19	\mem_reg[5]/qbint	497.702423	$poly
Rg550	scan_en#101	scan_en#231	290.345947	$poly
Rg551	scan_en#231	scan_en#29	463.865936	$poly
Rg554	\mem_reg[14]/qbint#5	\mem_reg[14]/qbint#21	411.744843	$poly
Rg555	\mem_reg[14]/qbint#21	\mem_reg[14]/qbint#11	188.418182	$poly
Rg558	\mem_reg[7]/qbint#3	\mem_reg[7]/qbint#20	497.702423	$poly
Rg559	\mem_reg[7]/qbint#20	\mem_reg[7]/qbint#9	247.062408	$poly
Rg562	\mem_reg[12]/net367#3	\mem_reg[12]/net367#7	235.011505	$poly
Rg563	\mem_reg[12]/net367#7	\mem_reg[12]/net367	365.151520	$poly
Rg566	\mem_reg[4]/net367	\mem_reg[4]/net367#7	365.151520	$poly
Rg567	\mem_reg[4]/net367#7	\mem_reg[4]/net367#3	235.011505	$poly
Rg570	\mem_reg[1]/net367#3	\mem_reg[1]/net367#7	235.011505	$poly
Rg571	\mem_reg[1]/net367#7	\mem_reg[1]/net367	365.151520	$poly
Rg574	\mem_reg[3]/net367	\mem_reg[3]/net367#7	365.151520	$poly
Rg575	\mem_reg[3]/net367#7	\mem_reg[3]/net367#3	235.011505	$poly
Rg578	q_control#11	q_control#26	465.569092	$poly
Rg579	q_control#26	q_control#3	279.195740	$poly
Rg582	\mem_reg[5]/qbint#9	\mem_reg[5]/qbint#20	247.062408	$poly
Rg583	\mem_reg[5]/qbint#20	\mem_reg[5]/qbint#3	497.702423	$poly
Rg586	\mem_reg[7]/n30	\mem_reg[7]/n30#7	219.748169	$poly
Rg587	\mem_reg[7]/n30#7	\mem_reg[7]/n30#3	515.374878	$poly
Rg590	\mem_reg[14]/net367	\mem_reg[14]/net367#7	365.151520	$poly
Rg591	\mem_reg[14]/net367#7	\mem_reg[14]/net367#3	235.011505	$poly
Rg594	q_control_reg/SEb#3	q_control_reg/SEb#7	137.809082	$poly
Rg596	scan_en#103	scan_en#232	400.944214	$poly
Rg597	scan_en#232	scan_en#31	407.290009	$poly
Rg600	\mem_reg[5]/n30#3	\mem_reg[5]/n30#7	515.374878	$poly
Rg601	\mem_reg[5]/n30#7	\mem_reg[5]/n30	219.748169	$poly
Rg604	q_control_reg/SEb	q_control_reg/SEb#8	169.942413	$poly
Rg606	scan_en#105	scan_en#233	290.345947	$poly
Rg607	scan_en#233	scan_en#33	463.865936	$poly
Rg610	n_2	n_2#19	410.042603	$poly
Rg611	n_2#19	n_2#7	209.209274	$poly
Rg614	scan_en#35	scan_en#234	463.865906	$poly
Rg615	scan_en#234	scan_en#107	290.345947	$poly
Rg618	addr[0]#11	addr[0]#31	342.659058	$poly
Rg619	addr[0]#31	addr[0]#1	267.145752	$poly
Rg622	\mem_reg[7]/qbint#5	\mem_reg[7]/qbint#21	411.744843	$poly
Rg623	\mem_reg[7]/qbint#21	\mem_reg[7]/qbint#11	188.418182	$poly
Rg626	scan_in[0]#3	scan_in[0]#7	356.315765	$poly
Rg627	scan_in[0]#7	scan_in[0]#1	388.449066	$poly
Rg630	n_8	n_8#25	422.895935	$poly
Rg631	n_8#25	n_8#9	196.355942	$poly
Rg634	mem[6]#9	mem[6]#20	465.569092	$poly
Rg635	mem[6]#20	mem[6]#3	279.195740	$poly
Rg638	mem[1]#5	mem[1]#21	251.079086	$poly
Rg639	mem[1]#21	mem[1]#11	358.725739	$poly
Rg642	mem[11]	mem[11]#19	279.195740	$poly
Rg643	mem[11]#19	mem[11]#7	465.569092	$poly
Rg646	n_2#9	n_2#20	191.632416	$poly
Rg647	n_2#20	n_2#3	418.172424	$poly
Rg650	\mem_reg[5]/qbint#11	\mem_reg[5]/qbint#21	188.418182	$poly
Rg651	\mem_reg[5]/qbint#21	\mem_reg[5]/qbint#5	411.744843	$poly
Rg654	scan_clk#51	scan_clk#116	137.809082	$poly
Rg655	scan_clk#116	scan_clk#15	606.955750	$poly
Rg658	\mem_reg[7]/net367	\mem_reg[7]/net367#7	365.151520	$poly
Rg659	\mem_reg[7]/net367#7	\mem_reg[7]/net367#3	235.011505	$poly
Rg662	\mem_reg[6]/SEb#3	\mem_reg[6]/SEb#7	137.809082	$poly
Rg664	\mem_reg[11]/SEb#3	\mem_reg[11]/SEb#7	137.809082	$poly
Rg666	mem[3]#5	mem[3]#21	418.172424	$poly
Rg667	mem[3]#21	mem[3]#11	191.632416	$poly
Rg670	mem[12]#5	mem[12]#21	263.932404	$poly
Rg671	mem[12]#21	mem[12]#11	345.872437	$poly
Rg674	scan_en#109	scan_en#235	400.944214	$poly
Rg675	scan_en#235	scan_en#37	407.290009	$poly
Rg678	scan_en#111	scan_en#236	400.944214	$poly
Rg679	scan_en#236	scan_en#39	407.290009	$poly
Rg682	\mem_reg[5]/net367#3	\mem_reg[5]/net367#7	235.011505	$poly
Rg683	\mem_reg[5]/net367#7	\mem_reg[5]/net367	365.151520	$poly
Rg686	\mem_reg[6]/SEb	\mem_reg[6]/SEb#8	169.942413	$poly
Rg688	\mem_reg[11]/SEb	\mem_reg[11]/SEb#8	169.942413	$poly
Rg690	n_1	n_1#7	418.172424	$poly
Rg691	n_1#7	n_1#3	191.632416	$poly
Rg694	addr[1]#1	addr[1]#49	271.869263	$poly
Rg695	addr[1]#49	addr[1]#17	347.382599	$poly
Rg698	mem[4]#9	mem[4]#20	356.315765	$poly
Rg699	mem[4]#20	mem[4]#3	388.449066	$poly
Rg702	mem[9]	mem[9]#19	388.449097	$poly
Rg703	mem[9]#19	mem[9]#7	356.315735	$poly
Rg706	q_control_reg/CKbb	q_control_reg/CKbb#13	169.942413	$poly
Rg708	scan_en#41	scan_en#237	463.865906	$poly
Rg709	scan_en#237	scan_en#113	290.345947	$poly
Rg712	n_2#11	n_2#21	209.209274	$poly
Rg713	n_2#21	n_2#5	410.042603	$poly
Rg716	addr[0]#3	addr[0]#32	268.655945	$poly
Rg717	addr[0]#32	addr[0]#13	350.595947	$poly
Rg720	q_control_reg/CKb	q_control_reg/CKb#19	418.796082	$poly
Rg721	q_control_reg/CKb#19	q_control_reg/CKb#7	386.662750	$poly
Rg724	scan_clk#53	scan_clk#117	137.809082	$poly
Rg725	scan_clk#117	scan_clk#17	606.955750	$poly
Rg728	scan_clk#19	scan_clk#118	606.955750	$poly
Rg729	scan_clk#118	scan_clk#55	137.809082	$poly
Rg732	q_control_reg/CKbb#5	q_control_reg/CKbb#14	137.809082	$poly
Rg734	addr[1]#19	addr[1]#50	568.299255	$poly
Rg735	addr[1]#50	addr[1]#3	185.912598	$poly
Rg738	mem[13]	mem[13]#19	279.195740	$poly
Rg739	mem[13]#19	mem[13]#7	465.569092	$poly
Rg742	mem[9]#9	mem[9]#20	358.725739	$poly
Rg743	mem[9]#20	mem[9]#3	251.079086	$poly
Rg746	q_control_reg/mout#3	q_control_reg/mout#7	465.569092	$poly
Rg747	q_control_reg/mout#7	q_control_reg/mout	279.195740	$poly
Rg750	n_7	n_7#25	422.895935	$poly
Rg751	n_7#25	n_7#9	196.355942	$poly
Rg754	\mem_reg[13]/SEb#3	\mem_reg[13]/SEb#7	137.809082	$poly
Rg756	n_8#3	n_8#26	422.895935	$poly
Rg757	n_8#26	n_8#11	196.355942	$poly
Rg760	mem[11]#9	mem[11]#20	191.632416	$poly
Rg761	mem[11]#20	mem[11]#3	418.172424	$poly
Rg764	mem[5]#11	mem[5]#21	168.335754	$poly
Rg765	mem[5]#21	mem[5]#5	576.429077	$poly
Rg768	q_control_reg/CKb#9	q_control_reg/CKb#20	361.039276	$poly
Rg769	q_control_reg/CKb#20	q_control_reg/CKb#3	393.172607	$poly
Rg772	scan_en#115	scan_en#238	400.944214	$poly
Rg773	scan_en#238	scan_en#43	407.290009	$poly
Rg776	\mem_reg[6]/CKbb	\mem_reg[6]/CKbb#13	169.942413	$poly
Rg778	\mem_reg[11]/CKbb	\mem_reg[11]/CKbb#13	169.942413	$poly
Rg780	\mem_reg[13]/SEb	\mem_reg[13]/SEb#8	169.942413	$poly
Rg782	addr[1]#7	addr[1]#51	196.189072	$poly
Rg784	mem[4]#5	mem[4]#21	263.932404	$poly
Rg785	mem[4]#21	mem[4]#11	345.872437	$poly
Rg788	mem[14]#5	mem[14]#21	268.655945	$poly
Rg789	mem[14]#21	mem[14]#11	350.595947	$poly
Rg792	addr[1]#21	addr[1]#52	347.382599	$poly
Rg793	addr[1]#52	addr[1]#5	271.869263	$poly
Rg796	\mem_reg[6]/CKb	\mem_reg[6]/CKb#19	418.796082	$poly
Rg797	\mem_reg[6]/CKb#19	\mem_reg[6]/CKb#7	386.662750	$poly
Rg800	\mem_reg[11]/CKb	\mem_reg[11]/CKb#19	418.796082	$poly
Rg801	\mem_reg[11]/CKb#19	\mem_reg[11]/CKb#7	386.662750	$poly
Rg804	\mem_reg[6]/CKbb#5	\mem_reg[6]/CKbb#14	137.809082	$poly
Rg806	\mem_reg[11]/CKbb#5	\mem_reg[11]/CKbb#14	137.809082	$poly
Rg808	g520__5122/S0b	g520__5122/S0b#7	477.977783	$poly
Rg809	g520__5122/S0b#7	g520__5122/S0b#3	387.081696	$poly
Rg812	mem[15]	mem[15]#19	388.449097	$poly
Rg813	mem[15]#19	mem[15]#7	356.315735	$poly
Rg816	addr[1]#23	addr[1]#53	133.085556	$poly
Rg818	n_0	n_0#7	418.172424	$poly
Rg819	n_0#7	n_0#3	191.632416	$poly
Rg822	\mem_reg[6]/mout#3	\mem_reg[6]/mout#7	465.569092	$poly
Rg823	\mem_reg[6]/mout#7	\mem_reg[6]/mout	279.195740	$poly
Rg826	\mem_reg[11]/mout	\mem_reg[11]/mout#7	279.195740	$poly
Rg827	\mem_reg[11]/mout#7	\mem_reg[11]/mout#3	465.569092	$poly
Rg830	q_control_reg/n20#3	q_control_reg/n20#7	574.822449	$poly
Rg831	q_control_reg/n20#7	q_control_reg/n20	169.942413	$poly
Rg834	scan_clk#21	scan_clk#119	606.955750	$poly
Rg835	scan_clk#119	scan_clk#57	137.809082	$poly
Rg838	mem[7]#9	mem[7]#20	295.262421	$poly
Rg839	mem[7]#20	mem[7]#3	449.502411	$poly
Rg842	addr[0]#5	addr[0]#33	268.655945	$poly
Rg843	addr[0]#33	addr[0]#15	350.595947	$poly
Rg846	q_control_reg/CKbb#7	q_control_reg/CKbb#15	152.809082	$poly
Rg848	\mem_reg[6]/CKb#9	\mem_reg[6]/CKb#20	361.039276	$poly
Rg849	\mem_reg[6]/CKb#20	\mem_reg[6]/CKb#3	393.172607	$poly
Rg852	\mem_reg[11]/CKb#3	\mem_reg[11]/CKb#20	393.172607	$poly
Rg853	\mem_reg[11]/CKb#20	\mem_reg[11]/CKb#9	361.039276	$poly
Rg856	scan_en#117	scan_en#239	290.345947	$poly
Rg857	scan_en#239	scan_en#45	463.865936	$poly
Rg860	g520__5122/n0#3	g520__5122/n0#7	354.002197	$poly
Rg861	g520__5122/n0#7	g520__5122/n0	246.355560	$poly
Rg864	addr[0]#7	addr[0]#34	267.145752	$poly
Rg865	addr[0]#34	addr[0]#17	342.659088	$poly
Rg868	q_control_reg/CKb#11	q_control_reg/CKb#21	386.662750	$poly
Rg869	q_control_reg/CKb#21	q_control_reg/CKb#5	418.796082	$poly
Rg872	q_control_reg/CKbb#3	q_control_reg/CKbb#16	169.942413	$poly
Rg874	\mem_reg[13]/CKbb	\mem_reg[13]/CKbb#13	169.942413	$poly
Rg876	n_7#3	n_7#26	422.895935	$poly
Rg877	n_7#26	n_7#11	196.355942	$poly
Rg880	mem[9]#11	mem[9]#21	465.569092	$poly
Rg881	mem[9]#21	mem[9]#5	279.195740	$poly
Rg884	addr[1]#9	addr[1]#54	418.172424	$poly
Rg885	addr[1]#54	addr[1]#25	191.632416	$poly
Rg888	q_control_reg/qbint#7	q_control_reg/qbint#19	247.062408	$poly
Rg889	q_control_reg/qbint#19	q_control_reg/qbint	497.702423	$poly
Rg892	\mem_reg[13]/CKb	\mem_reg[13]/CKb#19	418.796082	$poly
Rg893	\mem_reg[13]/CKb#19	\mem_reg[13]/CKb#7	386.662750	$poly
Rg896	\mem_reg[6]/n20#3	\mem_reg[6]/n20#7	574.822449	$poly
Rg897	\mem_reg[6]/n20#7	\mem_reg[6]/n20	169.942413	$poly
Rg900	\mem_reg[11]/n20	\mem_reg[11]/n20#7	169.942413	$poly
Rg901	\mem_reg[11]/n20#7	\mem_reg[11]/n20#3	574.822449	$poly
Rg904	\mem_reg[13]/CKbb#5	\mem_reg[13]/CKbb#14	137.809082	$poly
Rg906	\mem_reg[9]/SEb#3	\mem_reg[9]/SEb#7	137.809082	$poly
Rg908	mem[6]#5	mem[6]#21	268.655945	$poly
Rg909	mem[6]#21	mem[6]#11	350.595947	$poly
Rg912	\mem_reg[6]/CKbb#7	\mem_reg[6]/CKbb#15	152.809082	$poly
Rg914	\mem_reg[11]/CKbb#7	\mem_reg[11]/CKbb#15	152.809082	$poly
Rg916	\mem_reg[13]/mout	\mem_reg[13]/mout#7	279.195740	$poly
Rg917	\mem_reg[13]/mout#7	\mem_reg[13]/mout#3	465.569092	$poly
Rg920	scan_en#119	scan_en#240	400.944214	$poly
Rg921	scan_en#240	scan_en#47	407.290009	$poly
Rg924	\mem_reg[9]/SEb	\mem_reg[9]/SEb#8	169.942413	$poly
Rg926	addr[1]#27	addr[1]#55	568.299255	$poly
Rg927	addr[1]#55	addr[1]#11	185.912598	$poly
Rg930	\mem_reg[6]/CKb#11	\mem_reg[6]/CKb#21	386.662750	$poly
Rg931	\mem_reg[6]/CKb#21	\mem_reg[6]/CKb#5	418.796082	$poly
Rg934	\mem_reg[11]/CKb#11	\mem_reg[11]/CKb#21	386.662750	$poly
Rg935	\mem_reg[11]/CKb#21	\mem_reg[11]/CKb#5	418.796082	$poly
Rg938	\mem_reg[6]/CKbb#3	\mem_reg[6]/CKbb#16	169.942413	$poly
Rg940	\mem_reg[11]/CKbb#3	\mem_reg[11]/CKbb#16	169.942413	$poly
Rg942	q_control_reg/qbint#9	q_control_reg/qbint#20	247.062408	$poly
Rg943	q_control_reg/qbint#20	q_control_reg/qbint#3	497.702423	$poly
Rg946	\mem_reg[13]/CKb#3	\mem_reg[13]/CKb#20	393.172607	$poly
Rg947	\mem_reg[13]/CKb#20	\mem_reg[13]/CKb#9	361.039276	$poly
Rg950	mem[10]#7	mem[10]#19	356.315765	$poly
Rg951	mem[10]#19	mem[10]	388.449066	$poly
Rg954	addr[2]#1	addr[2]#19	422.895935	$poly
Rg955	addr[2]#19	addr[2]#7	196.355942	$poly
Rg958	mem[13]#9	mem[13]#20	168.335754	$poly
Rg959	mem[13]#20	mem[13]#3	576.429077	$poly
Rg962	\mem_reg[6]/qbint#7	\mem_reg[6]/qbint#19	247.062408	$poly
Rg963	\mem_reg[6]/qbint#19	\mem_reg[6]/qbint	497.702423	$poly
Rg966	\mem_reg[11]/qbint	\mem_reg[11]/qbint#19	497.702423	$poly
Rg967	\mem_reg[11]/qbint#19	\mem_reg[11]/qbint#7	247.062408	$poly
Rg970	scan_clk#59	scan_clk#120	137.809082	$poly
Rg971	scan_clk#120	scan_clk#23	606.955750	$poly
Rg974	q_control_reg/n30#3	q_control_reg/n30#7	515.374878	$poly
Rg975	q_control_reg/n30#7	q_control_reg/n30	219.748169	$poly
Rg978	addr[1]#13	addr[1]#56	196.189072	$poly
Rg980	addr[1]#15	addr[1]#57	373.185730	$poly
Rg981	addr[1]#57	addr[1]#29	236.619080	$poly
Rg984	n_6	n_6#7	255.802612	$poly
Rg985	n_6#7	n_6#3	363.449280	$poly
Rg988	\mem_reg[13]/n20	\mem_reg[13]/n20#7	169.942413	$poly
Rg989	\mem_reg[13]/n20#7	\mem_reg[13]/n20#3	574.822449	$poly
Rg992	g519__1705/S0b	g519__1705/S0b#7	477.977783	$poly
Rg993	g519__1705/S0b#7	g519__1705/S0b#3	387.081696	$poly
Rg996	addr[1]#31	addr[1]#58	133.085556	$poly
Rg998	q_control_reg/qbint#11	q_control_reg/qbint#21	188.418182	$poly
Rg999	q_control_reg/qbint#21	q_control_reg/qbint#5	411.744843	$poly
Rg1002	\mem_reg[6]/qbint#9	\mem_reg[6]/qbint#20	247.062408	$poly
Rg1003	\mem_reg[6]/qbint#20	\mem_reg[6]/qbint#3	497.702423	$poly
Rg1006	\mem_reg[11]/qbint#3	\mem_reg[11]/qbint#20	497.702423	$poly
Rg1007	\mem_reg[11]/qbint#20	\mem_reg[11]/qbint#9	247.062408	$poly
Rg1010	\mem_reg[13]/CKbb#7	\mem_reg[13]/CKbb#15	152.809082	$poly
Rg1012	mem[15]#9	mem[15]#20	295.262421	$poly
Rg1013	mem[15]#20	mem[15]#3	449.502411	$poly
Rg1016	\mem_reg[9]/CKbb	\mem_reg[9]/CKbb#13	169.942413	$poly
Rg1018	n_12	n_12#19	422.895935	$poly
Rg1019	n_12#19	n_12#7	196.355942	$poly
Rg1022	\mem_reg[13]/CKb#11	\mem_reg[13]/CKb#21	386.662750	$poly
Rg1023	\mem_reg[13]/CKb#21	\mem_reg[13]/CKb#5	418.796082	$poly
Rg1026	\mem_reg[13]/CKbb#3	\mem_reg[13]/CKbb#16	169.942413	$poly
Rg1028	\mem_reg[9]/CKb	\mem_reg[9]/CKb#19	418.796082	$poly
Rg1029	\mem_reg[9]/CKb#19	\mem_reg[9]/CKb#7	386.662750	$poly
Rg1032	\mem_reg[9]/CKbb#5	\mem_reg[9]/CKbb#14	137.809082	$poly
Rg1034	\mem_reg[6]/n30#3	\mem_reg[6]/n30#7	515.374878	$poly
Rg1035	\mem_reg[6]/n30#7	\mem_reg[6]/n30	219.748169	$poly
Rg1038	\mem_reg[11]/n30	\mem_reg[11]/n30#7	219.748169	$poly
Rg1039	\mem_reg[11]/n30#7	\mem_reg[11]/n30#3	515.374878	$poly
Rg1042	q_control_reg/net367#3	q_control_reg/net367#7	235.011505	$poly
Rg1043	q_control_reg/net367#7	q_control_reg/net367	365.151520	$poly
Rg1046	g519__1705/n0#3	g519__1705/n0#7	354.002197	$poly
Rg1047	g519__1705/n0#7	g519__1705/n0	246.355560	$poly
Rg1050	n_10	n_10#7	251.079086	$poly
Rg1051	n_10#7	n_10#3	358.725739	$poly
Rg1054	addr[0]#9	addr[0]#35	418.172424	$poly
Rg1055	addr[0]#35	addr[0]#19	191.632416	$poly
Rg1058	\mem_reg[13]/qbint	\mem_reg[13]/qbint#19	497.702423	$poly
Rg1059	\mem_reg[13]/qbint#19	\mem_reg[13]/qbint#7	247.062408	$poly
Rg1062	\mem_reg[9]/mout#3	\mem_reg[9]/mout#7	465.569092	$poly
Rg1063	\mem_reg[9]/mout#7	\mem_reg[9]/mout	279.195740	$poly
Rg1066	n_4	n_4#7	418.172424	$poly
Rg1067	n_4#7	n_4#3	191.632416	$poly
Rg1070	n_12#3	n_12#20	267.145752	$poly
Rg1071	n_12#20	n_12#9	342.659088	$poly
Rg1074	\mem_reg[6]/qbint#11	\mem_reg[6]/qbint#21	188.418182	$poly
Rg1075	\mem_reg[6]/qbint#21	\mem_reg[6]/qbint#5	411.744843	$poly
Rg1078	\mem_reg[11]/qbint#5	\mem_reg[11]/qbint#21	411.744843	$poly
Rg1079	\mem_reg[11]/qbint#21	\mem_reg[11]/qbint#11	188.418182	$poly
Rg1082	\mem_reg[9]/CKb#9	\mem_reg[9]/CKb#20	361.039276	$poly
Rg1083	\mem_reg[9]/CKb#20	\mem_reg[9]/CKb#3	393.172607	$poly
Rg1086	q_control#13	q_control#27	290.345947	$poly
Rg1087	q_control#27	q_control#5	463.865936	$poly
Rg1090	n_11	n_11#13	255.802612	$poly
Rg1091	n_11#13	n_11#5	363.449280	$poly
Rg1094	\mem_reg[13]/qbint#3	\mem_reg[13]/qbint#20	497.702423	$poly
Rg1095	\mem_reg[13]/qbint#20	\mem_reg[13]/qbint#9	247.062408	$poly
Rg1098	\mem_reg[6]/net367#3	\mem_reg[6]/net367#7	235.011505	$poly
Rg1099	\mem_reg[6]/net367#7	\mem_reg[6]/net367	365.151520	$poly
Rg1102	\mem_reg[11]/net367	\mem_reg[11]/net367#7	365.151520	$poly
Rg1103	\mem_reg[11]/net367#7	\mem_reg[11]/net367#3	235.011505	$poly
Rg1106	n_8#13	n_8#27	209.209274	$poly
Rg1107	n_8#27	n_8#5	410.042603	$poly
Rg1110	n_15#5	n_15#13	465.569092	$poly
Rg1111	n_15#13	n_15	279.195740	$poly
Rg1114	\mem_reg[9]/n20#3	\mem_reg[9]/n20#7	574.822449	$poly
Rg1115	\mem_reg[9]/n20#7	\mem_reg[9]/n20	169.942413	$poly
Rg1118	\mem_reg[13]/n30	\mem_reg[13]/n30#7	219.748169	$poly
Rg1119	\mem_reg[13]/n30#7	\mem_reg[13]/n30#3	515.374878	$poly
Rg1122	q_reg_reg/SEb#3	q_reg_reg/SEb#7	137.809082	$poly
Rg1124	scan_en#49	scan_en#241	463.865906	$poly
Rg1125	scan_en#241	scan_en#121	290.345947	$poly
Rg1128	addr[2]#3	addr[2]#20	422.895935	$poly
Rg1129	addr[2]#20	addr[2]#9	196.355942	$poly
Rg1132	mem[0]#7	mem[0]#19	358.725739	$poly
Rg1133	mem[0]#19	mem[0]	251.079086	$poly
Rg1136	\mem_reg[9]/CKbb#7	\mem_reg[9]/CKbb#15	152.809082	$poly
Rg1138	q_control#15	q_control#28	400.944214	$poly
Rg1139	q_control#28	q_control#7	407.290009	$poly
Rg1142	q_reg_reg/SEb	q_reg_reg/SEb#8	169.942413	$poly
Rg1144	scan_en#123	scan_en#242	290.345947	$poly
Rg1145	scan_en#242	scan_en#51	463.865936	$poly
Rg1148	scan_en#53	scan_en#243	463.865906	$poly
Rg1149	scan_en#243	scan_en#125	290.345947	$poly
Rg1152	mem[2]#7	mem[2]#19	191.632416	$poly
Rg1153	mem[2]#19	mem[2]	418.172424	$poly
Rg1156	mem[8]	mem[8]#19	279.195740	$poly
Rg1157	mem[8]#19	mem[8]#7	465.569092	$poly
Rg1160	n_5	n_5#7	255.802612	$poly
Rg1161	n_5#7	n_5#3	363.449280	$poly
Rg1164	\mem_reg[13]/qbint#5	\mem_reg[13]/qbint#21	411.744843	$poly
Rg1165	\mem_reg[13]/qbint#21	\mem_reg[13]/qbint#11	188.418182	$poly
Rg1168	\mem_reg[9]/CKb#11	\mem_reg[9]/CKb#21	386.662750	$poly
Rg1169	\mem_reg[9]/CKb#21	\mem_reg[9]/CKb#5	418.796082	$poly
Rg1172	\mem_reg[9]/CKbb#3	\mem_reg[9]/CKbb#16	169.942413	$poly
Rg1174	x#5	x#13	356.315765	$poly
Rg1175	x#13	x#1	388.449066	$poly
Rg1178	\mem_reg[8]/SEb#3	\mem_reg[8]/SEb#7	137.809082	$poly
Rg1180	mem[15]#11	mem[15]#21	465.569092	$poly
Rg1181	mem[15]#21	mem[15]#5	279.195740	$poly
Rg1184	FE_OFN1_scan_out_0#9	FE_OFN1_scan_out_0#48	279.195740	$poly
Rg1185	FE_OFN1_scan_out_0#48	FE_OFN1_scan_out_0#23	465.569092	$poly
Rg1188	n_7#13	n_7#27	347.382599	$poly
Rg1189	n_7#27	n_7#5	271.869263	$poly
Rg1192	\mem_reg[9]/qbint#7	\mem_reg[9]/qbint#19	247.062408	$poly
Rg1193	\mem_reg[9]/qbint#19	\mem_reg[9]/qbint	497.702423	$poly
Rg1196	clk#3	clk#7	137.809082	$poly
Rg1197	clk#7	clk#1	606.955750	$poly
Rg1200	scan_en#127	scan_en#244	400.944214	$poly
Rg1201	scan_en#244	scan_en#55	407.290009	$poly
Rg1204	\mem_reg[13]/net367	\mem_reg[13]/net367#7	365.151520	$poly
Rg1205	\mem_reg[13]/net367#7	\mem_reg[13]/net367#3	235.011505	$poly
Rg1208	\mem_reg[8]/SEb	\mem_reg[8]/SEb#8	169.942413	$poly
Rg1210	\mem_reg[15]/SEb#3	\mem_reg[15]/SEb#7	137.809082	$poly
Rg1212	mux_control_reg/SEb#3	mux_control_reg/SEb#7	137.809082	$poly
Rg1214	n_12#5	n_12#21	422.895935	$poly
Rg1215	n_12#21	n_12#11	196.355942	$poly
Rg1218	scan_en#129	scan_en#245	400.944214	$poly
Rg1219	scan_en#245	scan_en#57	407.290009	$poly
Rg1222	scan_en#131	scan_en#246	400.944214	$poly
Rg1223	scan_en#246	scan_en#59	407.290009	$poly
Rg1226	mem[2]#3	mem[2]#20	388.449097	$poly
Rg1227	mem[2]#20	mem[2]#9	356.315735	$poly
Rg1230	\mem_reg[15]/SEb	\mem_reg[15]/SEb#8	169.942413	$poly
Rg1232	mux_control_reg/SEb	mux_control_reg/SEb#8	169.942413	$poly
Rg1234	n_9	n_9#7	251.079086	$poly
Rg1235	n_9#7	n_9#3	358.725739	$poly
Rg1238	\mem_reg[9]/qbint#9	\mem_reg[9]/qbint#20	247.062408	$poly
Rg1239	\mem_reg[9]/qbint#20	\mem_reg[9]/qbint#3	497.702423	$poly
Rg1242	scan_clk#25	scan_clk#121	606.955750	$poly
Rg1243	scan_clk#121	scan_clk#61	137.809082	$poly
Rg1246	mem[7]#11	mem[7]#21	356.315765	$poly
Rg1247	mem[7]#21	mem[7]#5	388.449066	$poly
Rg1250	mem[11]#5	mem[11]#21	388.449097	$poly
Rg1251	mem[11]#21	mem[11]#11	356.315735	$poly
Rg1254	q_reg_reg/CKbb	q_reg_reg/CKbb#13	169.942413	$poly
Rg1256	n_3	n_3#7	418.172424	$poly
Rg1257	n_3#7	n_3#3	191.632416	$poly
Rg1260	scan_en#61	scan_en#247	463.865906	$poly
Rg1261	scan_en#247	scan_en#133	290.345947	$poly
Rg1264	scan_en#135	scan_en#248	290.345947	$poly
Rg1265	scan_en#248	scan_en#63	463.865936	$poly
Rg1268	q_reg_reg/CKb	q_reg_reg/CKb#19	418.796082	$poly
Rg1269	q_reg_reg/CKb#19	q_reg_reg/CKb#7	386.662750	$poly
Rg1272	scan_clk#63	scan_clk#122	137.809082	$poly
Rg1273	scan_clk#122	scan_clk#27	606.955750	$poly
Rg1276	scan_clk#29	scan_clk#123	606.955750	$poly
Rg1277	scan_clk#123	scan_clk#65	137.809082	$poly
Rg1280	q_reg_reg/CKbb#5	q_reg_reg/CKbb#14	137.809082	$poly
Rg1282	\mem_reg[9]/n30#3	\mem_reg[9]/n30#7	515.374878	$poly
Rg1283	\mem_reg[9]/n30#7	\mem_reg[9]/n30	219.748169	$poly
Rg1286	mem[0]#3	mem[0]#20	279.195740	$poly
Rg1287	mem[0]#20	mem[0]#9	465.569092	$poly
Rg1290	mem[2]#11	mem[2]#21	465.569092	$poly
Rg1291	mem[2]#21	mem[2]#5	279.195740	$poly
Rg1294	n_11#3	n_11#14	255.802612	$poly
Rg1295	n_11#14	n_11#7	363.449280	$poly
Rg1298	q_reg_reg/mout#3	q_reg_reg/mout#7	465.569092	$poly
Rg1299	q_reg_reg/mout#7	q_reg_reg/mout	279.195740	$poly
Rg1302	\mem_reg[8]/CKbb	\mem_reg[8]/CKbb#13	169.942413	$poly
Rg1304	\mem_reg[0]/SEb#3	\mem_reg[0]/SEb#7	137.809082	$poly
Rg1306	\mem_reg[2]/SEb#3	\mem_reg[2]/SEb#7	137.809082	$poly
Rg1308	\mem_reg[9]/qbint#11	\mem_reg[9]/qbint#21	188.418182	$poly
Rg1309	\mem_reg[9]/qbint#21	\mem_reg[9]/qbint#5	411.744843	$poly
Rg1312	\mem_reg[8]/CKb	\mem_reg[8]/CKb#19	418.796082	$poly
Rg1313	\mem_reg[8]/CKb#19	\mem_reg[8]/CKb#7	386.662750	$poly
Rg1316	\mem_reg[8]/CKbb#5	\mem_reg[8]/CKbb#14	137.809082	$poly
Rg1318	q_reg_reg/CKb#9	q_reg_reg/CKb#20	361.039276	$poly
Rg1319	q_reg_reg/CKb#20	q_reg_reg/CKb#3	393.172607	$poly
Rg1322	scan_en#137	scan_en#249	400.944214	$poly
Rg1323	scan_en#249	scan_en#65	407.290009	$poly
Rg1326	scan_en#139	scan_en#250	400.944214	$poly
Rg1327	scan_en#250	scan_en#67	407.290009	$poly
Rg1330	\mem_reg[15]/CKbb	\mem_reg[15]/CKbb#13	169.942413	$poly
Rg1332	mux_control_reg/CKbb	mux_control_reg/CKbb#13	169.942413
+ $poly
Rg1334	\mem_reg[0]/SEb	\mem_reg[0]/SEb#8	169.942413	$poly
Rg1336	\mem_reg[2]/SEb	\mem_reg[2]/SEb#8	169.942413	$poly
Rg1338	scan_en#69	scan_en#251	463.865906	$poly
Rg1339	scan_en#251	scan_en#141	290.345947	$poly
Rg1342	\mem_reg[15]/CKb	\mem_reg[15]/CKb#19	418.796082	$poly
Rg1343	\mem_reg[15]/CKb#19	\mem_reg[15]/CKb#7	386.662750	$poly
Rg1346	mux_control_reg/CKb	mux_control_reg/CKb#19	418.796082	$poly
Rg1347	mux_control_reg/CKb#19	mux_control_reg/CKb#7	386.662750	$poly
Rg1350	\mem_reg[8]/mout	\mem_reg[8]/mout#7	279.195740	$poly
Rg1351	\mem_reg[8]/mout#7	\mem_reg[8]/mout#3	465.569092	$poly
Rg1354	\mem_reg[15]/CKbb#5	\mem_reg[15]/CKbb#14	137.809082	$poly
Rg1356	mux_control_reg/CKbb#5	mux_control_reg/CKbb#14	137.809082
+ $poly
Rg1358	\mem_reg[9]/net367#3	\mem_reg[9]/net367#7	235.011505	$poly
Rg1359	\mem_reg[9]/net367#7	\mem_reg[9]/net367	365.151520	$poly
Rg1362	mem[13]#5	mem[13]#21	388.449097	$poly
Rg1363	mem[13]#21	mem[13]#11	356.315735	$poly
Rg1366	mem[0]#11	mem[0]#21	356.315765	$poly
Rg1367	mem[0]#21	mem[0]#5	388.449066	$poly
Rg1370	mem[10]#3	mem[10]#20	279.195740	$poly
Rg1371	mem[10]#20	mem[10]#9	465.569092	$poly
Rg1374	\mem_reg[15]/mout#3	\mem_reg[15]/mout#7	465.569092	$poly
Rg1375	\mem_reg[15]/mout#7	\mem_reg[15]/mout	279.195740	$poly
Rg1378	mux_control_reg/mout	mux_control_reg/mout#7	279.195740	$poly
Rg1379	mux_control_reg/mout#7	mux_control_reg/mout#3	465.569092	$poly
Rg1382	q_reg_reg/n20#3	q_reg_reg/n20#7	574.822449	$poly
Rg1383	q_reg_reg/n20#7	q_reg_reg/n20	169.942413	$poly
Rg1386	scan_clk#31	scan_clk#124	606.955750	$poly
Rg1387	scan_clk#124	scan_clk#67	137.809082	$poly
Rg1390	scan_clk#69	scan_clk#125	137.809082	$poly
Rg1391	scan_clk#125	scan_clk#33	606.955750	$poly
Rg1394	\mem_reg[8]/CKb#3	\mem_reg[8]/CKb#20	393.172607	$poly
Rg1395	\mem_reg[8]/CKb#20	\mem_reg[8]/CKb#9	361.039276	$poly
Rg1398	\mem_reg[10]/SEb#3	\mem_reg[10]/SEb#7	137.809082	$poly
Rg1400	q_reg_reg/CKbb#7	q_reg_reg/CKbb#15	152.809082	$poly
Rg1402	\mem_reg[15]/CKb#9	\mem_reg[15]/CKb#20	361.039276	$poly
Rg1403	\mem_reg[15]/CKb#20	\mem_reg[15]/CKb#3	393.172607	$poly
Rg1406	mux_control_reg/CKb#3	mux_control_reg/CKb#20	393.172607	$poly
Rg1407	mux_control_reg/CKb#20	mux_control_reg/CKb#9	361.039276	$poly
Rg1410	scan_en#143	scan_en#252	400.944214	$poly
Rg1411	scan_en#252	scan_en#71	407.290009	$poly
Rg1414	\mem_reg[10]/SEb	\mem_reg[10]/SEb#8	169.942413	$poly
Rg1416	q_reg_reg/CKb#11	q_reg_reg/CKb#21	386.662750	$poly
Rg1417	q_reg_reg/CKb#21	q_reg_reg/CKb#5	418.796082	$poly
Rg1420	q_reg_reg/CKbb#3	q_reg_reg/CKbb#16	169.942413	$poly
Rg1422	\mem_reg[8]/n20	\mem_reg[8]/n20#7	169.942413	$poly
Rg1423	\mem_reg[8]/n20#7	\mem_reg[8]/n20#3	574.822449	$poly
Rg1426	\mem_reg[0]/CKbb	\mem_reg[0]/CKbb#13	169.942413	$poly
Rg1428	\mem_reg[2]/CKbb	\mem_reg[2]/CKbb#13	169.942413	$poly
Rg1430	mem[8]#3	mem[8]#20	388.449097	$poly
Rg1431	mem[8]#20	mem[8]#9	356.315735	$poly
Rg1434	q_reg_reg/qbint#7	q_reg_reg/qbint#19	247.062408	$poly
Rg1435	q_reg_reg/qbint#19	q_reg_reg/qbint	497.702423	$poly
Rg1438	\mem_reg[8]/CKbb#7	\mem_reg[8]/CKbb#15	152.809082	$poly
Rg1440	\mem_reg[0]/CKb	\mem_reg[0]/CKb#19	418.796082	$poly
Rg1441	\mem_reg[0]/CKb#19	\mem_reg[0]/CKb#7	386.662750	$poly
Rg1444	\mem_reg[2]/CKb	\mem_reg[2]/CKb#19	418.796082	$poly
Rg1445	\mem_reg[2]/CKb#19	\mem_reg[2]/CKb#7	386.662750	$poly
Rg1448	\mem_reg[15]/n20#3	\mem_reg[15]/n20#7	574.822449	$poly
Rg1449	\mem_reg[15]/n20#7	\mem_reg[15]/n20	169.942413	$poly
Rg1452	mux_control_reg/n20	mux_control_reg/n20#7	169.942413	$poly
Rg1453	mux_control_reg/n20#7	mux_control_reg/n20#3	574.822449	$poly
Rg1456	\mem_reg[0]/CKbb#5	\mem_reg[0]/CKbb#14	137.809082	$poly
Rg1458	\mem_reg[2]/CKbb#5	\mem_reg[2]/CKbb#14	137.809082	$poly
Rg1460	scan_clk#35	scan_clk#126	606.955750	$poly
Rg1461	scan_clk#126	scan_clk#71	137.809082	$poly
Rg1464	\mem_reg[8]/CKb#11	\mem_reg[8]/CKb#21	386.662750	$poly
Rg1465	\mem_reg[8]/CKb#21	\mem_reg[8]/CKb#5	418.796082	$poly
Rg1468	\mem_reg[8]/CKbb#3	\mem_reg[8]/CKbb#16	169.942413	$poly
Rg1470	\mem_reg[15]/CKbb#7	\mem_reg[15]/CKbb#15	152.809082	$poly
Rg1472	mux_control_reg/CKbb#7	mux_control_reg/CKbb#15	152.809082
+ $poly
Rg1474	\mem_reg[0]/mout	\mem_reg[0]/mout#7	279.195740	$poly
Rg1475	\mem_reg[0]/mout#7	\mem_reg[0]/mout#3	465.569092	$poly
Rg1478	\mem_reg[2]/mout#3	\mem_reg[2]/mout#7	465.569092	$poly
Rg1479	\mem_reg[2]/mout#7	\mem_reg[2]/mout	279.195740	$poly
Rg1482	FE_OFN0_q#15	FE_OFN0_q#25	206.496658	$poly
Rg1483	FE_OFN0_q#25	FE_OFN0_q#26	62.011234	$poly
Rg1484	FE_OFN0_q#26	FE_OFN0_q#27	36.929379	$poly
Rg1485	FE_OFN0_q#27	FE_OFN0_q#28	36.929379	$poly
Rg1486	FE_OFN0_q#28	FE_OFN0_q#29	62.011227	$poly
Rg1487	FE_OFN0_q#29	FE_OFN0_q	433.036652	$poly
Rg1489	FE_OFN0_q#25	FE_OFN0_q#7	433.036652	$poly
Rg1491	FE_OFN0_q#26	FE_OFN0_q#5	430.479736	$poly
Rg1493	FE_OFN0_q#26	FE_OFN0_q#13	203.939743	$poly
Rg1495	FE_OFN0_q#28	FE_OFN0_q#3	430.479736	$poly
Rg1497	FE_OFN0_q#28	FE_OFN0_q#11	203.939743	$poly
Rg1499	FE_OFN0_q#29	FE_OFN0_q#9	206.496658	$poly
Rg1502	\mem_reg[15]/CKb#11	\mem_reg[15]/CKb#21	386.662750	$poly
Rg1503	\mem_reg[15]/CKb#21	\mem_reg[15]/CKb#5	418.796082	$poly
Rg1506	mux_control_reg/CKb#11	mux_control_reg/CKb#21	386.662750	$poly
Rg1507	mux_control_reg/CKb#21	mux_control_reg/CKb#5	418.796082	$poly
Rg1510	\mem_reg[8]/qbint	\mem_reg[8]/qbint#19	497.702423	$poly
Rg1511	\mem_reg[8]/qbint#19	\mem_reg[8]/qbint#7	247.062408	$poly
Rg1514	\mem_reg[15]/CKbb#3	\mem_reg[15]/CKbb#16	169.942413	$poly
Rg1516	mux_control_reg/CKbb#3	mux_control_reg/CKbb#16	169.942413
+ $poly
Rg1518	q_reg_reg/qbint#9	q_reg_reg/qbint#20	247.062408	$poly
Rg1519	q_reg_reg/qbint#20	q_reg_reg/qbint#3	497.702423	$poly
Rg1522	\mem_reg[0]/CKb#3	\mem_reg[0]/CKb#20	393.172607	$poly
Rg1523	\mem_reg[0]/CKb#20	\mem_reg[0]/CKb#9	361.039276	$poly
Rg1526	\mem_reg[2]/CKb#9	\mem_reg[2]/CKb#20	361.039276	$poly
Rg1527	\mem_reg[2]/CKb#20	\mem_reg[2]/CKb#3	393.172607	$poly
Rg1530	\mem_reg[10]/CKbb	\mem_reg[10]/CKbb#13	169.942413	$poly
Rg1532	\mem_reg[15]/qbint#7	\mem_reg[15]/qbint#19	247.062408	$poly
Rg1533	\mem_reg[15]/qbint#19	\mem_reg[15]/qbint	497.702423	$poly
Rg1536	mux_control_reg/qbint	mux_control_reg/qbint#19	497.702423
+ $poly
Rg1537	mux_control_reg/qbint#19	mux_control_reg/qbint#7
+ 247.062408	$poly
Rg1540	\mem_reg[10]/CKb	\mem_reg[10]/CKb#19	418.796082	$poly
Rg1541	\mem_reg[10]/CKb#19	\mem_reg[10]/CKb#7	386.662750	$poly
Rg1544	\mem_reg[10]/CKbb#5	\mem_reg[10]/CKbb#14	137.809082	$poly
Rg1546	q_reg_reg/n30#3	q_reg_reg/n30#7	515.374878	$poly
Rg1547	q_reg_reg/n30#7	q_reg_reg/n30	219.748169	$poly
Rg1550	\mem_reg[8]/qbint#3	\mem_reg[8]/qbint#20	497.702423	$poly
Rg1551	\mem_reg[8]/qbint#20	\mem_reg[8]/qbint#9	247.062408	$poly
Rg1554	\mem_reg[0]/n20	\mem_reg[0]/n20#7	169.942413	$poly
Rg1555	\mem_reg[0]/n20#7	\mem_reg[0]/n20#3	574.822449	$poly
Rg1558	\mem_reg[2]/n20#3	\mem_reg[2]/n20#7	574.822449	$poly
Rg1559	\mem_reg[2]/n20#7	\mem_reg[2]/n20	169.942413	$poly
Rg1562	\mem_reg[10]/mout	\mem_reg[10]/mout#7	279.195740	$poly
Rg1563	\mem_reg[10]/mout#7	\mem_reg[10]/mout#3	465.569092	$poly
Rg1566	q_reg_reg/qbint#11	q_reg_reg/qbint#21	188.418182	$poly
Rg1567	q_reg_reg/qbint#21	q_reg_reg/qbint#5	411.744843	$poly
Rg1570	\mem_reg[15]/qbint#9	\mem_reg[15]/qbint#20	247.062408	$poly
Rg1571	\mem_reg[15]/qbint#20	\mem_reg[15]/qbint#3	497.702423	$poly
Rg1574	mux_control_reg/qbint#3	mux_control_reg/qbint#20
+ 497.702423	$poly
Rg1575	mux_control_reg/qbint#20	mux_control_reg/qbint#9
+ 247.062408	$poly
Rg1578	\mem_reg[0]/CKbb#7	\mem_reg[0]/CKbb#15	152.809082	$poly
Rg1580	\mem_reg[2]/CKbb#7	\mem_reg[2]/CKbb#15	152.809082	$poly
Rg1582	\mem_reg[10]/CKb#3	\mem_reg[10]/CKb#20	393.172607	$poly
Rg1583	\mem_reg[10]/CKb#20	\mem_reg[10]/CKb#9	361.039276	$poly
Rg1586	\mem_reg[8]/n30	\mem_reg[8]/n30#7	219.748169	$poly
Rg1587	\mem_reg[8]/n30#7	\mem_reg[8]/n30#3	515.374878	$poly
Rg1590	\mem_reg[0]/CKb#11	\mem_reg[0]/CKb#21	386.662750	$poly
Rg1591	\mem_reg[0]/CKb#21	\mem_reg[0]/CKb#5	418.796082	$poly
Rg1594	\mem_reg[2]/CKb#11	\mem_reg[2]/CKb#21	386.662750	$poly
Rg1595	\mem_reg[2]/CKb#21	\mem_reg[2]/CKb#5	418.796082	$poly
Rg1598	\mem_reg[0]/CKbb#3	\mem_reg[0]/CKbb#16	169.942413	$poly
Rg1600	\mem_reg[2]/CKbb#3	\mem_reg[2]/CKbb#16	169.942413	$poly
Rg1602	\mem_reg[15]/n30#3	\mem_reg[15]/n30#7	515.374878	$poly
Rg1603	\mem_reg[15]/n30#7	\mem_reg[15]/n30	219.748169	$poly
Rg1606	mux_control_reg/n30	mux_control_reg/n30#7	219.748169	$poly
Rg1607	mux_control_reg/n30#7	mux_control_reg/n30#3	515.374878	$poly
Rg1610	q_reg_reg/net367#3	q_reg_reg/net367#7	235.011505	$poly
Rg1611	q_reg_reg/net367#7	q_reg_reg/net367	365.151520	$poly
Rg1614	\mem_reg[8]/qbint#5	\mem_reg[8]/qbint#21	411.744843	$poly
Rg1615	\mem_reg[8]/qbint#21	\mem_reg[8]/qbint#11	188.418182	$poly
Rg1618	\mem_reg[0]/qbint	\mem_reg[0]/qbint#19	497.702423	$poly
Rg1619	\mem_reg[0]/qbint#19	\mem_reg[0]/qbint#7	247.062408	$poly
Rg1622	\mem_reg[2]/qbint#7	\mem_reg[2]/qbint#19	247.062408	$poly
Rg1623	\mem_reg[2]/qbint#19	\mem_reg[2]/qbint	497.702423	$poly
Rg1626	\mem_reg[10]/n20	\mem_reg[10]/n20#7	169.942413	$poly
Rg1627	\mem_reg[10]/n20#7	\mem_reg[10]/n20#3	574.822449	$poly
Rg1630	\mem_reg[15]/qbint#11	\mem_reg[15]/qbint#21	188.418182	$poly
Rg1631	\mem_reg[15]/qbint#21	\mem_reg[15]/qbint#5	411.744843	$poly
Rg1634	mux_control_reg/qbint#5	mux_control_reg/qbint#21
+ 411.744843	$poly
Rg1635	mux_control_reg/qbint#21	mux_control_reg/qbint#11
+ 188.418182	$poly
Rg1638	\mem_reg[10]/CKbb#7	\mem_reg[10]/CKbb#15	152.809082	$poly
Rg1640	\mem_reg[8]/net367	\mem_reg[8]/net367#7	365.151520	$poly
Rg1641	\mem_reg[8]/net367#7	\mem_reg[8]/net367#3	235.011505	$poly
Rg1644	FE_OFN1_scan_out_0#25	FE_OFN1_scan_out_0#49	334.567383	$poly
Rg1645	FE_OFN1_scan_out_0#49	FE_OFN1_scan_out_0#11	437.394043	$poly
Rg1648	\mem_reg[10]/CKb#11	\mem_reg[10]/CKb#21	386.662750	$poly
Rg1649	\mem_reg[10]/CKb#21	\mem_reg[10]/CKb#5	418.796082	$poly
Rg1652	\mem_reg[0]/qbint#3	\mem_reg[0]/qbint#20	497.702423	$poly
Rg1653	\mem_reg[0]/qbint#20	\mem_reg[0]/qbint#9	247.062408	$poly
Rg1656	\mem_reg[2]/qbint#9	\mem_reg[2]/qbint#20	247.062408	$poly
Rg1657	\mem_reg[2]/qbint#20	\mem_reg[2]/qbint#3	497.702423	$poly
Rg1660	\mem_reg[10]/CKbb#3	\mem_reg[10]/CKbb#16	169.942413	$poly
Rg1662	\mem_reg[15]/net367#3	\mem_reg[15]/net367#7	235.011505	$poly
Rg1663	\mem_reg[15]/net367#7	\mem_reg[15]/net367	365.151520	$poly
Rg1666	mux_control_reg/net367	mux_control_reg/net367#7	365.151520
+ $poly
Rg1667	mux_control_reg/net367#7	mux_control_reg/net367#3
+ 235.011505	$poly
Rg1670	n_15#7	n_15#14	216.594559	$poly
Rg1671	n_15#14	n_15#3	459.523346	$poly
Rg1674	\mem_reg[10]/qbint	\mem_reg[10]/qbint#19	497.702423	$poly
Rg1675	\mem_reg[10]/qbint#19	\mem_reg[10]/qbint#7	247.062408	$poly
Rg1678	\mem_reg[0]/n30	\mem_reg[0]/n30#7	219.748169	$poly
Rg1679	\mem_reg[0]/n30#7	\mem_reg[0]/n30#3	515.374878	$poly
Rg1682	\mem_reg[2]/n30#3	\mem_reg[2]/n30#7	515.374878	$poly
Rg1683	\mem_reg[2]/n30#7	\mem_reg[2]/n30	219.748169	$poly
Rg1686	addr[3]#1	addr[3]#13	185.912613	$poly
Rg1687	addr[3]#13	addr[3]#5	568.299255	$poly
Rg1690	g493__2398/S0b#3	g493__2398/S0b#7	165.925751	$poly
Rg1692	FE_OFN1_scan_out_0#27	FE_OFN1_scan_out_0#50	467.157410	$poly
Rg1693	FE_OFN1_scan_out_0#50	FE_OFN1_scan_out_0#13	326.456696	$poly
Rg1696	n_8#15	n_8#28	209.209274	$poly
Rg1697	n_8#28	n_8#7	410.042603	$poly
Rg1700	n_13	n_13#7	576.429077	$poly
Rg1701	n_13#7	n_13#3	168.335754	$poly
Rg1704	g493__2398/S0b	g493__2398/S0b#8	229.929077	$poly
Rg1706	addr[2]#5	addr[2]#21	373.185730	$poly
Rg1707	addr[2]#21	addr[2]#11	236.619080	$poly
Rg1710	\mem_reg[0]/qbint#5	\mem_reg[0]/qbint#21	411.744843	$poly
Rg1711	\mem_reg[0]/qbint#21	\mem_reg[0]/qbint#11	188.418182	$poly
Rg1714	\mem_reg[2]/qbint#11	\mem_reg[2]/qbint#21	188.418182	$poly
Rg1715	\mem_reg[2]/qbint#21	\mem_reg[2]/qbint#5	411.744843	$poly
Rg1718	\mem_reg[10]/qbint#3	\mem_reg[10]/qbint#20	497.702423	$poly
Rg1719	\mem_reg[10]/qbint#20	\mem_reg[10]/qbint#9	247.062408	$poly
Rg1722	addr[3]#3	addr[3]#14	196.189072	$poly
Rg1724	x#7	x#14	217.339081	$poly
Rg1725	x#14	x#3	454.322418	$poly
Rg1728	mem[8]#11	mem[8]#21	358.725739	$poly
Rg1729	mem[8]#21	mem[8]#5	251.079086	$poly
Rg1732	g494__5107/S0b	g494__5107/S0b#7	477.977783	$poly
Rg1733	g494__5107/S0b#7	g494__5107/S0b#3	387.081696	$poly
Rg1736	\mem_reg[0]/net367	\mem_reg[0]/net367#7	365.151520	$poly
Rg1737	\mem_reg[0]/net367#7	\mem_reg[0]/net367#3	235.011505	$poly
Rg1740	\mem_reg[2]/net367#3	\mem_reg[2]/net367#7	235.011505	$poly
Rg1741	\mem_reg[2]/net367#7	\mem_reg[2]/net367	365.151520	$poly
Rg1744	\mem_reg[10]/n30	\mem_reg[10]/n30#7	219.748169	$poly
Rg1745	\mem_reg[10]/n30#7	\mem_reg[10]/n30#3	515.374878	$poly
Rg1748	addr[3]#7	addr[3]#15	133.085556	$poly
Rg1750	mem[10]#11	mem[10]#21	191.632416	$poly
Rg1751	mem[10]#21	mem[10]#5	418.172424	$poly
Rg1754	n_14	n_14#7	449.502411	$poly
Rg1755	n_14#7	n_14#3	295.262421	$poly
Rg1758	n_7#15	n_7#28	347.382599	$poly
Rg1759	n_7#28	n_7#7	271.869263	$poly
Rg1762	\mem_reg[10]/qbint#5	\mem_reg[10]/qbint#21	411.744843	$poly
Rg1763	\mem_reg[10]/qbint#21	\mem_reg[10]/qbint#11	188.418182	$poly
Rg1766	g494__5107/n0	g494__5107/n0#7	246.355560	$poly
Rg1767	g494__5107/n0#7	g494__5107/n0#3	354.002197	$poly
Rg1770	g493__2398/n0#15	g493__2398/n0#25	373.589996	$poly
Rg1771	g493__2398/n0#25	g493__2398/n0#26	62.011234	$poly
Rg1772	g493__2398/n0#26	g493__2398/n0#27	36.929379	$poly
Rg1773	g493__2398/n0#27	g493__2398/n0#28	36.929379	$poly
Rg1774	g493__2398/n0#28	g493__2398/n0#29	62.011227	$poly
Rg1775	g493__2398/n0#29	g493__2398/n0	265.943329	$poly
Rg1777	g493__2398/n0#25	g493__2398/n0#7	265.943329	$poly
Rg1779	g493__2398/n0#26	g493__2398/n0#5	263.386414	$poly
Rg1781	g493__2398/n0#26	g493__2398/n0#13	371.033081	$poly
Rg1783	g493__2398/n0#28	g493__2398/n0#3	263.386414	$poly
Rg1785	g493__2398/n0#28	g493__2398/n0#11	371.033081	$poly
Rg1787	g493__2398/n0#29	g493__2398/n0#9	373.589996	$poly
Rg1790	FE_OFC0_q/n0#63	FE_OFC0_q/n0#97	315.750000	$poly
Rg1791	FE_OFC0_q/n0#97	FE_OFC0_q/n0#98	62.011234	$poly
Rg1792	FE_OFC0_q/n0#98	FE_OFC0_q/n0#99	73.858757	$poly
Rg1793	FE_OFC0_q/n0#99	FE_OFC0_q/n0#100	73.858757	$poly
Rg1794	FE_OFC0_q/n0#100	FE_OFC0_q/n0#101	73.858757	$poly
Rg1795	FE_OFC0_q/n0#101	FE_OFC0_q/n0#102	73.858757	$poly
Rg1796	FE_OFC0_q/n0#102	FE_OFC0_q/n0#103	73.858757	$poly
Rg1797	FE_OFC0_q/n0#103	FE_OFC0_q/n0#104	73.858757	$poly
Rg1798	FE_OFC0_q/n0#104	FE_OFC0_q/n0#105	36.929379	$poly
Rg1799	FE_OFC0_q/n0#105	FE_OFC0_q/n0#106	36.929379	$poly
Rg1800	FE_OFC0_q/n0#106	FE_OFC0_q/n0#107	73.858757	$poly
Rg1801	FE_OFC0_q/n0#107	FE_OFC0_q/n0#108	73.858757	$poly
Rg1802	FE_OFC0_q/n0#108	FE_OFC0_q/n0#109	73.858757	$poly
Rg1803	FE_OFC0_q/n0#109	FE_OFC0_q/n0#110	73.858757	$poly
Rg1804	FE_OFC0_q/n0#110	FE_OFC0_q/n0#111	73.858757	$poly
Rg1805	FE_OFC0_q/n0#111	FE_OFC0_q/n0#112	73.858757	$poly
Rg1806	FE_OFC0_q/n0#112	FE_OFC0_q/n0#113	62.011227	$poly
Rg1807	FE_OFC0_q/n0#113	FE_OFC0_q/n0	323.783325	$poly
Rg1809	FE_OFC0_q/n0#97	FE_OFC0_q/n0#31	323.783325	$poly
Rg1811	FE_OFC0_q/n0#98	FE_OFC0_q/n0#29	321.226410	$poly
Rg1813	FE_OFC0_q/n0#98	FE_OFC0_q/n0#61	313.193085	$poly
Rg1815	FE_OFC0_q/n0#99	FE_OFC0_q/n0#27	321.226410	$poly
Rg1817	FE_OFC0_q/n0#99	FE_OFC0_q/n0#59	313.193085	$poly
Rg1819	FE_OFC0_q/n0#100	FE_OFC0_q/n0#25	321.226410	$poly
Rg1821	FE_OFC0_q/n0#100	FE_OFC0_q/n0#57	313.193085	$poly
Rg1823	FE_OFC0_q/n0#101	FE_OFC0_q/n0#23	321.226410	$poly
Rg1825	FE_OFC0_q/n0#101	FE_OFC0_q/n0#55	313.193085	$poly
Rg1827	FE_OFC0_q/n0#102	FE_OFC0_q/n0#21	321.226410	$poly
Rg1829	FE_OFC0_q/n0#102	FE_OFC0_q/n0#53	313.193085	$poly
Rg1831	FE_OFC0_q/n0#103	FE_OFC0_q/n0#19	321.226410	$poly
Rg1833	FE_OFC0_q/n0#103	FE_OFC0_q/n0#51	313.193085	$poly
Rg1835	FE_OFC0_q/n0#104	FE_OFC0_q/n0#17	321.226410	$poly
Rg1837	FE_OFC0_q/n0#104	FE_OFC0_q/n0#49	313.193085	$poly
Rg1839	FE_OFC0_q/n0#106	FE_OFC0_q/n0#15	321.226410	$poly
Rg1841	FE_OFC0_q/n0#106	FE_OFC0_q/n0#47	313.193085	$poly
Rg1843	FE_OFC0_q/n0#107	FE_OFC0_q/n0#13	321.226410	$poly
Rg1845	FE_OFC0_q/n0#107	FE_OFC0_q/n0#45	313.193085	$poly
Rg1847	FE_OFC0_q/n0#108	FE_OFC0_q/n0#11	321.226410	$poly
Rg1849	FE_OFC0_q/n0#108	FE_OFC0_q/n0#43	313.193085	$poly
Rg1851	FE_OFC0_q/n0#109	FE_OFC0_q/n0#9	321.226410	$poly
Rg1853	FE_OFC0_q/n0#109	FE_OFC0_q/n0#41	313.193085	$poly
Rg1855	FE_OFC0_q/n0#110	FE_OFC0_q/n0#7	321.226410	$poly
Rg1857	FE_OFC0_q/n0#110	FE_OFC0_q/n0#39	313.193085	$poly
Rg1859	FE_OFC0_q/n0#111	FE_OFC0_q/n0#5	321.226410	$poly
Rg1861	FE_OFC0_q/n0#111	FE_OFC0_q/n0#37	313.193085	$poly
Rg1863	FE_OFC0_q/n0#112	FE_OFC0_q/n0#3	321.226410	$poly
Rg1865	FE_OFC0_q/n0#112	FE_OFC0_q/n0#35	313.193085	$poly
Rg1867	FE_OFC0_q/n0#113	FE_OFC0_q/n0#33	315.750000	$poly
Rg1870	\mem_reg[10]/net367	\mem_reg[10]/net367#7	365.151520	$poly
Rg1871	\mem_reg[10]/net367#7	\mem_reg[10]/net367#3	235.011505	$poly
Rf1	\mem_reg[12]/SEb#9	\mem_reg[12]/SEb#10	18.764706	$li
Rf2	\mem_reg[12]/SEb#11	\mem_reg[12]/SEb#12	25.917646	$li
Rf3	\mem_reg[4]/SEb#9	\mem_reg[4]/SEb#10	25.917646	$li
Rf4	\mem_reg[4]/SEb#11	\mem_reg[4]/SEb#12	18.764706	$li
Rf5	\mem_reg[1]/SEb#9	\mem_reg[1]/SEb#10	18.764706	$li
Rf6	\mem_reg[1]/SEb#11	\mem_reg[1]/SEb#12	25.917646	$li
Rf7	\mem_reg[3]/SEb#9	\mem_reg[3]/SEb#10	25.917646	$li
Rf8	\mem_reg[3]/SEb#11	\mem_reg[3]/SEb#12	18.764706	$li
Rf9	FE_OFC1_scan_out_0/n0#114	FE_OFC1_scan_out_0/n0#115
+ 7.500000	$li
Rf10	FE_OFC1_scan_out_0/n0#116	FE_OFC1_scan_out_0/n0#117
+ 32.105885	$li
Rf11	\mem_reg[14]/SEb#9	\mem_reg[14]/SEb#10	25.917646	$li
Rf12	\mem_reg[14]/SEb#11	\mem_reg[14]/SEb#12	18.764706	$li
Rf13	mem[12]#22	mem[12]#19	168.129410	$li
Rf14	mem[4]#22	mem[4]#19	16.129412	$li
Rf15	mem[1]#22	mem[1]#19	168.129410	$li
Rf16	mem[3]#22	mem[3]#19	16.129412	$li
Rf17	\mem_reg[12]/SEb#7	\mem_reg[12]/SEb#14	247.941177	$li
Rf18	\mem_reg[4]/SEb#7	\mem_reg[4]/SEb#14	247.941177	$li
Rf19	\mem_reg[1]/SEb#7	\mem_reg[1]/SEb#14	247.941177	$li
Rf20	\mem_reg[3]/SEb#7	\mem_reg[3]/SEb#14	247.941177	$li
Rf21	\mem_reg[12]/Db	\mem_reg[12]/Db#2	18.764706	$li
Rf22	\mem_reg[12]/Db#3	\mem_reg[12]/Db#4	177.917648	$li
Rf23	\mem_reg[4]/Db	\mem_reg[4]/Db#2	177.917648	$li
Rf24	\mem_reg[4]/Db#3	\mem_reg[4]/Db#4	18.764706	$li
Rf25	\mem_reg[1]/Db	\mem_reg[1]/Db#2	18.764706	$li
Rf26	\mem_reg[1]/Db#3	\mem_reg[1]/Db#4	177.917648	$li
Rf27	\mem_reg[3]/Db	\mem_reg[3]/Db#2	177.917648	$li
Rf28	\mem_reg[3]/Db#3	\mem_reg[3]/Db#4	18.764706	$li
Rf29	FE_OFC1_scan_out_0/n0#118	FE_OFC1_scan_out_0/n0#119
+ 7.500000	$li
Rf30	FE_OFC1_scan_out_0/n0#120	FE_OFC1_scan_out_0/n0#121
+ 5.000000	$li
Rf31	scan_en#222	scan_en#254	53.566681	$li
Rf32	scan_en#254	scan_en#217	47.640217	$li
Rf33	scan_en#223	scan_en#257	53.566681	$li
Rf34	scan_en#257	scan_en#218	47.640217	$li
Rf35	scan_en#224	scan_en#260	53.566681	$li
Rf36	scan_en#260	scan_en#219	47.640217	$li
Rf37	scan_en#225	scan_en#263	53.566681	$li
Rf38	scan_en#263	scan_en#220	47.640217	$li
Rf39	FE_OFN1_scan_out_0#45	FE_OFN1_scan_out_0#52	20.058825	$li
Rf40	mem[14]#22	mem[14]#19	16.129412	$li
Rf41	\mem_reg[12]/SEb#8	\mem_reg[12]/SEb#16	280.317627	$li
Rf42	\mem_reg[4]/SEb#8	\mem_reg[4]/SEb#16	280.317627	$li
Rf43	\mem_reg[1]/SEb#8	\mem_reg[1]/SEb#16	280.317627	$li
Rf44	\mem_reg[3]/SEb#8	\mem_reg[3]/SEb#16	280.317627	$li
Rf45	\mem_reg[14]/SEb#7	\mem_reg[14]/SEb#14	247.941177	$li
Rf46	\mem_reg[14]/Db	\mem_reg[14]/Db#2	177.917648	$li
Rf47	\mem_reg[14]/Db#3	\mem_reg[14]/Db#4	18.764706	$li
Rf48	scan_en#226	scan_en#266	53.566681	$li
Rf49	scan_en#266	scan_en#221	47.640217	$li
Rf50	\mem_reg[7]/SEb#9	\mem_reg[7]/SEb#10	25.917646	$li
Rf51	\mem_reg[7]/SEb#11	\mem_reg[7]/SEb#12	18.764706	$li
Rf52	\mem_reg[14]/SEb#8	\mem_reg[14]/SEb#16	280.317627	$li
Rf53	mem[6]#22	mem[6]#19	22.905882	$li
Rf54	mem[1]#24	mem[1]#20	22.905882	$li
Rf55	mem[3]#24	mem[3]#20	22.905882	$li
Rf56	q_control#29	q_control#25	22.905882	$li
Rf57	scan_out[0]#1	scan_out[0]#2	7.500000	$li
Rf58	scan_out[0]#3	scan_out[0]#4	5.000000	$li
Rf59	\mem_reg[5]/SEb#9	\mem_reg[5]/SEb#10	18.764706	$li
Rf60	\mem_reg[5]/SEb#11	\mem_reg[5]/SEb#12	25.917646	$li
Rf61	scan_clk#109	scan_clk#128	39.470589	$li
Rf62	scan_clk#110	scan_clk#130	39.470589	$li
Rf63	scan_clk#111	scan_clk#132	39.470589	$li
Rf64	scan_clk#112	scan_clk#134	39.470589	$li
Rf65	\mem_reg[12]/CKb#22	\mem_reg[12]/CKb#23	18.764706	$li
Rf66	\mem_reg[12]/CKb#24	\mem_reg[12]/CKb#25	25.917646	$li
Rf67	\mem_reg[4]/CKb#22	\mem_reg[4]/CKb#23	25.917646	$li
Rf68	\mem_reg[4]/CKb#24	\mem_reg[4]/CKb#25	18.764706	$li
Rf69	\mem_reg[1]/CKb#22	\mem_reg[1]/CKb#23	18.764706	$li
Rf70	\mem_reg[1]/CKb#24	\mem_reg[1]/CKb#25	25.917646	$li
Rf71	\mem_reg[3]/CKb#22	\mem_reg[3]/CKb#23	25.917646	$li
Rf72	\mem_reg[3]/CKb#24	\mem_reg[3]/CKb#25	18.764706	$li
Rf73	mem[12]#24	mem[12]#20	22.905882	$li
Rf74	mem[7]#22	mem[7]#19	16.129412	$li
Rf75	scan_clk#113	scan_clk#136	39.470589	$li
Rf76	scan_out[0]#5	scan_out[0]#6	7.500000	$li
Rf77	scan_out[0]#7	scan_out[0]#8	5.000000	$li
Rf78	\mem_reg[7]/SEb#7	\mem_reg[7]/SEb#14	247.941177	$li
Rf79	\mem_reg[14]/CKb#22	\mem_reg[14]/CKb#23	25.917646	$li
Rf80	\mem_reg[14]/CKb#24	\mem_reg[14]/CKb#25	18.764706	$li
Rf81	\mem_reg[12]/Db#5	\mem_reg[12]/Db#6	18.764706	$li
Rf82	\mem_reg[12]/Db#7	\mem_reg[12]/Db#8	177.917648	$li
Rf83	\mem_reg[4]/Db#5	\mem_reg[4]/Db#6	177.917648	$li
Rf84	\mem_reg[4]/Db#7	\mem_reg[4]/Db#8	18.764706	$li
Rf85	\mem_reg[1]/Db#5	\mem_reg[1]/Db#6	18.764706	$li
Rf86	\mem_reg[1]/Db#7	\mem_reg[1]/Db#8	177.917648	$li
Rf87	\mem_reg[3]/Db#5	\mem_reg[3]/Db#6	177.917648	$li
Rf88	\mem_reg[3]/Db#7	\mem_reg[3]/Db#8	18.764706	$li
Rf89	\mem_reg[7]/Db	\mem_reg[7]/Db#2	177.917648	$li
Rf90	\mem_reg[7]/Db#3	\mem_reg[7]/Db#4	18.764706	$li
Rf91	scan_en#229	scan_en#269	53.566681	$li
Rf92	scan_en#269	scan_en#227	47.640217	$li
Rf93	mem[5]#22	mem[5]#19	168.129410	$li
Rf94	\mem_reg[7]/SEb#8	\mem_reg[7]/SEb#16	280.317627	$li
Rf95	\mem_reg[5]/SEb#7	\mem_reg[5]/SEb#14	247.941177	$li
Rf96	\mem_reg[12]/n20#8	\mem_reg[12]/n20#9	18.764706	$li
Rf97	\mem_reg[12]/n20#10	\mem_reg[12]/n20#11	177.917648	$li
Rf98	\mem_reg[4]/n20#8	\mem_reg[4]/n20#9	177.917648	$li
Rf99	\mem_reg[4]/n20#10	\mem_reg[4]/n20#11	18.764706	$li
Rf100	\mem_reg[1]/n20#8	\mem_reg[1]/n20#9	18.764706	$li
Rf101	\mem_reg[1]/n20#10	\mem_reg[1]/n20#11	177.917648	$li
Rf102	\mem_reg[3]/n20#8	\mem_reg[3]/n20#9	177.917648	$li
Rf103	\mem_reg[3]/n20#10	\mem_reg[3]/n20#11	18.764706	$li
Rf104	\mem_reg[14]/Db#5	\mem_reg[14]/Db#6	177.917648	$li
Rf105	\mem_reg[14]/Db#7	\mem_reg[14]/Db#8	18.764706	$li
Rf106	\mem_reg[5]/Db	\mem_reg[5]/Db#2	18.764706	$li
Rf107	\mem_reg[5]/Db#3	\mem_reg[5]/Db#4	177.917648	$li
Rf108	scan_en#230	scan_en#272	53.566681	$li
Rf109	scan_en#272	scan_en#228	47.640217	$li
Rf110	scan_out[0]#9	scan_out[0]#10	7.500000	$li
Rf111	scan_out[0]#11	scan_out[0]#12	5.000000	$li
Rf112	\mem_reg[5]/SEb#8	\mem_reg[5]/SEb#16	280.317627	$li
Rf113	mem[5]#24	mem[5]#20	22.905882	$li
Rf114	\mem_reg[14]/n20#8	\mem_reg[14]/n20#9	177.917648	$li
Rf115	\mem_reg[14]/n20#10	\mem_reg[14]/n20#11	18.764706	$li
Rf116	scan_clk#114	scan_clk#138	39.470589	$li
Rf117	\mem_reg[7]/CKb#22	\mem_reg[7]/CKb#23	25.917646	$li
Rf118	\mem_reg[7]/CKb#24	\mem_reg[7]/CKb#25	18.764706	$li
Rf119	mem[14]#24	mem[14]#20	22.905882	$li
Rf120	scan_out[0]#13	scan_out[0]#14	7.500000	$li
Rf121	scan_out[0]#15	scan_out[0]#16	5.000000	$li
Rf122	scan_clk#115	scan_clk#140	39.470589	$li
Rf123	\mem_reg[12]/CKbb#17	\mem_reg[12]/CKbb#18	18.764706	$li
Rf124	\mem_reg[12]/CKbb#19	\mem_reg[12]/CKbb#20	25.917646	$li
Rf125	\mem_reg[4]/CKbb#17	\mem_reg[4]/CKbb#18	25.917646	$li
Rf126	\mem_reg[4]/CKbb#19	\mem_reg[4]/CKbb#20	18.764706	$li
Rf127	\mem_reg[1]/CKbb#17	\mem_reg[1]/CKbb#18	18.764706	$li
Rf128	\mem_reg[1]/CKbb#19	\mem_reg[1]/CKbb#20	25.917646	$li
Rf129	\mem_reg[3]/CKbb#17	\mem_reg[3]/CKbb#18	25.917646	$li
Rf130	\mem_reg[3]/CKbb#19	\mem_reg[3]/CKbb#20	18.764706	$li
Rf131	\mem_reg[5]/CKb#22	\mem_reg[5]/CKb#23	18.764706	$li
Rf132	\mem_reg[5]/CKb#24	\mem_reg[5]/CKb#25	25.917646	$li
Rf133	\mem_reg[12]/CKbb#21	\mem_reg[12]/CKbb#13	280.317627	$li
Rf134	\mem_reg[4]/CKbb#21	\mem_reg[4]/CKbb#13	280.317627	$li
Rf135	\mem_reg[1]/CKbb#21	\mem_reg[1]/CKbb#13	280.317627	$li
Rf136	\mem_reg[3]/CKbb#21	\mem_reg[3]/CKbb#13	280.317627	$li
Rf137	\mem_reg[7]/Db#5	\mem_reg[7]/Db#6	177.917648	$li
Rf138	\mem_reg[7]/Db#7	\mem_reg[7]/Db#8	18.764706	$li
Rf139	\mem_reg[14]/CKbb#17	\mem_reg[14]/CKbb#18	25.917646	$li
Rf140	\mem_reg[14]/CKbb#19	\mem_reg[14]/CKbb#20	18.764706	$li
Rf141	\mem_reg[14]/CKbb#21	\mem_reg[14]/CKbb#13	280.317627	$li
Rf142	\mem_reg[7]/n20#8	\mem_reg[7]/n20#9	177.917648	$li
Rf143	\mem_reg[7]/n20#10	\mem_reg[7]/n20#11	18.764706	$li
Rf144	\mem_reg[5]/Db#5	\mem_reg[5]/Db#6	18.764706	$li
Rf145	\mem_reg[5]/Db#7	\mem_reg[5]/Db#8	177.917648	$li
Rf146	scan_out[0]#17	scan_out[0]#18	7.500000	$li
Rf147	scan_out[0]#19	scan_out[0]#20	5.000000	$li
Rf148	\mem_reg[12]/n20#7	\mem_reg[12]/n20#13	31.188236	$li
Rf149	\mem_reg[4]/n20#7	\mem_reg[4]/n20#13	31.188236	$li
Rf150	\mem_reg[1]/n20#7	\mem_reg[1]/n20#13	31.188236	$li
Rf151	\mem_reg[3]/n20#7	\mem_reg[3]/n20#13	31.188236	$li
Rf152	\mem_reg[12]/mout#8	\mem_reg[12]/mout#9	18.764706	$li
Rf153	\mem_reg[12]/mout#10	\mem_reg[12]/mout#11	25.917646	$li
Rf154	\mem_reg[4]/mout#8	\mem_reg[4]/mout#9	25.917646	$li
Rf155	\mem_reg[4]/mout#10	\mem_reg[4]/mout#11	18.764706	$li
Rf156	\mem_reg[1]/mout#8	\mem_reg[1]/mout#9	18.764706	$li
Rf157	\mem_reg[1]/mout#10	\mem_reg[1]/mout#11	25.917646	$li
Rf158	\mem_reg[3]/mout#8	\mem_reg[3]/mout#9	25.917646	$li
Rf159	\mem_reg[3]/mout#10	\mem_reg[3]/mout#11	18.764706	$li
Rf160	\mem_reg[12]/mout#12	\mem_reg[12]/mout#7	287.094116	$li
Rf161	\mem_reg[4]/mout#12	\mem_reg[4]/mout#7	287.094116	$li
Rf162	\mem_reg[1]/mout#12	\mem_reg[1]/mout#7	287.094116	$li
Rf163	\mem_reg[3]/mout#12	\mem_reg[3]/mout#7	287.094116	$li
Rf164	\mem_reg[5]/n20#8	\mem_reg[5]/n20#9	18.764706	$li
Rf165	\mem_reg[5]/n20#10	\mem_reg[5]/n20#11	177.917648	$li
Rf166	\mem_reg[14]/n20#7	\mem_reg[14]/n20#13	31.188236	$li
Rf167	\mem_reg[12]/n30#8	\mem_reg[12]/n30#9	18.764706	$li
Rf168	\mem_reg[12]/n30#10	\mem_reg[12]/n30#11	25.917646	$li
Rf169	\mem_reg[4]/n30#8	\mem_reg[4]/n30#9	25.917646	$li
Rf170	\mem_reg[4]/n30#10	\mem_reg[4]/n30#11	18.764706	$li
Rf171	\mem_reg[1]/n30#8	\mem_reg[1]/n30#9	18.764706	$li
Rf172	\mem_reg[1]/n30#10	\mem_reg[1]/n30#11	25.917646	$li
Rf173	\mem_reg[3]/n30#8	\mem_reg[3]/n30#9	25.917646	$li
Rf174	\mem_reg[3]/n30#10	\mem_reg[3]/n30#11	18.764706	$li
Rf175	\mem_reg[14]/mout#8	\mem_reg[14]/mout#9	25.917646	$li
Rf176	\mem_reg[14]/mout#10	\mem_reg[14]/mout#11	18.764706	$li
Rf177	\mem_reg[14]/mout#12	\mem_reg[14]/mout#7	287.094116	$li
Rf178	\mem_reg[12]/CKb#21	\mem_reg[12]/CKb#27	130.576462	$li
Rf179	\mem_reg[12]/CKb#27	\mem_reg[12]/CKb#28	85.458824	$li
Rf180	\mem_reg[12]/CKb#28	\mem_reg[12]/CKb#29	223.529419	$li
Rf181	\mem_reg[4]/CKb#21	\mem_reg[4]/CKb#27	130.576462	$li
Rf182	\mem_reg[4]/CKb#27	\mem_reg[4]/CKb#28	85.458824	$li
Rf183	\mem_reg[4]/CKb#28	\mem_reg[4]/CKb#29	223.529419	$li
Rf184	\mem_reg[1]/CKb#21	\mem_reg[1]/CKb#27	130.576462	$li
Rf185	\mem_reg[1]/CKb#27	\mem_reg[1]/CKb#28	85.458824	$li
Rf186	\mem_reg[1]/CKb#28	\mem_reg[1]/CKb#29	223.529419	$li
Rf187	\mem_reg[3]/CKb#21	\mem_reg[3]/CKb#27	130.576462	$li
Rf188	\mem_reg[3]/CKb#27	\mem_reg[3]/CKb#28	85.458824	$li
Rf189	\mem_reg[3]/CKb#28	\mem_reg[3]/CKb#29	223.529419	$li
Rf190	scan_out[0]#21	scan_out[0]#22	7.500000	$li
Rf191	scan_out[0]#23	scan_out[0]#24	5.000000	$li
Rf192	\mem_reg[7]/CKbb#17	\mem_reg[7]/CKbb#18	25.917646	$li
Rf193	\mem_reg[7]/CKbb#19	\mem_reg[7]/CKbb#20	18.764706	$li
Rf194	\mem_reg[14]/n30#8	\mem_reg[14]/n30#9	25.917646	$li
Rf195	\mem_reg[14]/n30#10	\mem_reg[14]/n30#11	18.764706	$li
Rf196	\mem_reg[12]/CKbb#23	\mem_reg[12]/CKbb#16	31.188236	$li
Rf197	\mem_reg[4]/CKbb#23	\mem_reg[4]/CKbb#16	31.188236	$li
Rf198	\mem_reg[1]/CKbb#23	\mem_reg[1]/CKbb#16	31.188236	$li
Rf199	\mem_reg[3]/CKbb#23	\mem_reg[3]/CKbb#16	31.188236	$li
Rf200	\mem_reg[12]/CKbb#25	\mem_reg[12]/CKbb#15	48.564705	$li
Rf201	\mem_reg[12]/CKbb#15	\mem_reg[12]/CKbb#27	87.717644	$li
Rf202	\mem_reg[12]/CKbb#27	\mem_reg[12]/CKbb#14	95.941177	$li
Rf203	\mem_reg[4]/CKbb#25	\mem_reg[4]/CKbb#15	48.564705	$li
Rf204	\mem_reg[4]/CKbb#15	\mem_reg[4]/CKbb#27	87.717644	$li
Rf205	\mem_reg[4]/CKbb#27	\mem_reg[4]/CKbb#14	95.941177	$li
Rf206	\mem_reg[1]/CKbb#25	\mem_reg[1]/CKbb#15	48.564705	$li
Rf207	\mem_reg[1]/CKbb#15	\mem_reg[1]/CKbb#27	87.717644	$li
Rf208	\mem_reg[1]/CKbb#27	\mem_reg[1]/CKbb#14	95.941177	$li
Rf209	\mem_reg[3]/CKbb#25	\mem_reg[3]/CKbb#15	48.564705	$li
Rf210	\mem_reg[3]/CKbb#15	\mem_reg[3]/CKbb#27	87.717644	$li
Rf211	\mem_reg[3]/CKbb#27	\mem_reg[3]/CKbb#14	95.941177	$li
Rf212	\mem_reg[7]/CKbb#21	\mem_reg[7]/CKbb#13	280.317627	$li
Rf213	\mem_reg[14]/CKb#21	\mem_reg[14]/CKb#27	130.576462	$li
Rf214	\mem_reg[14]/CKb#27	\mem_reg[14]/CKb#28	85.458824	$li
Rf215	\mem_reg[14]/CKb#28	\mem_reg[14]/CKb#29	223.529419	$li
Rf216	\mem_reg[5]/CKbb#17	\mem_reg[5]/CKbb#18	18.764706	$li
Rf217	\mem_reg[5]/CKbb#19	\mem_reg[5]/CKbb#20	25.917646	$li
Rf218	\mem_reg[14]/CKbb#23	\mem_reg[14]/CKbb#16	31.188236	$li
Rf219	\mem_reg[14]/CKbb#25	\mem_reg[14]/CKbb#15	48.564705	$li
Rf220	\mem_reg[14]/CKbb#15	\mem_reg[14]/CKbb#27	87.717644	$li
Rf221	\mem_reg[14]/CKbb#27	\mem_reg[14]/CKbb#14	95.941177	$li
Rf222	\mem_reg[5]/CKbb#21	\mem_reg[5]/CKbb#13	280.317627	$li
Rf223	scan_out[0]#25	scan_out[0]#26	7.500000	$li
Rf224	scan_out[0]#27	scan_out[0]#28	5.000000	$li
Rf225	\mem_reg[7]/n20#7	\mem_reg[7]/n20#13	31.188236	$li
Rf226	\mem_reg[12]/net367#8	\mem_reg[12]/net367#9	30.435293	$li
Rf227	\mem_reg[12]/net367#10	\mem_reg[12]/net367#11	30.811764	$li
Rf228	\mem_reg[4]/net367#8	\mem_reg[4]/net367#9	30.811764	$li
Rf229	\mem_reg[4]/net367#10	\mem_reg[4]/net367#11	30.435293	$li
Rf230	\mem_reg[1]/net367#8	\mem_reg[1]/net367#9	30.435293	$li
Rf231	\mem_reg[1]/net367#10	\mem_reg[1]/net367#11	30.811764	$li
Rf232	\mem_reg[3]/net367#8	\mem_reg[3]/net367#9	30.811764	$li
Rf233	\mem_reg[3]/net367#10	\mem_reg[3]/net367#11	30.435293	$li
Rf234	\mem_reg[7]/mout#8	\mem_reg[7]/mout#9	25.917646	$li
Rf235	\mem_reg[7]/mout#10	\mem_reg[7]/mout#11	18.764706	$li
Rf236	\mem_reg[7]/mout#12	\mem_reg[7]/mout#7	287.094116	$li
Rf237	\mem_reg[5]/n20#7	\mem_reg[5]/n20#13	31.188236	$li
Rf238	\mem_reg[7]/n30#8	\mem_reg[7]/n30#9	25.917646	$li
Rf239	\mem_reg[7]/n30#10	\mem_reg[7]/n30#11	18.764706	$li
Rf240	\mem_reg[14]/net367#8	\mem_reg[14]/net367#9	30.811764	$li
Rf241	\mem_reg[14]/net367#10	\mem_reg[14]/net367#11	30.435293	$li
Rf242	\mem_reg[5]/mout#8	\mem_reg[5]/mout#9	18.764706	$li
Rf243	\mem_reg[5]/mout#10	\mem_reg[5]/mout#11	25.917646	$li
Rf244	\mem_reg[5]/mout#12	\mem_reg[5]/mout#7	287.094116	$li
Rf245	\mem_reg[7]/CKb#21	\mem_reg[7]/CKb#27	130.576462	$li
Rf246	\mem_reg[7]/CKb#27	\mem_reg[7]/CKb#28	85.458824	$li
Rf247	\mem_reg[7]/CKb#28	\mem_reg[7]/CKb#29	223.529419	$li
Rf248	scan_out[0]#29	scan_out[0]#30	7.500000	$li
Rf249	scan_out[0]#31	scan_out[0]#32	5.000000	$li
Rf250	FE_OFC1_scan_out_0/n0#105	FE_OFC1_scan_out_0/n0#123
+ 292.388245	$li
Rf251	FE_OFC1_scan_out_0/n0#123	FE_OFC1_scan_out_0/n0#124
+ 228.423523	$li
Rf252	\mem_reg[12]/n30#7	\mem_reg[12]/n30#13	333.398804	$li
Rf253	\mem_reg[4]/n30#7	\mem_reg[4]/n30#13	333.398804	$li
Rf254	\mem_reg[1]/n30#7	\mem_reg[1]/n30#13	333.398804	$li
Rf255	\mem_reg[3]/n30#7	\mem_reg[3]/n30#13	333.398804	$li
Rf256	\mem_reg[12]/qbint#22	\mem_reg[12]/qbint#23	17.258823	$li
Rf257	\mem_reg[4]/qbint#22	\mem_reg[4]/qbint#23	17.258823	$li
Rf258	\mem_reg[1]/qbint#22	\mem_reg[1]/qbint#23	17.258823	$li
Rf259	\mem_reg[3]/qbint#22	\mem_reg[3]/qbint#23	17.258823	$li
Rf260	\mem_reg[5]/n30#8	\mem_reg[5]/n30#9	18.764706	$li
Rf261	\mem_reg[5]/n30#10	\mem_reg[5]/n30#11	25.917646	$li
Rf262	\mem_reg[7]/CKbb#23	\mem_reg[7]/CKbb#16	31.188236	$li
Rf263	\mem_reg[7]/CKbb#25	\mem_reg[7]/CKbb#15	48.564705	$li
Rf264	\mem_reg[7]/CKbb#15	\mem_reg[7]/CKbb#27	87.717644	$li
Rf265	\mem_reg[7]/CKbb#27	\mem_reg[7]/CKbb#14	95.941177	$li
Rf266	\mem_reg[5]/CKb#21	\mem_reg[5]/CKb#27	130.576462	$li
Rf267	\mem_reg[5]/CKb#27	\mem_reg[5]/CKb#28	85.458824	$li
Rf268	\mem_reg[5]/CKb#28	\mem_reg[5]/CKb#29	223.529419	$li
Rf269	\mem_reg[14]/n30#7	\mem_reg[14]/n30#13	333.398804	$li
Rf270	\mem_reg[14]/qbint#22	\mem_reg[14]/qbint#23	17.258823	$li
Rf271	mem[12]#26	mem[12]#27	8.423712	$li
Rf272	mem[12]#28	mem[12]#29	5.616868	$li
Rf273	mem[4]#24	mem[4]#25	56.283535	$li
Rf274	mem[4]#26	mem[4]#27	8.423712	$li
Rf275	mem[1]#26	mem[1]#27	84.423714	$li
Rf276	mem[1]#28	mem[1]#29	5.616868	$li
Rf277	mem[3]#26	mem[3]#27	56.283535	$li
Rf278	mem[3]#28	mem[3]#29	8.423712	$li
Rf279	\mem_reg[5]/CKbb#23	\mem_reg[5]/CKbb#16	31.188236	$li
Rf280	\mem_reg[5]/CKbb#25	\mem_reg[5]/CKbb#15	48.564705	$li
Rf281	\mem_reg[5]/CKbb#15	\mem_reg[5]/CKbb#27	87.717644	$li
Rf282	\mem_reg[5]/CKbb#27	\mem_reg[5]/CKbb#14	95.941177	$li
Rf283	\mem_reg[12]/qbint#21	\mem_reg[12]/qbint#25	63.074757	$li
Rf284	\mem_reg[12]/qbint#25	\mem_reg[12]/qbint#26	48.074757	$li
Rf285	\mem_reg[12]/qbint#26	\mem_reg[12]/qbint#19	86.529411	$li
Rf286	\mem_reg[12]/qbint#25	\mem_reg[12]/qbint#28	40.316555	$li
Rf287	\mem_reg[12]/qbint#28	\mem_reg[12]/qbint#29	31.941177	$li
Rf288	\mem_reg[4]/qbint#21	\mem_reg[4]/qbint#25	63.074757	$li
Rf289	\mem_reg[4]/qbint#25	\mem_reg[4]/qbint#26	48.074757	$li
Rf290	\mem_reg[4]/qbint#26	\mem_reg[4]/qbint#19	86.529411	$li
Rf291	\mem_reg[4]/qbint#25	\mem_reg[4]/qbint#28	40.316555	$li
Rf292	\mem_reg[4]/qbint#28	\mem_reg[4]/qbint#29	31.941177	$li
Rf293	\mem_reg[1]/qbint#21	\mem_reg[1]/qbint#25	63.074757	$li
Rf294	\mem_reg[1]/qbint#25	\mem_reg[1]/qbint#26	48.074757	$li
Rf295	\mem_reg[1]/qbint#26	\mem_reg[1]/qbint#19	86.529411	$li
Rf296	\mem_reg[1]/qbint#25	\mem_reg[1]/qbint#28	40.316555	$li
Rf297	\mem_reg[1]/qbint#28	\mem_reg[1]/qbint#29	31.941177	$li
Rf298	\mem_reg[3]/qbint#21	\mem_reg[3]/qbint#25	63.074757	$li
Rf299	\mem_reg[3]/qbint#25	\mem_reg[3]/qbint#26	48.074757	$li
Rf300	\mem_reg[3]/qbint#26	\mem_reg[3]/qbint#19	86.529411	$li
Rf301	\mem_reg[3]/qbint#25	\mem_reg[3]/qbint#28	40.316555	$li
Rf302	\mem_reg[3]/qbint#28	\mem_reg[3]/qbint#29	31.941177	$li
Rf303	q_control_reg/SEb#9	q_control_reg/SEb#10	18.764706	$li
Rf304	q_control_reg/SEb#11	q_control_reg/SEb#12	25.917646	$li
Rf305	\mem_reg[7]/net367#8	\mem_reg[7]/net367#9	30.811764	$li
Rf306	\mem_reg[7]/net367#10	\mem_reg[7]/net367#11	30.435293	$li
Rf307	mem[14]#26	mem[14]#27	56.283535	$li
Rf308	mem[14]#28	mem[14]#29	8.423712	$li
Rf309	\mem_reg[14]/qbint#21	\mem_reg[14]/qbint#25	63.074757	$li
Rf310	\mem_reg[14]/qbint#25	\mem_reg[14]/qbint#26	48.074757	$li
Rf311	\mem_reg[14]/qbint#26	\mem_reg[14]/qbint#19	86.529411	$li
Rf312	\mem_reg[14]/qbint#25	\mem_reg[14]/qbint#28	40.316555	$li
Rf313	\mem_reg[14]/qbint#28	\mem_reg[14]/qbint#29	31.941177	$li
Rf314	\mem_reg[12]/net367#7	\mem_reg[12]/net367#13	335.281128	$li
Rf315	\mem_reg[4]/net367#7	\mem_reg[4]/net367#13	335.281128	$li
Rf316	\mem_reg[1]/net367#7	\mem_reg[1]/net367#13	335.281128	$li
Rf317	\mem_reg[3]/net367#7	\mem_reg[3]/net367#13	335.281128	$li
Rf318	UNCONNECTED4	UNCONNECTED4#2	8.283673	$li
Rf319	UNCONNECTED4#3	UNCONNECTED4#4	5.616868	$li
Rf320	UNCONNECTED17	UNCONNECTED17#2	5.616868	$li
Rf321	UNCONNECTED17#3	UNCONNECTED17#4	8.283673	$li
Rf322	UNCONNECTED10	UNCONNECTED10#2	8.283673	$li
Rf323	UNCONNECTED10#3	UNCONNECTED10#4	5.616868	$li
Rf324	UNCONNECTED8	UNCONNECTED8#2	5.616868	$li
Rf325	UNCONNECTED8#3	UNCONNECTED8#4	8.283673	$li
Rf326	\mem_reg[5]/net367#8	\mem_reg[5]/net367#9	30.435293	$li
Rf327	\mem_reg[5]/net367#10	\mem_reg[5]/net367#11	30.811764	$li
Rf328	q_control#31	q_control#26	168.129410	$li
Rf329	\mem_reg[7]/n30#7	\mem_reg[7]/n30#13	333.398804	$li
Rf330	\mem_reg[14]/net367#7	\mem_reg[14]/net367#13	335.281128	$li
Rf331	\mem_reg[7]/qbint#22	\mem_reg[7]/qbint#23	17.258823	$li
Rf332	UNCONNECTED6	UNCONNECTED6#2	5.616868	$li
Rf333	UNCONNECTED6#3	UNCONNECTED6#4	8.283673	$li
Rf334	q_control_reg/SEb#7	q_control_reg/SEb#14	247.941177	$li
Rf335	q_control_reg/Db	q_control_reg/Db#2	18.764706	$li
Rf336	q_control_reg/Db#3	q_control_reg/Db#4	177.917648	$li
Rf337	scan_en#232	scan_en#275	53.566681	$li
Rf338	scan_en#275	scan_en#231	47.640217	$li
Rf339	\mem_reg[6]/SEb#9	\mem_reg[6]/SEb#10	18.764706	$li
Rf340	\mem_reg[6]/SEb#11	\mem_reg[6]/SEb#12	25.917646	$li
Rf341	g521__1617/net89	g521__1617/net89#2	10.991489	$li
Rf342	\mem_reg[11]/SEb#9	\mem_reg[11]/SEb#10	25.917646	$li
Rf343	\mem_reg[11]/SEb#11	\mem_reg[11]/SEb#12	18.764706	$li
Rf344	\mem_reg[5]/n30#7	\mem_reg[5]/n30#13	333.398804	$li
Rf345	\mem_reg[5]/qbint#22	\mem_reg[5]/qbint#23	17.258823	$li
Rf346	mem[7]#24	mem[7]#25	5.616868	$li
Rf347	mem[7]#26	mem[7]#27	84.423714	$li
Rf348	q_control_reg/SEb#8	q_control_reg/SEb#16	280.317627	$li
Rf349	addr[0]#31	addr[0]#37	19.141176	$li
Rf350	n_2#22	n_2#19	18.388235	$li
Rf351	\mem_reg[7]/qbint#21	\mem_reg[7]/qbint#25	63.074757	$li
Rf352	\mem_reg[7]/qbint#25	\mem_reg[7]/qbint#26	48.074757	$li
Rf353	\mem_reg[7]/qbint#26	\mem_reg[7]/qbint#19	86.529411	$li
Rf354	\mem_reg[7]/qbint#25	\mem_reg[7]/qbint#28	40.316555	$li
Rf355	\mem_reg[7]/qbint#28	\mem_reg[7]/qbint#29	31.941177	$li
Rf356	g498__5526/net122	g498__5526/net122#2	7.258823	$li
Rf357	n_7#29	n_7#30	19.995237	$li
Rf358	mem[5]#26	mem[5]#27	8.423712	$li
Rf359	mem[5]#28	mem[5]#29	5.616868	$li
Rf360	scan_in[0]#8	scan_in[0]#7	22.905882	$li
Rf361	mem[1]#30	mem[1]#21	16.129412	$li
Rf362	n_8#29	n_8#25	18.388235	$li
Rf363	n_2#20	n_2#25	20.647058	$li
Rf364	g521__1617/net89#3	g521__1617/net89#4	10.991489	$li
Rf365	n_4#8	n_4#9	10.135294	$li
Rf366	\mem_reg[5]/qbint#21	\mem_reg[5]/qbint#25	63.074757	$li
Rf367	\mem_reg[5]/qbint#25	\mem_reg[5]/qbint#26	48.074757	$li
Rf368	\mem_reg[5]/qbint#26	\mem_reg[5]/qbint#19	86.529411	$li
Rf369	\mem_reg[5]/qbint#25	\mem_reg[5]/qbint#28	40.316555	$li
Rf370	\mem_reg[5]/qbint#28	\mem_reg[5]/qbint#29	31.941177	$li
Rf371	mem[6]#24	mem[6]#20	16.129412	$li
Rf372	mem[11]#22	mem[11]#19	16.129412	$li
Rf373	\mem_reg[7]/net367#7	\mem_reg[7]/net367#13	335.281128	$li
Rf374	n_7#31	n_7#32	5.301177	$li
Rf375	scan_clk#116	scan_clk#142	39.470589	$li
Rf376	UNCONNECTED13	UNCONNECTED13#2	5.616868	$li
Rf377	UNCONNECTED13#3	UNCONNECTED13#4	8.283673	$li
Rf378	\mem_reg[6]/SEb#7	\mem_reg[6]/SEb#14	247.941177	$li
Rf379	\mem_reg[11]/SEb#7	\mem_reg[11]/SEb#14	247.941177	$li
Rf380	q_control_reg/CKb#22	q_control_reg/CKb#23	18.764706	$li
Rf381	q_control_reg/CKb#24	q_control_reg/CKb#25	25.917646	$li
Rf382	mem[12]#30	mem[12]#21	16.129412	$li
Rf383	\mem_reg[6]/Db	\mem_reg[6]/Db#2	18.764706	$li
Rf384	\mem_reg[6]/Db#3	\mem_reg[6]/Db#4	177.917648	$li
Rf385	n_4#10	n_4#11	6.505882	$li
Rf386	\mem_reg[11]/Db	\mem_reg[11]/Db#2	177.917648	$li
Rf387	\mem_reg[11]/Db#3	\mem_reg[11]/Db#4	18.764706	$li
Rf388	scan_en#235	scan_en#278	53.566681	$li
Rf389	scan_en#278	scan_en#233	47.640217	$li
Rf390	scan_en#236	scan_en#281	53.566681	$li
Rf391	scan_en#281	scan_en#234	47.640217	$li
Rf392	g498__5526/net122#3	g498__5526/net122#4	159.258820	$li
Rf393	n_9#8	n_9#9	20.676472	$li
Rf394	mem[3]#30	mem[3]#21	18.011765	$li
Rf395	\mem_reg[5]/net367#7	\mem_reg[5]/net367#13	335.281128	$li
Rf396	UNCONNECTED0	UNCONNECTED0#2	8.283673	$li
Rf397	UNCONNECTED0#3	UNCONNECTED0#4	5.616868	$li
Rf398	\mem_reg[6]/SEb#8	\mem_reg[6]/SEb#16	280.317627	$li
Rf399	\mem_reg[11]/SEb#8	\mem_reg[11]/SEb#16	280.317627	$li
Rf400	n_1#8	n_1#7	18.388235	$li
Rf401	g521__1617/net89#5	g521__1617/net89#6	37.376472	$li
Rf402	q_control_reg/Db#5	q_control_reg/Db#6	18.764706	$li
Rf403	q_control_reg/Db#7	q_control_reg/Db#8	177.917648	$li
Rf404	g498__5526/net118	g498__5526/net118#2	35.117649	$li
Rf405	\mem_reg[13]/SEb#9	\mem_reg[13]/SEb#10	25.917646	$li
Rf406	\mem_reg[13]/SEb#11	\mem_reg[13]/SEb#12	18.764706	$li
Rf407	g522__2802/net89	g522__2802/net89#2	10.991489	$li
Rf408	mem[4]#28	mem[4]#20	22.905882	$li
Rf409	mem[9]#22	mem[9]#19	22.905882	$li
Rf410	q_control_reg/n20#8	q_control_reg/n20#9	18.764706	$li
Rf411	q_control_reg/n20#10	q_control_reg/n20#11	177.917648	$li
Rf412	addr[1]#59	addr[1]#49	50.764706	$li
Rf413	g498__5526/net122#5	g498__5526/net122#6	7.258823	$li
Rf414	n_2#26	n_2#21	18.388235	$li
Rf415	g520__5122/S0b#8	g520__5122/S0b#9	15.000000	$li
Rf416	g520__5122/S0b#10	g520__5122/S0b#11	15.000000	$li
Rf417	addr[0]#38	addr[0]#32	16.129412	$li
Rf418	scan_clk#117	scan_clk#144	39.470589	$li
Rf419	scan_clk#118	scan_clk#146	39.470589	$li
Rf420	\mem_reg[6]/CKb#22	\mem_reg[6]/CKb#23	18.764706	$li
Rf421	\mem_reg[6]/CKb#24	\mem_reg[6]/CKb#25	25.917646	$li
Rf422	\mem_reg[11]/CKb#22	\mem_reg[11]/CKb#23	25.917646	$li
Rf423	\mem_reg[11]/CKb#24	\mem_reg[11]/CKb#25	18.764706	$li
Rf424	mem[9]#24	mem[9]#20	16.129412	$li
Rf425	g498__5526/net118#3	g498__5526/net118#4	187.117645	$li
Rf426	n_3#8	n_3#9	10.135294	$li
Rf427	g522__2802/net89#3	g522__2802/net89#4	10.991489	$li
Rf428	g497__8428/net122	g497__8428/net122#2	7.258823	$li
Rf429	mem[13]#22	mem[13]#19	168.129410	$li
Rf430	n_7#25	n_7#34	41.729412	$li
Rf431	\mem_reg[13]/SEb#7	\mem_reg[13]/SEb#14	247.941177	$li
Rf432	mem[5]#30	mem[5]#21	16.129412	$li
Rf433	\mem_reg[6]/Db#5	\mem_reg[6]/Db#6	18.764706	$li
Rf434	\mem_reg[6]/Db#7	\mem_reg[6]/Db#8	177.917648	$li
Rf435	\mem_reg[11]/Db#5	\mem_reg[11]/Db#6	177.917648	$li
Rf436	\mem_reg[11]/Db#7	\mem_reg[11]/Db#8	18.764706	$li
Rf437	n_8#31	n_8#26	18.388235	$li
Rf438	\mem_reg[13]/Db	\mem_reg[13]/Db#2	177.917648	$li
Rf439	\mem_reg[13]/Db#3	\mem_reg[13]/Db#4	18.764706	$li
Rf440	n_9#10	n_9#11	10.991489	$li
Rf441	n_3#10	n_3#11	6.505882	$li
Rf442	scan_en#238	scan_en#284	53.566681	$li
Rf443	scan_en#284	scan_en#237	47.640217	$li
Rf444	mem[11]#24	mem[11]#20	18.011765	$li
Rf445	q_control_reg/CKbb#17	q_control_reg/CKbb#18	18.764706	$li
Rf446	q_control_reg/CKbb#19	q_control_reg/CKbb#20	25.917646	$li
Rf447	\mem_reg[13]/SEb#8	\mem_reg[13]/SEb#16	280.317627	$li
Rf448	g520__5122/S0b#7	g520__5122/S0b#13	247.941177	$li
Rf449	mem[4]#30	mem[4]#21	16.129412	$li
Rf450	q_control_reg/CKbb#21	q_control_reg/CKbb#13	280.317627	$li
Rf451	\mem_reg[6]/n20#8	\mem_reg[6]/n20#9	18.764706	$li
Rf452	\mem_reg[6]/n20#10	\mem_reg[6]/n20#11	177.917648	$li
Rf453	\mem_reg[11]/n20#8	\mem_reg[11]/n20#9	177.917648	$li
Rf454	\mem_reg[11]/n20#10	\mem_reg[11]/n20#11	18.764706	$li
Rf455	g498__5526/net118#5	g498__5526/net118#6	35.117649	$li
Rf456	n_9#12	n_9#13	8.423712	$li
Rf457	g522__2802/net89#5	g522__2802/net89#6	37.376472	$li
Rf458	g520__5122/n0#8	g520__5122/n0#9	15.000000	$li
Rf459	g520__5122/n0#10	g520__5122/n0#11	15.000000	$li
Rf460	g497__8428/net122#3	g497__8428/net122#4	159.258820	$li
Rf461	n_10#8	n_10#9	20.676472	$li
Rf462	addr[1]#51	addr[1]#62	30.870588	$li
Rf463	addr[1]#62	addr[1]#50	53.023529	$li
Rf464	addr[1]#51	addr[1]#53	138.013397	$li
Rf465	mem[15]#22	mem[15]#19	22.905882	$li
Rf466	mem[14]#30	mem[14]#21	50.764706	$li
Rf467	n_0#8	n_0#7	18.388235	$li
Rf468	addr[1]#65	addr[1]#52	50.764706	$li
Rf469	g497__8428/net118	g497__8428/net118#2	35.117649	$li
Rf470	q_control_reg/n20#7	q_control_reg/n20#13	31.188236	$li
Rf471	scan_clk#119	scan_clk#148	191.470581	$li
Rf472	q_control_reg/mout#8	q_control_reg/mout#9	18.764706	$li
Rf473	q_control_reg/mout#10	q_control_reg/mout#11	25.917646	$li
Rf474	\mem_reg[13]/CKb#22	\mem_reg[13]/CKb#23	25.917646	$li
Rf475	\mem_reg[13]/CKb#24	\mem_reg[13]/CKb#25	18.764706	$li
Rf476	mem[7]#28	mem[7]#20	24.411764	$li
Rf477	q_control_reg/mout#12	q_control_reg/mout#7	287.094116	$li
Rf478	g497__8428/net122#5	g497__8428/net122#6	7.258823	$li
Rf479	\mem_reg[9]/SEb#9	\mem_reg[9]/SEb#10	18.764706	$li
Rf480	\mem_reg[9]/SEb#11	\mem_reg[9]/SEb#12	25.917646	$li
Rf481	addr[0]#40	addr[0]#33	16.129412	$li
Rf482	\mem_reg[6]/CKbb#17	\mem_reg[6]/CKbb#18	18.764706	$li
Rf483	\mem_reg[6]/CKbb#19	\mem_reg[6]/CKbb#20	25.917646	$li
Rf484	\mem_reg[11]/CKbb#17	\mem_reg[11]/CKbb#18	25.917646	$li
Rf485	\mem_reg[11]/CKbb#19	\mem_reg[11]/CKbb#20	18.764706	$li
Rf486	q_control_reg/n30#8	q_control_reg/n30#9	18.764706	$li
Rf487	q_control_reg/n30#10	q_control_reg/n30#11	25.917646	$li
Rf488	g520__5122/n0#7	g520__5122/n0#13	15.000000	$li
Rf489	\mem_reg[6]/CKbb#21	\mem_reg[6]/CKbb#13	280.317627	$li
Rf490	\mem_reg[11]/CKbb#21	\mem_reg[11]/CKbb#13	280.317627	$li
Rf491	q_control_reg/CKb#21	q_control_reg/CKb#27	130.576462	$li
Rf492	q_control_reg/CKb#27	q_control_reg/CKb#28	85.458824	$li
Rf493	q_control_reg/CKb#28	q_control_reg/CKb#29	223.529419	$li
Rf494	addr[0]#34	addr[0]#43	19.141176	$li
Rf495	\mem_reg[13]/Db#5	\mem_reg[13]/Db#6	177.917648	$li
Rf496	\mem_reg[13]/Db#7	\mem_reg[13]/Db#8	18.764706	$li
Rf497	n_0#10	n_0#11	8.423712	$li
Rf498	n_0#12	n_0#13	86.991486	$li
Rf499	g497__8428/net118#3	g497__8428/net118#4	187.117645	$li
Rf500	n_8#33	n_8#34	19.995237	$li
Rf501	n_7#26	n_7#36	41.729412	$li
Rf502	q_control_reg/CKbb#23	q_control_reg/CKbb#16	31.188236	$li
Rf503	q_control_reg/CKbb#25	q_control_reg/CKbb#15	48.564705	$li
Rf504	q_control_reg/CKbb#15	q_control_reg/CKbb#27	87.717644	$li
Rf505	q_control_reg/CKbb#27	q_control_reg/CKbb#14	95.941177	$li
Rf506	\mem_reg[13]/n20#8	\mem_reg[13]/n20#9	177.917648	$li
Rf507	\mem_reg[13]/n20#10	\mem_reg[13]/n20#11	18.764706	$li
Rf508	addr[1]#54	addr[1]#68	20.647058	$li
Rf509	n_10#10	n_10#11	10.991489	$li
Rf510	mem[9]#26	mem[9]#21	16.129412	$li
Rf511	n_8#35	n_8#36	5.301177	$li
Rf512	\mem_reg[6]/n20#7	\mem_reg[6]/n20#13	31.188236	$li
Rf513	\mem_reg[11]/n20#7	\mem_reg[11]/n20#13	31.188236	$li
Rf514	\mem_reg[6]/mout#8	\mem_reg[6]/mout#9	18.764706	$li
Rf515	\mem_reg[6]/mout#10	\mem_reg[6]/mout#11	25.917646	$li
Rf516	\mem_reg[11]/mout#8	\mem_reg[11]/mout#9	25.917646	$li
Rf517	\mem_reg[11]/mout#10	\mem_reg[11]/mout#11	18.764706	$li
Rf518	\mem_reg[9]/SEb#7	\mem_reg[9]/SEb#14	247.941177	$li
Rf519	\mem_reg[6]/mout#12	\mem_reg[6]/mout#7	287.094116	$li
Rf520	\mem_reg[11]/mout#12	\mem_reg[11]/mout#7	287.094116	$li
Rf521	g497__8428/net118#5	g497__8428/net118#6	35.117649	$li
Rf522	n_10#12	n_10#13	8.423712	$li
Rf523	\mem_reg[9]/Db	\mem_reg[9]/Db#2	18.764706	$li
Rf524	\mem_reg[9]/Db#3	\mem_reg[9]/Db#4	177.917648	$li
Rf525	scan_en#240	scan_en#287	53.566681	$li
Rf526	scan_en#287	scan_en#239	47.640217	$li
Rf527	g519__1705/S0b#8	g519__1705/S0b#9	15.000000	$li
Rf528	g519__1705/S0b#10	g519__1705/S0b#11	15.000000	$li
Rf529	\mem_reg[6]/n30#8	\mem_reg[6]/n30#9	18.764706	$li
Rf530	\mem_reg[6]/n30#10	\mem_reg[6]/n30#11	25.917646	$li
Rf531	\mem_reg[11]/n30#8	\mem_reg[11]/n30#9	25.917646	$li
Rf532	\mem_reg[11]/n30#10	\mem_reg[11]/n30#11	18.764706	$li
Rf533	q_control_reg/net367#8	q_control_reg/net367#9	30.435293	$li
Rf534	q_control_reg/net367#10	q_control_reg/net367#11
+ 30.811764	$li
Rf535	\mem_reg[9]/SEb#8	\mem_reg[9]/SEb#16	280.317627	$li
Rf536	mem[6]#26	mem[6]#21	50.764706	$li
Rf537	\mem_reg[6]/CKb#21	\mem_reg[6]/CKb#27	130.576462	$li
Rf538	\mem_reg[6]/CKb#27	\mem_reg[6]/CKb#28	85.458824	$li
Rf539	\mem_reg[6]/CKb#28	\mem_reg[6]/CKb#29	223.529419	$li
Rf540	\mem_reg[11]/CKb#21	\mem_reg[11]/CKb#27	130.576462	$li
Rf541	\mem_reg[11]/CKb#27	\mem_reg[11]/CKb#28	85.458824	$li
Rf542	\mem_reg[11]/CKb#28	\mem_reg[11]/CKb#29	223.529419	$li
Rf543	\mem_reg[13]/CKbb#17	\mem_reg[13]/CKbb#18	25.917646	$li
Rf544	\mem_reg[13]/CKbb#19	\mem_reg[13]/CKbb#20	18.764706	$li
Rf545	\mem_reg[6]/CKbb#23	\mem_reg[6]/CKbb#16	31.188236	$li
Rf546	\mem_reg[11]/CKbb#23	\mem_reg[11]/CKbb#16	31.188236	$li
Rf547	\mem_reg[6]/CKbb#25	\mem_reg[6]/CKbb#15	48.564705	$li
Rf548	\mem_reg[6]/CKbb#15	\mem_reg[6]/CKbb#27	87.717644	$li
Rf549	\mem_reg[6]/CKbb#27	\mem_reg[6]/CKbb#14	95.941177	$li
Rf550	\mem_reg[11]/CKbb#25	\mem_reg[11]/CKbb#15	48.564705	$li
Rf551	\mem_reg[11]/CKbb#15	\mem_reg[11]/CKbb#27	87.717644	$li
Rf552	\mem_reg[11]/CKbb#27	\mem_reg[11]/CKbb#14	95.941177	$li
Rf553	mem[13]#24	mem[13]#20	16.129412	$li
Rf554	addr[2]#22	addr[2]#19	18.388235	$li
Rf555	mem[10]#22	mem[10]#19	22.905882	$li
Rf556	\mem_reg[13]/CKbb#21	\mem_reg[13]/CKbb#13	280.317627	$li
Rf557	g496__4319/n1	g496__4319/n1#2	19.170588	$li
Rf558	q_control_reg/n30#7	q_control_reg/n30#13	333.398804	$li
Rf559	scan_clk#120	scan_clk#150	39.470589	$li
Rf560	q_control_reg/qbint#22	q_control_reg/qbint#23	17.258823	$li
Rf561	g519__1705/S0b#7	g519__1705/S0b#13	247.941177	$li
Rf562	\mem_reg[9]/CKb#22	\mem_reg[9]/CKb#23	18.764706	$li
Rf563	\mem_reg[9]/CKb#24	\mem_reg[9]/CKb#25	25.917646	$li
Rf564	addr[1]#57	addr[1]#70	16.129412	$li
Rf565	g519__1705/n0#8	g519__1705/n0#9	15.000000	$li
Rf566	g519__1705/n0#10	g519__1705/n0#11	15.000000	$li
Rf567	n_13#8	n_13#9	35.117649	$li
Rf568	n_2#28	n_2#29	9.517647	$li
Rf569	n_2#30	n_2#31	8.566667	$li
Rf570	n_6#7	n_6#9	18.388235	$li
Rf571	\mem_reg[13]/n20#7	\mem_reg[13]/n20#13	31.188236	$li
Rf572	addr[1]#56	addr[1]#72	30.870588	$li
Rf573	addr[1]#72	addr[1]#55	53.023529	$li
Rf574	addr[1]#56	addr[1]#58	138.013397	$li
Rf575	\mem_reg[6]/net367#8	\mem_reg[6]/net367#9	30.435293	$li
Rf576	\mem_reg[6]/net367#10	\mem_reg[6]/net367#11	30.811764	$li
Rf577	\mem_reg[11]/net367#8	\mem_reg[11]/net367#9	30.811764	$li
Rf578	\mem_reg[11]/net367#10	\mem_reg[11]/net367#11	30.435293	$li
Rf579	q_control#33	q_control#34	84.423714	$li
Rf580	q_control#35	q_control#36	5.616868	$li
Rf581	\mem_reg[13]/mout#8	\mem_reg[13]/mout#9	25.917646	$li
Rf582	\mem_reg[13]/mout#10	\mem_reg[13]/mout#11	18.764706	$li
Rf583	\mem_reg[13]/mout#12	\mem_reg[13]/mout#7	287.094116	$li
Rf584	\mem_reg[9]/Db#5	\mem_reg[9]/Db#6	18.764706	$li
Rf585	\mem_reg[9]/Db#7	\mem_reg[9]/Db#8	177.917648	$li
Rf586	q_control_reg/qbint#21	q_control_reg/qbint#25	63.074757	$li
Rf587	q_control_reg/qbint#25	q_control_reg/qbint#26	48.074757	$li
Rf588	q_control_reg/qbint#26	q_control_reg/qbint#19	86.529411	$li
Rf589	q_control_reg/qbint#25	q_control_reg/qbint#28	40.316555	$li
Rf590	q_control_reg/qbint#28	q_control_reg/qbint#29	31.941177	$li
Rf591	g496__4319/n0	g496__4319/n0#2	23.688236	$li
Rf592	\mem_reg[13]/n30#8	\mem_reg[13]/n30#9	25.917646	$li
Rf593	\mem_reg[13]/n30#10	\mem_reg[13]/n30#11	18.764706	$li
Rf594	n_12#19	n_12#23	46.247059	$li
Rf595	mem[15]#24	mem[15]#20	24.411764	$li
Rf596	\mem_reg[9]/n20#8	\mem_reg[9]/n20#9	18.764706	$li
Rf597	\mem_reg[9]/n20#10	\mem_reg[9]/n20#11	177.917648	$li
Rf598	\mem_reg[13]/CKb#21	\mem_reg[13]/CKb#27	130.576462	$li
Rf599	\mem_reg[13]/CKb#27	\mem_reg[13]/CKb#28	85.458824	$li
Rf600	\mem_reg[13]/CKb#28	\mem_reg[13]/CKb#29	223.529419	$li
Rf601	g496__4319/n1#3	g496__4319/n1#4	19.170588	$li
Rf602	\mem_reg[6]/n30#7	\mem_reg[6]/n30#13	333.398804	$li
Rf603	\mem_reg[11]/n30#7	\mem_reg[11]/n30#13	333.398804	$li
Rf604	q_control_reg/net367#7	q_control_reg/net367#13	335.281128
+ $li
Rf605	\mem_reg[6]/qbint#22	\mem_reg[6]/qbint#23	17.258823	$li
Rf606	\mem_reg[11]/qbint#22	\mem_reg[11]/qbint#23	17.258823	$li
Rf607	UNCONNECTED11	UNCONNECTED11#2	8.283673	$li
Rf608	UNCONNECTED11#3	UNCONNECTED11#4	5.616868	$li
Rf609	g519__1705/n0#7	g519__1705/n0#13	15.000000	$li
Rf610	\mem_reg[13]/CKbb#23	\mem_reg[13]/CKbb#16	31.188236	$li
Rf611	\mem_reg[13]/CKbb#25	\mem_reg[13]/CKbb#15	48.564705	$li
Rf612	\mem_reg[13]/CKbb#15	\mem_reg[13]/CKbb#27	87.717644	$li
Rf613	\mem_reg[13]/CKbb#27	\mem_reg[13]/CKbb#14	95.941177	$li
Rf614	n_10#7	n_10#15	47.752941	$li
Rf615	addr[0]#35	addr[0]#45	16.129412	$li
Rf616	n_1#10	n_1#11	8.423712	$li
Rf617	n_1#12	n_1#13	86.991486	$li
Rf618	n_13#10	n_13#11	187.117645	$li
Rf619	g496__4319/n0#3	g496__4319/n0#4	175.688232	$li
Rf620	n_11#15	n_11#16	8.864151	$li
Rf621	mem[6]#28	mem[6]#29	8.423712	$li
Rf622	mem[6]#30	mem[6]#31	5.616868	$li
Rf623	mem[11]#26	mem[11]#27	5.616868	$li
Rf624	mem[11]#28	mem[11]#29	8.423712	$li
Rf625	n_4#7	n_4#13	45.494118	$li
Rf626	n_12#24	n_12#20	16.129412	$li
Rf627	n_13#12	n_13#13	23.688236	$li
Rf628	n_11#17	n_11#18	20.299999	$li
Rf629	\mem_reg[6]/qbint#21	\mem_reg[6]/qbint#25	63.074757	$li
Rf630	\mem_reg[6]/qbint#25	\mem_reg[6]/qbint#26	48.074757	$li
Rf631	\mem_reg[6]/qbint#26	\mem_reg[6]/qbint#19	86.529411	$li
Rf632	\mem_reg[6]/qbint#25	\mem_reg[6]/qbint#28	40.316555	$li
Rf633	\mem_reg[6]/qbint#28	\mem_reg[6]/qbint#29	31.941177	$li
Rf634	\mem_reg[11]/qbint#21	\mem_reg[11]/qbint#25	63.074757	$li
Rf635	\mem_reg[11]/qbint#25	\mem_reg[11]/qbint#26	48.074757	$li
Rf636	\mem_reg[11]/qbint#26	\mem_reg[11]/qbint#19	86.529411	$li
Rf637	\mem_reg[11]/qbint#25	\mem_reg[11]/qbint#28	40.316555	$li
Rf638	\mem_reg[11]/qbint#28	\mem_reg[11]/qbint#29	31.941177	$li
Rf639	q_reg_reg/SEb#9	q_reg_reg/SEb#10	18.764706	$li
Rf640	q_reg_reg/SEb#11	q_reg_reg/SEb#12	25.917646	$li
Rf641	\mem_reg[13]/net367#8	\mem_reg[13]/net367#9	30.811764	$li
Rf642	\mem_reg[13]/net367#10	\mem_reg[13]/net367#11	30.435293	$li
Rf643	\mem_reg[9]/CKbb#17	\mem_reg[9]/CKbb#18	18.764706	$li
Rf644	\mem_reg[9]/CKbb#19	\mem_reg[9]/CKbb#20	25.917646	$li
Rf645	\mem_reg[9]/CKbb#21	\mem_reg[9]/CKbb#13	280.317627	$li
Rf646	g496__4319/n0#5	g496__4319/n0#6	23.688236	$li
Rf647	n_11#19	n_11#13	16.129412	$li
Rf648	g509__6783/net89	g509__6783/net89#2	10.991489	$li
Rf649	\mem_reg[6]/net367#7	\mem_reg[6]/net367#13	335.281128	$li
Rf650	\mem_reg[11]/net367#7	\mem_reg[11]/net367#13	335.281128	$li
Rf651	UNCONNECTED12	UNCONNECTED12#2	8.283673	$li
Rf652	UNCONNECTED12#3	UNCONNECTED12#4	5.616868	$li
Rf653	UNCONNECTED16	UNCONNECTED16#2	5.616868	$li
Rf654	UNCONNECTED16#3	UNCONNECTED16#4	8.283673	$li
Rf655	n_8#37	n_8#27	18.388235	$li
Rf656	\mem_reg[8]/SEb#9	\mem_reg[8]/SEb#10	25.917646	$li
Rf657	\mem_reg[8]/SEb#11	\mem_reg[8]/SEb#12	18.764706	$li
Rf658	n_15#15	n_15#13	168.129410	$li
Rf659	\mem_reg[13]/n30#7	\mem_reg[13]/n30#13	333.398804	$li
Rf660	\mem_reg[9]/n20#7	\mem_reg[9]/n20#13	31.188236	$li
Rf661	\mem_reg[13]/qbint#22	\mem_reg[13]/qbint#23	17.258823	$li
Rf662	\mem_reg[9]/mout#8	\mem_reg[9]/mout#9	18.764706	$li
Rf663	\mem_reg[9]/mout#10	\mem_reg[9]/mout#11	25.917646	$li
Rf664	q_reg_reg/SEb#7	q_reg_reg/SEb#14	247.941177	$li
Rf665	addr[2]#24	addr[2]#20	18.388235	$li
Rf666	\mem_reg[9]/mout#12	\mem_reg[9]/mout#7	287.094116	$li
Rf667	mem[0]#22	mem[0]#19	16.129412	$li
Rf668	q_reg_reg/Db	q_reg_reg/Db#2	18.764706	$li
Rf669	q_reg_reg/Db#3	q_reg_reg/Db#4	177.917648	$li
Rf670	q_control#28	q_control#38	53.566681	$li
Rf671	q_control#38	q_control#27	47.640217	$li
Rf672	n_6#10	n_6#11	10.135294	$li
Rf673	g509__6783/net89#3	g509__6783/net89#4	10.991489	$li
Rf674	g495__6260/n1	g495__6260/n1#2	19.170588	$li
Rf675	\mem_reg[15]/SEb#9	\mem_reg[15]/SEb#10	18.764706	$li
Rf676	\mem_reg[15]/SEb#11	\mem_reg[15]/SEb#12	25.917646	$li
Rf677	mux_control_reg/SEb#9	mux_control_reg/SEb#10	25.917646	$li
Rf678	mux_control_reg/SEb#11	mux_control_reg/SEb#12	18.764706	$li
Rf679	mem[13]#26	mem[13]#27	56.283535	$li
Rf680	mem[13]#28	mem[13]#29	8.423712	$li
Rf681	\mem_reg[9]/n30#8	\mem_reg[9]/n30#9	18.764706	$li
Rf682	\mem_reg[9]/n30#10	\mem_reg[9]/n30#11	25.917646	$li
Rf683	q_reg_reg/SEb#8	q_reg_reg/SEb#16	280.317627	$li
Rf684	\mem_reg[9]/CKb#21	\mem_reg[9]/CKb#27	130.576462	$li
Rf685	\mem_reg[9]/CKb#27	\mem_reg[9]/CKb#28	85.458824	$li
Rf686	\mem_reg[9]/CKb#28	\mem_reg[9]/CKb#29	223.529419	$li
Rf687	n_6#12	n_6#13	6.505882	$li
Rf688	n_14#8	n_14#9	35.117649	$li
Rf689	\mem_reg[13]/qbint#21	\mem_reg[13]/qbint#25	63.074757	$li
Rf690	\mem_reg[13]/qbint#25	\mem_reg[13]/qbint#26	48.074757	$li
Rf691	\mem_reg[13]/qbint#26	\mem_reg[13]/qbint#19	86.529411	$li
Rf692	\mem_reg[13]/qbint#25	\mem_reg[13]/qbint#28	40.316555	$li
Rf693	\mem_reg[13]/qbint#28	\mem_reg[13]/qbint#29	31.941177	$li
Rf694	mem[2]#22	mem[2]#19	18.011765	$li
Rf695	n_5#7	n_5#9	18.388235	$li
Rf696	mem[8]#22	mem[8]#19	16.129412	$li
Rf697	\mem_reg[8]/SEb#7	\mem_reg[8]/SEb#14	247.941177	$li
Rf698	\mem_reg[9]/CKbb#23	\mem_reg[9]/CKbb#16	31.188236	$li
Rf699	\mem_reg[9]/CKbb#25	\mem_reg[9]/CKbb#15	48.564705	$li
Rf700	\mem_reg[9]/CKbb#15	\mem_reg[9]/CKbb#27	87.717644	$li
Rf701	\mem_reg[9]/CKbb#27	\mem_reg[9]/CKbb#14	95.941177	$li
Rf702	x#15	x#13	22.905882	$li
Rf703	g509__6783/net89#5	g509__6783/net89#6	37.376472	$li
Rf704	\mem_reg[8]/Db	\mem_reg[8]/Db#2	177.917648	$li
Rf705	\mem_reg[8]/Db#3	\mem_reg[8]/Db#4	18.764706	$li
Rf706	scan_en#244	scan_en#290	53.566681	$li
Rf707	scan_en#290	scan_en#241	47.640217	$li
Rf708	g495__6260/n0	g495__6260/n0#2	23.688236	$li
Rf709	mem[15]#26	mem[15]#21	168.129410	$li
Rf710	FE_OFN1_scan_out_0#53	FE_OFN1_scan_out_0#48	168.129410	$li
Rf711	\mem_reg[13]/net367#7	\mem_reg[13]/net367#13	335.281128	$li
Rf712	clk#7	clk#9	39.470589	$li
Rf713	UNCONNECTED5	UNCONNECTED5#2	5.616868	$li
Rf714	UNCONNECTED5#3	UNCONNECTED5#4	8.283673	$li
Rf715	\mem_reg[8]/SEb#8	\mem_reg[8]/SEb#16	280.317627	$li
Rf716	n_12#21	n_12#27	46.247059	$li
Rf717	\mem_reg[15]/SEb#7	\mem_reg[15]/SEb#14	247.941177	$li
Rf718	mux_control_reg/SEb#7	mux_control_reg/SEb#14	247.941177	$li
Rf719	q_reg_reg/CKb#22	q_reg_reg/CKb#23	18.764706	$li
Rf720	q_reg_reg/CKb#24	q_reg_reg/CKb#25	25.917646	$li
Rf721	n_7#37	n_7#27	50.764706	$li
Rf722	\mem_reg[15]/Db	\mem_reg[15]/Db#2	18.764706	$li
Rf723	\mem_reg[15]/Db#3	\mem_reg[15]/Db#4	177.917648	$li
Rf724	mux_control_reg/Db	mux_control_reg/Db#2	177.917648	$li
Rf725	mux_control_reg/Db#3	mux_control_reg/Db#4	18.764706	$li
Rf726	scan_en#245	scan_en#293	53.566681	$li
Rf727	scan_en#293	scan_en#242	47.640217	$li
Rf728	scan_en#246	scan_en#296	53.566681	$li
Rf729	scan_en#296	scan_en#243	47.640217	$li
Rf730	g495__6260/n1#3	g495__6260/n1#4	19.170588	$li
Rf731	\mem_reg[9]/net367#8	\mem_reg[9]/net367#9	30.435293	$li
Rf732	\mem_reg[9]/net367#10	\mem_reg[9]/net367#11	30.811764	$li
Rf733	\mem_reg[15]/SEb#8	\mem_reg[15]/SEb#16	280.317627	$li
Rf734	mux_control_reg/SEb#8	mux_control_reg/SEb#16	280.317627	$li
Rf735	n_9#7	n_9#15	47.752941	$li
Rf736	mem[2]#24	mem[2]#20	22.905882	$li
Rf737	q_reg_reg/Db#5	q_reg_reg/Db#6	18.764706	$li
Rf738	q_reg_reg/Db#7	q_reg_reg/Db#8	177.917648	$li
Rf739	n_14#10	n_14#11	187.117645	$li
Rf740	g495__6260/n0#3	g495__6260/n0#4	175.688232	$li
Rf741	\mem_reg[0]/SEb#9	\mem_reg[0]/SEb#10	25.917646	$li
Rf742	\mem_reg[0]/SEb#11	\mem_reg[0]/SEb#12	18.764706	$li
Rf743	\mem_reg[2]/SEb#9	\mem_reg[2]/SEb#10	18.764706	$li
Rf744	\mem_reg[2]/SEb#11	\mem_reg[2]/SEb#12	25.917646	$li
Rf745	scan_clk#121	scan_clk#152	39.470589	$li
Rf746	n_3#7	n_3#13	45.494118	$li
Rf747	\mem_reg[8]/CKb#22	\mem_reg[8]/CKb#23	25.917646	$li
Rf748	\mem_reg[8]/CKb#24	\mem_reg[8]/CKb#25	18.764706	$li
Rf749	mem[7]#30	mem[7]#21	22.905882	$li
Rf750	mem[11]#30	mem[11]#21	22.905882	$li
Rf751	q_reg_reg/n20#8	q_reg_reg/n20#9	18.764706	$li
Rf752	q_reg_reg/n20#10	q_reg_reg/n20#11	177.917648	$li
Rf753	n_14#12	n_14#13	23.688236	$li
Rf754	\mem_reg[9]/n30#7	\mem_reg[9]/n30#13	333.398804	$li
Rf755	scan_clk#122	scan_clk#154	39.470589	$li
Rf756	scan_clk#123	scan_clk#156	39.470589	$li
Rf757	\mem_reg[9]/qbint#22	\mem_reg[9]/qbint#23	17.258823	$li
Rf758	\mem_reg[15]/CKb#22	\mem_reg[15]/CKb#23	18.764706	$li
Rf759	\mem_reg[15]/CKb#24	\mem_reg[15]/CKb#25	25.917646	$li
Rf760	mux_control_reg/CKb#22	mux_control_reg/CKb#23	25.917646	$li
Rf761	mux_control_reg/CKb#24	mux_control_reg/CKb#25	18.764706	$li
Rf762	\mem_reg[8]/Db#5	\mem_reg[8]/Db#6	177.917648	$li
Rf763	\mem_reg[8]/Db#7	\mem_reg[8]/Db#8	18.764706	$li
Rf764	g495__6260/n0#5	g495__6260/n0#6	23.688236	$li
Rf765	n_11#21	n_11#14	16.129412	$li
Rf766	mem[0]#24	mem[0]#20	168.129410	$li
Rf767	mem[2]#26	mem[2]#21	16.129412	$li
Rf768	mem[9]#28	mem[9]#29	8.423712	$li
Rf769	mem[9]#30	mem[9]#31	56.283535	$li
Rf770	\mem_reg[0]/SEb#7	\mem_reg[0]/SEb#14	247.941177	$li
Rf771	\mem_reg[2]/SEb#7	\mem_reg[2]/SEb#14	247.941177	$li
Rf772	\mem_reg[8]/n20#8	\mem_reg[8]/n20#9	177.917648	$li
Rf773	\mem_reg[8]/n20#10	\mem_reg[8]/n20#11	18.764706	$li
Rf774	\mem_reg[15]/Db#5	\mem_reg[15]/Db#6	18.764706	$li
Rf775	\mem_reg[15]/Db#7	\mem_reg[15]/Db#8	177.917648	$li
Rf776	mux_control_reg/Db#5	mux_control_reg/Db#6	177.917648	$li
Rf777	mux_control_reg/Db#7	mux_control_reg/Db#8	18.764706	$li
Rf778	\mem_reg[0]/Db	\mem_reg[0]/Db#2	177.917648	$li
Rf779	\mem_reg[0]/Db#3	\mem_reg[0]/Db#4	18.764706	$li
Rf780	\mem_reg[2]/Db	\mem_reg[2]/Db#2	18.764706	$li
Rf781	\mem_reg[2]/Db#3	\mem_reg[2]/Db#4	177.917648	$li
Rf782	scan_en#249	scan_en#299	53.566681	$li
Rf783	scan_en#299	scan_en#247	47.640217	$li
Rf784	scan_en#250	scan_en#302	53.566681	$li
Rf785	scan_en#302	scan_en#248	47.640217	$li
Rf786	\mem_reg[9]/qbint#21	\mem_reg[9]/qbint#25	63.074757	$li
Rf787	\mem_reg[9]/qbint#25	\mem_reg[9]/qbint#26	48.074757	$li
Rf788	\mem_reg[9]/qbint#26	\mem_reg[9]/qbint#19	86.529411	$li
Rf789	\mem_reg[9]/qbint#25	\mem_reg[9]/qbint#28	40.316555	$li
Rf790	\mem_reg[9]/qbint#28	\mem_reg[9]/qbint#29	31.941177	$li
Rf791	\mem_reg[10]/SEb#9	\mem_reg[10]/SEb#10	25.917646	$li
Rf792	\mem_reg[10]/SEb#11	\mem_reg[10]/SEb#12	18.764706	$li
Rf793	q_reg_reg/CKbb#17	q_reg_reg/CKbb#18	18.764706	$li
Rf794	q_reg_reg/CKbb#19	q_reg_reg/CKbb#20	25.917646	$li
Rf795	\mem_reg[0]/SEb#8	\mem_reg[0]/SEb#16	280.317627	$li
Rf796	\mem_reg[2]/SEb#8	\mem_reg[2]/SEb#16	280.317627	$li
Rf797	q_reg_reg/CKbb#21	q_reg_reg/CKbb#13	280.317627	$li
Rf798	\mem_reg[15]/n20#8	\mem_reg[15]/n20#9	18.764706	$li
Rf799	\mem_reg[15]/n20#10	\mem_reg[15]/n20#11	177.917648	$li
Rf800	mux_control_reg/n20#8	mux_control_reg/n20#9	177.917648	$li
Rf801	mux_control_reg/n20#10	mux_control_reg/n20#11	18.764706	$li
Rf802	\mem_reg[9]/net367#7	\mem_reg[9]/net367#13	335.281128	$li
Rf803	UNCONNECTED3	UNCONNECTED3#2	8.283673	$li
Rf804	UNCONNECTED3#3	UNCONNECTED3#4	5.616868	$li
Rf805	mem[13]#30	mem[13]#21	22.905882	$li
Rf806	mem[0]#26	mem[0]#21	22.905882	$li
Rf807	mem[10]#24	mem[10]#20	16.129412	$li
Rf808	q_reg_reg/n20#7	q_reg_reg/n20#13	31.188236	$li
Rf809	scan_clk#124	scan_clk#158	39.470589	$li
Rf810	scan_clk#125	scan_clk#160	39.470589	$li
Rf811	\mem_reg[8]/CKbb#17	\mem_reg[8]/CKbb#18	25.917646	$li
Rf812	\mem_reg[8]/CKbb#19	\mem_reg[8]/CKbb#20	18.764706	$li
Rf813	q_reg_reg/mout#8	q_reg_reg/mout#9	18.764706	$li
Rf814	q_reg_reg/mout#10	q_reg_reg/mout#11	25.917646	$li
Rf815	\mem_reg[10]/SEb#7	\mem_reg[10]/SEb#14	247.941177	$li
Rf816	\mem_reg[0]/CKb#22	\mem_reg[0]/CKb#23	25.917646	$li
Rf817	\mem_reg[0]/CKb#24	\mem_reg[0]/CKb#25	18.764706	$li
Rf818	\mem_reg[2]/CKb#22	\mem_reg[2]/CKb#23	18.764706	$li
Rf819	\mem_reg[2]/CKb#24	\mem_reg[2]/CKb#25	25.917646	$li
Rf820	q_reg_reg/mout#12	q_reg_reg/mout#7	287.094116	$li
Rf821	\mem_reg[8]/CKbb#21	\mem_reg[8]/CKbb#13	280.317627	$li
Rf822	\mem_reg[10]/Db	\mem_reg[10]/Db#2	177.917648	$li
Rf823	\mem_reg[10]/Db#3	\mem_reg[10]/Db#4	18.764706	$li
Rf824	scan_en#252	scan_en#305	53.566681	$li
Rf825	scan_en#305	scan_en#251	47.640217	$li
Rf826	\mem_reg[15]/CKbb#17	\mem_reg[15]/CKbb#18	18.764706	$li
Rf827	\mem_reg[15]/CKbb#19	\mem_reg[15]/CKbb#20	25.917646	$li
Rf828	mux_control_reg/CKbb#17	mux_control_reg/CKbb#18
+ 25.917646	$li
Rf829	mux_control_reg/CKbb#19	mux_control_reg/CKbb#20
+ 18.764706	$li
Rf830	q_reg_reg/n30#8	q_reg_reg/n30#9	18.764706	$li
Rf831	q_reg_reg/n30#10	q_reg_reg/n30#11	25.917646	$li
Rf832	\mem_reg[10]/SEb#8	\mem_reg[10]/SEb#16	280.317627	$li
Rf833	\mem_reg[15]/CKbb#21	\mem_reg[15]/CKbb#13	280.317627	$li
Rf834	mux_control_reg/CKbb#21	mux_control_reg/CKbb#13
+ 280.317627	$li
Rf835	q_reg_reg/CKb#21	q_reg_reg/CKb#27	130.576462	$li
Rf836	q_reg_reg/CKb#27	q_reg_reg/CKb#28	85.458824	$li
Rf837	q_reg_reg/CKb#28	q_reg_reg/CKb#29	223.529419	$li
Rf838	\mem_reg[0]/Db#5	\mem_reg[0]/Db#6	177.917648	$li
Rf839	\mem_reg[0]/Db#7	\mem_reg[0]/Db#8	18.764706	$li
Rf840	\mem_reg[2]/Db#5	\mem_reg[2]/Db#6	18.764706	$li
Rf841	\mem_reg[2]/Db#7	\mem_reg[2]/Db#8	177.917648	$li
Rf842	FE_OFC0_q/n0#114	FE_OFC0_q/n0#115	7.500000	$li
Rf843	FE_OFC0_q/n0#116	FE_OFC0_q/n0#117	32.105885	$li
Rf844	\mem_reg[8]/n20#7	\mem_reg[8]/n20#13	31.188236	$li
Rf845	\mem_reg[8]/mout#8	\mem_reg[8]/mout#9	25.917646	$li
Rf846	\mem_reg[8]/mout#10	\mem_reg[8]/mout#11	18.764706	$li
Rf847	q_reg_reg/CKbb#23	q_reg_reg/CKbb#16	31.188236	$li
Rf848	q_reg_reg/CKbb#25	q_reg_reg/CKbb#15	48.564705	$li
Rf849	q_reg_reg/CKbb#15	q_reg_reg/CKbb#27	87.717644	$li
Rf850	q_reg_reg/CKbb#27	q_reg_reg/CKbb#14	95.941177	$li
Rf851	mem[8]#24	mem[8]#20	22.905882	$li
Rf852	\mem_reg[8]/mout#12	\mem_reg[8]/mout#7	287.094116	$li
Rf853	\mem_reg[0]/n20#8	\mem_reg[0]/n20#9	177.917648	$li
Rf854	\mem_reg[0]/n20#10	\mem_reg[0]/n20#11	18.764706	$li
Rf855	\mem_reg[2]/n20#8	\mem_reg[2]/n20#9	18.764706	$li
Rf856	\mem_reg[2]/n20#10	\mem_reg[2]/n20#11	177.917648	$li
Rf857	\mem_reg[15]/n20#7	\mem_reg[15]/n20#13	31.188236	$li
Rf858	mux_control_reg/n20#7	mux_control_reg/n20#13	31.188236	$li
Rf859	scan_clk#126	scan_clk#162	191.470581	$li
Rf860	\mem_reg[8]/n30#8	\mem_reg[8]/n30#9	25.917646	$li
Rf861	\mem_reg[8]/n30#10	\mem_reg[8]/n30#11	18.764706	$li
Rf862	\mem_reg[15]/mout#8	\mem_reg[15]/mout#9	18.764706	$li
Rf863	\mem_reg[15]/mout#10	\mem_reg[15]/mout#11	25.917646	$li
Rf864	mux_control_reg/mout#8	mux_control_reg/mout#9	25.917646	$li
Rf865	mux_control_reg/mout#10	mux_control_reg/mout#11
+ 18.764706	$li
Rf866	\mem_reg[10]/CKb#22	\mem_reg[10]/CKb#23	25.917646	$li
Rf867	\mem_reg[10]/CKb#24	\mem_reg[10]/CKb#25	18.764706	$li
Rf868	\mem_reg[15]/mout#12	\mem_reg[15]/mout#7	287.094116	$li
Rf869	mux_control_reg/mout#12	mux_control_reg/mout#7	287.094116
+ $li
Rf870	\mem_reg[8]/CKb#21	\mem_reg[8]/CKb#27	130.576462	$li
Rf871	\mem_reg[8]/CKb#27	\mem_reg[8]/CKb#28	85.458824	$li
Rf872	\mem_reg[8]/CKb#28	\mem_reg[8]/CKb#29	223.529419	$li
Rf873	FE_OFC0_q/n0#118	FE_OFC0_q/n0#119	7.500000	$li
Rf874	FE_OFC0_q/n0#120	FE_OFC0_q/n0#121	5.000000	$li
Rf875	FE_OFN0_q#27	FE_OFN0_q#31	20.058825	$li
Rf876	\mem_reg[15]/n30#8	\mem_reg[15]/n30#9	18.764706	$li
Rf877	\mem_reg[15]/n30#10	\mem_reg[15]/n30#11	25.917646	$li
Rf878	mux_control_reg/n30#8	mux_control_reg/n30#9	25.917646	$li
Rf879	mux_control_reg/n30#10	mux_control_reg/n30#11	18.764706	$li
Rf880	q_reg_reg/net367#8	q_reg_reg/net367#9	30.435293	$li
Rf881	q_reg_reg/net367#10	q_reg_reg/net367#11	30.811764	$li
Rf882	\mem_reg[8]/CKbb#23	\mem_reg[8]/CKbb#16	31.188236	$li
Rf883	\mem_reg[8]/CKbb#25	\mem_reg[8]/CKbb#15	48.564705	$li
Rf884	\mem_reg[8]/CKbb#15	\mem_reg[8]/CKbb#27	87.717644	$li
Rf885	\mem_reg[8]/CKbb#27	\mem_reg[8]/CKbb#14	95.941177	$li
Rf886	\mem_reg[15]/CKb#21	\mem_reg[15]/CKb#27	130.576462	$li
Rf887	\mem_reg[15]/CKb#27	\mem_reg[15]/CKb#28	85.458824	$li
Rf888	\mem_reg[15]/CKb#28	\mem_reg[15]/CKb#29	223.529419	$li
Rf889	mux_control_reg/CKb#21	mux_control_reg/CKb#27	130.576462	$li
Rf890	mux_control_reg/CKb#27	mux_control_reg/CKb#28	85.458824	$li
Rf891	mux_control_reg/CKb#28	mux_control_reg/CKb#29	223.529419	$li
Rf892	\mem_reg[10]/Db#5	\mem_reg[10]/Db#6	177.917648	$li
Rf893	\mem_reg[10]/Db#7	\mem_reg[10]/Db#8	18.764706	$li
Rf894	\mem_reg[0]/CKbb#17	\mem_reg[0]/CKbb#18	25.917646	$li
Rf895	\mem_reg[0]/CKbb#19	\mem_reg[0]/CKbb#20	18.764706	$li
Rf896	\mem_reg[2]/CKbb#17	\mem_reg[2]/CKbb#18	18.764706	$li
Rf897	\mem_reg[2]/CKbb#19	\mem_reg[2]/CKbb#20	25.917646	$li
Rf898	\mem_reg[15]/CKbb#23	\mem_reg[15]/CKbb#16	31.188236	$li
Rf899	mux_control_reg/CKbb#23	mux_control_reg/CKbb#16
+ 31.188236	$li
Rf900	\mem_reg[15]/CKbb#25	\mem_reg[15]/CKbb#15	48.564705	$li
Rf901	\mem_reg[15]/CKbb#15	\mem_reg[15]/CKbb#27	87.717644	$li
Rf902	\mem_reg[15]/CKbb#27	\mem_reg[15]/CKbb#14	95.941177	$li
Rf903	mux_control_reg/CKbb#25	mux_control_reg/CKbb#15
+ 48.564705	$li
Rf904	mux_control_reg/CKbb#15	mux_control_reg/CKbb#27
+ 87.717644	$li
Rf905	mux_control_reg/CKbb#27	mux_control_reg/CKbb#14
+ 95.941177	$li
Rf906	\mem_reg[0]/CKbb#21	\mem_reg[0]/CKbb#13	280.317627	$li
Rf907	\mem_reg[2]/CKbb#21	\mem_reg[2]/CKbb#13	280.317627	$li
Rf908	\mem_reg[10]/n20#8	\mem_reg[10]/n20#9	177.917648	$li
Rf909	\mem_reg[10]/n20#10	\mem_reg[10]/n20#11	18.764706	$li
Rf910	q#1	q#2	7.500000	$li
Rf911	q#3	q#4	55.666668	$li
Rf912	q_reg_reg/n30#7	q_reg_reg/n30#13	333.398804	$li
Rf913	q_reg_reg/qbint#22	q_reg_reg/qbint#23	17.258823	$li
Rf914	\mem_reg[8]/net367#8	\mem_reg[8]/net367#9	30.811764	$li
Rf915	\mem_reg[8]/net367#10	\mem_reg[8]/net367#11	30.435293	$li
Rf916	\mem_reg[0]/n20#7	\mem_reg[0]/n20#13	31.188236	$li
Rf917	\mem_reg[2]/n20#7	\mem_reg[2]/n20#13	31.188236	$li
Rf918	\mem_reg[15]/net367#8	\mem_reg[15]/net367#9	30.435293	$li
Rf919	\mem_reg[15]/net367#10	\mem_reg[15]/net367#11	30.811764	$li
Rf920	mux_control_reg/net367#8	mux_control_reg/net367#9
+ 30.811764	$li
Rf921	mux_control_reg/net367#10	mux_control_reg/net367#11
+ 30.435293	$li
Rf922	FE_OFN0_q#32	FE_OFN0_q#33	84.423714	$li
Rf923	FE_OFN0_q#34	FE_OFN0_q#35	5.616868	$li
Rf924	\mem_reg[0]/mout#8	\mem_reg[0]/mout#9	25.917646	$li
Rf925	\mem_reg[0]/mout#10	\mem_reg[0]/mout#11	18.764706	$li
Rf926	\mem_reg[2]/mout#8	\mem_reg[2]/mout#9	18.764706	$li
Rf927	\mem_reg[2]/mout#10	\mem_reg[2]/mout#11	25.917646	$li
Rf928	q#5	q#6	7.500000	$li
Rf929	q#7	q#8	5.000000	$li
Rf930	\mem_reg[0]/mout#12	\mem_reg[0]/mout#7	287.094116	$li
Rf931	\mem_reg[2]/mout#12	\mem_reg[2]/mout#7	287.094116	$li
Rf932	q_reg_reg/qbint#21	q_reg_reg/qbint#25	63.074757	$li
Rf933	q_reg_reg/qbint#25	q_reg_reg/qbint#26	48.074757	$li
Rf934	q_reg_reg/qbint#26	q_reg_reg/qbint#19	86.529411	$li
Rf935	q_reg_reg/qbint#25	q_reg_reg/qbint#28	40.316555	$li
Rf936	q_reg_reg/qbint#28	q_reg_reg/qbint#29	31.941177	$li
Rf937	\mem_reg[8]/n30#7	\mem_reg[8]/n30#13	333.398804	$li
Rf938	\mem_reg[8]/qbint#22	\mem_reg[8]/qbint#23	17.258823	$li
Rf939	\mem_reg[0]/n30#8	\mem_reg[0]/n30#9	25.917646	$li
Rf940	\mem_reg[0]/n30#10	\mem_reg[0]/n30#11	18.764706	$li
Rf941	\mem_reg[2]/n30#8	\mem_reg[2]/n30#9	18.764706	$li
Rf942	\mem_reg[2]/n30#10	\mem_reg[2]/n30#11	25.917646	$li
Rf943	\mem_reg[10]/CKbb#17	\mem_reg[10]/CKbb#18	25.917646	$li
Rf944	\mem_reg[10]/CKbb#19	\mem_reg[10]/CKbb#20	18.764706	$li
Rf945	\mem_reg[10]/CKbb#21	\mem_reg[10]/CKbb#13	280.317627	$li
Rf946	\mem_reg[0]/CKb#21	\mem_reg[0]/CKb#27	130.576462	$li
Rf947	\mem_reg[0]/CKb#27	\mem_reg[0]/CKb#28	85.458824	$li
Rf948	\mem_reg[0]/CKb#28	\mem_reg[0]/CKb#29	223.529419	$li
Rf949	\mem_reg[2]/CKb#21	\mem_reg[2]/CKb#27	130.576462	$li
Rf950	\mem_reg[2]/CKb#27	\mem_reg[2]/CKb#28	85.458824	$li
Rf951	\mem_reg[2]/CKb#28	\mem_reg[2]/CKb#29	223.529419	$li
Rf952	\mem_reg[15]/n30#7	\mem_reg[15]/n30#13	333.398804	$li
Rf953	mux_control_reg/n30#7	mux_control_reg/n30#13	333.398804	$li
Rf954	q_reg_reg/net367#7	q_reg_reg/net367#13	335.281128	$li
Rf955	\mem_reg[15]/qbint#22	\mem_reg[15]/qbint#23	17.258823	$li
Rf956	mux_control_reg/qbint#22	mux_control_reg/qbint#23
+ 17.258823	$li
Rf957	UNCONNECTED	UNCONNECTED#2	8.283673	$li
Rf958	UNCONNECTED#3	UNCONNECTED#4	5.616868	$li
Rf959	q#9	q#10	7.500000	$li
Rf960	q#11	q#12	5.000000	$li
Rf961	mem[8]#26	mem[8]#27	5.616868	$li
Rf962	mem[8]#28	mem[8]#29	8.423712	$li
Rf963	\mem_reg[0]/CKbb#23	\mem_reg[0]/CKbb#16	31.188236	$li
Rf964	\mem_reg[2]/CKbb#23	\mem_reg[2]/CKbb#16	31.188236	$li
Rf965	\mem_reg[0]/CKbb#25	\mem_reg[0]/CKbb#15	48.564705	$li
Rf966	\mem_reg[0]/CKbb#15	\mem_reg[0]/CKbb#27	87.717644	$li
Rf967	\mem_reg[0]/CKbb#27	\mem_reg[0]/CKbb#14	95.941177	$li
Rf968	\mem_reg[2]/CKbb#25	\mem_reg[2]/CKbb#15	48.564705	$li
Rf969	\mem_reg[2]/CKbb#15	\mem_reg[2]/CKbb#27	87.717644	$li
Rf970	\mem_reg[2]/CKbb#27	\mem_reg[2]/CKbb#14	95.941177	$li
Rf971	\mem_reg[8]/qbint#21	\mem_reg[8]/qbint#25	63.074757	$li
Rf972	\mem_reg[8]/qbint#25	\mem_reg[8]/qbint#26	48.074757	$li
Rf973	\mem_reg[8]/qbint#26	\mem_reg[8]/qbint#19	86.529411	$li
Rf974	\mem_reg[8]/qbint#25	\mem_reg[8]/qbint#28	40.316555	$li
Rf975	\mem_reg[8]/qbint#28	\mem_reg[8]/qbint#29	31.941177	$li
Rf976	\mem_reg[10]/n20#7	\mem_reg[10]/n20#13	31.188236	$li
Rf977	mem[15]#28	mem[15]#29	84.423714	$li
Rf978	mem[15]#30	mem[15]#31	5.616868	$li
Rf979	FE_OFN1_scan_out_0#55	FE_OFN1_scan_out_0#56	56.283535	$li
Rf980	FE_OFN1_scan_out_0#57	FE_OFN1_scan_out_0#58	8.423712	$li
Rf981	\mem_reg[10]/mout#8	\mem_reg[10]/mout#9	25.917646	$li
Rf982	\mem_reg[10]/mout#10	\mem_reg[10]/mout#11	18.764706	$li
Rf983	\mem_reg[10]/mout#12	\mem_reg[10]/mout#7	287.094116	$li
Rf984	\mem_reg[15]/qbint#21	\mem_reg[15]/qbint#25	63.074757	$li
Rf985	\mem_reg[15]/qbint#25	\mem_reg[15]/qbint#26	48.074757	$li
Rf986	\mem_reg[15]/qbint#26	\mem_reg[15]/qbint#19	86.529411	$li
Rf987	\mem_reg[15]/qbint#25	\mem_reg[15]/qbint#28	40.316555	$li
Rf988	\mem_reg[15]/qbint#28	\mem_reg[15]/qbint#29	31.941177	$li
Rf989	mux_control_reg/qbint#21	mux_control_reg/qbint#25
+ 63.074757	$li
Rf990	mux_control_reg/qbint#25	mux_control_reg/qbint#26
+ 48.074757	$li
Rf991	mux_control_reg/qbint#26	mux_control_reg/qbint#19
+ 86.529411	$li
Rf992	mux_control_reg/qbint#25	mux_control_reg/qbint#28
+ 40.316555	$li
Rf993	mux_control_reg/qbint#28	mux_control_reg/qbint#29
+ 31.941177	$li
Rf994	g493__2398/S0b#9	g493__2398/S0b#10	18.764706	$li
Rf995	g493__2398/S0b#11	g493__2398/S0b#12	25.917646	$li
Rf996	\mem_reg[8]/net367#7	\mem_reg[8]/net367#13	335.281128	$li
Rf997	q#13	q#14	7.500000	$li
Rf998	q#15	q#16	5.000000	$li
Rf999	UNCONNECTED14	UNCONNECTED14#2	5.616868	$li
Rf1000	UNCONNECTED14#3	UNCONNECTED14#4	8.283673	$li
Rf1001	\mem_reg[0]/net367#8	\mem_reg[0]/net367#9	30.811764	$li
Rf1002	\mem_reg[0]/net367#10	\mem_reg[0]/net367#11	30.435293	$li
Rf1003	\mem_reg[2]/net367#8	\mem_reg[2]/net367#9	30.435293	$li
Rf1004	\mem_reg[2]/net367#10	\mem_reg[2]/net367#11	30.811764	$li
Rf1005	\mem_reg[10]/n30#8	\mem_reg[10]/n30#9	25.917646	$li
Rf1006	\mem_reg[10]/n30#10	\mem_reg[10]/n30#11	18.764706	$li
Rf1007	\mem_reg[10]/CKb#21	\mem_reg[10]/CKb#27	130.576462	$li
Rf1008	\mem_reg[10]/CKb#27	\mem_reg[10]/CKb#28	85.458824	$li
Rf1009	\mem_reg[10]/CKb#28	\mem_reg[10]/CKb#29	223.529419	$li
Rf1010	\mem_reg[15]/net367#7	\mem_reg[15]/net367#13	335.281128	$li
Rf1011	mux_control_reg/net367#7	mux_control_reg/net367#13
+ 335.281128	$li
Rf1012	UNCONNECTED7	UNCONNECTED7#2	8.283673	$li
Rf1013	UNCONNECTED7#3	UNCONNECTED7#4	5.616868	$li
Rf1014	UNCONNECTED2	UNCONNECTED2#2	5.616868	$li
Rf1015	UNCONNECTED2#3	UNCONNECTED2#4	8.283673	$li
Rf1016	n_15#17	n_15#14	18.764706	$li
Rf1017	\mem_reg[10]/CKbb#23	\mem_reg[10]/CKbb#16	31.188236	$li
Rf1018	\mem_reg[10]/CKbb#25	\mem_reg[10]/CKbb#15	48.564705	$li
Rf1019	\mem_reg[10]/CKbb#15	\mem_reg[10]/CKbb#27	87.717644	$li
Rf1020	\mem_reg[10]/CKbb#27	\mem_reg[10]/CKbb#14	95.941177	$li
Rf1021	g494__5107/S0b#8	g494__5107/S0b#9	15.000000	$li
Rf1022	g494__5107/S0b#10	g494__5107/S0b#11	15.000000	$li
Rf1023	\mem_reg[0]/n30#7	\mem_reg[0]/n30#13	333.398804	$li
Rf1024	\mem_reg[2]/n30#7	\mem_reg[2]/n30#13	333.398804	$li
Rf1025	q#17	q#18	7.500000	$li
Rf1026	q#19	q#20	5.000000	$li
Rf1027	\mem_reg[0]/qbint#22	\mem_reg[0]/qbint#23	17.258823	$li
Rf1028	\mem_reg[2]/qbint#22	\mem_reg[2]/qbint#23	17.258823	$li
Rf1029	FE_OFN1_scan_out_0#59	FE_OFN1_scan_out_0#60	3.388236	$li
Rf1030	FE_OFN1_scan_out_0#60	FE_OFN1_scan_out_0#49	94.434082	$li
Rf1031	g493__2398/n0#30	g493__2398/n0#31	18.011765	$li
Rf1032	g493__2398/n0#32	g493__2398/n0#33	13.147058	$li
Rf1033	g510__3680/net89	g510__3680/net89#2	10.991489	$li
Rf1034	mem[0]#28	mem[0]#29	5.616868	$li
Rf1035	mem[0]#30	mem[0]#31	8.423712	$li
Rf1036	mem[2]#28	mem[2]#29	8.423712	$li
Rf1037	mem[2]#30	mem[2]#31	5.616868	$li
Rf1038	\mem_reg[10]/net367#8	\mem_reg[10]/net367#9	30.811764	$li
Rf1039	\mem_reg[10]/net367#10	\mem_reg[10]/net367#11	30.435293	$li
Rf1040	g493__2398/S0b#7	g493__2398/S0b#8	107.821373	$li
Rf1041	g493__2398/S0b#8	g493__2398/S0b#15	269.835297	$li
Rf1042	n_13#14	n_13#7	16.129412	$li
Rf1043	addr[2]#21	addr[2]#27	16.129412	$li
Rf1044	n_8#39	n_8#28	18.388235	$li
Rf1045	n_12#28	n_12#29	9.517647	$li
Rf1046	n_12#30	n_12#31	8.566667	$li
Rf1047	\mem_reg[0]/qbint#21	\mem_reg[0]/qbint#25	63.074757	$li
Rf1048	\mem_reg[0]/qbint#25	\mem_reg[0]/qbint#26	48.074757	$li
Rf1049	\mem_reg[0]/qbint#26	\mem_reg[0]/qbint#19	86.529411	$li
Rf1050	\mem_reg[0]/qbint#25	\mem_reg[0]/qbint#28	40.316555	$li
Rf1051	\mem_reg[0]/qbint#28	\mem_reg[0]/qbint#29	31.941177	$li
Rf1052	\mem_reg[2]/qbint#21	\mem_reg[2]/qbint#25	63.074757	$li
Rf1053	\mem_reg[2]/qbint#25	\mem_reg[2]/qbint#26	48.074757	$li
Rf1054	\mem_reg[2]/qbint#26	\mem_reg[2]/qbint#19	86.529411	$li
Rf1055	\mem_reg[2]/qbint#25	\mem_reg[2]/qbint#28	40.316555	$li
Rf1056	\mem_reg[2]/qbint#28	\mem_reg[2]/qbint#29	31.941177	$li
Rf1057	q#21	q#22	7.500000	$li
Rf1058	q#23	q#24	5.000000	$li
Rf1059	g494__5107/S0b#7	g494__5107/S0b#13	247.941177	$li
Rf1060	mem[8]#30	mem[8]#21	16.129412	$li
Rf1061	g494__5107/n0#8	g494__5107/n0#9	15.000000	$li
Rf1062	g494__5107/n0#10	g494__5107/n0#11	15.000000	$li
Rf1063	n_5#10	n_5#11	10.135294	$li
Rf1064	g510__3680/net89#3	g510__3680/net89#4	10.991489	$li
Rf1065	\mem_reg[0]/net367#7	\mem_reg[0]/net367#13	335.281128	$li
Rf1066	\mem_reg[2]/net367#7	\mem_reg[2]/net367#13	335.281128	$li
Rf1067	\mem_reg[10]/n30#7	\mem_reg[10]/n30#13	333.398804	$li
Rf1068	UNCONNECTED9	UNCONNECTED9#2	5.616868	$li
Rf1069	UNCONNECTED9#3	UNCONNECTED9#4	8.283673	$li
Rf1070	UNCONNECTED1	UNCONNECTED1#2	8.283673	$li
Rf1071	UNCONNECTED1#3	UNCONNECTED1#4	5.616868	$li
Rf1072	\mem_reg[10]/qbint#22	\mem_reg[10]/qbint#23	17.258823	$li
Rf1073	addr[3]#14	addr[3]#17	30.870588	$li
Rf1074	addr[3]#17	addr[3]#13	53.023529	$li
Rf1075	addr[3]#14	addr[3]#15	138.013397	$li
Rf1076	n_5#12	n_5#13	6.505882	$li
Rf1077	mux#1	mux#2	23.688236	$li
Rf1078	mux#3	mux#4	34.741177	$li
Rf1079	q#25	q#26	7.500000	$li
Rf1080	q#27	q#28	5.000000	$li
Rf1081	mem[10]#26	mem[10]#21	18.011765	$li
Rf1082	mem[10]#28	mem[10]#29	5.616868	$li
Rf1083	mem[10]#30	mem[10]#31	8.423712	$li
Rf1084	n_14#14	n_14#7	24.411764	$li
Rf1085	x#17	x#14	87.658829	$li
Rf1086	g510__3680/net89#5	g510__3680/net89#6	37.376472	$li
Rf1087	\mem_reg[10]/qbint#21	\mem_reg[10]/qbint#25	63.074757	$li
Rf1088	\mem_reg[10]/qbint#25	\mem_reg[10]/qbint#26	48.074757	$li
Rf1089	\mem_reg[10]/qbint#26	\mem_reg[10]/qbint#19	86.529411	$li
Rf1090	\mem_reg[10]/qbint#25	\mem_reg[10]/qbint#28	40.316555	$li
Rf1091	\mem_reg[10]/qbint#28	\mem_reg[10]/qbint#29	31.941177	$li
Rf1092	g494__5107/n0#7	g494__5107/n0#13	15.000000	$li
Rf1093	n_7#39	n_7#28	50.764706	$li
Rf1094	n_15#19	n_15#20	10.991489	$li
Rf1095	n_15#21	n_15#22	84.423714	$li
Rf1096	mux#5	mux#6	175.688232	$li
Rf1097	mux#7	mux#8	186.741180	$li
Rf1098	q#29	q#30	7.500000	$li
Rf1099	q#31	q#32	5.000000	$li
Rf1100	g493__2398/n0#27	g493__2398/n0#35	85.188232	$li
Rf1101	FE_OFC0_q/n0#105	FE_OFC0_q/n0#123	292.388245	$li
Rf1102	FE_OFC0_q/n0#123	FE_OFC0_q/n0#124	228.423523	$li
Rf1103	\mem_reg[10]/net367#7	\mem_reg[10]/net367#13	335.281128	$li
Rf1104	UNCONNECTED15	UNCONNECTED15#2	5.616868	$li
Rf1105	UNCONNECTED15#3	UNCONNECTED15#4	8.283673	$li
Rf1106	VSS#1	VSS#2	104.198799	$li
Rf1107	VSS#2	VSS#3	13.273622	$li
Rf1108	VSS#3	VSS#4	32.492813	$li
Rf1110	VSS#4	VSS#7	10.988811	$li
Rf1111	VSS#7	VSS#8	12.249621	$li
Rf1112	VSS#8	VSS#9	30.681622	$li
Rf1113	VSS#9	VSS#10	16.857622	$li
Rf1114	VSS#10	VSS#11	26.073622	$li
Rf1115	VSS#11	VSS#12	43.481621	$li
Rf1116	VSS#12	VSS#13	16.857622	$li
Rf1117	VSS#13	VSS#14	26.073622	$li
Rf1118	VSS#14	VSS#15	21.465622	$li
Rf1119	VSS#15	VSS#16	21.465622	$li
Rf1120	VSS#16	VSS#17	27.372812	$li
Rf1122	VSS#17	VSS#19	16.108810	$li
Rf1123	VSS#19	VSS#20	41.612263	$li
Rf1124	VSS#20	VSS#21	8.510903	$li
Rf1125	VSS#21	VSS#22	37.004265	$li
Rf1126	VSS#22	VSS#23	48.089619	$li
Rf1127	VSS#23	VSS#24	11.737621	$li
Rf1128	VSS#24	VSS#25	70.105621	$li
Rf1129	VSS#25	VSS#26	48.089619	$li
Rf1130	VSS#26	VSS#27	34.265621	$li
Rf1131	VSS#27	VSS#28	33.516811	$li
Rf1133	VSS#28	VSS#30	9.964811	$li
Rf1134	VSS#30	VSS#31	8.665622	$li
Rf1135	VSS#31	VSS#32	48.089619	$li
Rf1136	VSS#32	VSS#33	35.801624	$li
Rf1137	VSS#33	VSS#34	69.642952	$li
Rf1138	VSS#34	VSS#35	8.252273	$li
Rf1139	VSS#35	VSS#36	87.703705	$li
Rf1140	VSS#36	VSS#37	26.726383	$li
Rf1141	VSS#37	VSS#38	53.996811	$li
Rf1143	VSS#2	VSS#40	73.927444	$li
Rf1144	VSS#3	VSS#41	73.927444	$li
Rf1145	VSS#7	VSS#42	73.927444	$li
Rf1146	VSS#8	VSS#43	68.627441	$li
Rf1147	VSS#9	VSS#44	73.927444	$li
Rf1148	VSS#10	VSS#45	68.627441	$li
Rf1149	VSS#11	VSS#46	73.927444	$li
Rf1150	VSS#12	VSS#47	73.927444	$li
Rf1151	VSS#13	VSS#48	68.627441	$li
Rf1152	VSS#14	VSS#49	73.927444	$li
Rf1153	VSS#15	VSS#50	68.627441	$li
Rf1154	VSS#16	VSS#51	73.927444	$li
Rf1155	VSS#19	VSS#52	73.927444	$li
Rf1156	VSS#20	VSS#53	77.681396	$li
Rf1157	VSS#21	VSS#54	72.381393	$li
Rf1158	VSS#22	VSS#55	73.927444	$li
Rf1159	VSS#23	VSS#56	68.627441	$li
Rf1160	VSS#24	VSS#57	73.927444	$li
Rf1161	VSS#25	VSS#58	68.627441	$li
Rf1162	VSS#26	VSS#59	68.627441	$li
Rf1163	VSS#27	VSS#60	73.927444	$li
Rf1164	VSS#30	VSS#61	73.927444	$li
Rf1165	VSS#31	VSS#62	68.627441	$li
Rf1166	VSS#32	VSS#63	68.627441	$li
Rf1167	VSS#33	VSS#64	73.927444	$li
Rf1168	VSS#34	VSS#65	74.367874	$li
Rf1169	VSS#35	VSS#66	69.067871	$li
Rf1170	VSS#36	VSS#67	74.754486	$li
Rf1171	VSS#36	VSS#68	80.054489	$li
Rf1172	VSS#37	VSS#69	73.927444	$li
Rf1173	VSS#38	VSS#70	77.523529	$li
Rf1174	VSS#38	VSS#71	33.516811	$li
Rf1175	VSS#71	VSS#72	45.391727	$li
Rf1176	VSS#72	VSS#73	75.632011	$li
Rf1177	VSS#71	VSS#74	73.927444	$li
Rf1178	VSS#72	VSS#75	8.389836	$li
Rf1179	VSS#75	VSS#76	37.095055	$li
Rf1180	VSS#76	VSS#77	60.919758	$li
Rf1181	VSS#75	VSS#78	75.632011	$li
Rf1182	VSS#76	VSS#79	48.248192	$li
Rf1183	VSS#79	VSS#80	66.448456	$li
Rf1184	VSS#79	VSS#81	59.880108	$li
Rf1185	VSS#81	VSS#82	79.123474	$li
Rf1186	VSS#79	VSS#83	79.123474	$li
Rf1187	VSS#81	VSS#84	46.068054	$li
Rf1189	VSS#84	VSS#86	23.540054	$li
Rf1190	VSS#86	VSS#87	73.823471	$li
Rf1191	VSS#81	VSS#88	79.123474	$li
Rf1192	VSS#86	VSS#89	47.592106	$li
Rf1193	VSS#89	VSS#90	73.823471	$li
Rf1194	VSS#86	VSS#91	73.823471	$li
Rf1195	VSS#89	VSS#92	87.016106	$li
Rf1196	VSS#92	VSS#93	73.823471	$li
Rf1197	VSS#89	VSS#94	73.823471	$li
Rf1198	VSS#92	VSS#95	47.592106	$li
Rf1199	VSS#95	VSS#96	73.823471	$li
Rf1200	VSS#92	VSS#97	73.823471	$li
Rf1201	VSS#95	VSS#98	49.140053	$li
Rf1203	VSS#98	VSS#100	64.500053	$li
Rf1204	VSS#100	VSS#101	73.823471	$li
Rf1205	VSS#95	VSS#102	73.823471	$li
Rf1206	VSS#100	VSS#103	87.016106	$li
Rf1207	VSS#103	VSS#104	73.823471	$li
Rf1208	VSS#100	VSS#105	73.823471	$li
Rf1210	VSS#103	VSS#107	73.823471	$li
Rf1211	VSS#5	VSS#4	2.142857	$li
Rf1212	VSS#5	VSS#17	1.000000	$li
Rf1213	VSS#5	VSS#28	1.000000	$li
Rf1214	VSS#5	VSS#38	1.000000	$li
Rf1215	VSS#5	VSS#84	2.142857	$li
Rf1216	VSS#5	VSS#98	1.000000	$li
Rf1217	VDD#1	VDD#2	129.452209	$li
Rf1218	VDD#2	VDD#3	32.492813	$li
Rf1220	VDD#3	VDD#6	10.988811	$li
Rf1221	VDD#6	VDD#7	44.433193	$li
Rf1222	VDD#7	VDD#8	42.385193	$li
Rf1223	VDD#8	VDD#9	18.393621	$li
Rf1224	VDD#9	VDD#10	24.537621	$li
Rf1225	VDD#10	VDD#11	44.134380	$li
Rf1226	VDD#11	VDD#12	41.436489	$li
Rf1227	VDD#12	VDD#13	8.389836	$li
Rf1228	VDD#13	VDD#14	20.066917	$li
Rf1230	VDD#14	VDD#16	16.108810	$li
Rf1231	VDD#16	VDD#17	42.319729	$li
Rf1232	VDD#17	VDD#18	8.389836	$li
Rf1233	VDD#18	VDD#19	36.175728	$li
Rf1234	VDD#19	VDD#20	10.201622	$li
Rf1235	VDD#20	VDD#21	49.625622	$li
Rf1236	VDD#21	VDD#22	62.799732	$li
Rf1237	VDD#22	VDD#23	8.389836	$li
Rf1238	VDD#23	VDD#24	46.927727	$li
Rf1239	VDD#24	VDD#25	32.729622	$li
Rf1240	VDD#25	VDD#26	35.052811	$li
Rf1242	VDD#26	VDD#28	19.180811	$li
Rf1243	VDD#28	VDD#29	27.609621	$li
Rf1244	VDD#29	VDD#30	19.929623	$li
Rf1245	VDD#30	VDD#31	49.625622	$li
Rf1246	VDD#31	VDD#32	48.089619	$li
Rf1247	VDD#32	VDD#33	15.321622	$li
Rf1248	VDD#33	VDD#34	71.641624	$li
Rf1249	VDD#34	VDD#35	15.321622	$li
Rf1250	VDD#35	VDD#36	32.217621	$li
Rf1251	VDD#36	VDD#37	49.388809	$li
Rf1253	VDD#37	VDD#39	57.442917	$li
Rf1254	VDD#39	VDD#40	8.389836	$li
Rf1255	VDD#40	VDD#41	86.351730	$li
Rf1256	VDD#41	VDD#42	21.465622	$li
Rf1257	VDD#42	VDD#43	60.128864	$li
Rf1258	VDD#43	VDD#44	46.068054	$li
Rf1260	VDD#44	VDD#46	23.540054	$li
Rf1261	VDD#46	VDD#47	47.592106	$li
Rf1262	VDD#47	VDD#48	87.016106	$li
Rf1263	VDD#48	VDD#49	47.592106	$li
Rf1264	VDD#49	VDD#50	49.140053	$li
Rf1266	VDD#50	VDD#52	64.500053	$li
Rf1267	VDD#52	VDD#53	87.016106	$li
Rf1269	VDD#2	VDD#55	85.356857	$li
Rf1270	VDD#6	VDD#56	85.356857	$li
Rf1271	VDD#7	VDD#57	91.976654	$li
Rf1272	VDD#7	VDD#58	91.976654	$li
Rf1273	VDD#8	VDD#59	85.356857	$li
Rf1274	VDD#9	VDD#60	85.356857	$li
Rf1275	VDD#10	VDD#61	85.356857	$li
Rf1276	VDD#11	VDD#62	82.660370	$li
Rf1277	VDD#11	VDD#63	91.483902	$li
Rf1278	VDD#12	VDD#64	87.061424	$li
Rf1279	VDD#13	VDD#65	78.237892	$li
Rf1280	VDD#16	VDD#66	85.356857	$li
Rf1281	VDD#17	VDD#67	87.061424	$li
Rf1282	VDD#18	VDD#68	78.237892	$li
Rf1283	VDD#19	VDD#69	85.356857	$li
Rf1284	VDD#20	VDD#70	76.533325	$li
Rf1285	VDD#21	VDD#71	85.356857	$li
Rf1286	VDD#22	VDD#72	78.237892	$li
Rf1287	VDD#23	VDD#73	78.237892	$li
Rf1288	VDD#24	VDD#74	76.533325	$li
Rf1289	VDD#25	VDD#75	76.533325	$li
Rf1290	VDD#28	VDD#76	76.533325	$li
Rf1291	VDD#29	VDD#77	85.356857	$li
Rf1292	VDD#30	VDD#78	76.533325	$li
Rf1293	VDD#31	VDD#79	76.533325	$li
Rf1294	VDD#32	VDD#80	76.533325	$li
Rf1295	VDD#33	VDD#81	76.533325	$li
Rf1296	VDD#34	VDD#82	76.533325	$li
Rf1297	VDD#35	VDD#83	76.533325	$li
Rf1298	VDD#36	VDD#84	76.533325	$li
Rf1299	VDD#39	VDD#85	87.061424	$li
Rf1300	VDD#40	VDD#86	78.237892	$li
Rf1301	VDD#41	VDD#87	76.533325	$li
Rf1302	VDD#42	VDD#88	77.755669	$li
Rf1303	VDD#43	VDD#89	90.552887	$li
Rf1304	VDD#43	VDD#90	90.552887	$li
Rf1305	VDD#46	VDD#91	81.729355	$li
Rf1306	VDD#46	VDD#92	81.729355	$li
Rf1307	VDD#47	VDD#93	81.729355	$li
Rf1308	VDD#47	VDD#94	81.729355	$li
Rf1309	VDD#48	VDD#95	81.729355	$li
Rf1310	VDD#48	VDD#96	81.729355	$li
Rf1311	VDD#49	VDD#97	81.729355	$li
Rf1312	VDD#49	VDD#98	81.729355	$li
Rf1313	VDD#52	VDD#99	81.729355	$li
Rf1314	VDD#52	VDD#100	81.729355	$li
Rf1315	VDD#53	VDD#101	81.729355	$li
Rf1316	VDD#53	VDD#102	81.729355	$li
Rf1317	VDD#4	VDD#3	2.142857	$li
Rf1318	VDD#4	VDD#14	1.000000	$li
Rf1319	VDD#4	VDD#26	1.000000	$li
Rf1320	VDD#4	VDD#37	1.000000	$li
Rf1321	VDD#4	VDD#44	2.142857	$li
Rf1322	VDD#4	VDD#50	1.000000	$li
Rf1323	VSS#108	VSS#109	118.022804	$li
Rf1324	VSS#109	VSS#110	43.481621	$li
Rf1325	VSS#110	VSS#111	45.017624	$li
Rf1326	VSS#111	VSS#112	36.588810	$li
Rf1328	VSS#112	VSS#115	8.428811	$li
Rf1329	VSS#115	VSS#116	14.809622	$li
Rf1330	VSS#116	VSS#117	69.856865	$li
Rf1331	VSS#117	VSS#118	47.840862	$li
Rf1332	VSS#118	VSS#119	21.465622	$li
Rf1333	VSS#119	VSS#120	48.089619	$li
Rf1334	VSS#120	VSS#121	16.857622	$li
Rf1335	VSS#121	VSS#122	48.089619	$li
Rf1336	VSS#122	VSS#123	21.465622	$li
Rf1337	VSS#123	VSS#124	50.924809	$li
Rf1339	VSS#109	VSS#126	73.927444	$li
Rf1340	VSS#110	VSS#127	73.927444	$li
Rf1341	VSS#111	VSS#128	73.927444	$li
Rf1342	VSS#115	VSS#129	75.403915	$li
Rf1343	VSS#116	VSS#130	73.927444	$li
Rf1344	VSS#117	VSS#131	79.123474	$li
Rf1345	VSS#117	VSS#132	73.823471	$li
Rf1346	VSS#118	VSS#133	68.627441	$li
Rf1347	VSS#119	VSS#134	68.627441	$li
Rf1348	VSS#120	VSS#135	68.627441	$li
Rf1349	VSS#121	VSS#136	68.627441	$li
Rf1350	VSS#122	VSS#137	68.627441	$li
Rf1351	VSS#123	VSS#138	68.627441	$li
Rf1352	VSS#124	VSS#139	72.223526	$li
Rf1353	VSS#124	VSS#140	41.196812	$li
Rf1354	VSS#140	VSS#141	70.105621	$li
Rf1355	VSS#141	VSS#142	68.627441	$li
Rf1356	VSS#140	VSS#143	68.627441	$li
Rf1357	VSS#141	VSS#144	16.857622	$li
Rf1358	VSS#144	VSS#145	70.105621	$li
Rf1359	VSS#145	VSS#146	68.627441	$li
Rf1360	VSS#144	VSS#147	68.627441	$li
Rf1361	VSS#145	VSS#148	11.737621	$li
Rf1362	VSS#148	VSS#149	69.856865	$li
Rf1363	VSS#149	VSS#150	79.123474	$li
Rf1364	VSS#148	VSS#151	73.927444	$li
Rf1365	VSS#149	VSS#152	47.840862	$li
Rf1366	VSS#152	VSS#153	21.228811	$li
Rf1368	VSS#153	VSS#155	72.223526	$li
Rf1369	VSS#149	VSS#156	73.823471	$li
Rf1370	VSS#152	VSS#157	68.627441	$li
Rf1371	VSS#153	VSS#158	48.876808	$li
Rf1372	VSS#158	VSS#159	68.627441	$li
Rf1373	VSS#158	VSS#160	16.857622	$li
Rf1374	VSS#160	VSS#161	48.089619	$li
Rf1375	VSS#161	VSS#162	21.465622	$li
Rf1376	VSS#162	VSS#163	68.627441	$li
Rf1377	VSS#160	VSS#164	68.627441	$li
Rf1378	VSS#161	VSS#165	68.627441	$li
Rf1379	VSS#162	VSS#166	48.089619	$li
Rf1380	VSS#166	VSS#167	68.627441	$li
Rf1381	VSS#166	VSS#168	43.481621	$li
Rf1382	VSS#168	VSS#169	70.105621	$li
Rf1383	VSS#169	VSS#170	68.627441	$li
Rf1384	VSS#168	VSS#171	68.627441	$li
Rf1385	VSS#169	VSS#172	16.857622	$li
Rf1386	VSS#172	VSS#173	35.052811	$li
Rf1388	VSS#173	VSS#175	35.052811	$li
Rf1389	VSS#175	VSS#176	68.627441	$li
Rf1390	VSS#172	VSS#177	68.627441	$li
Rf1391	VSS#175	VSS#178	11.737621	$li
Rf1392	VSS#178	VSS#179	39.897621	$li
Rf1393	VSS#179	VSS#180	73.927444	$li
Rf1394	VSS#178	VSS#181	73.927444	$li
Rf1395	VSS#179	VSS#182	29.657621	$li
Rf1396	VSS#182	VSS#183	13.273622	$li
Rf1397	VSS#183	VSS#184	73.927444	$li
Rf1398	VSS#182	VSS#185	68.627441	$li
Rf1399	VSS#183	VSS#186	34.265621	$li
Rf1400	VSS#186	VSS#187	8.665622	$li
Rf1401	VSS#187	VSS#188	73.927444	$li
Rf1402	VSS#186	VSS#189	68.627441	$li
Rf1403	VSS#187	VSS#190	43.481621	$li
Rf1404	VSS#190	VSS#191	73.927444	$li
Rf1405	VSS#190	VSS#192	34.265621	$li
Rf1406	VSS#192	VSS#193	8.665622	$li
Rf1407	VSS#193	VSS#194	73.927444	$li
Rf1408	VSS#192	VSS#195	68.627441	$li
Rf1409	VSS#193	VSS#196	37.004265	$li
Rf1410	VSS#196	VSS#197	8.510903	$li
Rf1411	VSS#197	VSS#198	77.681396	$li
Rf1412	VSS#196	VSS#199	72.381393	$li
Rf1413	VSS#197	VSS#200	42.911453	$li
Rf1415	VSS#200	VSS#202	77.523529	$li
Rf1416	VSS#200	VSS#203	42.732811	$li
Rf1417	VSS#203	VSS#204	73.927444	$li
Rf1418	VSS#203	VSS#205	21.465622	$li
Rf1419	VSS#205	VSS#206	21.465622	$li
Rf1420	VSS#206	VSS#207	73.927444	$li
Rf1421	VSS#205	VSS#208	68.627441	$li
Rf1422	VSS#206	VSS#209	43.481621	$li
Rf1423	VSS#209	VSS#210	73.927444	$li
Rf1424	VSS#209	VSS#211	21.465622	$li
Rf1425	VSS#211	VSS#212	95.903778	$li
Rf1426	VSS#211	VSS#213	68.627441	$li
Rf1427	VSS#113	VSS#112	1.000000	$li
Rf1428	VSS#113	VSS#124	1.000000	$li
Rf1429	VSS#113	VSS#153	1.000000	$li
Rf1430	VSS#113	VSS#173	1.000000	$li
Rf1431	VSS#113	VSS#200	1.000000	$li
Rf1432	VDD#103	VDD#104	129.452209	$li
Rf1433	VDD#104	VDD#105	43.481621	$li
Rf1434	VDD#105	VDD#106	11.500811	$li
Rf1436	VDD#106	VDD#109	79.084816	$li
Rf1437	VDD#109	VDD#110	85.465622	$li
Rf1438	VDD#110	VDD#111	46.316811	$li
Rf1440	VDD#111	VDD#113	23.788811	$li
Rf1441	VDD#113	VDD#114	48.089619	$li
Rf1442	VDD#114	VDD#115	87.513618	$li
Rf1443	VDD#115	VDD#116	48.089619	$li
Rf1444	VDD#116	VDD#117	49.388809	$li
Rf1446	VDD#117	VDD#119	64.748810	$li
Rf1447	VDD#119	VDD#120	87.513618	$li
Rf1448	VDD#120	VDD#121	82.393623	$li
Rf1449	VDD#121	VDD#122	46.316811	$li
Rf1451	VDD#122	VDD#124	23.788811	$li
Rf1452	VDD#124	VDD#125	48.089619	$li
Rf1453	VDD#125	VDD#126	87.513618	$li
Rf1454	VDD#126	VDD#127	48.089619	$li
Rf1455	VDD#127	VDD#128	49.388809	$li
Rf1457	VDD#128	VDD#130	64.748810	$li
Rf1458	VDD#130	VDD#131	87.513618	$li
Rf1459	VDD#131	VDD#132	52.185623	$li
Rf1460	VDD#132	VDD#133	41.196812	$li
Rf1462	VDD#104	VDD#135	85.356857	$li
Rf1463	VDD#105	VDD#136	85.356857	$li
Rf1464	VDD#109	VDD#137	79.574501	$li
Rf1465	VDD#110	VDD#138	85.356857	$li
Rf1466	VDD#113	VDD#139	76.533325	$li
Rf1467	VDD#114	VDD#140	76.533325	$li
Rf1468	VDD#115	VDD#141	76.533325	$li
Rf1469	VDD#116	VDD#142	76.533325	$li
Rf1470	VDD#119	VDD#143	76.533325	$li
Rf1471	VDD#120	VDD#144	76.533325	$li
Rf1472	VDD#121	VDD#145	85.356857	$li
Rf1473	VDD#124	VDD#146	76.533325	$li
Rf1474	VDD#125	VDD#147	76.533325	$li
Rf1475	VDD#126	VDD#148	76.533325	$li
Rf1476	VDD#127	VDD#149	76.533325	$li
Rf1477	VDD#130	VDD#150	76.533325	$li
Rf1478	VDD#131	VDD#151	76.533325	$li
Rf1479	VDD#132	VDD#152	85.356857	$li
Rf1480	VDD#133	VDD#153	46.316811	$li
Rf1481	VDD#153	VDD#154	43.481621	$li
Rf1482	VDD#154	VDD#155	43.481621	$li
Rf1483	VDD#155	VDD#156	43.481621	$li
Rf1484	VDD#156	VDD#157	44.780811	$li
Rf1486	VDD#157	VDD#159	42.732811	$li
Rf1487	VDD#159	VDD#160	43.481621	$li
Rf1488	VDD#160	VDD#161	43.481621	$li
Rf1489	VDD#161	VDD#162	129.349182	$li
Rf1490	VDD#133	VDD#163	88.952942	$li
Rf1491	VDD#153	VDD#164	85.356857	$li
Rf1492	VDD#154	VDD#165	85.356857	$li
Rf1493	VDD#155	VDD#166	85.356857	$li
Rf1494	VDD#156	VDD#167	85.356857	$li
Rf1495	VDD#157	VDD#168	88.952942	$li
Rf1496	VDD#159	VDD#169	85.356857	$li
Rf1497	VDD#160	VDD#170	85.356857	$li
Rf1498	VDD#161	VDD#171	85.356857	$li
Rf1499	VDD#107	VDD#106	1.666667	$li
Rf1500	VDD#107	VDD#111	2.142857	$li
Rf1501	VDD#107	VDD#117	1.000000	$li
Rf1502	VDD#107	VDD#122	2.142857	$li
Rf1503	VDD#107	VDD#128	1.000000	$li
Rf1504	VDD#107	VDD#133	3.750000	$li
Rf1505	VDD#107	VDD#157	1.000000	$li
Rf1507	VDD#173	VDD#174	46.316811	$li
Rf1509	VDD#174	VDD#177	23.788811	$li
Rf1510	VDD#177	VDD#178	10.713622	$li
Rf1511	VDD#178	VDD#179	36.825623	$li
Rf1512	VDD#179	VDD#180	45.017624	$li
Rf1513	VDD#180	VDD#181	41.945621	$li
Rf1514	VDD#181	VDD#182	27.609621	$li
Rf1515	VDD#182	VDD#183	19.929623	$li
Rf1516	VDD#183	VDD#184	27.609621	$li
Rf1517	VDD#184	VDD#185	21.228811	$li
Rf1519	VDD#185	VDD#187	65.401566	$li
Rf1520	VDD#187	VDD#188	48.742382	$li
Rf1521	VDD#188	VDD#189	37.337620	$li
Rf1522	VDD#189	VDD#190	51.161621	$li
Rf1523	VDD#190	VDD#191	24.537621	$li
Rf1524	VDD#191	VDD#192	62.425617	$li
Rf1525	VDD#192	VDD#193	24.537621	$li
Rf1526	VDD#193	VDD#194	35.052811	$li
Rf1528	VDD#194	VDD#196	9.964811	$li
Rf1529	VDD#196	VDD#197	36.825623	$li
Rf1530	VDD#197	VDD#198	12.249621	$li
Rf1531	VDD#198	VDD#199	43.481621	$li
Rf1532	VDD#199	VDD#200	13.273622	$li
Rf1533	VDD#200	VDD#201	47.858582	$li
Rf1534	VDD#201	VDD#202	8.203547	$li
Rf1535	VDD#202	VDD#203	79.090584	$li
Rf1536	VDD#203	VDD#204	48.089619	$li
Rf1537	VDD#204	VDD#205	49.388809	$li
Rf1539	VDD#205	VDD#207	11.500811	$li
Rf1540	VDD#207	VDD#208	52.697620	$li
Rf1541	VDD#208	VDD#209	87.264862	$li
Rf1542	VDD#209	VDD#210	81.896111	$li
Rf1543	VDD#210	VDD#211	46.068054	$li
Rf1545	VDD#211	VDD#213	23.540054	$li
Rf1546	VDD#213	VDD#214	47.592106	$li
Rf1547	VDD#214	VDD#215	87.016106	$li
Rf1548	VDD#215	VDD#216	47.592106	$li
Rf1549	VDD#216	VDD#217	49.140053	$li
Rf1551	VDD#217	VDD#219	64.500053	$li
Rf1552	VDD#219	VDD#220	87.016106	$li
Rf1554	VDD#173	VDD#222	85.356857	$li
Rf1555	VDD#177	VDD#223	76.533325	$li
Rf1556	VDD#178	VDD#224	85.356857	$li
Rf1557	VDD#179	VDD#225	76.533325	$li
Rf1558	VDD#180	VDD#226	85.356857	$li
Rf1559	VDD#181	VDD#227	76.533325	$li
Rf1560	VDD#182	VDD#228	76.533325	$li
Rf1561	VDD#183	VDD#229	76.533325	$li
Rf1562	VDD#184	VDD#230	76.533325	$li
Rf1563	VDD#187	VDD#231	82.660370	$li
Rf1564	VDD#187	VDD#232	82.660370	$li
Rf1565	VDD#188	VDD#233	76.533325	$li
Rf1566	VDD#189	VDD#234	76.533325	$li
Rf1567	VDD#190	VDD#235	85.356857	$li
Rf1568	VDD#191	VDD#236	76.533325	$li
Rf1569	VDD#192	VDD#237	85.356857	$li
Rf1570	VDD#193	VDD#238	76.533325	$li
Rf1571	VDD#196	VDD#239	85.356857	$li
Rf1572	VDD#197	VDD#240	85.356857	$li
Rf1573	VDD#198	VDD#241	85.356857	$li
Rf1574	VDD#199	VDD#242	85.356857	$li
Rf1575	VDD#200	VDD#243	76.533325	$li
Rf1576	VDD#201	VDD#244	76.708710	$li
Rf1577	VDD#202	VDD#245	85.532242	$li
Rf1578	VDD#203	VDD#246	76.533325	$li
Rf1579	VDD#204	VDD#247	76.533325	$li
Rf1580	VDD#207	VDD#248	85.356857	$li
Rf1581	VDD#208	VDD#249	76.533325	$li
Rf1582	VDD#209	VDD#250	90.552887	$li
Rf1583	VDD#209	VDD#251	81.729355	$li
Rf1584	VDD#210	VDD#252	90.552887	$li
Rf1585	VDD#210	VDD#253	90.552887	$li
Rf1586	VDD#213	VDD#254	81.729355	$li
Rf1587	VDD#213	VDD#255	81.729355	$li
Rf1588	VDD#214	VDD#256	81.729355	$li
Rf1589	VDD#214	VDD#257	81.729355	$li
Rf1590	VDD#215	VDD#258	81.729355	$li
Rf1591	VDD#215	VDD#259	81.729355	$li
Rf1592	VDD#216	VDD#260	81.729355	$li
Rf1593	VDD#216	VDD#261	81.729355	$li
Rf1594	VDD#219	VDD#262	81.729355	$li
Rf1595	VDD#219	VDD#263	81.729355	$li
Rf1596	VDD#220	VDD#264	81.729355	$li
Rf1597	VDD#220	VDD#265	81.729355	$li
Rf1598	VDD#175	VDD#174	2.142857	$li
Rf1599	VDD#175	VDD#185	1.000000	$li
Rf1600	VDD#175	VDD#194	1.000000	$li
Rf1601	VDD#175	VDD#205	1.000000	$li
Rf1602	VDD#175	VDD#211	2.142857	$li
Rf1603	VDD#175	VDD#217	1.000000	$li
Rf1605	VDD#267	VDD#268	55.769623	$li
Rf1606	VDD#268	VDD#269	34.265621	$li
Rf1607	VDD#269	VDD#270	35.052811	$li
Rf1609	VDD#267	VDD#273	85.356857	$li
Rf1610	VDD#268	VDD#274	85.356857	$li
Rf1611	VDD#269	VDD#275	76.533325	$li
Rf1612	VDD#270	VDD#276	47.993568	$li
Rf1613	VDD#276	VDD#277	91.483902	$li
Rf1614	VDD#270	VDD#278	80.129410	$li
Rf1615	VDD#276	VDD#279	69.222382	$li
Rf1616	VDD#279	VDD#280	76.533325	$li
Rf1617	VDD#276	VDD#281	82.660370	$li
Rf1618	VDD#279	VDD#282	18.393621	$li
Rf1619	VDD#282	VDD#283	29.145622	$li
Rf1620	VDD#283	VDD#284	76.533325	$li
Rf1621	VDD#282	VDD#285	76.533325	$li
Rf1622	VDD#283	VDD#286	18.393621	$li
Rf1623	VDD#286	VDD#287	68.569618	$li
Rf1624	VDD#287	VDD#288	76.533325	$li
Rf1625	VDD#286	VDD#289	76.533325	$li
Rf1626	VDD#287	VDD#290	46.563141	$li
Rf1627	VDD#290	VDD#291	84.378944	$li
Rf1628	VDD#290	VDD#292	50.934330	$li
Rf1630	VDD#292	VDD#294	35.052811	$li
Rf1631	VDD#294	VDD#295	29.145622	$li
Rf1632	VDD#295	VDD#296	76.533325	$li
Rf1633	VDD#290	VDD#297	84.378944	$li
Rf1634	VDD#294	VDD#298	76.533325	$li
Rf1635	VDD#295	VDD#299	87.513618	$li
Rf1636	VDD#299	VDD#300	76.533325	$li
Rf1637	VDD#299	VDD#301	23.001621	$li
Rf1638	VDD#301	VDD#302	27.609621	$li
Rf1639	VDD#302	VDD#303	85.356857	$li
Rf1640	VDD#301	VDD#304	85.356857	$li
Rf1641	VDD#302	VDD#305	68.569618	$li
Rf1642	VDD#305	VDD#306	18.393621	$li
Rf1643	VDD#306	VDD#307	85.356857	$li
Rf1644	VDD#305	VDD#308	85.356857	$li
Rf1645	VDD#306	VDD#309	60.140812	$li
Rf1647	VDD#309	VDD#311	10.617572	$li
Rf1648	VDD#311	VDD#312	91.483902	$li
Rf1649	VDD#311	VDD#313	92.774384	$li
Rf1650	VDD#313	VDD#314	77.755669	$li
Rf1651	VDD#311	VDD#315	82.660370	$li
Rf1652	VDD#313	VDD#316	6.284263	$li
Rf1653	VDD#316	VDD#317	88.716263	$li
Rf1654	VDD#317	VDD#318	82.393623	$li
Rf1655	VDD#318	VDD#319	10.713622	$li
Rf1656	VDD#319	VDD#320	85.356857	$li
Rf1657	VDD#316	VDD#321	89.110809	$li
Rf1658	VDD#317	VDD#322	76.533325	$li
Rf1659	VDD#318	VDD#323	85.356857	$li
Rf1660	VDD#319	VDD#324	58.604813	$li
Rf1662	VDD#324	VDD#326	23.788811	$li
Rf1663	VDD#326	VDD#327	85.356857	$li
Rf1664	VDD#324	VDD#328	80.129410	$li
Rf1665	VDD#326	VDD#329	24.537621	$li
Rf1666	VDD#329	VDD#330	45.017624	$li
Rf1667	VDD#330	VDD#331	76.533325	$li
Rf1668	VDD#329	VDD#332	76.533325	$li
Rf1669	VDD#330	VDD#333	40.783730	$li
Rf1670	VDD#333	VDD#334	8.389836	$li
Rf1671	VDD#334	VDD#335	78.237892	$li
Rf1672	VDD#333	VDD#336	78.237892	$li
Rf1673	VDD#334	VDD#337	40.783730	$li
Rf1674	VDD#337	VDD#338	45.017624	$li
Rf1675	VDD#338	VDD#339	76.533325	$li
Rf1676	VDD#337	VDD#340	76.533325	$li
Rf1677	VDD#338	VDD#341	48.089619	$li
Rf1678	VDD#341	VDD#342	76.533325	$li
Rf1679	VDD#341	VDD#343	19.929623	$li
Rf1680	VDD#343	VDD#344	52.460812	$li
Rf1682	VDD#344	VDD#346	33.890919	$li
Rf1683	VDD#346	VDD#347	8.389836	$li
Rf1684	VDD#347	VDD#348	78.237892	$li
Rf1685	VDD#343	VDD#349	76.533325	$li
Rf1686	VDD#346	VDD#350	78.237892	$li
Rf1687	VDD#347	VDD#351	86.351730	$li
Rf1688	VDD#351	VDD#352	76.533325	$li
Rf1690	VDD#271	VDD#270	1.000000	$li
Rf1691	VDD#271	VDD#292	1.000000	$li
Rf1692	VDD#271	VDD#309	1.000000	$li
Rf1693	VDD#271	VDD#324	1.000000	$li
Rf1694	VDD#271	VDD#344	1.000000	$li
Rf1696	VSS#215	VSS#216	88.819145	$li
Rf1697	VSS#216	VSS#217	35.705570	$li
Rf1699	VSS#217	VSS#220	23.788811	$li
Rf1700	VSS#220	VSS#221	23.001621	$li
Rf1701	VSS#221	VSS#222	46.553623	$li
Rf1702	VSS#222	VSS#223	23.001621	$li
Rf1703	VSS#223	VSS#224	24.537621	$li
Rf1704	VSS#224	VSS#225	23.001621	$li
Rf1705	VSS#225	VSS#226	63.961624	$li
Rf1706	VSS#226	VSS#227	23.001621	$li
Rf1707	VSS#227	VSS#228	24.537621	$li
Rf1708	VSS#228	VSS#229	23.001621	$li
Rf1709	VSS#229	VSS#230	49.388809	$li
Rf1711	VSS#230	VSS#232	41.196812	$li
Rf1712	VSS#232	VSS#233	23.001621	$li
Rf1713	VSS#233	VSS#234	63.961624	$li
Rf1714	VSS#234	VSS#235	23.001621	$li
Rf1715	VSS#235	VSS#236	58.841621	$li
Rf1716	VSS#236	VSS#237	70.105621	$li
Rf1717	VSS#237	VSS#238	35.801624	$li
Rf1718	VSS#238	VSS#239	11.737621	$li
Rf1719	VSS#239	VSS#240	21.228811	$li
Rf1721	VSS#240	VSS#242	9.964811	$li
Rf1722	VSS#242	VSS#243	44.400948	$li
Rf1723	VSS#243	VSS#244	10.608946	$li
Rf1724	VSS#244	VSS#245	36.208946	$li
Rf1725	VSS#245	VSS#246	11.120946	$li
Rf1726	VSS#246	VSS#247	85.977623	$li
Rf1727	VSS#247	VSS#248	26.726383	$li
Rf1728	VSS#248	VSS#249	87.283142	$li
Rf1729	VSS#249	VSS#250	35.705570	$li
Rf1731	VSS#250	VSS#252	23.788811	$li
Rf1732	VSS#252	VSS#253	23.001621	$li
Rf1733	VSS#253	VSS#254	46.553623	$li
Rf1734	VSS#254	VSS#255	23.001621	$li
Rf1735	VSS#255	VSS#256	24.537621	$li
Rf1736	VSS#256	VSS#257	23.001621	$li
Rf1737	VSS#257	VSS#258	63.961624	$li
Rf1738	VSS#258	VSS#259	23.001621	$li
Rf1739	VSS#259	VSS#260	24.537621	$li
Rf1740	VSS#260	VSS#261	23.001621	$li
Rf1741	VSS#261	VSS#262	49.388809	$li
Rf1743	VSS#262	VSS#264	41.196812	$li
Rf1744	VSS#264	VSS#265	23.001621	$li
Rf1745	VSS#265	VSS#266	63.961624	$li
Rf1746	VSS#266	VSS#267	23.001621	$li
Rf1748	VSS#215	VSS#269	80.054489	$li
Rf1749	VSS#215	VSS#270	80.054489	$li
Rf1750	VSS#216	VSS#271	80.054489	$li
Rf1751	VSS#216	VSS#272	74.754486	$li
Rf1752	VSS#220	VSS#273	73.927444	$li
Rf1753	VSS#221	VSS#274	73.927444	$li
Rf1754	VSS#222	VSS#275	68.627441	$li
Rf1755	VSS#223	VSS#276	68.627441	$li
Rf1756	VSS#224	VSS#277	68.627441	$li
Rf1757	VSS#225	VSS#278	68.627441	$li
Rf1758	VSS#226	VSS#279	68.627441	$li
Rf1759	VSS#227	VSS#280	68.627441	$li
Rf1760	VSS#228	VSS#281	68.627441	$li
Rf1761	VSS#229	VSS#282	68.627441	$li
Rf1762	VSS#232	VSS#283	68.627441	$li
Rf1763	VSS#233	VSS#284	68.627441	$li
Rf1764	VSS#234	VSS#285	68.627441	$li
Rf1765	VSS#235	VSS#286	68.627441	$li
Rf1766	VSS#236	VSS#287	73.927444	$li
Rf1767	VSS#237	VSS#288	68.627441	$li
Rf1768	VSS#238	VSS#289	73.927444	$li
Rf1769	VSS#239	VSS#290	68.627441	$li
Rf1770	VSS#242	VSS#291	73.927444	$li
Rf1771	VSS#243	VSS#292	60.919758	$li
Rf1772	VSS#244	VSS#293	68.627441	$li
Rf1773	VSS#245	VSS#294	60.919758	$li
Rf1774	VSS#246	VSS#295	68.627441	$li
Rf1775	VSS#247	VSS#296	73.927444	$li
Rf1776	VSS#248	VSS#297	74.754486	$li
Rf1777	VSS#248	VSS#298	80.054489	$li
Rf1778	VSS#249	VSS#299	74.754486	$li
Rf1779	VSS#249	VSS#300	80.054489	$li
Rf1780	VSS#252	VSS#301	73.927444	$li
Rf1781	VSS#253	VSS#302	73.927444	$li
Rf1782	VSS#254	VSS#303	68.627441	$li
Rf1783	VSS#255	VSS#304	68.627441	$li
Rf1784	VSS#256	VSS#305	68.627441	$li
Rf1785	VSS#257	VSS#306	68.627441	$li
Rf1786	VSS#258	VSS#307	68.627441	$li
Rf1787	VSS#259	VSS#308	68.627441	$li
Rf1788	VSS#260	VSS#309	68.627441	$li
Rf1789	VSS#261	VSS#310	68.627441	$li
Rf1790	VSS#264	VSS#311	68.627441	$li
Rf1791	VSS#265	VSS#312	68.627441	$li
Rf1792	VSS#266	VSS#313	68.627441	$li
Rf1793	VSS#267	VSS#314	68.627441	$li
Rf1794	VSS#218	VSS#217	1.000000	$li
Rf1795	VSS#218	VSS#230	1.000000	$li
Rf1796	VSS#218	VSS#240	1.000000	$li
Rf1797	VSS#218	VSS#250	1.000000	$li
Rf1798	VSS#218	VSS#262	1.000000	$li
Rf1800	VDD#355	VDD#356	46.316811	$li
Rf1802	VDD#356	VDD#359	23.788811	$li
Rf1803	VDD#359	VDD#360	48.089619	$li
Rf1804	VDD#360	VDD#361	87.513618	$li
Rf1805	VDD#361	VDD#362	48.089619	$li
Rf1806	VDD#362	VDD#363	49.388809	$li
Rf1808	VDD#363	VDD#365	64.748810	$li
Rf1809	VDD#365	VDD#366	87.513618	$li
Rf1810	VDD#366	VDD#367	82.393623	$li
Rf1811	VDD#367	VDD#368	46.316811	$li
Rf1813	VDD#368	VDD#370	23.788811	$li
Rf1814	VDD#370	VDD#371	48.089619	$li
Rf1815	VDD#371	VDD#372	87.513618	$li
Rf1816	VDD#372	VDD#373	48.089619	$li
Rf1817	VDD#373	VDD#374	49.388809	$li
Rf1819	VDD#374	VDD#376	64.748810	$li
Rf1820	VDD#376	VDD#377	87.513618	$li
Rf1821	VDD#377	VDD#378	82.393623	$li
Rf1822	VDD#378	VDD#379	46.316811	$li
Rf1824	VDD#379	VDD#381	23.788811	$li
Rf1825	VDD#381	VDD#382	48.089619	$li
Rf1826	VDD#382	VDD#383	87.513618	$li
Rf1827	VDD#383	VDD#384	48.089619	$li
Rf1828	VDD#384	VDD#385	49.388809	$li
Rf1830	VDD#385	VDD#387	64.748810	$li
Rf1831	VDD#387	VDD#388	87.513618	$li
Rf1832	VDD#388	VDD#389	82.156807	$li
Rf1833	VDD#389	VDD#357	5.000000	$li
Rf1834	VDD#355	VDD#390	85.356857	$li
Rf1835	VDD#359	VDD#391	76.533325	$li
Rf1836	VDD#360	VDD#392	76.533325	$li
Rf1837	VDD#361	VDD#393	76.533325	$li
Rf1838	VDD#362	VDD#394	76.533325	$li
Rf1839	VDD#365	VDD#395	76.533325	$li
Rf1840	VDD#366	VDD#396	76.533325	$li
Rf1841	VDD#367	VDD#397	85.356857	$li
Rf1842	VDD#370	VDD#398	76.533325	$li
Rf1843	VDD#371	VDD#399	76.533325	$li
Rf1844	VDD#372	VDD#400	76.533325	$li
Rf1845	VDD#373	VDD#401	76.533325	$li
Rf1846	VDD#376	VDD#402	76.533325	$li
Rf1847	VDD#377	VDD#403	76.533325	$li
Rf1848	VDD#378	VDD#404	85.356857	$li
Rf1849	VDD#381	VDD#405	76.533325	$li
Rf1850	VDD#382	VDD#406	76.533325	$li
Rf1851	VDD#383	VDD#407	76.533325	$li
Rf1852	VDD#384	VDD#408	76.533325	$li
Rf1853	VDD#387	VDD#409	76.533325	$li
Rf1854	VDD#388	VDD#410	76.533325	$li
Rf1855	VDD#357	VDD#356	2.142857	$li
Rf1856	VDD#357	VDD#363	1.000000	$li
Rf1857	VDD#357	VDD#368	2.142857	$li
Rf1858	VDD#357	VDD#374	1.000000	$li
Rf1859	VDD#357	VDD#379	2.142857	$li
Rf1860	VDD#357	VDD#385	1.000000	$li
Rf1862	VSS#316	VSS#317	46.068054	$li
Rf1864	VSS#317	VSS#320	23.540054	$li
Rf1865	VSS#320	VSS#321	47.592106	$li
Rf1866	VSS#321	VSS#322	87.016106	$li
Rf1867	VSS#322	VSS#323	47.592106	$li
Rf1868	VSS#323	VSS#324	49.140053	$li
Rf1870	VSS#324	VSS#326	64.500053	$li
Rf1871	VSS#326	VSS#327	87.016106	$li
Rf1872	VSS#327	VSS#328	74.002190	$li
Rf1873	VSS#328	VSS#329	8.252273	$li
Rf1874	VSS#329	VSS#330	69.642952	$li
Rf1875	VSS#330	VSS#331	9.177622	$li
Rf1876	VSS#331	VSS#332	38.361622	$li
Rf1877	VSS#332	VSS#333	21.228811	$li
Rf1879	VSS#333	VSS#335	14.572811	$li
Rf1880	VSS#335	VSS#336	51.161621	$li
Rf1881	VSS#336	VSS#337	38.873623	$li
Rf1882	VSS#337	VSS#338	8.665622	$li
Rf1883	VSS#338	VSS#339	87.513618	$li
Rf1884	VSS#339	VSS#340	26.073622	$li
Rf1885	VSS#340	VSS#341	63.961624	$li
Rf1886	VSS#341	VSS#342	23.001621	$li
Rf1887	VSS#342	VSS#343	35.052811	$li
Rf1889	VSS#343	VSS#345	23.788811	$li
Rf1890	VSS#345	VSS#346	23.001621	$li
Rf1891	VSS#346	VSS#347	46.553623	$li
Rf1892	VSS#347	VSS#348	23.001621	$li
Rf1893	VSS#348	VSS#349	24.537621	$li
Rf1894	VSS#349	VSS#350	23.001621	$li
Rf1895	VSS#350	VSS#351	63.961624	$li
Rf1896	VSS#351	VSS#352	23.001621	$li
Rf1897	VSS#352	VSS#353	24.537621	$li
Rf1898	VSS#353	VSS#354	23.001621	$li
Rf1899	VSS#354	VSS#355	49.388809	$li
Rf1901	VSS#355	VSS#357	41.196812	$li
Rf1902	VSS#357	VSS#358	23.001621	$li
Rf1903	VSS#358	VSS#359	63.961624	$li
Rf1904	VSS#359	VSS#360	23.001621	$li
Rf1905	VSS#360	VSS#361	82.156807	$li
Rf1906	VSS#361	VSS#318	5.000000	$li
Rf1907	VSS#316	VSS#362	79.123474	$li
Rf1908	VSS#316	VSS#363	79.123474	$li
Rf1909	VSS#320	VSS#364	73.823471	$li
Rf1910	VSS#320	VSS#365	73.823471	$li
Rf1911	VSS#321	VSS#366	73.823471	$li
Rf1912	VSS#321	VSS#367	73.823471	$li
Rf1913	VSS#322	VSS#368	73.823471	$li
Rf1914	VSS#322	VSS#369	73.823471	$li
Rf1915	VSS#323	VSS#370	73.823471	$li
Rf1916	VSS#323	VSS#371	73.823471	$li
Rf1917	VSS#326	VSS#372	73.823471	$li
Rf1918	VSS#326	VSS#373	73.823471	$li
Rf1919	VSS#327	VSS#374	73.823471	$li
Rf1920	VSS#327	VSS#375	73.823471	$li
Rf1921	VSS#328	VSS#376	74.367874	$li
Rf1922	VSS#329	VSS#377	74.367874	$li
Rf1923	VSS#330	VSS#378	68.627441	$li
Rf1924	VSS#331	VSS#379	73.927444	$li
Rf1925	VSS#332	VSS#380	68.627441	$li
Rf1926	VSS#335	VSS#381	73.927444	$li
Rf1927	VSS#336	VSS#382	68.627441	$li
Rf1928	VSS#337	VSS#383	68.627441	$li
Rf1929	VSS#338	VSS#384	68.627441	$li
Rf1930	VSS#339	VSS#385	73.927444	$li
Rf1931	VSS#340	VSS#386	68.627441	$li
Rf1932	VSS#341	VSS#387	68.627441	$li
Rf1933	VSS#342	VSS#388	68.627441	$li
Rf1934	VSS#345	VSS#389	73.927444	$li
Rf1935	VSS#346	VSS#390	73.927444	$li
Rf1936	VSS#347	VSS#391	68.627441	$li
Rf1937	VSS#348	VSS#392	68.627441	$li
Rf1938	VSS#349	VSS#393	68.627441	$li
Rf1939	VSS#350	VSS#394	68.627441	$li
Rf1940	VSS#351	VSS#395	68.627441	$li
Rf1941	VSS#352	VSS#396	68.627441	$li
Rf1942	VSS#353	VSS#397	68.627441	$li
Rf1943	VSS#354	VSS#398	68.627441	$li
Rf1944	VSS#357	VSS#399	68.627441	$li
Rf1945	VSS#358	VSS#400	68.627441	$li
Rf1946	VSS#359	VSS#401	68.627441	$li
Rf1947	VSS#360	VSS#402	68.627441	$li
Rf1948	VSS#318	VSS#317	2.142857	$li
Rf1949	VSS#318	VSS#324	1.000000	$li
Rf1950	VSS#318	VSS#333	1.000000	$li
Rf1951	VSS#318	VSS#343	1.000000	$li
Rf1952	VSS#318	VSS#355	1.000000	$li
Rf2019	\mem_reg[12]/CKb#20	\mem_reg[12]/CKb#27	15.000000	$li
Rf2020	\mem_reg[12]/CKb#19	\mem_reg[12]/CKb#28	15.000000	$li
Rf2022	\mem_reg[4]/CKb#20	\mem_reg[4]/CKb#27	15.000000	$li
Rf2023	\mem_reg[4]/CKb#19	\mem_reg[4]/CKb#28	15.000000	$li
Rf2025	\mem_reg[1]/CKb#20	\mem_reg[1]/CKb#27	15.000000	$li
Rf2026	\mem_reg[1]/CKb#19	\mem_reg[1]/CKb#28	15.000000	$li
Rf2028	\mem_reg[3]/CKb#20	\mem_reg[3]/CKb#27	15.000000	$li
Rf2029	\mem_reg[3]/CKb#19	\mem_reg[3]/CKb#28	15.000000	$li
Rf2044	\mem_reg[14]/CKb#20	\mem_reg[14]/CKb#27	15.000000	$li
Rf2045	\mem_reg[14]/CKb#19	\mem_reg[14]/CKb#28	15.000000	$li
Rf2055	\mem_reg[7]/CKb#20	\mem_reg[7]/CKb#27	15.000000	$li
Rf2056	\mem_reg[7]/CKb#19	\mem_reg[7]/CKb#28	15.000000	$li
Rf2066	\mem_reg[5]/CKb#20	\mem_reg[5]/CKb#27	15.000000	$li
Rf2067	\mem_reg[5]/CKb#19	\mem_reg[5]/CKb#28	15.000000	$li
Rf2073	\mem_reg[12]/qbint#20	\mem_reg[12]/qbint#26	15.000000	$li
Rf2076	\mem_reg[4]/qbint#20	\mem_reg[4]/qbint#26	15.000000	$li
Rf2079	\mem_reg[1]/qbint#20	\mem_reg[1]/qbint#26	15.000000	$li
Rf2082	\mem_reg[3]/qbint#20	\mem_reg[3]/qbint#26	15.000000	$li
Rf2085	\mem_reg[14]/qbint#20	\mem_reg[14]/qbint#26	15.000000	$li
Rf2102	\mem_reg[7]/qbint#20	\mem_reg[7]/qbint#26	15.000000	$li
Rf2109	\mem_reg[5]/qbint#20	\mem_reg[5]/qbint#26	15.000000	$li
Rf2163	q_control_reg/CKb#20	q_control_reg/CKb#27	15.000000	$li
Rf2164	q_control_reg/CKb#19	q_control_reg/CKb#28	15.000000	$li
Rf2182	\mem_reg[6]/CKb#20	\mem_reg[6]/CKb#27	15.000000	$li
Rf2183	\mem_reg[6]/CKb#19	\mem_reg[6]/CKb#28	15.000000	$li
Rf2185	\mem_reg[11]/CKb#20	\mem_reg[11]/CKb#27	15.000000	$li
Rf2186	\mem_reg[11]/CKb#19	\mem_reg[11]/CKb#28	15.000000	$li
Rf2208	q_control_reg/qbint#20	q_control_reg/qbint#26	15.000000	$li
Rf2213	\mem_reg[13]/CKb#20	\mem_reg[13]/CKb#27	15.000000	$li
Rf2214	\mem_reg[13]/CKb#19	\mem_reg[13]/CKb#28	15.000000	$li
Rf2227	\mem_reg[6]/qbint#20	\mem_reg[6]/qbint#26	15.000000	$li
Rf2230	\mem_reg[11]/qbint#20	\mem_reg[11]/qbint#26	15.000000	$li
Rf2248	\mem_reg[9]/CKb#20	\mem_reg[9]/CKb#27	15.000000	$li
Rf2249	\mem_reg[9]/CKb#19	\mem_reg[9]/CKb#28	15.000000	$li
Rf2251	\mem_reg[13]/qbint#20	\mem_reg[13]/qbint#26	15.000000	$li
Rf2297	\mem_reg[9]/qbint#20	\mem_reg[9]/qbint#26	15.000000	$li
Rf2318	q_reg_reg/CKb#20	q_reg_reg/CKb#27	15.000000	$li
Rf2319	q_reg_reg/CKb#19	q_reg_reg/CKb#28	15.000000	$li
Rf2332	\mem_reg[8]/CKb#20	\mem_reg[8]/CKb#27	15.000000	$li
Rf2333	\mem_reg[8]/CKb#19	\mem_reg[8]/CKb#28	15.000000	$li
Rf2339	\mem_reg[15]/CKb#20	\mem_reg[15]/CKb#27	15.000000	$li
Rf2340	\mem_reg[15]/CKb#19	\mem_reg[15]/CKb#28	15.000000	$li
Rf2342	mux_control_reg/CKb#20	mux_control_reg/CKb#27	15.000000	$li
Rf2343	mux_control_reg/CKb#19	mux_control_reg/CKb#28	15.000000	$li
Rf2358	q_reg_reg/qbint#20	q_reg_reg/qbint#26	15.000000	$li
Rf2363	\mem_reg[0]/CKb#20	\mem_reg[0]/CKb#27	15.000000	$li
Rf2364	\mem_reg[0]/CKb#19	\mem_reg[0]/CKb#28	15.000000	$li
Rf2366	\mem_reg[2]/CKb#20	\mem_reg[2]/CKb#27	15.000000	$li
Rf2367	\mem_reg[2]/CKb#19	\mem_reg[2]/CKb#28	15.000000	$li
Rf2378	\mem_reg[8]/qbint#20	\mem_reg[8]/qbint#26	15.000000	$li
Rf2383	\mem_reg[15]/qbint#20	\mem_reg[15]/qbint#26	15.000000	$li
Rf2386	mux_control_reg/qbint#20	mux_control_reg/qbint#26
+ 15.000000	$li
Rf2390	\mem_reg[10]/CKb#20	\mem_reg[10]/CKb#27	15.000000	$li
Rf2391	\mem_reg[10]/CKb#19	\mem_reg[10]/CKb#28	15.000000	$li
Rf2400	FE_OFN1_scan_out_0#50	FE_OFN1_scan_out_0#60	15.000000	$li
Rf2408	\mem_reg[0]/qbint#20	\mem_reg[0]/qbint#26	15.000000	$li
Rf2411	\mem_reg[2]/qbint#20	\mem_reg[2]/qbint#26	15.000000	$li
Rf2425	\mem_reg[10]/qbint#20	\mem_reg[10]/qbint#26	15.000000	$li
Re1	\mem_reg[12]/SEb#10	\mem_reg[12]/SEb#14	152.239136	$metal1
Re2	\mem_reg[12]/SEb#14	\mem_reg[12]/SEb#16	0.739130	$metal1
Re3	\mem_reg[12]/SEb#16	\mem_reg[12]/SEb#11	152.239136	$metal1
Re4	\mem_reg[4]/SEb#10	\mem_reg[4]/SEb#16	152.239136	$metal1
Re5	\mem_reg[4]/SEb#16	\mem_reg[4]/SEb#14	0.739130	$metal1
Re6	\mem_reg[4]/SEb#14	\mem_reg[4]/SEb#11	152.239136	$metal1
Re7	\mem_reg[1]/SEb#10	\mem_reg[1]/SEb#14	152.239136	$metal1
Re8	\mem_reg[1]/SEb#14	\mem_reg[1]/SEb#16	0.739130	$metal1
Re9	\mem_reg[1]/SEb#16	\mem_reg[1]/SEb#11	152.239136	$metal1
Re10	\mem_reg[3]/SEb#10	\mem_reg[3]/SEb#16	152.239136	$metal1
Re11	\mem_reg[3]/SEb#16	\mem_reg[3]/SEb#14	0.739130	$metal1
Re12	\mem_reg[3]/SEb#14	\mem_reg[3]/SEb#11	152.239136	$metal1
Re13	FE_OFC1_scan_out_0/n0#115	FE_OFC1_scan_out_0/n0#124
+ 76.489754	$metal1
Re14	FE_OFC1_scan_out_0/n0#124	FE_OFC1_scan_out_0/n0#117
+ 152.718277	$metal1
Re15	\mem_reg[14]/SEb#10	\mem_reg[14]/SEb#16	152.239136	$metal1
Re16	\mem_reg[14]/SEb#16	\mem_reg[14]/SEb#14	0.739130	$metal1
Re17	\mem_reg[14]/SEb#14	\mem_reg[14]/SEb#11	152.239136	$metal1
Re18	FE_OFN1_scan_out_0#62	FE_OFN1_scan_out_0#52	152.103363	$metal1
Re20	scan_en#308	scan_en#254	152.000000	$metal1
Re22	mem[12]#22	mem[12]#34	0.003906	$metal1
Re23	mem[4]#22	mem[4]#33	152.425781	$metal1
Re24	scan_en#257	scan_en#311	156.930588	$metal1
Re26	scan_en#313	scan_en#260	152.000000	$metal1
Re28	mem[1]#22	mem[1]#34	0.003906	$metal1
Re29	mem[3]#22	mem[3]#33	152.425781	$metal1
Re31	scan_en#316	scan_en#263	152.000000	$metal1
Re32	mem[14]#22	mem[14]#33	152.425781	$metal1
Re33	scan_en#266	scan_en#319	156.930588	$metal1
Re34	FE_OFC1_scan_out_0/n0#119	FE_OFC1_scan_out_0/n0#129
+ 76.540764	$metal1
Re35	FE_OFC1_scan_out_0/n0#129	FE_OFC1_scan_out_0/n0#121
+ 51.221016	$metal1
Re36	\mem_reg[7]/SEb#10	\mem_reg[7]/SEb#16	152.239136	$metal1
Re37	\mem_reg[7]/SEb#16	\mem_reg[7]/SEb#14	0.739130	$metal1
Re38	\mem_reg[7]/SEb#14	\mem_reg[7]/SEb#11	152.239136	$metal1
Re39	scan_clk#128	scan_clk#164	152.171417	$metal1
Re40	mem[6]#22	mem[6]#33	152.379807	$metal1
Re41	mem[1]#24	mem[1]#36	152.146942	$metal1
Re43	scan_clk#130	scan_clk#166	156.677887	$metal1
Re44	scan_clk#167	scan_clk#132	156.535156	$metal1
Re45	mem[3]#24	mem[3]#35	152.379807	$metal1
Re46	q_control#29	q_control#41	152.146942	$metal1
Re48	scan_clk#134	scan_clk#170	152.177887	$metal1
Re49	\mem_reg[5]/SEb#10	\mem_reg[5]/SEb#14	152.239136	$metal1
Re50	\mem_reg[5]/SEb#14	\mem_reg[5]/SEb#16	0.739130	$metal1
Re51	\mem_reg[5]/SEb#16	\mem_reg[5]/SEb#11	152.239136	$metal1
Re52	mem[7]#22	mem[7]#33	152.204620	$metal1
Re54	scan_en#269	scan_en#321	152.430588	$metal1
Re55	mem[12]#24	mem[12]#36	152.146942	$metal1
Re57	scan_clk#136	scan_clk#172	152.177887	$metal1
Re58	\mem_reg[12]/CKb#23	\mem_reg[12]/CKb#29	152.608688	$metal1
Re59	\mem_reg[12]/CKb#29	\mem_reg[12]/CKb#24	152.608688	$metal1
Re60	\mem_reg[4]/CKb#23	\mem_reg[4]/CKb#29	152.608688	$metal1
Re61	\mem_reg[4]/CKb#29	\mem_reg[4]/CKb#24	152.608688	$metal1
Re62	\mem_reg[1]/CKb#23	\mem_reg[1]/CKb#29	152.608688	$metal1
Re63	\mem_reg[1]/CKb#29	\mem_reg[1]/CKb#24	152.608688	$metal1
Re64	\mem_reg[3]/CKb#23	\mem_reg[3]/CKb#29	152.608688	$metal1
Re65	\mem_reg[3]/CKb#29	\mem_reg[3]/CKb#24	152.608688	$metal1
Re67	scan_en#323	scan_en#272	152.000000	$metal1
Re69	mem[5]#22	mem[5]#34	0.003906	$metal1
Re70	\mem_reg[14]/CKb#23	\mem_reg[14]/CKb#29	152.608688	$metal1
Re71	\mem_reg[14]/CKb#29	\mem_reg[14]/CKb#24	152.608688	$metal1
Re72	\mem_reg[12]/Db#6	\mem_reg[12]/Db#7	153.217392	$metal1
Re73	\mem_reg[12]/Db#7	\mem_reg[12]/Db#3	1.308160	$metal1
Re74	\mem_reg[12]/Db#3	\mem_reg[12]/Db#2	153.217392	$metal1
Re75	\mem_reg[4]/Db#6	\mem_reg[4]/Db#2	1.308160	$metal1
Re76	\mem_reg[4]/Db#2	\mem_reg[4]/Db#3	153.217392	$metal1
Re77	\mem_reg[4]/Db#6	\mem_reg[4]/Db#7	153.217392	$metal1
Re78	\mem_reg[1]/Db#6	\mem_reg[1]/Db#7	153.217392	$metal1
Re79	\mem_reg[1]/Db#7	\mem_reg[1]/Db#3	1.308160	$metal1
Re80	\mem_reg[1]/Db#3	\mem_reg[1]/Db#2	153.217392	$metal1
Re81	\mem_reg[3]/Db#6	\mem_reg[3]/Db#2	1.308160	$metal1
Re82	\mem_reg[3]/Db#2	\mem_reg[3]/Db#3	153.217392	$metal1
Re83	\mem_reg[3]/Db#6	\mem_reg[3]/Db#7	153.217392	$metal1
Re84	\mem_reg[14]/Db#6	\mem_reg[14]/Db#2	1.308160	$metal1
Re85	\mem_reg[14]/Db#2	\mem_reg[14]/Db#3	153.217392	$metal1
Re86	\mem_reg[14]/Db#6	\mem_reg[14]/Db#7	153.217392	$metal1
Re87	mem[5]#24	mem[5]#36	152.146942	$metal1
Re89	scan_clk#138	scan_clk#174	152.177887	$metal1
Re90	scan_clk#140	scan_clk#176	152.171417	$metal1
Re91	mem[14]#24	mem[14]#35	152.379807	$metal1
Re92	\mem_reg[7]/CKb#23	\mem_reg[7]/CKb#29	152.608688	$metal1
Re93	\mem_reg[7]/CKb#29	\mem_reg[7]/CKb#24	152.608688	$metal1
Re94	\mem_reg[12]/CKbb#18	\mem_reg[12]/CKbb#30	152.239136	$metal1
Re95	\mem_reg[12]/CKbb#30	\mem_reg[12]/CKbb#21	0.739130	$metal1
Re96	\mem_reg[12]/CKbb#21	\mem_reg[12]/CKbb#19	152.239136	$metal1
Re97	\mem_reg[4]/CKbb#18	\mem_reg[4]/CKbb#21	152.239136	$metal1
Re98	\mem_reg[4]/CKbb#21	\mem_reg[4]/CKbb#31	0.739130	$metal1
Re99	\mem_reg[4]/CKbb#31	\mem_reg[4]/CKbb#19	152.239136	$metal1
Re100	\mem_reg[1]/CKbb#18	\mem_reg[1]/CKbb#30	152.239136	$metal1
Re101	\mem_reg[1]/CKbb#30	\mem_reg[1]/CKbb#21	0.739130	$metal1
Re102	\mem_reg[1]/CKbb#21	\mem_reg[1]/CKbb#19	152.239136	$metal1
Re103	\mem_reg[3]/CKbb#18	\mem_reg[3]/CKbb#21	152.239136	$metal1
Re104	\mem_reg[3]/CKbb#21	\mem_reg[3]/CKbb#31	0.739130	$metal1
Re105	\mem_reg[3]/CKbb#31	\mem_reg[3]/CKbb#19	152.239136	$metal1
Re106	\mem_reg[5]/CKb#23	\mem_reg[5]/CKb#29	152.608688	$metal1
Re107	\mem_reg[5]/CKb#29	\mem_reg[5]/CKb#24	152.608688	$metal1
Re108	\mem_reg[7]/Db#6	\mem_reg[7]/Db#2	1.308160	$metal1
Re109	\mem_reg[7]/Db#2	\mem_reg[7]/Db#3	153.217392	$metal1
Re110	\mem_reg[7]/Db#6	\mem_reg[7]/Db#7	153.217392	$metal1
Re111	\mem_reg[14]/CKbb#18	\mem_reg[14]/CKbb#21	152.239136	$metal1
Re112	\mem_reg[14]/CKbb#21	\mem_reg[14]/CKbb#31	0.739130	$metal1
Re113	\mem_reg[14]/CKbb#31	\mem_reg[14]/CKbb#19	152.239136	$metal1
Re114	\mem_reg[12]/n20#13	\mem_reg[12]/n20#10	153.547287	$metal1
Re115	\mem_reg[12]/n20#10	\mem_reg[12]/n20#9	153.217392	$metal1
Re116	\mem_reg[4]/n20#13	\mem_reg[4]/n20#9	153.547287	$metal1
Re117	\mem_reg[4]/n20#9	\mem_reg[4]/n20#10	153.217392	$metal1
Re118	\mem_reg[1]/n20#13	\mem_reg[1]/n20#10	153.547287	$metal1
Re119	\mem_reg[1]/n20#10	\mem_reg[1]/n20#9	153.217392	$metal1
Re120	\mem_reg[3]/n20#13	\mem_reg[3]/n20#9	153.547287	$metal1
Re121	\mem_reg[3]/n20#9	\mem_reg[3]/n20#10	153.217392	$metal1
Re122	\mem_reg[5]/Db#6	\mem_reg[5]/Db#7	153.217392	$metal1
Re123	\mem_reg[5]/Db#7	\mem_reg[5]/Db#3	1.308160	$metal1
Re124	\mem_reg[5]/Db#3	\mem_reg[5]/Db#2	153.217392	$metal1
Re125	\mem_reg[12]/mout#9	\mem_reg[12]/mout#12	152.793472	$metal1
Re126	\mem_reg[12]/mout#12	\mem_reg[12]/mout#10	152.423920	$metal1
Re127	\mem_reg[4]/mout#9	\mem_reg[4]/mout#12	152.423920	$metal1
Re128	\mem_reg[4]/mout#12	\mem_reg[4]/mout#10	152.793472	$metal1
Re129	\mem_reg[1]/mout#9	\mem_reg[1]/mout#12	152.793472	$metal1
Re130	\mem_reg[1]/mout#12	\mem_reg[1]/mout#10	152.423920	$metal1
Re131	\mem_reg[3]/mout#9	\mem_reg[3]/mout#12	152.423920	$metal1
Re132	\mem_reg[3]/mout#12	\mem_reg[3]/mout#10	152.793472	$metal1
Re133	\mem_reg[14]/n20#13	\mem_reg[14]/n20#9	153.547287	$metal1
Re134	\mem_reg[14]/n20#9	\mem_reg[14]/n20#10	153.217392	$metal1
Re135	\mem_reg[12]/n30#9	\mem_reg[12]/n30#13	152.793472	$metal1
Re136	\mem_reg[12]/n30#13	\mem_reg[12]/n30#10	152.423920	$metal1
Re137	\mem_reg[4]/n30#9	\mem_reg[4]/n30#13	152.423920	$metal1
Re138	\mem_reg[4]/n30#13	\mem_reg[4]/n30#10	152.793472	$metal1
Re139	\mem_reg[1]/n30#9	\mem_reg[1]/n30#13	152.793472	$metal1
Re140	\mem_reg[1]/n30#13	\mem_reg[1]/n30#10	152.423920	$metal1
Re141	\mem_reg[3]/n30#9	\mem_reg[3]/n30#13	152.423920	$metal1
Re142	\mem_reg[3]/n30#13	\mem_reg[3]/n30#10	152.793472	$metal1
Re143	\mem_reg[14]/mout#9	\mem_reg[14]/mout#12	152.423920	$metal1
Re144	\mem_reg[14]/mout#12	\mem_reg[14]/mout#10	152.793472	$metal1
Re145	\mem_reg[12]/CKbb#25	\mem_reg[12]/CKbb#23	304.739136	$metal1
Re146	\mem_reg[4]/CKbb#23	\mem_reg[4]/CKbb#25	304.739136	$metal1
Re147	\mem_reg[1]/CKbb#25	\mem_reg[1]/CKbb#23	304.739136	$metal1
Re148	\mem_reg[3]/CKbb#23	\mem_reg[3]/CKbb#25	304.739136	$metal1
Re149	\mem_reg[7]/CKbb#18	\mem_reg[7]/CKbb#21	152.239136	$metal1
Re150	\mem_reg[7]/CKbb#21	\mem_reg[7]/CKbb#31	0.739130	$metal1
Re151	\mem_reg[7]/CKbb#31	\mem_reg[7]/CKbb#19	152.239136	$metal1
Re152	\mem_reg[14]/n30#9	\mem_reg[14]/n30#13	152.423920	$metal1
Re153	\mem_reg[14]/n30#13	\mem_reg[14]/n30#10	152.793472	$metal1
Re154	\mem_reg[14]/CKbb#23	\mem_reg[14]/CKbb#25	304.739136	$metal1
Re155	\mem_reg[5]/CKbb#18	\mem_reg[5]/CKbb#30	152.239136	$metal1
Re156	\mem_reg[5]/CKbb#30	\mem_reg[5]/CKbb#21	0.739130	$metal1
Re157	\mem_reg[5]/CKbb#21	\mem_reg[5]/CKbb#19	152.239136	$metal1
Re158	\mem_reg[7]/n20#13	\mem_reg[7]/n20#9	153.547287	$metal1
Re159	\mem_reg[7]/n20#9	\mem_reg[7]/n20#10	153.217392	$metal1
Re160	\mem_reg[12]/net367#9	\mem_reg[12]/net367#13	76.524460	$metal1
Re161	\mem_reg[12]/net367#13	\mem_reg[12]/net367#10	76.573372	$metal1
Re162	\mem_reg[4]/net367#9	\mem_reg[4]/net367#13	76.573372	$metal1
Re163	\mem_reg[4]/net367#13	\mem_reg[4]/net367#10	76.524460	$metal1
Re164	\mem_reg[1]/net367#9	\mem_reg[1]/net367#13	76.524460	$metal1
Re165	\mem_reg[1]/net367#13	\mem_reg[1]/net367#10	76.573372	$metal1
Re166	\mem_reg[3]/net367#9	\mem_reg[3]/net367#13	76.573372	$metal1
Re167	\mem_reg[3]/net367#13	\mem_reg[3]/net367#10	76.524460	$metal1
Re168	\mem_reg[7]/mout#9	\mem_reg[7]/mout#12	152.423920	$metal1
Re169	\mem_reg[7]/mout#12	\mem_reg[7]/mout#10	152.793472	$metal1
Re170	\mem_reg[5]/n20#13	\mem_reg[5]/n20#10	153.547287	$metal1
Re171	\mem_reg[5]/n20#10	\mem_reg[5]/n20#9	153.217392	$metal1
Re172	\mem_reg[7]/n30#9	\mem_reg[7]/n30#13	152.423920	$metal1
Re173	\mem_reg[7]/n30#13	\mem_reg[7]/n30#10	152.793472	$metal1
Re174	\mem_reg[14]/net367#9	\mem_reg[14]/net367#13	76.573372	$metal1
Re175	\mem_reg[14]/net367#13	\mem_reg[14]/net367#10	76.524460	$metal1
Re176	\mem_reg[5]/mout#9	\mem_reg[5]/mout#12	152.793472	$metal1
Re177	\mem_reg[5]/mout#12	\mem_reg[5]/mout#10	152.423920	$metal1
Re178	scan_out[0]#30	scan_out[0]#34	76.565460	$metal1
Re179	scan_out[0]#34	scan_out[0]#35	0.320913	$metal1
Re180	scan_out[0]#35	scan_out[0]#36	0.432471	$metal1
Re181	scan_out[0]#36	scan_out[0]#37	0.432471	$metal1
Re182	scan_out[0]#37	scan_out[0]#38	0.432471	$metal1
Re183	scan_out[0]#38	scan_out[0]#39	0.432471	$metal1
Re184	scan_out[0]#39	scan_out[0]#40	0.432471	$metal1
Re185	scan_out[0]#40	scan_out[0]#41	0.398794	$metal1
Re186	scan_out[0]#41	scan_out[0]#42	0.260249	$metal1
Re187	scan_out[0]#34	scan_out[0]#32	50.954773	$metal1
Re188	scan_out[0]#35	scan_out[0]#28	51.064968	$metal1
Re189	scan_out[0]#35	scan_out[0]#26	76.661888	$metal1
Re190	scan_out[0]#36	scan_out[0]#24	51.064968	$metal1
Re191	scan_out[0]#36	scan_out[0]#22	76.661888	$metal1
Re192	scan_out[0]#37	scan_out[0]#20	51.064968	$metal1
Re193	scan_out[0]#37	scan_out[0]#18	76.661888	$metal1
Re194	scan_out[0]#38	scan_out[0]#16	51.064968	$metal1
Re195	scan_out[0]#38	scan_out[0]#14	76.661888	$metal1
Re196	scan_out[0]#39	scan_out[0]#12	51.064968	$metal1
Re197	scan_out[0]#39	scan_out[0]#10	76.661888	$metal1
Re198	scan_out[0]#40	scan_out[0]#8	51.064968	$metal1
Re199	scan_out[0]#40	scan_out[0]#6	76.661888	$metal1
Re200	scan_out[0]#41	scan_out[0]#2	76.674561	$metal1
Re201	scan_out[0]#42	scan_out[0]#4	50.760506	$metal1
Re202	\mem_reg[12]/qbint#28	\mem_reg[12]/qbint#22	229.184784	$metal1
Re203	\mem_reg[4]/qbint#23	\mem_reg[4]/qbint#28	229.184784	$metal1
Re204	\mem_reg[1]/qbint#28	\mem_reg[1]/qbint#22	229.184784	$metal1
Re205	\mem_reg[3]/qbint#23	\mem_reg[3]/qbint#28	229.184784	$metal1
Re206	\mem_reg[7]/CKbb#23	\mem_reg[7]/CKbb#25	304.739136	$metal1
Re207	\mem_reg[5]/n30#9	\mem_reg[5]/n30#13	152.793472	$metal1
Re208	\mem_reg[5]/n30#13	\mem_reg[5]/n30#10	152.423920	$metal1
Re209	\mem_reg[14]/qbint#23	\mem_reg[14]/qbint#28	229.184784	$metal1
Re210	\mem_reg[5]/CKbb#25	\mem_reg[5]/CKbb#23	304.739136	$metal1
Re211	mem[12]#27	mem[12]#39	76.301895	$metal1
Re212	mem[12]#39	mem[12]#28	51.066391	$metal1
Re213	mem[4]#25	mem[4]#35	0.099010	$metal1
Re214	mem[4]#25	mem[4]#26	76.702896	$metal1
Re215	mem[1]#27	mem[1]#39	0.030357	$metal1
Re216	mem[1]#27	mem[1]#28	51.369568	$metal1
Re217	mem[3]#27	mem[3]#37	4.599010	$metal1
Re218	mem[3]#27	mem[3]#28	76.702896	$metal1
Re219	q_control_reg/SEb#10	q_control_reg/SEb#14	152.239136	$metal1
Re220	q_control_reg/SEb#14	q_control_reg/SEb#16	0.739130	$metal1
Re221	q_control_reg/SEb#16	q_control_reg/SEb#11	152.239136	$metal1
Re222	\mem_reg[7]/net367#9	\mem_reg[7]/net367#13	76.573372	$metal1
Re223	\mem_reg[7]/net367#13	\mem_reg[7]/net367#10	76.524460	$metal1
Re224	mem[14]#27	mem[14]#37	0.099010	$metal1
Re225	mem[14]#27	mem[14]#28	76.702896	$metal1
Re226	UNCONNECTED4#2	UNCONNECTED4#3	127.371384	$metal1
Re227	UNCONNECTED17#2	UNCONNECTED17#3	127.371384	$metal1
Re228	UNCONNECTED10#2	UNCONNECTED10#3	127.371384	$metal1
Re229	UNCONNECTED8#2	UNCONNECTED8#3	127.371384	$metal1
Re230	\mem_reg[5]/net367#9	\mem_reg[5]/net367#13	76.524460	$metal1
Re231	\mem_reg[5]/net367#13	\mem_reg[5]/net367#10	76.573372	$metal1
Re232	scan_en#275	scan_en#326	156.930588	$metal1
Re234	q_control#31	q_control#45	0.003906	$metal1
Re235	\mem_reg[7]/qbint#23	\mem_reg[7]/qbint#28	229.184784	$metal1
Re236	UNCONNECTED6#2	UNCONNECTED6#3	127.371384	$metal1
Re237	\mem_reg[6]/SEb#10	\mem_reg[6]/SEb#14	152.239136	$metal1
Re238	\mem_reg[6]/SEb#14	\mem_reg[6]/SEb#16	0.739130	$metal1
Re239	\mem_reg[6]/SEb#16	\mem_reg[6]/SEb#11	152.239136	$metal1
Re240	\mem_reg[11]/SEb#10	\mem_reg[11]/SEb#16	152.239136	$metal1
Re241	\mem_reg[11]/SEb#16	\mem_reg[11]/SEb#14	0.739130	$metal1
Re242	\mem_reg[11]/SEb#14	\mem_reg[11]/SEb#11	152.239136	$metal1
Re243	\mem_reg[5]/qbint#28	\mem_reg[5]/qbint#22	229.184784	$metal1
Re244	n_2#22	n_2#33	152.346451	$metal1
Re245	addr[0]#37	addr[0]#47	156.819717	$metal1
Re246	mem[7]#25	mem[7]#36	50.733696	$metal1
Re247	mem[7]#36	mem[7]#26	0.635870	$metal1
Re248	mem[7]#26	mem[7]#38	0.030797	$metal1
Re249	n_8#29	n_8#42	152.202667	$metal1
Re250	scan_en#278	scan_en#328	152.430588	$metal1
Re251	mem[6]#24	mem[6]#35	156.925781	$metal1
Re252	mem[1]#30	mem[1]#42	152.356064	$metal1
Re253	n_2#25	n_2#35	156.611328	$metal1
Re254	mem[11]#22	mem[11]#33	156.925781	$metal1
Re255	scan_en#281	scan_en#330	152.430588	$metal1
Re256	scan_clk#142	scan_clk#178	152.171417	$metal1
Re257	scan_in[0]#8	scan_in[0]#11	152.146942	$metal1
Re259	mem[5]#27	mem[5]#39	76.301895	$metal1
Re260	mem[5]#39	mem[5]#28	51.066391	$metal1
Re261	UNCONNECTED13#2	UNCONNECTED13#3	127.371384	$metal1
Re262	mem[12]#30	mem[12]#42	152.127411	$metal1
Re263	mem[3]#30	mem[3]#40	152.091644	$metal1
Re265	n_7#29	n_7#42	153.132660	$metal1
Re266	n_7#42	n_7#32	50.739517	$metal1
Re267	q_control_reg/CKb#23	q_control_reg/CKb#29	152.608688	$metal1
Re268	q_control_reg/CKb#29	q_control_reg/CKb#24	152.608688	$metal1
Re269	UNCONNECTED0#2	UNCONNECTED0#3	127.371384	$metal1
Re270	n_1#8	n_1#15	152.312805	$metal1
Re271	n_4#9	n_4#15	153.212952	$metal1
Re272	n_4#15	n_4#11	152.098923	$metal1
Re273	q_control_reg/Db#6	q_control_reg/Db#7	153.217392	$metal1
Re274	q_control_reg/Db#7	q_control_reg/Db#3	1.308160	$metal1
Re275	q_control_reg/Db#3	q_control_reg/Db#2	153.217392	$metal1
Re276	g521__1617/net89#5	g521__1617/net89#8	152.462601	$metal1
Re277	g521__1617/net89#8	g521__1617/net89	76.521576	$metal1
Re278	g521__1617/net89#8	g521__1617/net89#3	76.062996	$metal1
Re279	\mem_reg[13]/SEb#10	\mem_reg[13]/SEb#16	152.239136	$metal1
Re280	\mem_reg[13]/SEb#16	\mem_reg[13]/SEb#14	0.739130	$metal1
Re281	\mem_reg[13]/SEb#14	\mem_reg[13]/SEb#11	152.239136	$metal1
Re282	addr[0]#38	addr[0]#49	152.223862	$metal1
Re283	scan_clk#144	scan_clk#180	152.171417	$metal1
Re284	mem[4]#28	mem[4]#38	152.379807	$metal1
Re285	addr[1]#59	addr[1]#76	152.332031	$metal1
Re286	n_2#26	n_2#37	152.125305	$metal1
Re288	mem[9]#22	mem[9]#33	156.873337	$metal1
Re289	scan_clk#146	scan_clk#182	152.177887	$metal1
Re290	g498__5526/net122#5	g498__5526/net122#3	152.467392	$metal1
Re291	g498__5526/net122#3	g498__5526/net122	152.467392	$metal1
Re292	g520__5122/S0b#8	g520__5122/S0b#13	152.692932	$metal1
Re293	g520__5122/S0b#13	g520__5122/S0b#10	152.630432	$metal1
Re295	mem[13]#22	mem[13]#34	0.003906	$metal1
Re296	scan_en#284	scan_en#332	152.430588	$metal1
Re297	n_7#34	n_7#45	152.106064	$metal1
Re298	mem[9]#34	mem[9]#24	152.060547	$metal1
Re299	\mem_reg[6]/CKb#23	\mem_reg[6]/CKb#29	152.608688	$metal1
Re300	\mem_reg[6]/CKb#29	\mem_reg[6]/CKb#24	152.608688	$metal1
Re301	\mem_reg[11]/CKb#23	\mem_reg[11]/CKb#29	152.608688	$metal1
Re302	\mem_reg[11]/CKb#29	\mem_reg[11]/CKb#24	152.608688	$metal1
Re303	mem[5]#30	mem[5]#42	152.217087	$metal1
Re305	addr[1]#78	addr[1]#62	152.056641	$metal1
Re306	n_8#31	n_8#44	152.202667	$metal1
Re307	mem[11]#24	mem[11]#35	152.091644	$metal1
Re309	\mem_reg[6]/Db#6	\mem_reg[6]/Db#7	153.217392	$metal1
Re310	\mem_reg[6]/Db#7	\mem_reg[6]/Db#3	1.308160	$metal1
Re311	\mem_reg[6]/Db#3	\mem_reg[6]/Db#2	153.217392	$metal1
Re312	\mem_reg[11]/Db#6	\mem_reg[11]/Db#2	1.308160	$metal1
Re313	\mem_reg[11]/Db#2	\mem_reg[11]/Db#3	153.217392	$metal1
Re314	\mem_reg[11]/Db#6	\mem_reg[11]/Db#7	153.217392	$metal1
Re315	q_control_reg/CKbb#18	q_control_reg/CKbb#30	152.239136	$metal1
Re316	q_control_reg/CKbb#30	q_control_reg/CKbb#21	0.739130	$metal1
Re317	q_control_reg/CKbb#21	q_control_reg/CKbb#19	152.239136	$metal1
Re318	mem[4]#30	mem[4]#40	152.084045	$metal1
Re319	n_3#10	n_3#15	152.819672	$metal1
Re320	n_3#15	n_3#8	152.537766	$metal1
Re321	n_9#12	n_9#17	76.344246	$metal1
Re322	n_9#17	n_9#9	152.859848	$metal1
Re323	n_9#17	n_9#11	76.555725	$metal1
Re324	g522__2802/net89#6	g522__2802/net89#8	152.462601	$metal1
Re325	g522__2802/net89#8	g522__2802/net89#2	76.521576	$metal1
Re326	g522__2802/net89#8	g522__2802/net89#4	76.062996	$metal1
Re327	g498__5526/net118#5	g498__5526/net118#3	152.467392	$metal1
Re328	g498__5526/net118#3	g498__5526/net118	152.516312	$metal1
Re329	mem[15]#22	mem[15]#33	152.146942	$metal1
Re332	scan_clk#148	scan_clk#185	0.038793	$metal1
Re333	mem[14]#30	mem[14]#40	152.308441	$metal1
Re334	n_0#8	n_0#15	152.312805	$metal1
Re335	addr[1]#80	addr[1]#65	152.080078	$metal1
Re336	q_control_reg/n20#13	q_control_reg/n20#10	153.547287	$metal1
Re337	q_control_reg/n20#10	q_control_reg/n20#9	153.217392	$metal1
Re338	mem[7]#28	mem[7]#40	152.401749	$metal1
Re339	addr[0]#40	addr[0]#51	156.639420	$metal1
Re340	q_control_reg/mout#9	q_control_reg/mout#12	152.793472	$metal1
Re341	q_control_reg/mout#12	q_control_reg/mout#10	152.423920	$metal1
Re342	\mem_reg[13]/CKb#23	\mem_reg[13]/CKb#29	152.608688	$metal1
Re343	\mem_reg[13]/CKb#29	\mem_reg[13]/CKb#24	152.608688	$metal1
Re344	g497__8428/net122#5	g497__8428/net122#3	152.467392	$metal1
Re345	g497__8428/net122#3	g497__8428/net122	152.467392	$metal1
Re346	\mem_reg[9]/SEb#10	\mem_reg[9]/SEb#14	152.239136	$metal1
Re347	\mem_reg[9]/SEb#14	\mem_reg[9]/SEb#16	0.739130	$metal1
Re348	\mem_reg[9]/SEb#16	\mem_reg[9]/SEb#11	152.239136	$metal1
Re349	g520__5122/n0#13	g520__5122/n0#15	152.673599	$metal1
Re350	g520__5122/n0#15	g520__5122/n0#10	152.446304	$metal1
Re351	g520__5122/n0#15	g520__5122/n0#8	152.867493	$metal1
Re352	\mem_reg[6]/CKbb#18	\mem_reg[6]/CKbb#30	152.239136	$metal1
Re353	\mem_reg[6]/CKbb#30	\mem_reg[6]/CKbb#21	0.739130	$metal1
Re354	\mem_reg[6]/CKbb#21	\mem_reg[6]/CKbb#19	152.239136	$metal1
Re355	\mem_reg[11]/CKbb#18	\mem_reg[11]/CKbb#21	152.239136	$metal1
Re356	\mem_reg[11]/CKbb#21	\mem_reg[11]/CKbb#31	0.739130	$metal1
Re357	\mem_reg[11]/CKbb#31	\mem_reg[11]/CKbb#19	152.239136	$metal1
Re358	q_control_reg/n30#9	q_control_reg/n30#13	152.793472	$metal1
Re359	q_control_reg/n30#13	q_control_reg/n30#10	152.423920	$metal1
Re360	addr[0]#43	addr[0]#53	152.110870	$metal1
Re361	n_7#36	n_7#47	156.606064	$metal1
Re362	\mem_reg[13]/Db#6	\mem_reg[13]/Db#2	1.308160	$metal1
Re363	\mem_reg[13]/Db#2	\mem_reg[13]/Db#3	153.217392	$metal1
Re364	\mem_reg[13]/Db#6	\mem_reg[13]/Db#7	153.217392	$metal1
Re365	q_control_reg/CKbb#25	q_control_reg/CKbb#23	304.739136	$metal1
Re366	n_0#10	n_0#13	77.057198	$metal1
Re367	n_0#13	n_0#18	0.029661	$metal1
Re368	addr[1]#68	addr[1]#83	152.091644	$metal1
Re371	scan_en#334	scan_en#287	152.000000	$metal1
Re372	mem[9]#26	mem[9]#37	152.425781	$metal1
Re373	\mem_reg[6]/n20#13	\mem_reg[6]/n20#10	153.547287	$metal1
Re374	\mem_reg[6]/n20#10	\mem_reg[6]/n20#9	153.217392	$metal1
Re375	\mem_reg[11]/n20#13	\mem_reg[11]/n20#9	153.547287	$metal1
Re376	\mem_reg[11]/n20#9	\mem_reg[11]/n20#10	153.217392	$metal1
Re377	n_8#34	n_8#46	153.132660	$metal1
Re378	n_8#46	n_8#35	50.739517	$metal1
Re379	\mem_reg[6]/mout#9	\mem_reg[6]/mout#12	152.793472	$metal1
Re380	\mem_reg[6]/mout#12	\mem_reg[6]/mout#10	152.423920	$metal1
Re381	\mem_reg[11]/mout#9	\mem_reg[11]/mout#12	152.423920	$metal1
Re382	\mem_reg[11]/mout#12	\mem_reg[11]/mout#10	152.793472	$metal1
Re383	n_10#12	n_10#17	76.332596	$metal1
Re384	n_10#17	n_10#9	152.923080	$metal1
Re385	n_10#17	n_10#19	0.555487	$metal1
Re386	n_10#19	n_10#11	76.001404	$metal1
Re387	g497__8428/net118#5	g497__8428/net118#3	152.467392	$metal1
Re388	g497__8428/net118#3	g497__8428/net118	152.516312	$metal1
Re389	g519__1705/S0b#8	g519__1705/S0b#13	152.692932	$metal1
Re390	g519__1705/S0b#13	g519__1705/S0b#10	152.630432	$metal1
Re391	\mem_reg[6]/n30#9	\mem_reg[6]/n30#13	152.793472	$metal1
Re392	\mem_reg[6]/n30#13	\mem_reg[6]/n30#10	152.423920	$metal1
Re393	\mem_reg[11]/n30#9	\mem_reg[11]/n30#13	152.423920	$metal1
Re394	\mem_reg[11]/n30#13	\mem_reg[11]/n30#10	152.793472	$metal1
Re395	q_control_reg/net367#9	q_control_reg/net367#13	76.524460
+ $metal1
Re396	q_control_reg/net367#13	q_control_reg/net367#10
+ 76.573372	$metal1
Re397	mem[6]#26	mem[6]#37	152.127701	$metal1
Re398	\mem_reg[6]/CKbb#25	\mem_reg[6]/CKbb#23	304.739136	$metal1
Re399	\mem_reg[11]/CKbb#23	\mem_reg[11]/CKbb#25	304.739136	$metal1
Re400	\mem_reg[13]/CKbb#18	\mem_reg[13]/CKbb#21	152.239136	$metal1
Re401	\mem_reg[13]/CKbb#21	\mem_reg[13]/CKbb#31	0.739130	$metal1
Re402	\mem_reg[13]/CKbb#31	\mem_reg[13]/CKbb#19	152.239136	$metal1
Re403	mem[13]#24	mem[13]#36	152.217087	$metal1
Re405	addr[1]#86	addr[1]#72	152.056641	$metal1
Re406	addr[2]#22	addr[2]#29	152.091644	$metal1
Re408	scan_clk#186	scan_clk#150	152.035156	$metal1
Re409	mem[10]#22	mem[10]#33	152.146942	$metal1
Re411	q_control_reg/qbint#28	q_control_reg/qbint#22	229.184784	$metal1
Re412	n_6#9	n_6#15	152.283951	$metal1
Re413	addr[1]#70	addr[1]#89	152.250748	$metal1
Re414	\mem_reg[13]/n20#13	\mem_reg[13]/n20#9	153.547287	$metal1
Re415	\mem_reg[13]/n20#9	\mem_reg[13]/n20#10	153.217392	$metal1
Re416	\mem_reg[9]/CKb#23	\mem_reg[9]/CKb#29	152.608688	$metal1
Re417	\mem_reg[9]/CKb#29	\mem_reg[9]/CKb#24	152.608688	$metal1
Re418	\mem_reg[6]/net367#9	\mem_reg[6]/net367#13	76.524460	$metal1
Re419	\mem_reg[6]/net367#13	\mem_reg[6]/net367#10	76.573372	$metal1
Re420	\mem_reg[11]/net367#9	\mem_reg[11]/net367#13	76.573372	$metal1
Re421	\mem_reg[11]/net367#13	\mem_reg[11]/net367#10	76.524460	$metal1
Re422	n_12#23	n_12#33	152.091644	$metal1
Re424	n_2#28	n_2#40	76.957405	$metal1
Re425	n_2#40	n_2#31	76.044228	$metal1
Re426	\mem_reg[13]/mout#9	\mem_reg[13]/mout#12	152.423920	$metal1
Re427	\mem_reg[13]/mout#12	\mem_reg[13]/mout#10	152.793472	$metal1
Re428	q_control#34	q_control#47	4.530357	$metal1
Re429	q_control#34	q_control#35	51.369568	$metal1
Re430	\mem_reg[9]/Db#6	\mem_reg[9]/Db#7	153.217392	$metal1
Re431	\mem_reg[9]/Db#7	\mem_reg[9]/Db#3	1.308160	$metal1
Re432	\mem_reg[9]/Db#3	\mem_reg[9]/Db#2	153.217392	$metal1
Re433	\mem_reg[13]/n30#9	\mem_reg[13]/n30#13	152.423920	$metal1
Re434	\mem_reg[13]/n30#13	\mem_reg[13]/n30#10	152.793472	$metal1
Re435	mem[15]#35	mem[15]#24	152.023438	$metal1
Re436	n_10#15	n_10#22	152.066803	$metal1
Re437	g496__4319/n1#3	g496__4319/n1	304.750000	$metal1
Re438	\mem_reg[6]/qbint#28	\mem_reg[6]/qbint#22	229.184784	$metal1
Re439	\mem_reg[11]/qbint#23	\mem_reg[11]/qbint#28	229.184784	$metal1
Re440	g519__1705/n0#13	g519__1705/n0#15	152.673599	$metal1
Re441	g519__1705/n0#15	g519__1705/n0#10	152.446304	$metal1
Re442	g519__1705/n0#15	g519__1705/n0#8	152.867493	$metal1
Re443	\mem_reg[13]/CKbb#23	\mem_reg[13]/CKbb#25	304.739136	$metal1
Re444	UNCONNECTED11#2	UNCONNECTED11#3	127.371384	$metal1
Re445	n_4#13	n_4#18	152.091644	$metal1
Re447	addr[0]#45	addr[0]#55	152.106064	$metal1
Re448	n_1#10	n_1#13	77.057198	$metal1
Re449	n_1#13	n_1#18	0.029661	$metal1
Re450	n_12#24	n_12#36	152.332031	$metal1
Re451	mem[6]#29	mem[6]#39	76.635872	$metal1
Re452	mem[6]#39	mem[6]#30	50.733696	$metal1
Re453	mem[11]#27	mem[11]#38	50.899723	$metal1
Re454	mem[11]#38	mem[11]#28	76.468567	$metal1
Re455	n_13#12	n_13#17	152.277145	$metal1
Re456	n_13#17	n_13#10	0.787504	$metal1
Re457	n_13#10	n_13#8	152.724136	$metal1
Re458	q_reg_reg/SEb#10	q_reg_reg/SEb#14	152.239136	$metal1
Re459	q_reg_reg/SEb#14	q_reg_reg/SEb#16	0.739130	$metal1
Re460	q_reg_reg/SEb#16	q_reg_reg/SEb#11	152.239136	$metal1
Re461	\mem_reg[13]/net367#9	\mem_reg[13]/net367#13	76.573372	$metal1
Re462	\mem_reg[13]/net367#13	\mem_reg[13]/net367#10	76.524460	$metal1
Re463	\mem_reg[9]/CKbb#18	\mem_reg[9]/CKbb#30	152.239136	$metal1
Re464	\mem_reg[9]/CKbb#30	\mem_reg[9]/CKbb#21	0.739130	$metal1
Re465	\mem_reg[9]/CKbb#21	\mem_reg[9]/CKbb#19	152.239136	$metal1
Re466	n_11#19	n_11#24	152.060547	$metal1
Re468	n_11#18	n_11#27	153.007645	$metal1
Re469	n_11#27	n_11#15	76.510529	$metal1
Re470	g496__4319/n0#6	g496__4319/n0#4	152.467392	$metal1
Re471	g496__4319/n0#4	g496__4319/n0#2	152.467392	$metal1
Re472	UNCONNECTED12#2	UNCONNECTED12#3	127.371384	$metal1
Re473	UNCONNECTED16#2	UNCONNECTED16#3	127.371384	$metal1
Re474	n_8#48	n_8#37	152.068359	$metal1
Re475	q_control#38	q_control#50	152.209442	$metal1
Re478	n_15#15	n_15#25	0.003906	$metal1
Re479	\mem_reg[9]/n20#13	\mem_reg[9]/n20#10	153.547287	$metal1
Re480	\mem_reg[9]/n20#10	\mem_reg[9]/n20#9	153.217392	$metal1
Re481	\mem_reg[8]/SEb#10	\mem_reg[8]/SEb#16	152.239136	$metal1
Re482	\mem_reg[8]/SEb#16	\mem_reg[8]/SEb#14	0.739130	$metal1
Re483	\mem_reg[8]/SEb#14	\mem_reg[8]/SEb#11	152.239136	$metal1
Re484	\mem_reg[13]/qbint#23	\mem_reg[13]/qbint#28	229.184784	$metal1
Re485	mem[0]#22	mem[0]#33	152.134918	$metal1
Re487	addr[2]#24	addr[2]#32	152.091644	$metal1
Re489	\mem_reg[9]/mout#9	\mem_reg[9]/mout#12	152.793472	$metal1
Re490	\mem_reg[9]/mout#12	\mem_reg[9]/mout#10	152.423920	$metal1
Re491	\mem_reg[15]/SEb#10	\mem_reg[15]/SEb#14	152.239136	$metal1
Re492	\mem_reg[15]/SEb#14	\mem_reg[15]/SEb#16	0.739130	$metal1
Re493	\mem_reg[15]/SEb#16	\mem_reg[15]/SEb#11	152.239136	$metal1
Re494	mux_control_reg/SEb#10	mux_control_reg/SEb#16	152.239136	$metal1
Re495	mux_control_reg/SEb#16	mux_control_reg/SEb#14	0.739130	$metal1
Re496	mux_control_reg/SEb#14	mux_control_reg/SEb#11	152.239136	$metal1
Re497	\mem_reg[9]/n30#9	\mem_reg[9]/n30#13	152.793472	$metal1
Re498	\mem_reg[9]/n30#13	\mem_reg[9]/n30#10	152.423920	$metal1
Re499	mem[2]#22	mem[2]#33	152.312805	$metal1
Re500	mem[8]#22	mem[8]#33	152.204620	$metal1
Re502	scan_en#290	scan_en#337	152.430588	$metal1
Re503	n_5#9	n_5#15	152.134918	$metal1
Re504	mem[13]#28	mem[13]#38	76.135231	$metal1
Re505	mem[13]#38	mem[13]#27	0.566389	$metal1
Re506	mem[13]#27	mem[13]#40	0.099010	$metal1
Re507	\mem_reg[9]/CKbb#25	\mem_reg[9]/CKbb#23	304.739136	$metal1
Re508	n_6#12	n_6#17	153.229538	$metal1
Re509	n_6#17	n_6#10	152.131271	$metal1
Re510	scan_en#293	scan_en#339	156.930588	$metal1
Re512	mem[15]#26	mem[15]#39	0.003906	$metal1
Re513	n_12#27	n_12#38	152.091644	$metal1
Re516	FE_OFN1_scan_out_0#53	FE_OFN1_scan_out_0#66	0.003906	$metal1
Re517	scan_en#296	scan_en#341	152.430588	$metal1
Re518	clk#9	clk#11	152.171417	$metal1
Re519	x#15	x#20	152.379807	$metal1
Re520	g509__6783/net89#6	g509__6783/net89#8	152.462601	$metal1
Re521	g509__6783/net89#8	g509__6783/net89#2	76.521576	$metal1
Re522	g509__6783/net89#8	g509__6783/net89#4	76.062996	$metal1
Re523	UNCONNECTED5#2	UNCONNECTED5#3	127.371384	$metal1
Re524	n_7#48	n_7#37	152.080078	$metal1
Re525	n_9#15	n_9#21	152.146637	$metal1
Re526	q_reg_reg/CKb#23	q_reg_reg/CKb#29	152.608688	$metal1
Re527	q_reg_reg/CKb#29	q_reg_reg/CKb#24	152.608688	$metal1
Re528	g495__6260/n1#3	g495__6260/n1	304.750000	$metal1
Re529	\mem_reg[9]/net367#9	\mem_reg[9]/net367#13	76.524460	$metal1
Re530	\mem_reg[9]/net367#13	\mem_reg[9]/net367#10	76.573372	$metal1
Re531	mem[2]#24	mem[2]#35	152.373337	$metal1
Re532	scan_clk#152	scan_clk#189	152.177887	$metal1
Re534	n_3#18	n_3#13	152.074219	$metal1
Re535	q_reg_reg/Db#6	q_reg_reg/Db#7	153.217392	$metal1
Re536	q_reg_reg/Db#7	q_reg_reg/Db#3	1.308160	$metal1
Re537	q_reg_reg/Db#3	q_reg_reg/Db#2	153.217392	$metal1
Re538	\mem_reg[0]/SEb#10	\mem_reg[0]/SEb#16	152.239136	$metal1
Re539	\mem_reg[0]/SEb#16	\mem_reg[0]/SEb#14	0.739130	$metal1
Re540	\mem_reg[0]/SEb#14	\mem_reg[0]/SEb#11	152.239136	$metal1
Re541	\mem_reg[2]/SEb#10	\mem_reg[2]/SEb#14	152.239136	$metal1
Re542	\mem_reg[2]/SEb#14	\mem_reg[2]/SEb#16	0.739130	$metal1
Re543	\mem_reg[2]/SEb#16	\mem_reg[2]/SEb#11	152.239136	$metal1
Re544	scan_clk#190	scan_clk#154	156.535156	$metal1
Re545	mem[7]#30	mem[7]#42	152.146942	$metal1
Re547	mem[11]#30	mem[11]#41	152.373337	$metal1
Re548	scan_clk#156	scan_clk#193	156.677887	$metal1
Re549	\mem_reg[8]/CKb#23	\mem_reg[8]/CKb#29	152.608688	$metal1
Re550	\mem_reg[8]/CKb#29	\mem_reg[8]/CKb#24	152.608688	$metal1
Re551	n_14#12	n_14#17	152.062073	$metal1
Re552	n_14#17	n_14#10	1.009997	$metal1
Re553	n_14#10	n_14#8	152.724136	$metal1
Re554	\mem_reg[9]/qbint#28	\mem_reg[9]/qbint#22	229.184784	$metal1
Re556	mem[0]#24	mem[0]#37	4.503906	$metal1
Re557	scan_en#299	scan_en#343	152.430588	$metal1
Re558	scan_en#302	scan_en#345	152.430588	$metal1
Re559	mem[2]#26	mem[2]#37	152.204620	$metal1
Re561	n_11#21	n_11#30	152.283951	$metal1
Re562	\mem_reg[15]/CKb#23	\mem_reg[15]/CKb#29	152.608688	$metal1
Re563	\mem_reg[15]/CKb#29	\mem_reg[15]/CKb#24	152.608688	$metal1
Re564	mux_control_reg/CKb#23	mux_control_reg/CKb#29	152.608688	$metal1
Re565	mux_control_reg/CKb#29	mux_control_reg/CKb#24	152.608688	$metal1
Re566	\mem_reg[8]/Db#6	\mem_reg[8]/Db#2	1.308160	$metal1
Re567	\mem_reg[8]/Db#2	\mem_reg[8]/Db#3	153.217392	$metal1
Re568	\mem_reg[8]/Db#6	\mem_reg[8]/Db#7	153.217392	$metal1
Re569	g495__6260/n0#6	g495__6260/n0#4	152.467392	$metal1
Re570	g495__6260/n0#4	g495__6260/n0#2	152.467392	$metal1
Re571	mem[9]#29	mem[9]#30	76.702896	$metal1
Re572	mem[9]#30	mem[9]#40	0.099638	$metal1
Re573	\mem_reg[15]/Db#6	\mem_reg[15]/Db#7	153.217392	$metal1
Re574	\mem_reg[15]/Db#7	\mem_reg[15]/Db#3	1.308160	$metal1
Re575	\mem_reg[15]/Db#3	\mem_reg[15]/Db#2	153.217392	$metal1
Re576	mux_control_reg/Db#6	mux_control_reg/Db#2	1.308160	$metal1
Re577	mux_control_reg/Db#2	mux_control_reg/Db#3	153.217392	$metal1
Re578	mux_control_reg/Db#6	mux_control_reg/Db#7	153.217392	$metal1
Re579	\mem_reg[10]/SEb#10	\mem_reg[10]/SEb#16	152.239136	$metal1
Re580	\mem_reg[10]/SEb#16	\mem_reg[10]/SEb#14	0.739130	$metal1
Re581	\mem_reg[10]/SEb#14	\mem_reg[10]/SEb#11	152.239136	$metal1
Re582	q_reg_reg/CKbb#18	q_reg_reg/CKbb#30	152.239136	$metal1
Re583	q_reg_reg/CKbb#30	q_reg_reg/CKbb#21	0.739130	$metal1
Re584	q_reg_reg/CKbb#21	q_reg_reg/CKbb#19	152.239136	$metal1
Re585	UNCONNECTED3#2	UNCONNECTED3#3	127.371384	$metal1
Re586	mem[13]#30	mem[13]#42	152.373337	$metal1
Re587	scan_clk#158	scan_clk#195	152.177887	$metal1
Re588	scan_clk#196	scan_clk#160	156.535156	$metal1
Re589	mem[0]#26	mem[0]#39	152.146942	$metal1
Re591	mem[10]#24	mem[10]#36	152.425781	$metal1
Re592	scan_en#305	scan_en#347	156.930588	$metal1
Re593	q_reg_reg/n20#13	q_reg_reg/n20#10	153.547287	$metal1
Re594	q_reg_reg/n20#10	q_reg_reg/n20#9	153.217392	$metal1
Re595	\mem_reg[8]/CKbb#18	\mem_reg[8]/CKbb#21	152.239136	$metal1
Re596	\mem_reg[8]/CKbb#21	\mem_reg[8]/CKbb#31	0.739130	$metal1
Re597	\mem_reg[8]/CKbb#31	\mem_reg[8]/CKbb#19	152.239136	$metal1
Re598	q_reg_reg/mout#9	q_reg_reg/mout#12	152.793472	$metal1
Re599	q_reg_reg/mout#12	q_reg_reg/mout#10	152.423920	$metal1
Re600	\mem_reg[0]/CKb#23	\mem_reg[0]/CKb#29	152.608688	$metal1
Re601	\mem_reg[0]/CKb#29	\mem_reg[0]/CKb#24	152.608688	$metal1
Re602	\mem_reg[2]/CKb#23	\mem_reg[2]/CKb#29	152.608688	$metal1
Re603	\mem_reg[2]/CKb#29	\mem_reg[2]/CKb#24	152.608688	$metal1
Re604	\mem_reg[15]/CKbb#18	\mem_reg[15]/CKbb#30	152.239136	$metal1
Re605	\mem_reg[15]/CKbb#30	\mem_reg[15]/CKbb#21	0.739130	$metal1
Re606	\mem_reg[15]/CKbb#21	\mem_reg[15]/CKbb#19	152.239136	$metal1
Re607	mux_control_reg/CKbb#18	mux_control_reg/CKbb#21
+ 152.239136	$metal1
Re608	mux_control_reg/CKbb#21	mux_control_reg/CKbb#31
+ 0.739130	$metal1
Re609	mux_control_reg/CKbb#31	mux_control_reg/CKbb#19
+ 152.239136	$metal1
Re610	q_reg_reg/n30#9	q_reg_reg/n30#13	152.793472	$metal1
Re611	q_reg_reg/n30#13	q_reg_reg/n30#10	152.423920	$metal1
Re612	\mem_reg[8]/n20#13	\mem_reg[8]/n20#9	153.547287	$metal1
Re613	\mem_reg[8]/n20#9	\mem_reg[8]/n20#10	153.217392	$metal1
Re614	\mem_reg[0]/Db#6	\mem_reg[0]/Db#2	1.308160	$metal1
Re615	\mem_reg[0]/Db#2	\mem_reg[0]/Db#3	153.217392	$metal1
Re616	\mem_reg[0]/Db#6	\mem_reg[0]/Db#7	153.217392	$metal1
Re617	\mem_reg[2]/Db#6	\mem_reg[2]/Db#7	153.217392	$metal1
Re618	\mem_reg[2]/Db#7	\mem_reg[2]/Db#3	1.308160	$metal1
Re619	\mem_reg[2]/Db#3	\mem_reg[2]/Db#2	153.217392	$metal1
Re620	FE_OFC0_q/n0#115	FE_OFC0_q/n0#124	76.489754	$metal1
Re621	FE_OFC0_q/n0#124	FE_OFC0_q/n0#117	152.718277	$metal1
Re622	q_reg_reg/CKbb#25	q_reg_reg/CKbb#23	304.739136	$metal1
Re623	FE_OFN0_q#36	FE_OFN0_q#31	152.324524	$metal1
Re624	mem[8]#24	mem[8]#36	152.373337	$metal1
Re626	scan_clk#162	scan_clk#200	0.038793	$metal1
Re627	\mem_reg[8]/mout#9	\mem_reg[8]/mout#12	152.423920	$metal1
Re628	\mem_reg[8]/mout#12	\mem_reg[8]/mout#10	152.793472	$metal1
Re629	\mem_reg[15]/n20#13	\mem_reg[15]/n20#10	153.547287	$metal1
Re630	\mem_reg[15]/n20#10	\mem_reg[15]/n20#9	153.217392	$metal1
Re631	mux_control_reg/n20#13	mux_control_reg/n20#9	153.547287	$metal1
Re632	mux_control_reg/n20#9	mux_control_reg/n20#10	153.217392	$metal1
Re633	\mem_reg[8]/n30#9	\mem_reg[8]/n30#13	152.423920	$metal1
Re634	\mem_reg[8]/n30#13	\mem_reg[8]/n30#10	152.793472	$metal1
Re635	\mem_reg[15]/mout#9	\mem_reg[15]/mout#12	152.793472	$metal1
Re636	\mem_reg[15]/mout#12	\mem_reg[15]/mout#10	152.423920	$metal1
Re637	mux_control_reg/mout#9	mux_control_reg/mout#12	152.423920
+ $metal1
Re638	mux_control_reg/mout#12	mux_control_reg/mout#10
+ 152.793472	$metal1
Re639	\mem_reg[10]/CKb#23	\mem_reg[10]/CKb#29	152.608688	$metal1
Re640	\mem_reg[10]/CKb#29	\mem_reg[10]/CKb#24	152.608688	$metal1
Re641	FE_OFC0_q/n0#119	FE_OFC0_q/n0#129	76.540764	$metal1
Re642	FE_OFC0_q/n0#129	FE_OFC0_q/n0#121	51.221016	$metal1
Re643	\mem_reg[8]/CKbb#23	\mem_reg[8]/CKbb#25	304.739136	$metal1
Re644	\mem_reg[15]/n30#9	\mem_reg[15]/n30#13	152.793472	$metal1
Re645	\mem_reg[15]/n30#13	\mem_reg[15]/n30#10	152.423920	$metal1
Re646	mux_control_reg/n30#9	mux_control_reg/n30#13	152.423920	$metal1
Re647	mux_control_reg/n30#13	mux_control_reg/n30#10	152.793472	$metal1
Re648	q_reg_reg/net367#9	q_reg_reg/net367#13	76.524460	$metal1
Re649	q_reg_reg/net367#13	q_reg_reg/net367#10	76.573372	$metal1
Re650	\mem_reg[10]/Db#6	\mem_reg[10]/Db#2	1.308160	$metal1
Re651	\mem_reg[10]/Db#2	\mem_reg[10]/Db#3	153.217392	$metal1
Re652	\mem_reg[10]/Db#6	\mem_reg[10]/Db#7	153.217392	$metal1
Re653	\mem_reg[15]/CKbb#25	\mem_reg[15]/CKbb#23	304.739136	$metal1
Re654	mux_control_reg/CKbb#23	mux_control_reg/CKbb#25
+ 304.739136	$metal1
Re655	\mem_reg[0]/CKbb#18	\mem_reg[0]/CKbb#21	152.239136	$metal1
Re656	\mem_reg[0]/CKbb#21	\mem_reg[0]/CKbb#31	0.739130	$metal1
Re657	\mem_reg[0]/CKbb#31	\mem_reg[0]/CKbb#19	152.239136	$metal1
Re658	\mem_reg[2]/CKbb#18	\mem_reg[2]/CKbb#30	152.239136	$metal1
Re659	\mem_reg[2]/CKbb#30	\mem_reg[2]/CKbb#21	0.739130	$metal1
Re660	\mem_reg[2]/CKbb#21	\mem_reg[2]/CKbb#19	152.239136	$metal1
Re661	q_reg_reg/qbint#28	q_reg_reg/qbint#22	229.184784	$metal1
Re662	\mem_reg[8]/net367#9	\mem_reg[8]/net367#13	76.573372	$metal1
Re663	\mem_reg[8]/net367#13	\mem_reg[8]/net367#10	76.524460	$metal1
Re664	\mem_reg[0]/n20#13	\mem_reg[0]/n20#9	153.547287	$metal1
Re665	\mem_reg[0]/n20#9	\mem_reg[0]/n20#10	153.217392	$metal1
Re666	\mem_reg[2]/n20#13	\mem_reg[2]/n20#10	153.547287	$metal1
Re667	\mem_reg[2]/n20#10	\mem_reg[2]/n20#9	153.217392	$metal1
Re668	\mem_reg[15]/net367#9	\mem_reg[15]/net367#13	76.524460	$metal1
Re669	\mem_reg[15]/net367#13	\mem_reg[15]/net367#10	76.573372	$metal1
Re670	mux_control_reg/net367#9	mux_control_reg/net367#13
+ 76.573372	$metal1
Re671	mux_control_reg/net367#13	mux_control_reg/net367#10
+ 76.524460	$metal1
Re672	\mem_reg[0]/mout#9	\mem_reg[0]/mout#12	152.423920	$metal1
Re673	\mem_reg[0]/mout#12	\mem_reg[0]/mout#10	152.793472	$metal1
Re674	\mem_reg[2]/mout#9	\mem_reg[2]/mout#12	152.793472	$metal1
Re675	\mem_reg[2]/mout#12	\mem_reg[2]/mout#10	152.423920	$metal1
Re676	FE_OFN0_q#33	FE_OFN0_q#39	0.030357	$metal1
Re677	FE_OFN0_q#33	FE_OFN0_q#34	51.369568	$metal1
Re678	\mem_reg[8]/qbint#23	\mem_reg[8]/qbint#28	229.184784	$metal1
Re679	\mem_reg[0]/n30#9	\mem_reg[0]/n30#13	152.423920	$metal1
Re680	\mem_reg[0]/n30#13	\mem_reg[0]/n30#10	152.793472	$metal1
Re681	\mem_reg[2]/n30#9	\mem_reg[2]/n30#13	152.793472	$metal1
Re682	\mem_reg[2]/n30#13	\mem_reg[2]/n30#10	152.423920	$metal1
Re683	\mem_reg[10]/CKbb#18	\mem_reg[10]/CKbb#21	152.239136	$metal1
Re684	\mem_reg[10]/CKbb#21	\mem_reg[10]/CKbb#31	0.739130	$metal1
Re685	\mem_reg[10]/CKbb#31	\mem_reg[10]/CKbb#19	152.239136	$metal1
Re686	\mem_reg[15]/qbint#28	\mem_reg[15]/qbint#22	229.184784	$metal1
Re687	mux_control_reg/qbint#23	mux_control_reg/qbint#28
+ 229.184784	$metal1
Re688	\mem_reg[0]/CKbb#23	\mem_reg[0]/CKbb#25	304.739136	$metal1
Re689	\mem_reg[2]/CKbb#25	\mem_reg[2]/CKbb#23	304.739136	$metal1
Re690	UNCONNECTED#2	UNCONNECTED#3	127.371384	$metal1
Re691	mem[8]#28	mem[8]#38	76.635872	$metal1
Re692	mem[8]#38	mem[8]#27	50.733696	$metal1
Re693	\mem_reg[10]/n20#13	\mem_reg[10]/n20#9	153.547287	$metal1
Re694	\mem_reg[10]/n20#9	\mem_reg[10]/n20#10	153.217392	$metal1
Re695	\mem_reg[10]/mout#9	\mem_reg[10]/mout#12	152.423920	$metal1
Re696	\mem_reg[10]/mout#12	\mem_reg[10]/mout#10	152.793472	$metal1
Re697	mem[15]#29	mem[15]#41	0.030357	$metal1
Re698	mem[15]#29	mem[15]#30	51.369568	$metal1
Re699	FE_OFN1_scan_out_0#56	FE_OFN1_scan_out_0#68	4.599010	$metal1
Re700	FE_OFN1_scan_out_0#56	FE_OFN1_scan_out_0#57	76.702896	$metal1
Re701	g493__2398/S0b#10	g493__2398/S0b#15	152.826080	$metal1
Re702	g493__2398/S0b#15	g493__2398/S0b#11	152.391312	$metal1
Re703	UNCONNECTED14#2	UNCONNECTED14#3	127.371384	$metal1
Re704	\mem_reg[0]/net367#9	\mem_reg[0]/net367#13	76.573372	$metal1
Re705	\mem_reg[0]/net367#13	\mem_reg[0]/net367#10	76.524460	$metal1
Re706	\mem_reg[2]/net367#9	\mem_reg[2]/net367#13	76.524460	$metal1
Re707	\mem_reg[2]/net367#13	\mem_reg[2]/net367#10	76.573372	$metal1
Re708	\mem_reg[10]/n30#9	\mem_reg[10]/n30#13	152.423920	$metal1
Re709	\mem_reg[10]/n30#13	\mem_reg[10]/n30#10	152.793472	$metal1
Re710	\mem_reg[10]/CKbb#23	\mem_reg[10]/CKbb#25	304.739136	$metal1
Re711	UNCONNECTED7#2	UNCONNECTED7#3	127.371384	$metal1
Re712	UNCONNECTED2#2	UNCONNECTED2#3	127.371384	$metal1
Re713	n_15#17	n_15#27	156.807999	$metal1
Re714	g494__5107/S0b#8	g494__5107/S0b#13	152.630432	$metal1
Re715	g494__5107/S0b#13	g494__5107/S0b#10	152.692932	$metal1
Re716	\mem_reg[0]/qbint#23	\mem_reg[0]/qbint#28	229.184784	$metal1
Re717	\mem_reg[2]/qbint#28	\mem_reg[2]/qbint#22	229.184784	$metal1
Re718	FE_OFN1_scan_out_0#59	FE_OFN1_scan_out_0#71	152.081894	$metal1
Re719	\mem_reg[10]/net367#9	\mem_reg[10]/net367#13	76.573372	$metal1
Re720	\mem_reg[10]/net367#13	\mem_reg[10]/net367#10	76.524460	$metal1
Re721	addr[3]#17	addr[3]#21	152.139725	$metal1
Re722	n_13#14	n_13#21	152.223557	$metal1
Re723	n_8#39	n_8#51	152.346451	$metal1
Re725	addr[2]#35	addr[2]#27	152.029297	$metal1
Re726	mem[0]#30	mem[0]#42	76.635872	$metal1
Re727	mem[0]#42	mem[0]#29	50.733696	$metal1
Re728	mem[2]#29	mem[2]#40	76.468567	$metal1
Re729	mem[2]#40	mem[2]#30	50.899723	$metal1
Re730	mem[8]#30	mem[8]#41	152.356064	$metal1
Re731	n_12#31	n_12#41	76.969246	$metal1
Re732	n_12#41	n_12#28	76.044228	$metal1
Re733	g493__2398/n0#35	g493__2398/n0#37	152.060272	$metal1
Re734	g493__2398/n0#37	g493__2398/n0#33	152.714798	$metal1
Re735	g493__2398/n0#37	g493__2398/n0#30	152.865875	$metal1
Re736	\mem_reg[10]/qbint#23	\mem_reg[10]/qbint#28	229.184784	$metal1
Re737	UNCONNECTED9#2	UNCONNECTED9#3	127.371384	$metal1
Re738	UNCONNECTED1#2	UNCONNECTED1#3	127.371384	$metal1
Re739	mem[10]#26	mem[10]#38	152.312805	$metal1
Re740	n_14#14	n_14#21	152.401749	$metal1
Re741	n_5#10	n_5#17	153.202652	$metal1
Re742	n_5#17	n_5#12	152.149536	$metal1
Re743	x#17	x#22	152.312805	$metal1
Re744	mem[10]#29	mem[10]#40	50.899723	$metal1
Re745	mem[10]#40	mem[10]#30	76.468567	$metal1
Re746	g510__3680/net89#6	g510__3680/net89#8	152.462601	$metal1
Re747	g510__3680/net89#8	g510__3680/net89#2	76.521576	$metal1
Re748	g510__3680/net89#8	g510__3680/net89#4	76.062996	$metal1
Re749	g494__5107/n0#13	g494__5107/n0#15	152.673599	$metal1
Re750	g494__5107/n0#15	g494__5107/n0#10	152.867493	$metal1
Re751	g494__5107/n0#15	g494__5107/n0#8	152.446304	$metal1
Re752	n_7#50	n_7#39	152.080078	$metal1
Re753	q#30	q#34	76.565460	$metal1
Re754	q#34	q#35	0.320913	$metal1
Re755	q#35	q#36	0.432471	$metal1
Re756	q#36	q#37	0.432471	$metal1
Re757	q#37	q#38	0.432471	$metal1
Re758	q#38	q#39	0.432471	$metal1
Re759	q#39	q#40	0.432471	$metal1
Re760	q#40	q#41	0.398794	$metal1
Re761	q#41	q#4	0.410976	$metal1
Re762	q#4	q#43	0.099273	$metal1
Re763	q#34	q#32	50.954773	$metal1
Re764	q#35	q#28	51.064968	$metal1
Re765	q#35	q#26	76.661888	$metal1
Re766	q#36	q#24	51.064968	$metal1
Re767	q#36	q#22	76.661888	$metal1
Re768	q#37	q#20	51.064968	$metal1
Re769	q#37	q#18	76.661888	$metal1
Re770	q#38	q#16	51.064968	$metal1
Re771	q#38	q#14	76.661888	$metal1
Re772	q#39	q#12	51.064968	$metal1
Re773	q#39	q#10	76.661888	$metal1
Re774	q#40	q#8	51.064968	$metal1
Re775	q#40	q#6	76.661888	$metal1
Re776	q#41	q#2	76.674561	$metal1
Re777	n_15#19	n_15#22	77.057198	$metal1
Re778	n_15#22	n_15#30	0.029661	$metal1
Re779	UNCONNECTED15#2	UNCONNECTED15#3	127.371384	$metal1
Re780	mux#5	mux#10	1.034520	$metal1
Re781	mux#10	mux#8	0.368655	$metal1
Re782	mux#8	mux#4	152.370697	$metal1
Re783	mux#5	mux#1	152.370697	$metal1
Re784	VDD	VDD#411	0.668750	$metal1
Re785	VDD#411	VDD#412	1.293750	$metal1
Re786	VDD#412	VDD#413	1.581250	$metal1
Re787	VDD#413	VDD#414	1.725000	$metal1
Re788	VDD#414	VDD#415	1.258303	$metal1
Re789	VDD#415	VDD#416	0.250000	$metal1
Re790	VDD#416	VDD#417	1.509375	$metal1
Re791	VDD#417	VDD#418	1.365625	$metal1
Re792	VDD#418	VDD	1.087500	$metal1
Re793	VDD	VDD#419	0.956250	$metal1
Re794	VDD#419	VDD#420	1.581250	$metal1
Re795	VDD#420	VDD#421	1.725000	$metal1
Re796	VDD#421	VDD#422	1.581250	$metal1
Re797	VDD#422	VDD#423	0.164286	$metal1
Re798	VDD#423	VDD#424	1.006250	$metal1
Re799	VDD#424	VDD#425	1.581250	$metal1
Re800	VDD#425	VDD#426	1.437500	$metal1
Re801	VDD#426	VDD	0.225000	$metal1
Re802	VSS	VSS#403	0.956250	$metal1
Re803	VSS#403	VSS#404	1.581250	$metal1
Re804	VSS#404	VSS#405	2.300000	$metal1
Re805	VSS#405	VSS#406	2.156250	$metal1
Re806	VSS#406	VSS#407	1.293750	$metal1
Re807	VSS#407	VSS#408	0.242958	$metal1
Re808	VSS#408	VSS#409	1.437500	$metal1
Re809	VSS#409	VSS	0.225000	$metal1
Re810	VDD	VDD#427	1.100000	$metal1
Re811	VDD#427	VDD#428	2.156250	$metal1
Re812	VDD#428	VDD#429	2.156250	$metal1
Re813	VDD#429	VDD#430	0.920000	$metal1
Re814	VDD#430	VDD#431	0.419271	$metal1
Re815	VDD#431	VDD#432	2.156250	$metal1
Re816	VDD#432	VDD	1.087500	$metal1
Re817	VSS	VSS#410	1.100000	$metal1
Re818	VSS#410	VSS#411	2.156250	$metal1
Re819	VSS#411	VSS#412	2.156250	$metal1
Re820	VSS#412	VSS#413	2.156250	$metal1
Re821	VSS#413	VSS#414	0.164286	$metal1
Re822	VSS#414	VSS#415	1.437500	$metal1
Re823	VSS#415	VSS	1.087500	$metal1
Re824	VDD	VDD#433	0.525000	$metal1
Re825	VDD#433	VDD#434	1.581250	$metal1
Re826	VDD#434	VDD#435	2.156250	$metal1
Re827	VDD#435	VDD#436	2.156250	$metal1
Re828	VDD#436	VDD#437	0.032857	$metal1
Re829	VDD#437	VDD#438	1.581250	$metal1
Re830	VDD#438	VDD#439	1.581250	$metal1
Re831	VDD#439	VDD	1.087500	$metal1
Re832	VSS	VSS#416	0.525000	$metal1
Re833	VSS#416	VSS#417	1.581250	$metal1
Re834	VSS#417	VSS#418	2.156250	$metal1
Re835	VSS#418	VSS#419	2.156250	$metal1
Re836	VSS#419	VSS#420	1.725000	$metal1
Re837	VSS#420	VSS#421	0.955801	$metal1
Re838	VSS#421	VSS#422	1.437500	$metal1
Re839	VSS#422	VSS	1.087500	$metal1
Re840	VDD	VDD#440	0.525000	$metal1
Re841	VDD#440	VDD#441	1.581250	$metal1
Re842	VDD#441	VDD#442	2.156250	$metal1
Re843	VDD#442	VDD#443	2.156250	$metal1
Re844	VDD#443	VDD#444	0.032857	$metal1
Re845	VDD#444	VDD#445	1.581250	$metal1
Re846	VDD#445	VDD#446	1.581250	$metal1
Re847	VDD#446	VDD	1.087500	$metal1
Re848	VSS	VSS#423	1.100000	$metal1
Re849	VSS#423	VSS#424	2.156250	$metal1
Re850	VSS#424	VSS#425	2.156250	$metal1
Re851	VSS#425	VSS#426	2.156250	$metal1
Re852	VSS#426	VSS#427	0.164286	$metal1
Re853	VSS#427	VSS#428	1.437500	$metal1
Re854	VSS#428	VSS	1.087500	$metal1
Re1233	q_control_reg/CKbb#27	q_control_reg/CKbb#30	152.000000	$metal1
Re1275	\mem_reg[6]/CKbb#27	\mem_reg[6]/CKbb#30	152.000000	$metal1
Re1280	\mem_reg[11]/CKbb#27	\mem_reg[11]/CKbb#31	152.000000	$metal1
Re1337	\mem_reg[13]/CKbb#27	\mem_reg[13]/CKbb#31	152.000000	$metal1
Re1410	\mem_reg[9]/CKbb#27	\mem_reg[9]/CKbb#30	152.000000	$metal1
Re1543	q_reg_reg/CKbb#27	q_reg_reg/CKbb#30	152.000000	$metal1
Re1559	\mem_reg[8]/CKbb#27	\mem_reg[8]/CKbb#31	152.000000	$metal1
Re1571	\mem_reg[15]/CKbb#27	\mem_reg[15]/CKbb#30	152.000000	$metal1
Re1576	mux_control_reg/CKbb#27	mux_control_reg/CKbb#31
+ 152.000000	$metal1
Re1622	FE_OFC0_q/n0#123	FE_OFC0_q/n0#129	152.000000	$metal1
Re1645	\mem_reg[0]/CKbb#27	\mem_reg[0]/CKbb#31	152.000000	$metal1
Re1648	\mem_reg[2]/CKbb#27	\mem_reg[2]/CKbb#30	152.000000	$metal1
Re1686	\mem_reg[10]/CKbb#27	\mem_reg[10]/CKbb#31	152.000000	$metal1
Re890	FE_OFC1_scan_out_0/n0#123	FE_OFC1_scan_out_0/n0#129
+ 152.000000	$metal1
Re957	\mem_reg[12]/CKbb#27	\mem_reg[12]/CKbb#30	152.000000	$metal1
Re962	\mem_reg[4]/CKbb#27	\mem_reg[4]/CKbb#31	152.000000	$metal1
Re965	\mem_reg[1]/CKbb#27	\mem_reg[1]/CKbb#30	152.000000	$metal1
Re970	\mem_reg[3]/CKbb#27	\mem_reg[3]/CKbb#31	152.000000	$metal1
Re981	\mem_reg[14]/CKbb#27	\mem_reg[14]/CKbb#31	152.000000	$metal1
Re1800	VDD#106	VDD#411	16.888889	$metal1
Re1801	VDD#111	VDD#412	21.714285	$metal1
Re1802	VDD#117	VDD#413	10.133333	$metal1
Re1803	VDD#122	VDD#414	21.714285	$metal1
Re1804	VDD#128	VDD#416	10.133333	$metal1
Re1805	VDD#133	VDD#417	38.000000	$metal1
Re1806	VDD#157	VDD#418	10.133333	$metal1
Re1807	VDD#356	VDD#419	21.714285	$metal1
Re1808	VDD#363	VDD#420	10.133333	$metal1
Re1809	VDD#368	VDD#421	21.714285	$metal1
Re1810	VDD#374	VDD#422	10.133333	$metal1
Re1811	VDD#379	VDD#424	21.714285	$metal1
Re1812	VDD#385	VDD#425	10.133333	$metal1
Re1813	VDD#389	VDD#426	50.666668	$metal1
Re1814	VSS#317	VSS#403	21.714285	$metal1
Re1815	VSS#324	VSS#404	10.133333	$metal1
Re1816	VSS#333	VSS#405	10.133333	$metal1
Re1817	VSS#343	VSS#406	10.133333	$metal1
Re1818	VSS#355	VSS#408	10.133333	$metal1
Re1819	VSS#361	VSS#409	50.666668	$metal1
Re1820	VDD#270	VDD#427	10.133333	$metal1
Re1821	VDD#292	VDD#428	10.133333	$metal1
Re1822	VDD#309	VDD#429	10.133333	$metal1
Re1823	VDD#324	VDD#431	10.133333	$metal1
Re1824	VDD#344	VDD#432	10.133333	$metal1
Re1825	VSS#217	VSS#410	10.133333	$metal1
Re1826	VSS#230	VSS#411	10.133333	$metal1
Re1827	VSS#240	VSS#412	10.133333	$metal1
Re1828	VSS#250	VSS#413	10.133333	$metal1
Re1829	VSS#262	VSS#415	10.133333	$metal1
Re1830	VDD#174	VDD#433	21.714285	$metal1
Re1831	VDD#185	VDD#434	10.133333	$metal1
Re1832	VDD#194	VDD#435	10.133333	$metal1
Re1833	VDD#205	VDD#436	10.133333	$metal1
Re1834	VDD#211	VDD#438	21.714285	$metal1
Re1835	VDD#217	VDD#439	10.133333	$metal1
Re1836	VSS#4	VSS#416	21.714285	$metal1
Re1837	VSS#17	VSS#417	10.133333	$metal1
Re1838	VSS#28	VSS#418	10.133333	$metal1
Re1839	VSS#38	VSS#419	10.133333	$metal1
Re1840	VSS#84	VSS#420	21.714285	$metal1
Re1841	VSS#98	VSS#422	10.133333	$metal1
Re1842	VDD#3	VDD#440	21.714285	$metal1
Re1843	VDD#14	VDD#441	10.133333	$metal1
Re1844	VDD#26	VDD#442	10.133333	$metal1
Re1845	VDD#37	VDD#443	10.133333	$metal1
Re1846	VDD#44	VDD#445	21.714285	$metal1
Re1847	VDD#50	VDD#446	10.133333	$metal1
Re1848	VSS#112	VSS#423	10.133333	$metal1
Re1849	VSS#124	VSS#424	10.133333	$metal1
Re1850	VSS#153	VSS#425	10.133333	$metal1
Re1039	\mem_reg[7]/CKbb#27	\mem_reg[7]/CKbb#31	152.000000	$metal1
Re1851	VSS#173	VSS#426	10.133333	$metal1
Re1852	VSS#200	VSS#428	10.133333	$metal1
Re1047	\mem_reg[5]/CKbb#27	\mem_reg[5]/CKbb#30	152.000000	$metal1
Rd1	mem[4]#33	mem[4]#42	4.500000	$metal2
Rd2	mem[14]#41	mem[14]#33	5.353534	$metal2
Rd3	FE_OFN1_scan_out_0#72	FE_OFN1_scan_out_0#62	8.073573	$metal2
Rd4	mem[1]#36	mem[1]#44	7.534275	$metal2
Rd5	mem[1]#44	mem[1]#34	6.462846	$metal2
Rd6	scan_out[0]#42	scan_out[0]#59	5.141418	$metal2
Rd7	mem[6]#33	mem[6]#42	4.500000	$metal2
Rd8	mem[3]#35	mem[3]#43	5.924962	$metal2
Rd9	mem[3]#43	mem[3]#33	6.069989	$metal2
Rd10	mem[12]#36	mem[12]#44	8.743216	$metal2
Rd11	mem[12]#44	mem[12]#34	5.605703	$metal2
Rd12	scan_clk#201	scan_clk#202	1.330026	$metal2
Rd13	scan_clk#202	scan_clk#166	5.992905	$metal2
Rd14	scan_clk#166	scan_clk#167	1.948160	$metal2
Rd15	scan_clk#167	scan_clk#205	1.868216	$metal2
Rd16	scan_clk#205	scan_clk#170	9.102528	$metal2
Rd17	scan_clk#201	scan_clk#172	4.623869	$metal2
Rd18	scan_clk#202	scan_clk#164	4.810629	$metal2
Rd19	q_control#52	q_control#41	6.073573	$metal2
Rd20	mem[7]#33	mem[7]#45	4.508929	$metal2
Rd21	scan_en#323	scan_en#349	4.856981	$metal2
Rd22	scan_en#349	scan_en#319	5.827429	$metal2
Rd23	scan_en#319	scan_en#351	2.718187	$metal2
Rd24	scan_en#351	scan_en#311	5.564317	$metal2
Rd25	scan_en#311	scan_en#353	2.341576	$metal2
Rd26	scan_en#353	scan_en#354	3.610069	$metal2
Rd27	scan_en#354	scan_en#316	5.573142	$metal2
Rd28	scan_en#349	scan_en#356	2.337246	$metal2
Rd29	scan_en#356	scan_en#321	4.504464	$metal2
Rd30	scan_en#354	scan_en#358	6.237566	$metal2
Rd31	mem[5]#36	mem[5]#44	8.100359	$metal2
Rd32	mem[5]#44	mem[5]#34	5.427132	$metal2
Rd33	scan_clk#209	scan_clk#174	5.837846	$metal2
Rd34	scan_clk#209	scan_clk#176	4.924962	$metal2
Rd35	mem[14]#35	mem[14]#44	8.870676	$metal2
Rd36	VSS#429	VSS#407	0.754464	$metal2
Rd37	VSS#431	VSS#414	0.754464	$metal2
Rd38	VSS#433	VSS#421	0.754464	$metal2
Rd39	VSS#435	VSS#427	0.754464	$metal2
Rd40	mem[12]#46	mem[12]#39	7.932322	$metal2
Rd41	mem[4]#35	mem[4]#44	7.910000	$metal2
Rd42	mem[1]#46	mem[1]#39	7.950179	$metal2
Rd43	mem[14]#45	mem[14]#37	8.763534	$metal2
Rd44	q_control#45	q_control#55	10.805001	$metal2
Rd45	addr[0]#56	addr[0]#47	1.058891	$metal2
Rd46	addr[0]#47	addr[0]#58	1.424962	$metal2
Rd47	mem[7]#36	mem[7]#47	4.508929	$metal2
Rd48	scan_en#328	scan_en#360	17.417898	$metal2
Rd49	scan_en#330	scan_en#326	7.203287	$metal2
Rd50	scan_en#326	scan_en#363	3.485893	$metal2
Rd51	n_8#52	n_8#42	8.709962	$metal2
Rd52	mem[1]#42	mem[1]#49	4.669643	$metal2
Rd53	scan_in[0]#11	scan_in[0]#14	4.504464	$metal2
Rd54	mem[6]#35	mem[6]#44	1.030021	$metal2
Rd55	mem[6]#35	mem[6]#45	2.415747	$metal2
Rd56	mem[3]#40	mem[3]#37	12.849219	$metal2
Rd57	mem[3]#37	mem[3]#47	1.592522	$metal2
Rd58	mem[12]#42	mem[12]#49	7.621390	$metal2
Rd59	n_1#19	n_1#15	6.719605	$metal2
Rd60	n_4#20	n_4#15	8.786570	$metal2
Rd61	addr[0]#49	addr[0]#60	4.551339	$metal2
Rd62	mem[4]#38	mem[4]#46	4.500000	$metal2
Rd63	scan_clk#212	scan_clk#180	5.782105	$metal2
Rd64	n_2#42	n_2#43	0.907384	$metal2
Rd65	n_2#43	n_2#35	1.145690	$metal2
Rd66	n_2#35	n_2#33	6.736855	$metal2
Rd67	scan_clk#182	scan_clk#215	6.085367	$metal2
Rd68	scan_clk#215	scan_clk#216	0.765507	$metal2
Rd69	scan_clk#216	scan_clk#178	4.856981	$metal2
Rd70	scan_clk#215	scan_clk#218	0.624669	$metal2
Rd71	scan_clk#216	scan_clk#219	5.176531	$metal2
Rd72	addr[1]#76	addr[1]#91	4.669643	$metal2
Rd73	n_7#45	n_7#53	7.210676	$metal2
Rd74	scan_en#332	scan_en#365	7.914464	$metal2
Rd75	mem[9]#34	mem[9]#42	4.500000	$metal2
Rd76	mem[5]#42	mem[5]#47	6.927132	$metal2
Rd77	mem[5]#47	mem[5]#39	4.643100	$metal2
Rd78	n_8#44	n_8#55	15.896575	$metal2
Rd79	mem[11]#35	mem[11]#33	9.271757	$metal2
Rd80	mem[11]#33	mem[11]#44	0.460676	$metal2
Rd81	n_9#17	n_9#23	4.657523	$metal2
Rd82	n_3#15	n_3#21	4.526786	$metal2
Rd83	mem[4]#47	mem[4]#40	4.853534	$metal2
Rd84	VDD#447	VDD#423	0.754464	$metal2
Rd85	VDD#449	VDD#430	0.754464	$metal2
Rd86	VDD#451	VDD#437	0.754464	$metal2
Rd87	VDD#453	VDD#444	0.754464	$metal2
Rd88	VDD#415	VDD#456	0.754368	$metal2
Rd89	addr[1]#80	addr[1]#93	5.246391	$metal2
Rd90	mem[14]#40	mem[14]#48	10.879723	$metal2
Rd91	mem[7]#48	mem[7]#49	0.740741	$metal2
Rd92	mem[7]#49	mem[7]#38	8.858855	$metal2
Rd93	mem[7]#49	mem[7]#40	5.263938	$metal2
Rd94	n_0#18	n_0#15	19.953316	$metal2
Rd95	scan_clk#220	scan_clk#185	7.748561	$metal2
Rd96	addr[0]#53	addr[0]#62	8.710676	$metal2
Rd97	addr[0]#62	addr[0]#51	3.265537	$metal2
Rd98	addr[0]#51	addr[0]#64	4.319109	$metal2
Rd99	n_7#54	n_7#47	1.183324	$metal2
Rd100	n_7#47	n_7#42	12.857868	$metal2
Rd101	scan_en#334	scan_en#367	4.500000	$metal2
Rd102	mem[15]#43	mem[15]#33	5.953918	$metal2
Rd103	mem[9]#37	mem[9]#44	5.514248	$metal2
Rd104	mem[9]#44	mem[9]#33	3.612472	$metal2
Rd105	mem[9]#33	mem[9]#46	2.889054	$metal2
Rd106	n_8#46	n_8#57	8.062716	$metal2
Rd107	mem[6]#37	mem[6]#47	7.910000	$metal2
Rd108	addr[1]#86	addr[1]#95	5.678410	$metal2
Rd109	addr[1]#95	addr[1]#78	6.499838	$metal2
Rd110	addr[1]#95	addr[1]#97	3.181872	$metal2
Rd111	addr[1]#97	addr[1]#98	9.310755	$metal2
Rd112	scan_clk#186	scan_clk#223	16.428995	$metal2
Rd113	addr[2]#29	addr[2]#38	5.442819	$metal2
Rd114	addr[1]#99	addr[1]#89	5.942820	$metal2
Rd115	mem[10]#42	mem[10]#33	10.106112	$metal2
Rd116	n_2#40	n_2#47	5.398176	$metal2
Rd117	n_10#22	n_10#19	18.068981	$metal2
Rd118	n_1#18	n_1#22	4.562500	$metal2
Rd119	n_4#18	n_4#23	6.532105	$metal2
Rd120	mem[6]#39	mem[6]#49	7.635507	$metal2
Rd121	n_13#22	n_13#17	4.513393	$metal2
Rd122	addr[0]#65	addr[0]#55	7.609287	$metal2
Rd123	q_control#50	q_control#47	6.798823	$metal2
Rd124	q_control#47	q_control#58	0.655022	$metal2
Rd125	n_8#48	n_8#59	6.433891	$metal2
Rd126	addr[2]#32	addr[2]#40	4.500000	$metal2
Rd127	n_6#17	n_6#15	16.177128	$metal2
Rd128	mem[13]#38	mem[13]#44	8.096343	$metal2
Rd129	mem[13]#44	mem[13]#34	8.850048	$metal2
Rd130	mem[13]#44	mem[13]#36	6.496081	$metal2
Rd131	mem[8]#42	mem[8]#33	5.219605	$metal2
Rd132	n_5#15	n_5#20	4.669643	$metal2
Rd133	FE_OFN1_scan_out_0#66	FE_OFN1_scan_out_0#75	6.440922	$metal2
Rd134	FE_OFN1_scan_out_0#75	FE_OFN1_scan_out_0#76	5.057407	$metal2
Rd135	FE_OFN1_scan_out_0#75	FE_OFN1_scan_out_0#77	0.453317	$metal2
Rd136	mem[15]#39	mem[15]#46	6.284275	$metal2
Rd137	mem[15]#46	mem[15]#47	8.100772	$metal2
Rd138	mem[15]#47	mem[15]#35	4.638293	$metal2
Rd139	n_12#38	n_12#44	5.263938	$metal2
Rd140	n_12#44	n_12#45	1.997650	$metal2
Rd141	n_12#45	n_12#33	13.723462	$metal2
Rd142	n_12#44	n_12#47	0.946098	$metal2
Rd143	n_12#45	n_12#36	4.806872	$metal2
Rd144	clk#12	clk#11	5.317819	$metal2
Rd145	n_9#21	n_9#25	5.130319	$metal2
Rd146	n_7#48	n_7#58	4.500000	$metal2
Rd147	n_3#18	n_3#23	7.286569	$metal2
Rd148	mem[11]#41	mem[11]#46	8.495386	$metal2
Rd149	mem[11]#46	mem[11]#38	4.623869	$metal2
Rd150	mem[7]#42	mem[7]#53	4.544643	$metal2
Rd151	n_14#17	n_14#23	4.669643	$metal2
Rd152	n_11#30	n_11#32	7.321267	$metal2
Rd153	n_11#32	n_11#27	4.856981	$metal2
Rd154	n_11#32	n_11#24	11.737613	$metal2
Rd155	scan_en#343	scan_en#369	5.478534	$metal2
Rd156	scan_en#369	scan_en#345	4.924962	$metal2
Rd157	mem[2]#42	mem[2]#43	0.790228	$metal2
Rd158	mem[2]#43	mem[2]#33	6.030208	$metal2
Rd159	mem[2]#42	mem[2]#45	0.407384	$metal2
Rd160	mem[2]#43	mem[2]#35	6.860443	$metal2
Rd161	scan_en#371	scan_en#341	14.312923	$metal2
Rd162	mem[9]#40	mem[9]#48	5.514248	$metal2
Rd163	mem[13]#42	mem[13]#40	12.488214	$metal2
Rd164	mem[0]#44	mem[0]#37	0.262449	$metal2
Rd165	mem[0]#37	mem[0]#46	3.892982	$metal2
Rd166	mem[0]#46	mem[0]#33	6.499838	$metal2
Rd167	mem[0]#46	mem[0]#39	6.389520	$metal2
Rd168	mem[10]#36	mem[10]#45	7.910000	$metal2
Rd169	scan_en#347	scan_en#339	8.298952	$metal2
Rd170	scan_en#339	scan_en#375	1.544006	$metal2
Rd171	scan_en#375	scan_en#337	4.623869	$metal2
Rd172	scan_en#347	scan_en#377	2.532105	$metal2
Rd173	scan_clk#200	scan_clk#225	5.221343	$metal2
Rd174	scan_clk#225	scan_clk#226	4.919002	$metal2
Rd175	scan_clk#226	scan_clk#190	1.942398	$metal2
Rd176	scan_clk#190	scan_clk#189	6.736855	$metal2
Rd177	scan_clk#225	scan_clk#193	9.960861	$metal2
Rd178	scan_clk#193	scan_clk#230	2.210956	$metal2
Rd179	scan_clk#226	scan_clk#196	8.236556	$metal2
Rd180	scan_clk#196	scan_clk#232	0.299396	$metal2
Rd181	scan_clk#232	scan_clk#195	6.031538	$metal2
Rd182	mem[8]#44	mem[8]#36	16.022463	$metal2
Rd183	n_14#24	n_14#25	5.368629	$metal2
Rd184	q#43	q#59	11.016827	$metal2
Rd185	FE_OFN0_q#39	FE_OFN0_q#36	17.057472	$metal2
Rd186	mem[15]#41	mem[15]#50	5.596775	$metal2
Rd187	FE_OFN1_scan_out_0#71	FE_OFN1_scan_out_0#68	10.093185	$metal2
Rd188	FE_OFN1_scan_out_0#68	FE_OFN1_scan_out_0#80	1.478534	$metal2
Rd189	addr[3]#21	addr[3]#23	23.861271	$metal2
Rd190	mem[2]#40	mem[2]#48	4.500000	$metal2
Rd191	addr[2]#41	addr[2]#35	4.553571	$metal2
Rd192	mem[0]#42	mem[0]#50	4.508929	$metal2
Rd193	n_8#51	n_8#61	4.656250	$metal2
Rd194	n_13#24	n_13#21	5.587846	$metal2
Rd195	mem[8]#41	mem[8]#47	11.215012	$metal2
Rd196	mem[8]#47	mem[8]#48	1.078918	$metal2
Rd197	n_12#49	n_12#41	9.302541	$metal2
Rd198	x#22	x#24	15.476637	$metal2
Rd199	x#24	x#25	11.941336	$metal2
Rd200	x#24	x#20	4.806872	$metal2
Rd201	mem[10]#40	mem[10]#47	6.712846	$metal2
Rd202	mem[10]#47	mem[10]#38	6.462846	$metal2
Rd203	n_14#21	n_14#27	6.799962	$metal2
Rd204	n_5#17	n_5#22	6.782105	$metal2
Rd205	n_7#50	n_7#60	5.014248	$metal2
Rd206	n_15#30	n_15#27	25.127897	$metal2
Rd207	n_15#27	n_15#25	13.848531	$metal2
Rd208	mux#10	mux#15	5.141418	$metal2
Rd229	scan_en#308	scan_en#351	4.500000	$metal2
Rd231	scan_en#313	scan_en#353	4.500000	$metal2
Rd265	n_2#37	n_2#43	4.500000	$metal2
Rd306	addr[1]#83	addr[1]#97	4.500000	$metal2
Rd349	mem[2]#37	mem[2]#42	4.500000	$metal2
Rd383	mem[8]#38	mem[8]#47	4.500000	$metal2
Rc1	scan_out[0]#59	scan_out[0]#61	3.496879	$metal3
Rc2	scan_clk#209	scan_clk#235	3.452594	$metal3
Rc3	mem[3]#47	mem[3]#43	7.680100	$metal3
Rc4	VSS#437	VSS#429	0.852500	$metal3
Rc5	VSS#439	VSS#431	1.705000	$metal3
Rc6	VSS#441	VSS#433	1.705000	$metal3
Rc7	VSS#443	VSS#435	0.852500	$metal3
Rc8	x#25	x#28	4.341212	$metal3
Rc9	mem[6]#44	mem[6]#42	8.166587	$metal3
Rc10	mem[7]#47	mem[7]#45	8.112500	$metal3
Rc11	addr[0]#58	addr[0]#68	3.593912	$metal3
Rc12	mem[5]#47	mem[5]#44	8.040434	$metal3
Rc13	mem[1]#49	mem[1]#46	4.010854	$metal3
Rc14	mem[1]#46	mem[1]#44	4.558367	$metal3
Rc15	scan_in[0]#14	scan_in[0]#16	3.569629	$metal3
Rc16	mem[14]#48	mem[14]#45	3.765633	$metal3
Rc17	mem[14]#45	mem[14]#44	0.788033	$metal3
Rc18	mem[14]#44	mem[14]#41	3.909767	$metal3
Rc19	mem[12]#49	mem[12]#46	3.837700	$metal3
Rc20	mem[12]#46	mem[12]#44	4.558367	$metal3
Rc21	scan_clk#219	scan_clk#205	8.256634	$metal3
Rc22	scan_en#378	scan_en#363	0.337446	$metal3
Rc23	scan_en#363	scan_en#358	4.774567	$metal3
Rc24	addr[0]#69	addr[0]#60	3.494453	$metal3
Rc25	scan_clk#212	scan_clk#239	4.749329	$metal3
Rc26	scan_clk#239	scan_clk#201	3.647962	$metal3
Rc27	mem[9]#42	mem[9]#46	6.967267	$metal3
Rc28	addr[0]#64	addr[0]#72	3.568846	$metal3
Rc29	addr[0]#72	addr[0]#56	3.603312	$metal3
Rc30	mem[4]#47	mem[4]#50	3.892338	$metal3
Rc31	mem[4]#50	mem[4]#44	0.356184	$metal3
Rc32	mem[4]#44	mem[4]#42	4.702500	$metal3
Rc33	mem[4]#50	mem[4]#46	3.870583	$metal3
Rc34	VDD#457	VDD#447	0.852500	$metal3
Rc35	VDD#459	VDD#449	1.705000	$metal3
Rc36	VDD#461	VDD#451	1.705000	$metal3
Rc37	VDD#463	VDD#453	1.705000	$metal3
Rc38	VDD#465	VDD#456	0.852500	$metal3
Rc39	n_7#54	n_7#62	3.448188	$metal3
Rc40	n_9#26	n_9#23	3.803099	$metal3
Rc41	addr[1]#101	addr[1]#91	7.303145	$metal3
Rc42	addr[1]#103	addr[1]#93	7.014879	$metal3
Rc43	mem[15]#51	mem[15]#43	3.481924	$metal3
Rc44	addr[1]#99	addr[1]#98	3.694107	$metal3
Rc45	addr[1]#98	addr[1]#107	0.015908	$metal3
Rc46	q_control#58	q_control#55	4.852154	$metal3
Rc47	q_control#55	q_control#52	4.486300	$metal3
Rc48	n_2#47	n_2#42	7.680100	$metal3
Rc49	mem[6]#52	mem[6]#47	0.219946	$metal3
Rc50	mem[6]#47	mem[6]#45	4.324187	$metal3
Rc51	mem[15]#47	mem[15]#54	3.759196	$metal3
Rc52	n_1#22	n_1#19	7.896300	$metal3
Rc53	n_4#23	n_4#20	7.896300	$metal3
Rc54	mem[6]#49	mem[6]#56	3.447453	$metal3
Rc55	addr[0]#65	addr[0]#62	7.517987	$metal3
Rc56	mem[11]#46	mem[11]#44	8.112500	$metal3
Rc57	addr[2]#43	addr[2]#38	3.846929	$metal3
Rc58	FE_OFN1_scan_out_0#77	FE_OFN1_scan_out_0#72	9.950274	$metal3
Rc59	addr[2]#40	addr[2]#46	6.951429	$metal3
Rc60	scan_en#381	scan_en#375	3.484061	$metal3
Rc61	n_7#58	n_7#64	3.902956	$metal3
Rc62	n_9#25	n_9#29	4.251204	$metal3
Rc63	mem[7]#56	mem[7]#48	4.592662	$metal3
Rc64	n_3#23	n_3#21	8.292741	$metal3
Rc65	mem[7]#53	mem[7]#59	3.516362	$metal3
Rc66	scan_clk#230	scan_clk#242	3.998112	$metal3
Rc67	scan_clk#242	scan_clk#223	0.412646	$metal3
Rc68	scan_clk#223	scan_clk#218	4.053900	$metal3
Rc69	scan_en#369	scan_en#384	3.684779	$metal3
Rc70	scan_en#384	scan_en#365	1.524233	$metal3
Rc71	scan_en#365	scan_en#360	0.427700	$metal3
Rc72	scan_en#360	scan_en#356	4.630434	$metal3
Rc73	mem[9]#48	mem[9]#44	8.112500	$metal3
Rc74	scan_clk#232	scan_clk#220	8.490962	$metal3
Rc75	scan_en#377	scan_en#371	3.549433	$metal3
Rc76	scan_en#371	scan_en#367	4.810675	$metal3
Rc77	n_14#25	n_14#23	7.463901	$metal3
Rc78	q#59	q#61	3.755279	$metal3
Rc79	mem[8]#48	mem[8]#44	3.837700	$metal3
Rc80	mem[8]#44	mem[8]#42	4.198033	$metal3
Rc81	mem[15]#50	mem[15]#46	8.166587	$metal3
Rc82	n_13#26	n_13#22	5.028195	$metal3
Rc83	FE_OFN1_scan_out_0#80	FE_OFN1_scan_out_0#76	8.544900	$metal3
Rc84	addr[3]#23	addr[3]#25	3.946412	$metal3
Rc85	mem[0]#50	mem[0]#44	8.112500	$metal3
Rc86	mem[2]#48	mem[2]#45	8.262154	$metal3
Rc87	n_8#61	n_8#63	5.729924	$metal3
Rc88	n_8#63	n_8#57	0.640587	$metal3
Rc89	n_8#57	n_8#52	0.772226	$metal3
Rc90	n_8#52	n_8#55	3.621741	$metal3
Rc91	n_8#63	n_8#59	3.561210	$metal3
Rc92	addr[2]#41	addr[2]#48	8.295629	$metal3
Rc93	n_13#24	n_13#29	3.512446	$metal3
Rc94	mem[10]#47	mem[10]#45	4.612454	$metal3
Rc95	mem[10]#45	mem[10]#42	4.486300	$metal3
Rc96	n_12#49	n_12#47	8.472834	$metal3
Rc97	n_14#27	n_14#24	7.752167	$metal3
Rc98	n_5#22	n_5#20	8.743120	$metal3
Rc99	clk#14	clk#12	5.354136	$metal3
Rc100	n_7#60	n_7#66	5.954953	$metal3
Rc101	n_7#66	n_7#67	0.670974	$metal3
Rc102	n_7#67	n_7#53	3.724729	$metal3
Rc103	mux#15	mux#17	3.728646	$metal3
Rb1	scan_out[0]	scan_out[0]#61	3.686687	$metal4
Rb2	x	x#28	3.536117	$metal4
Rb3	scan_clk#235	scan_clk#239	7.962100	$metal4
Rb4	VSS#445	VSS#443	0.910618	$metal4
Rb5	VSS#445	VSS#441	0.151108	$metal4
Rb6	VSS#441	VSS#439	0.209226	$metal4
Rb7	VSS#439	VSS#437	1.061726	$metal4
Rb8	addr[0]	addr[0]#68	4.587350	$metal4
Rb9	scan_in[0]	scan_in[0]#16	3.631683	$metal4
Rb10	addr[0]#69	addr[0]#72	9.029074	$metal4
Rb11	scan_en	scan_en#378	3.822817	$metal4
Rb12	VDD#467	VDD#459	0.023247	$metal4
Rb13	VDD#459	VDD#461	0.209226	$metal4
Rb14	VDD#461	VDD#463	0.209226	$metal4
Rb15	VDD#463	VDD#465	1.060841	$metal4
Rb16	VDD#467	VDD#457	1.038478	$metal4
Rb17	n_7#67	n_7#62	7.388700	$metal4
Rb18	addr[1]	addr[1]#103	1.478224	$metal4
Rb19	addr[1]#103	addr[1]#101	0.664267	$metal4
Rb20	addr[1]#101	addr[1]#107	3.883133	$metal4
Rb21	mem[15]#51	mem[15]#54	7.579833	$metal4
Rb22	n_9#29	n_9#26	8.517111	$metal4
Rb23	mem[6]#56	mem[6]#52	7.348004	$metal4
Rb24	scan_clk	scan_clk#242	3.727250	$metal4
Rb25	n_7#64	n_7#66	7.921404	$metal4
Rb26	scan_en#384	scan_en#381	7.962100	$metal4
Rb27	addr[2]	addr[2]#48	0.945521	$metal4
Rb28	addr[2]#48	addr[2]#46	1.603446	$metal4
Rb29	addr[2]#46	addr[2]#43	4.552111	$metal4
Rb30	mem[7]#56	mem[7]#59	8.933508	$metal4
Rb31	q	q#61	3.727250	$metal4
Rb32	addr[3]	addr[3]#25	3.918383	$metal4
Rb33	mux	mux#17	3.631683	$metal4
Rb34	n_13#29	n_13#26	7.104350	$metal4
Rb35	clk	clk#14	4.089561	$metal4
Ra1	VDD	VDD#473	0.360378	$metal5
Ra2	VDD#473	VDD	0.229524	$metal5
Ra3	VSS	VSS#450	0.457988	$metal5
Ra4	VSS#450	VSS	0.131915	$metal5
Ra5	VDD#467	VDD#473	0.380000	$metal5
Ra6	VSS#445	VSS#450	0.380000	$metal5
*
*       CAPACITOR CARDS
*
*
C1	scan_in[0]	VSS	2.29745e-16
C2	addr[0]	VSS	1.00492e-15
C3	scan_en	VSS	3.47682e-16
C4	addr[3]	VSS	1.96581e-16
C5	VDD	VSS	1.7799e-14
C6	addr[2]	VSS	4.5008e-16
C7	q	VSS	1.47333e-16
C8	mux	VSS	1.19663e-16
C9	x	VSS	9.77303e-17
C10	scan_out[0]	VSS	1.94674e-16
C11	scan_clk	VSS	1.45916e-16
C12	clk	VSS	2.73481e-16
C13	addr[1]	VSS	7.15407e-16
C14	FE_OFN1_scan_out_0	VSS	8.55471e-17
C15	FE_OFN0_q	VSS	7.72526e-17
C16	mem[3]	VSS	4.59218e-17
C17	q_control	VSS	3.79704e-17
C18	UNCONNECTED8	VSS	2.5225e-20
C19	mem[1]	VSS	4.59804e-17
C20	mem[4]	VSS	4.73628e-17
C21	mem[12]	VSS	4.57876e-17
C22	mem[6]	VSS	3.81285e-17
C23	mem[14]	VSS	4.61779e-17
C24	mem[7]	VSS	4.72897e-17
C25	mem[5]	VSS	4.72811e-17
C26	mem[11]	VSS	4.70101e-17
C27	mem[9]	VSS	3.76655e-17
C28	mem[13]	VSS	4.63315e-17
C29	mem[15]	VSS	3.43264e-17
C30	mem[10]	VSS	3.52125e-17
C31	n_15	VSS	4.82567e-17
C32	UNCONNECTED	VSS	9.18043e-19
C33	mem[8]	VSS	4.74697e-17
C34	mem[2]	VSS	6.68057e-17
C35	UNCONNECTED14	VSS	1.32498e-21
C36	mem[0]	VSS	6.03627e-17
C37	UNCONNECTED15	VSS	2.48244e-18
C38	n_0	VSS	6.35282e-17
C39	n_1	VSS	6.58303e-17
C40	n_13	VSS	4.63739e-17
C41	n_14	VSS	4.82639e-17
C42	n_6	VSS	6.9844e-17
C43	n_12	VSS	7.0227e-17
C44	n_10	VSS	6.65023e-17
C45	n_4	VSS	6.42852e-17
C46	n_11	VSS	6.52118e-17
C47	n_5	VSS	6.87111e-17
C48	n_9	VSS	6.74577e-17
C49	n_3	VSS	6.11673e-17
C50	n_8	VSS	7.10421e-17
C51	n_7	VSS	7.05253e-17
C52	n_2	VSS	6.62116e-17
C53	712	VSS	4.17272e-16
C54	g525__6131/net41	VSS	2.15777e-16
C55	g524__7098/net41	VSS	2.1352e-16
C56	g523__8246/n0	VSS	1.62936e-16
C57	g510__3680/net102	VSS	1.61955e-16
C58	g510__3680/net98	VSS	1.66144e-16
C59	g509__6783/net102	VSS	1.5586e-16
C60	g509__6783/net98	VSS	1.717e-16
C61	g522__2802/net89	VSS	5.61204e-19
C62	g522__2802/net102	VSS	1.5147e-16
C63	g522__2802/net98	VSS	1.68354e-16
C64	g521__1617/net89	VSS	1.62873e-16
C65	g521__1617/net102	VSS	1.60539e-16
C66	g521__1617/net98	VSS	1.6826e-16
C67	g497__8428/net122	VSS	9.42829e-17
C68	g497__8428/net118	VSS	1.84535e-16
C69	g497__8428/net139	VSS	1.53188e-16
C70	g497__8428/net147	VSS	1.71701e-16
C71	g497__8428/net155	VSS	1.3948e-16
C72	g498__5526/net122	VSS	1.46436e-16
C73	g498__5526/net118	VSS	1.88249e-16
C74	g498__5526/net139	VSS	1.52702e-16
C75	g498__5526/net147	VSS	1.60669e-16
C76	g498__5526/net155	VSS	1.38139e-16
C77	g495__6260/n1	VSS	2.09874e-16
C78	g495__6260/n2	VSS	2.16742e-16
C79	g495__6260/n3	VSS	2.3242e-16
C80	g495__6260/n4	VSS	2.17251e-16
C81	g496__4319/n1	VSS	2.11824e-16
C82	g496__4319/n2	VSS	2.12346e-16
C83	g496__4319/n3	VSS	2.36467e-16
C84	g496__4319/n4	VSS	2.10309e-16
C85	g494__5107/S0b	VSS	5.00857e-17
C86	g494__5107/n0	VSS	7.62792e-17
C87	g494__5107/n3	VSS	1.00324e-16
C88	g494__5107/n4	VSS	1.47248e-16
C89	g494__5107/n5	VSS	1.05535e-16
C90	g494__5107/n6	VSS	1.24159e-16
C91	g519__1705/S0b	VSS	5.4712e-17
C92	g519__1705/n0	VSS	7.03943e-17
C93	g519__1705/n3	VSS	9.49234e-17
C94	g519__1705/n4	VSS	1.43147e-16
C95	g519__1705/n5	VSS	1.0184e-16
C96	g519__1705/n6	VSS	1.27244e-16
C97	g520__5122/S0b	VSS	5.18541e-17
C98	g520__5122/n0	VSS	7.78949e-17
C99	g520__5122/n3	VSS	9.356e-17
C100	g520__5122/n4	VSS	1.39593e-16
C101	g520__5122/n5	VSS	9.92716e-17
C102	g520__5122/n6	VSS	1.28831e-16
C103	g493__2398/S0b	VSS	5.47855e-17
C104	g493__2398/n0	VSS	7.5917e-17
C105	g493__2398/n3	VSS	1.38772e-16
C106	g493__2398/n4	VSS	1.96035e-16
C107	g493__2398/n5	VSS	1.1839e-16
C108	g493__2398/n6	VSS	1.73782e-16
C109	\mem_reg[10]/SEb	VSS	4.14265e-17
C110	\mem_reg[10]/CKbb	VSS	4.99096e-17
C111	\mem_reg[10]/mout	VSS	4.4121e-17
C112	\mem_reg[10]/CKb	VSS	5.1902e-17
C113	\mem_reg[10]/n20	VSS	5.32247e-17
C114	\mem_reg[10]/qbint	VSS	5.30848e-17
C115	\mem_reg[10]/n30	VSS	5.03019e-17
C116	\mem_reg[10]/net367	VSS	6.92287e-17
C117	\mem_reg[10]/Db	VSS	2.23182e-19
C118	\mem_reg[10]/net449	VSS	1.10686e-16
C119	\mem_reg[10]/net384	VSS	1.46197e-16
C120	\mem_reg[10]/net461	VSS	8.95431e-17
C121	\mem_reg[10]/net392	VSS	1.3426e-16
C122	\mem_reg[10]/net429	VSS	1.04532e-16
C123	\mem_reg[10]/net356	VSS	1.39483e-16
C124	\mem_reg[10]/net436	VSS	1.09783e-16
C125	\mem_reg[10]/net376	VSS	1.43905e-16
C126	\mem_reg[0]/SEb	VSS	3.9175e-17
C127	\mem_reg[0]/CKbb	VSS	5.66679e-17
C128	\mem_reg[0]/mout	VSS	4.29243e-17
C129	\mem_reg[0]/CKb	VSS	4.90498e-17
C130	\mem_reg[0]/n20	VSS	5.41112e-17
C131	\mem_reg[0]/qbint	VSS	5.31393e-17
C132	\mem_reg[0]/n30	VSS	5.16196e-17
C133	\mem_reg[0]/net367	VSS	5.94569e-17
C134	\mem_reg[0]/Db	VSS	2.73354e-19
C135	\mem_reg[0]/net449	VSS	1.11636e-16
C136	\mem_reg[0]/net384	VSS	1.50297e-16
C137	\mem_reg[0]/net461	VSS	8.92081e-17
C138	\mem_reg[0]/net392	VSS	1.34797e-16
C139	\mem_reg[0]/net429	VSS	1.03698e-16
C140	\mem_reg[0]/net356	VSS	1.43472e-16
C141	\mem_reg[0]/net436	VSS	1.0662e-16
C142	\mem_reg[0]/net376	VSS	1.36706e-16
C143	\mem_reg[2]/SEb	VSS	4.25995e-17
C144	\mem_reg[2]/CKbb	VSS	5.69347e-17
C145	\mem_reg[2]/mout	VSS	4.78865e-17
C146	\mem_reg[2]/CKb	VSS	5.32373e-17
C147	\mem_reg[2]/n20	VSS	5.16763e-17
C148	\mem_reg[2]/qbint	VSS	5.50978e-17
C149	\mem_reg[2]/n30	VSS	5.14871e-17
C150	\mem_reg[2]/net367	VSS	6.19421e-17
C151	\mem_reg[2]/Db	VSS	7.58306e-19
C152	\mem_reg[2]/net449	VSS	1.11361e-16
C153	\mem_reg[2]/net384	VSS	1.45625e-16
C154	\mem_reg[2]/net461	VSS	9.48162e-17
C155	\mem_reg[2]/net392	VSS	1.40703e-16
C156	\mem_reg[2]/net429	VSS	1.03856e-16
C157	\mem_reg[2]/net356	VSS	1.47229e-16
C158	\mem_reg[2]/net436	VSS	1.06498e-16
C159	\mem_reg[2]/net376	VSS	1.37502e-16
C160	\mem_reg[15]/SEb	VSS	4.21747e-17
C161	\mem_reg[15]/CKbb	VSS	5.60664e-17
C162	\mem_reg[15]/mout	VSS	4.57733e-17
C163	\mem_reg[15]/CKb	VSS	5.24374e-17
C164	\mem_reg[15]/n20	VSS	5.16502e-17
C165	\mem_reg[15]/qbint	VSS	5.73953e-17
C166	\mem_reg[15]/n30	VSS	5.21532e-17
C167	\mem_reg[15]/net367	VSS	6.16699e-17
C168	\mem_reg[15]/Db	VSS	3.72018e-20
C169	\mem_reg[15]/net449	VSS	1.14237e-16
C170	\mem_reg[15]/net384	VSS	1.49521e-16
C171	\mem_reg[15]/net461	VSS	8.88548e-17
C172	\mem_reg[15]/net392	VSS	1.35218e-16
C173	\mem_reg[15]/net429	VSS	1.10117e-16
C174	\mem_reg[15]/net356	VSS	1.45684e-16
C175	\mem_reg[15]/net436	VSS	1.07747e-16
C176	\mem_reg[15]/net376	VSS	1.45767e-16
C177	mux_control_reg/SEb	VSS	3.78383e-17
C178	mux_control_reg/CKbb	VSS	5.58997e-17
C179	mux_control_reg/mout	VSS	4.62419e-17
C180	mux_control_reg/CKb	VSS	5.25434e-17
C181	mux_control_reg/n20	VSS	5.36268e-17
C182	mux_control_reg/qbint	VSS	5.54578e-17
C183	mux_control_reg/n30	VSS	5.06592e-17
C184	mux_control_reg/net367	VSS	6.32872e-17
C185	mux_control_reg/Db	VSS	2.89695e-19
C186	mux_control_reg/net449	VSS	1.10721e-16
C187	mux_control_reg/net384	VSS	1.46884e-16
C188	mux_control_reg/net461	VSS	8.74101e-17
C189	mux_control_reg/net392	VSS	1.33527e-16
C190	mux_control_reg/net429	VSS	1.12629e-16
C191	mux_control_reg/net356	VSS	1.40349e-16
C192	mux_control_reg/net436	VSS	1.09727e-16
C193	mux_control_reg/net376	VSS	1.37586e-16
C194	\mem_reg[8]/SEb	VSS	4.57183e-17
C195	\mem_reg[8]/CKbb	VSS	5.45134e-17
C196	\mem_reg[8]/mout	VSS	4.7662e-17
C197	\mem_reg[8]/CKb	VSS	5.16418e-17
C198	\mem_reg[8]/n20	VSS	5.1724e-17
C199	\mem_reg[8]/qbint	VSS	5.37716e-17
C200	\mem_reg[8]/n30	VSS	4.87222e-17
C201	\mem_reg[8]/net367	VSS	7.09254e-17
C202	\mem_reg[8]/Db	VSS	8.16029e-20
C203	\mem_reg[8]/net449	VSS	1.10051e-16
C204	\mem_reg[8]/net384	VSS	1.39757e-16
C205	\mem_reg[8]/net461	VSS	8.75765e-17
C206	\mem_reg[8]/net392	VSS	1.32416e-16
C207	\mem_reg[8]/net429	VSS	1.06263e-16
C208	\mem_reg[8]/net356	VSS	1.32661e-16
C209	\mem_reg[8]/net436	VSS	1.08854e-16
C210	\mem_reg[8]/net376	VSS	1.36101e-16
C211	q_reg_reg/SEb	VSS	4.29014e-17
C212	q_reg_reg/CKbb	VSS	5.76173e-17
C213	q_reg_reg/mout	VSS	4.80003e-17
C214	q_reg_reg/CKb	VSS	5.35841e-17
C215	q_reg_reg/n20	VSS	5.79231e-17
C216	q_reg_reg/qbint	VSS	5.84524e-17
C217	q_reg_reg/n30	VSS	5.218e-17
C218	q_reg_reg/net367	VSS	6.87829e-17
C219	q_reg_reg/net449	VSS	1.11689e-16
C220	q_reg_reg/net384	VSS	1.48288e-16
C221	q_reg_reg/net461	VSS	8.9342e-17
C222	q_reg_reg/net392	VSS	1.47012e-16
C223	q_reg_reg/net429	VSS	1.04311e-16
C224	q_reg_reg/net356	VSS	1.46055e-16
C225	q_reg_reg/net436	VSS	1.07477e-16
C226	q_reg_reg/net376	VSS	1.43776e-16
C227	\mem_reg[9]/SEb	VSS	4.32276e-17
C228	\mem_reg[9]/CKbb	VSS	5.46988e-17
C229	\mem_reg[9]/mout	VSS	4.60765e-17
C230	\mem_reg[9]/CKb	VSS	5.26265e-17
C231	\mem_reg[9]/n20	VSS	5.59823e-17
C232	\mem_reg[9]/qbint	VSS	6.06359e-17
C233	\mem_reg[9]/n30	VSS	5.18987e-17
C234	\mem_reg[9]/net367	VSS	6.55324e-17
C235	\mem_reg[9]/Db	VSS	8.85712e-19
C236	\mem_reg[9]/net449	VSS	1.13555e-16
C237	\mem_reg[9]/net384	VSS	1.46253e-16
C238	\mem_reg[9]/net461	VSS	9.53115e-17
C239	\mem_reg[9]/net392	VSS	1.52438e-16
C240	\mem_reg[9]/net429	VSS	1.04946e-16
C241	\mem_reg[9]/net356	VSS	1.44529e-16
C242	\mem_reg[9]/net436	VSS	1.06814e-16
C243	\mem_reg[9]/net376	VSS	1.42768e-16
C244	\mem_reg[13]/SEb	VSS	4.13833e-17
C245	\mem_reg[13]/CKbb	VSS	5.4008e-17
C246	\mem_reg[13]/mout	VSS	4.35419e-17
C247	\mem_reg[13]/CKb	VSS	4.74334e-17
C248	\mem_reg[13]/n20	VSS	5.44656e-17
C249	\mem_reg[13]/qbint	VSS	5.36083e-17
C250	\mem_reg[13]/n30	VSS	5.16554e-17
C251	\mem_reg[13]/net367	VSS	6.60876e-17
C252	\mem_reg[13]/Db	VSS	1.42647e-19
C253	\mem_reg[13]/net449	VSS	1.10347e-16
C254	\mem_reg[13]/net384	VSS	1.43161e-16
C255	\mem_reg[13]/net461	VSS	8.86139e-17
C256	\mem_reg[13]/net392	VSS	1.37512e-16
C257	\mem_reg[13]/net429	VSS	1.08296e-16
C258	\mem_reg[13]/net356	VSS	1.43594e-16
C259	\mem_reg[13]/net436	VSS	1.09342e-16
C260	\mem_reg[13]/net376	VSS	1.37575e-16
C261	\mem_reg[6]/SEb	VSS	4.02103e-17
C262	\mem_reg[6]/CKbb	VSS	5.6211e-17
C263	\mem_reg[6]/mout	VSS	4.81209e-17
C264	\mem_reg[6]/CKb	VSS	5.12251e-17
C265	\mem_reg[6]/n20	VSS	5.37892e-17
C266	\mem_reg[6]/qbint	VSS	5.56755e-17
C267	\mem_reg[6]/n30	VSS	5.55931e-17
C268	\mem_reg[6]/net367	VSS	6.32004e-17
C269	\mem_reg[6]/Db	VSS	2.80101e-19
C270	\mem_reg[6]/net449	VSS	1.09997e-16
C271	\mem_reg[6]/net384	VSS	1.44633e-16
C272	\mem_reg[6]/net461	VSS	9.68827e-17
C273	\mem_reg[6]/net392	VSS	1.42453e-16
C274	\mem_reg[6]/net429	VSS	1.14167e-16
C275	\mem_reg[6]/net356	VSS	1.47162e-16
C276	\mem_reg[6]/net436	VSS	1.07305e-16
C277	\mem_reg[6]/net376	VSS	1.37616e-16
C278	\mem_reg[11]/SEb	VSS	3.99822e-17
C279	\mem_reg[11]/CKbb	VSS	5.35244e-17
C280	\mem_reg[11]/mout	VSS	4.1087e-17
C281	\mem_reg[11]/CKb	VSS	5.04433e-17
C282	\mem_reg[11]/n20	VSS	5.39808e-17
C283	\mem_reg[11]/qbint	VSS	5.41321e-17
C284	\mem_reg[11]/n30	VSS	4.83114e-17
C285	\mem_reg[11]/net367	VSS	6.28973e-17
C286	\mem_reg[11]/Db	VSS	2.31969e-19
C287	\mem_reg[11]/net449	VSS	1.13838e-16
C288	\mem_reg[11]/net384	VSS	1.43038e-16
C289	\mem_reg[11]/net461	VSS	9.33453e-17
C290	\mem_reg[11]/net392	VSS	1.30674e-16
C291	\mem_reg[11]/net429	VSS	1.09617e-16
C292	\mem_reg[11]/net356	VSS	1.46143e-16
C293	\mem_reg[11]/net436	VSS	1.10698e-16
C294	\mem_reg[11]/net376	VSS	1.40935e-16
C295	q_control_reg/SEb	VSS	4.12736e-17
C296	q_control_reg/CKbb	VSS	5.54408e-17
C297	q_control_reg/mout	VSS	4.74921e-17
C298	q_control_reg/CKb	VSS	5.37518e-17
C299	q_control_reg/n20	VSS	5.85921e-17
C300	q_control_reg/qbint	VSS	6.05343e-17
C301	q_control_reg/n30	VSS	5.52033e-17
C302	q_control_reg/net367	VSS	6.64054e-17
C303	q_control_reg/Db	VSS	1.40825e-19
C304	q_control_reg/net449	VSS	1.13018e-16
C305	q_control_reg/net384	VSS	1.44938e-16
C306	q_control_reg/net461	VSS	9.11687e-17
C307	q_control_reg/net392	VSS	1.35404e-16
C308	q_control_reg/net429	VSS	1.02487e-16
C309	q_control_reg/net356	VSS	1.49237e-16
C310	q_control_reg/net436	VSS	1.05446e-16
C311	q_control_reg/net376	VSS	1.42554e-16
C312	\mem_reg[5]/SEb	VSS	4.37261e-17
C313	\mem_reg[5]/CKbb	VSS	5.8302e-17
C314	\mem_reg[5]/mout	VSS	4.5975e-17
C315	\mem_reg[5]/CKb	VSS	5.22493e-17
C316	\mem_reg[5]/n20	VSS	5.47996e-17
C317	\mem_reg[5]/qbint	VSS	5.62308e-17
C318	\mem_reg[5]/n30	VSS	4.91212e-17
C319	\mem_reg[5]/net367	VSS	6.57027e-17
C320	\mem_reg[5]/Db	VSS	6.04603e-20
C321	\mem_reg[5]/net449	VSS	1.12488e-16
C322	\mem_reg[5]/net384	VSS	1.46027e-16
C323	\mem_reg[5]/net461	VSS	9.39194e-17
C324	\mem_reg[5]/net392	VSS	1.35125e-16
C325	\mem_reg[5]/net429	VSS	1.03952e-16
C326	\mem_reg[5]/net356	VSS	1.43098e-16
C327	\mem_reg[5]/net436	VSS	1.08283e-16
C328	\mem_reg[5]/net376	VSS	1.37324e-16
C329	\mem_reg[7]/SEb	VSS	4.04888e-17
C330	\mem_reg[7]/CKbb	VSS	5.63742e-17
C331	\mem_reg[7]/mout	VSS	4.29243e-17
C332	\mem_reg[7]/CKb	VSS	4.90498e-17
C333	\mem_reg[7]/n20	VSS	5.17952e-17
C334	\mem_reg[7]/qbint	VSS	5.19856e-17
C335	\mem_reg[7]/n30	VSS	5.16196e-17
C336	\mem_reg[7]/net367	VSS	6.56154e-17
C337	\mem_reg[7]/net449	VSS	1.12769e-16
C338	\mem_reg[7]/net384	VSS	1.42144e-16
C339	\mem_reg[7]/net461	VSS	9.56921e-17
C340	\mem_reg[7]/net392	VSS	1.41036e-16
C341	\mem_reg[7]/net429	VSS	1.09637e-16
C342	\mem_reg[7]/net356	VSS	1.43471e-16
C343	\mem_reg[7]/net436	VSS	1.0755e-16
C344	\mem_reg[7]/net376	VSS	1.35487e-16
C345	\mem_reg[14]/SEb	VSS	4.13767e-17
C346	\mem_reg[14]/CKbb	VSS	5.57642e-17
C347	\mem_reg[14]/mout	VSS	4.64848e-17
C348	\mem_reg[14]/CKb	VSS	5.24151e-17
C349	\mem_reg[14]/n20	VSS	5.45627e-17
C350	\mem_reg[14]/qbint	VSS	5.46421e-17
C351	\mem_reg[14]/n30	VSS	4.9291e-17
C352	\mem_reg[14]/net367	VSS	6.30005e-17
C353	\mem_reg[14]/Db	VSS	4.12328e-19
C354	\mem_reg[14]/net449	VSS	1.09963e-16
C355	\mem_reg[14]/net384	VSS	1.4633e-16
C356	\mem_reg[14]/net461	VSS	8.9022e-17
C357	\mem_reg[14]/net392	VSS	1.40382e-16
C358	\mem_reg[14]/net429	VSS	1.06087e-16
C359	\mem_reg[14]/net356	VSS	1.35148e-16
C360	\mem_reg[14]/net436	VSS	1.09397e-16
C361	\mem_reg[14]/net376	VSS	1.34133e-16
C362	\mem_reg[12]/SEb	VSS	4.20303e-17
C363	\mem_reg[12]/CKbb	VSS	5.32808e-17
C364	\mem_reg[12]/mout	VSS	4.69162e-17
C365	\mem_reg[12]/CKb	VSS	5.15205e-17
C366	\mem_reg[12]/n20	VSS	5.04682e-17
C367	\mem_reg[12]/qbint	VSS	5.34367e-17
C368	\mem_reg[12]/n30	VSS	5.08292e-17
C369	\mem_reg[12]/net367	VSS	6.84795e-17
C370	\mem_reg[12]/Db	VSS	5.91014e-20
C371	\mem_reg[12]/net449	VSS	1.13513e-16
C372	\mem_reg[12]/net384	VSS	1.47649e-16
C373	\mem_reg[12]/net461	VSS	9.40153e-17
C374	\mem_reg[12]/net392	VSS	1.37133e-16
C375	\mem_reg[12]/net429	VSS	1.03918e-16
C376	\mem_reg[12]/net356	VSS	1.47772e-16
C377	\mem_reg[12]/net436	VSS	1.05245e-16
C378	\mem_reg[12]/net376	VSS	1.39433e-16
C379	\mem_reg[4]/SEb	VSS	4.20236e-17
C380	\mem_reg[4]/CKbb	VSS	5.52038e-17
C381	\mem_reg[4]/mout	VSS	4.68181e-17
C382	\mem_reg[4]/CKb	VSS	5.19228e-17
C383	\mem_reg[4]/n20	VSS	5.3106e-17
C384	\mem_reg[4]/qbint	VSS	5.51728e-17
C385	\mem_reg[4]/n30	VSS	5.22635e-17
C386	\mem_reg[4]/net367	VSS	6.16027e-17
C387	\mem_reg[4]/Db	VSS	1.15399e-18
C388	\mem_reg[4]/net449	VSS	1.1419e-16
C389	\mem_reg[4]/net384	VSS	1.5201e-16
C390	\mem_reg[4]/net461	VSS	9.48185e-17
C391	\mem_reg[4]/net392	VSS	1.38874e-16
C392	\mem_reg[4]/net429	VSS	1.03941e-16
C393	\mem_reg[4]/net356	VSS	1.41019e-16
C394	\mem_reg[4]/net436	VSS	1.08205e-16
C395	\mem_reg[4]/net376	VSS	1.40071e-16
C396	\mem_reg[1]/SEb	VSS	4.18997e-17
C397	\mem_reg[1]/CKbb	VSS	5.5714e-17
C398	\mem_reg[1]/mout	VSS	4.7155e-17
C399	\mem_reg[1]/CKb	VSS	5.23412e-17
C400	\mem_reg[1]/n20	VSS	5.37263e-17
C401	\mem_reg[1]/qbint	VSS	5.59106e-17
C402	\mem_reg[1]/n30	VSS	5.24652e-17
C403	\mem_reg[1]/net367	VSS	5.89221e-17
C404	\mem_reg[1]/Db	VSS	9.2758e-18
C405	\mem_reg[1]/net449	VSS	1.14428e-16
C406	\mem_reg[1]/net384	VSS	1.46648e-16
C407	\mem_reg[1]/net461	VSS	9.87994e-17
C408	\mem_reg[1]/net392	VSS	1.36989e-16
C409	\mem_reg[1]/net429	VSS	1.11878e-16
C410	\mem_reg[1]/net356	VSS	1.39779e-16
C411	\mem_reg[1]/net436	VSS	1.14986e-16
C412	\mem_reg[1]/net376	VSS	1.43603e-16
C413	\mem_reg[3]/SEb	VSS	4.30936e-17
C414	\mem_reg[3]/CKbb	VSS	5.66699e-17
C415	\mem_reg[3]/mout	VSS	4.54024e-17
C416	\mem_reg[3]/CKb	VSS	5.18024e-17
C417	\mem_reg[3]/n20	VSS	5.44589e-17
C418	\mem_reg[3]/qbint	VSS	5.4581e-17
C419	\mem_reg[3]/n30	VSS	5.10658e-17
C420	\mem_reg[3]/net367	VSS	6.42453e-17
C421	\mem_reg[3]/Db	VSS	4.02291e-19
C422	\mem_reg[3]/net449	VSS	1.14966e-16
C423	\mem_reg[3]/net384	VSS	1.4869e-16
C424	\mem_reg[3]/net461	VSS	9.84292e-17
C425	\mem_reg[3]/net392	VSS	1.44697e-16
C426	\mem_reg[3]/net429	VSS	1.11397e-16
C427	\mem_reg[3]/net356	VSS	1.4318e-16
C428	\mem_reg[3]/net436	VSS	1.11599e-16
C429	\mem_reg[3]/net376	VSS	1.40506e-16
C430	FE_OFC0_q/n0	VSS	6.07459e-17
C431	FE_OFC1_scan_out_0/n0	VSS	5.97014e-17
C432	FE_OFC0_q/n0#31	VSS	7.88676e-17
C433	g493__2398/n0#7	VSS	6.17562e-17
C434	FE_OFC0_q/n0#29	VSS	5.87739e-17
C435	g493__2398/n0#5	VSS	6.38247e-17
C436	\mem_reg[10]/qbint#5	VSS	6.90813e-17
C437	FE_OFC0_q/n0#27	VSS	5.92604e-17
C438	g493__2398/n0#3	VSS	6.65306e-17
C439	n_7#7	VSS	6.86134e-17
C440	FE_OFC0_q/n0#25	VSS	6.1767e-17
C441	mem[10]#5	VSS	6.92767e-17
C442	FE_OFC0_q/n0#23	VSS	6.05426e-17
C443	mem[8]#5	VSS	6.68921e-17
C444	addr[3]#3	VSS	4.93462e-17
C445	x#3	VSS	6.30822e-17
C446	\mem_reg[0]/qbint#5	VSS	6.3314e-17
C447	\mem_reg[2]/qbint#5	VSS	6.59703e-17
C448	\mem_reg[10]/qbint#3	VSS	5.66027e-17
C449	FE_OFC0_q/n0#21	VSS	5.81799e-17
C450	n_8#7	VSS	7.01755e-17
C451	addr[2]#5	VSS	8.13217e-17
C452	FE_OFC0_q/n0#19	VSS	6.12765e-17
C453	addr[3]#1	VSS	5.10622e-17
C454	FE_OFN1_scan_out_0#13	VSS	6.5816e-17
C455	FE_OFC0_q/n0#17	VSS	6.09231e-17
C456	n_15#3	VSS	6.94992e-17
C457	\mem_reg[0]/qbint#3	VSS	5.44995e-17
C458	\mem_reg[2]/qbint#3	VSS	5.72001e-17
C459	\mem_reg[10]/CKbb#3	VSS	4.86865e-17
C460	FE_OFC0_q/n0#15	VSS	5.79471e-17
C461	FE_OFN1_scan_out_0#11	VSS	6.32346e-17
C462	\mem_reg[10]/CKb#5	VSS	4.86643e-17
C463	\mem_reg[15]/qbint#5	VSS	6.78534e-17
C464	mux_control_reg/qbint#5	VSS	6.34575e-17
C465	FE_OFC0_q/n0#13	VSS	6.47963e-17
C466	\mem_reg[8]/qbint#5	VSS	7.51101e-17
C467	FE_OFC0_q/n0#11	VSS	6.19618e-17
C468	\mem_reg[0]/CKbb#3	VSS	4.89261e-17
C469	\mem_reg[2]/CKbb#3	VSS	5.15147e-17
C470	FE_OFC0_q/n0#9	VSS	6.25657e-17
C471	\mem_reg[0]/CKb#5	VSS	4.96572e-17
C472	\mem_reg[2]/CKb#5	VSS	4.96973e-17
C473	\mem_reg[10]/CKb#3	VSS	5.23124e-17
C474	FE_OFC0_q/n0#7	VSS	6.18226e-17
C475	\mem_reg[15]/qbint#3	VSS	5.40219e-17
C476	mux_control_reg/qbint#3	VSS	5.30014e-17
C477	q_reg_reg/qbint#5	VSS	6.97218e-17
C478	FE_OFC0_q/n0#5	VSS	6.46766e-17
C479	\mem_reg[8]/qbint#3	VSS	5.44601e-17
C480	FE_OFC0_q/n0#3	VSS	6.45725e-17
C481	\mem_reg[0]/CKb#3	VSS	4.8494e-17
C482	\mem_reg[2]/CKb#3	VSS	5.2872e-17
C483	\mem_reg[15]/CKbb#3	VSS	5.02947e-17
C484	mux_control_reg/CKbb#3	VSS	4.91968e-17
C485	q_reg_reg/qbint#3	VSS	5.82612e-17
C486	FE_OFN0_q#7	VSS	7.44034e-17
C487	\mem_reg[15]/CKb#5	VSS	5.28475e-17
C488	mux_control_reg/CKb#5	VSS	5.03708e-17
C489	\mem_reg[8]/CKbb#3	VSS	4.77924e-17
C490	FE_OFN0_q#5	VSS	7.43545e-17
C491	scan_clk#35	VSS	5.00791e-17
C492	\mem_reg[8]/CKb#5	VSS	4.84208e-17
C493	FE_OFN0_q#3	VSS	6.7948e-17
C494	mem[8]#3	VSS	4.12188e-17
C495	q_reg_reg/CKbb#3	VSS	5.3557e-17
C496	\mem_reg[15]/CKb#3	VSS	5.06256e-17
C497	mux_control_reg/CKb#3	VSS	5.58696e-17
C498	q_reg_reg/CKb#5	VSS	5.08411e-17
C499	scan_en#71	VSS	5.43863e-17
C500	scan_clk#31	VSS	4.23244e-17
C501	scan_clk#33	VSS	4.85653e-17
C502	\mem_reg[8]/CKb#3	VSS	4.99975e-17
C503	mem[10]#3	VSS	4.83191e-17
C504	mem[13]#5	VSS	3.50535e-17
C505	mem[0]#5	VSS	3.50933e-17
C506	scan_en#69	VSS	5.61705e-17
C507	q_reg_reg/CKb#3	VSS	5.30834e-17
C508	\mem_reg[9]/qbint#5	VSS	6.94262e-17
C509	scan_en#65	VSS	6.01849e-17
C510	scan_en#67	VSS	5.89618e-17
C511	mem[0]#3	VSS	4.69152e-17
C512	mem[2]#5	VSS	4.86947e-17
C513	n_11#3	VSS	6.57678e-17
C514	scan_clk#27	VSS	4.49881e-17
C515	scan_clk#29	VSS	4.57139e-17
C516	scan_en#61	VSS	6.0506e-17
C517	scan_en#63	VSS	5.06859e-17
C518	mem[7]#5	VSS	3.39082e-17
C519	mem[11]#5	VSS	3.57591e-17
C520	scan_clk#25	VSS	4.20457e-17
C521	\mem_reg[9]/qbint#3	VSS	5.70037e-17
C522	mem[2]#3	VSS	3.61377e-17
C523	n_12#5	VSS	6.99558e-17
C524	scan_en#57	VSS	5.67227e-17
C525	scan_en#59	VSS	5.7129e-17
C526	clk#1	VSS	4.65108e-17
C527	mem[15]#5	VSS	4.79044e-17
C528	FE_OFN1_scan_out_0#9	VSS	4.64572e-17
C529	scan_en#55	VSS	5.81078e-17
C530	n_7#5	VSS	7.21477e-17
C531	x#1	VSS	3.92044e-17
C532	\mem_reg[13]/qbint#5	VSS	6.82822e-17
C533	\mem_reg[9]/CKbb#3	VSS	5.36288e-17
C534	scan_en#51	VSS	5.82529e-17
C535	scan_en#53	VSS	5.59542e-17
C536	\mem_reg[9]/CKb#5	VSS	5.30428e-17
C537	scan_en#49	VSS	5.46216e-17
C538	addr[2]#3	VSS	7.08731e-17
C539	q_control#7	VSS	6.36011e-17
C540	n_8#5	VSS	7.00295e-17
C541	\mem_reg[13]/qbint#3	VSS	5.4476e-17
C542	q_control#5	VSS	6.14349e-17
C543	\mem_reg[9]/CKb#3	VSS	5.21355e-17
C544	\mem_reg[6]/qbint#5	VSS	6.97494e-17
C545	\mem_reg[11]/qbint#5	VSS	6.8179e-17
C546	n_12#3	VSS	7.00374e-17
C547	addr[0]#9	VSS	7.36787e-17
C548	\mem_reg[13]/CKbb#3	VSS	4.98113e-17
C549	mem[15]#3	VSS	5.48349e-17
C550	\mem_reg[13]/CKb#5	VSS	4.65843e-17
C551	\mem_reg[6]/qbint#3	VSS	5.94473e-17
C552	\mem_reg[11]/qbint#3	VSS	5.51034e-17
C553	q_control_reg/qbint#5	VSS	6.89653e-17
C554	addr[1]#15	VSS	7.90917e-17
C555	addr[1]#13	VSS	4.73028e-17
C556	scan_clk#23	VSS	4.4352e-17
C557	addr[2]#1	VSS	7.03661e-17
C558	mem[13]#3	VSS	4.98192e-17
C559	\mem_reg[13]/CKb#3	VSS	4.97743e-17
C560	\mem_reg[6]/CKbb#3	VSS	5.10601e-17
C561	\mem_reg[11]/CKbb#3	VSS	4.87088e-17
C562	q_control_reg/qbint#3	VSS	5.87577e-17
C563	addr[1]#11	VSS	5.44956e-17
C564	\mem_reg[6]/CKb#5	VSS	5.20271e-17
C565	\mem_reg[11]/CKb#5	VSS	4.99582e-17
C566	scan_en#47	VSS	6.01389e-17
C567	mem[6]#5	VSS	6.60471e-17
C568	addr[1]#9	VSS	6.99055e-17
C569	mem[9]#5	VSS	4.72438e-17
C570	n_7#3	VSS	6.89411e-17
C571	q_control_reg/CKbb#3	VSS	5.05149e-17
C572	addr[0]#7	VSS	6.86146e-17
C573	scan_en#45	VSS	5.55112e-17
C574	\mem_reg[6]/CKb#3	VSS	4.97969e-17
C575	\mem_reg[11]/CKb#3	VSS	4.61797e-17
C576	q_control_reg/CKb#5	VSS	5.14355e-17
C577	addr[0]#5	VSS	6.70886e-17
C578	mem[7]#3	VSS	5.33443e-17
C579	scan_clk#21	VSS	4.2709e-17
C580	mem[4]#5	VSS	6.11907e-17
C581	addr[1]#7	VSS	4.91818e-17
C582	mem[14]#5	VSS	6.97025e-17
C583	addr[1]#5	VSS	6.71126e-17
C584	q_control_reg/CKb#3	VSS	5.32913e-17
C585	n_8#3	VSS	6.68878e-17
C586	mem[5]#5	VSS	4.61979e-17
C587	scan_en#43	VSS	5.90268e-17
C588	mem[11]#3	VSS	6.61813e-17
C589	addr[1]#3	VSS	5.25325e-17
C590	mem[9]#3	VSS	6.52426e-17
C591	scan_clk#17	VSS	4.46796e-17
C592	scan_clk#19	VSS	4.26381e-17
C593	scan_en#41	VSS	5.64061e-17
C594	n_2#5	VSS	6.95597e-17
C595	addr[0]#3	VSS	6.58832e-17
C596	mem[4]#3	VSS	4.28696e-17
C597	addr[1]#1	VSS	6.70271e-17
C598	mem[12]#5	VSS	6.58007e-17
C599	scan_en#37	VSS	5.22756e-17
C600	mem[3]#5	VSS	6.39345e-17
C601	scan_en#39	VSS	5.29051e-17
C602	scan_clk#15	VSS	4.81083e-17
C603	\mem_reg[5]/qbint#5	VSS	7.46904e-17
C604	n_2#3	VSS	6.73679e-17
C605	mem[6]#3	VSS	4.60099e-17
C606	mem[1]#5	VSS	6.36041e-17
C607	scan_in[0]#1	VSS	3.29231e-17
C608	\mem_reg[7]/qbint#5	VSS	6.49037e-17
C609	addr[0]#1	VSS	7.13219e-17
C610	scan_en#33	VSS	5.72058e-17
C611	scan_en#35	VSS	5.63036e-17
C612	scan_en#31	VSS	6.04004e-17
C613	\mem_reg[5]/qbint#3	VSS	5.40103e-17
C614	q_control#3	VSS	4.71602e-17
C615	\mem_reg[7]/qbint#3	VSS	5.44995e-17
C616	\mem_reg[14]/qbint#5	VSS	7.37599e-17
C617	scan_en#29	VSS	5.50211e-17
C618	\mem_reg[12]/qbint#5	VSS	7.31279e-17
C619	\mem_reg[4]/qbint#5	VSS	6.72324e-17
C620	\mem_reg[1]/qbint#5	VSS	6.6713e-17
C621	\mem_reg[3]/qbint#5	VSS	6.19106e-17
C622	\mem_reg[5]/CKbb#3	VSS	4.97887e-17
C623	FE_OFC1_scan_out_0/n0#31	VSS	6.93325e-17
C624	\mem_reg[5]/CKb#5	VSS	4.64302e-17
C625	\mem_reg[7]/CKbb#3	VSS	4.76435e-17
C626	\mem_reg[14]/qbint#3	VSS	5.36258e-17
C627	FE_OFC1_scan_out_0/n0#29	VSS	5.83235e-17
C628	\mem_reg[7]/CKb#5	VSS	4.82744e-17
C629	\mem_reg[12]/qbint#3	VSS	5.41241e-17
C630	\mem_reg[4]/qbint#3	VSS	5.47497e-17
C631	\mem_reg[1]/qbint#3	VSS	5.49121e-17
C632	\mem_reg[3]/qbint#3	VSS	5.35049e-17
C633	FE_OFC1_scan_out_0/n0#27	VSS	6.14485e-17
C634	FE_OFC1_scan_out_0/n0#25	VSS	6.41928e-17
C635	\mem_reg[5]/CKb#3	VSS	5.2106e-17
C636	\mem_reg[14]/CKbb#3	VSS	4.86162e-17
C637	FE_OFC1_scan_out_0/n0#23	VSS	6.40612e-17
C638	\mem_reg[7]/CKb#3	VSS	4.86491e-17
C639	\mem_reg[14]/CKb#5	VSS	4.80451e-17
C640	\mem_reg[12]/CKbb#3	VSS	4.70023e-17
C641	\mem_reg[4]/CKbb#3	VSS	5.0377e-17
C642	\mem_reg[1]/CKbb#3	VSS	5.04679e-17
C643	\mem_reg[3]/CKbb#3	VSS	4.92229e-17
C644	FE_OFC1_scan_out_0/n0#21	VSS	6.40612e-17
C645	\mem_reg[12]/CKb#5	VSS	4.71797e-17
C646	\mem_reg[4]/CKb#5	VSS	4.75997e-17
C647	\mem_reg[1]/CKb#5	VSS	4.73932e-17
C648	\mem_reg[3]/CKb#5	VSS	4.94066e-17
C649	FE_OFC1_scan_out_0/n0#19	VSS	6.40612e-17
C650	FE_OFC1_scan_out_0/n0#17	VSS	6.40612e-17
C651	\mem_reg[14]/CKb#3	VSS	5.06223e-17
C652	FE_OFC1_scan_out_0/n0#15	VSS	6.40612e-17
C653	scan_clk#13	VSS	4.86725e-17
C654	\mem_reg[12]/CKb#3	VSS	5.12715e-17
C655	\mem_reg[4]/CKb#3	VSS	5.14215e-17
C656	\mem_reg[1]/CKb#3	VSS	5.21153e-17
C657	\mem_reg[3]/CKb#3	VSS	5.12057e-17
C658	FE_OFC1_scan_out_0/n0#13	VSS	6.40977e-17
C659	mem[14]#3	VSS	3.82233e-17
C660	scan_clk#11	VSS	4.39772e-17
C661	FE_OFC1_scan_out_0/n0#11	VSS	6.40612e-17
C662	mem[5]#3	VSS	3.52673e-17
C663	FE_OFC1_scan_out_0/n0#9	VSS	6.55644e-17
C664	scan_en#27	VSS	6.09045e-17
C665	FE_OFC1_scan_out_0/n0#7	VSS	6.79549e-17
C666	scan_en#25	VSS	5.84403e-17
C667	scan_clk#9	VSS	4.56171e-17
C668	FE_OFC1_scan_out_0/n0#5	VSS	5.94674e-17
C669	scan_en#23	VSS	5.70356e-17
C670	mem[12]#3	VSS	3.65291e-17
C671	scan_clk#1	VSS	4.47336e-17
C672	scan_clk#3	VSS	4.85988e-17
C673	scan_clk#5	VSS	4.46068e-17
C674	scan_clk#7	VSS	4.27958e-17
C675	FE_OFC1_scan_out_0/n0#3	VSS	5.93777e-17
C676	scan_en#21	VSS	5.72816e-17
C677	mem[1]#3	VSS	3.98067e-17
C678	mem[3]#3	VSS	3.75706e-17
C679	scan_en#19	VSS	5.8274e-17
C680	FE_OFN1_scan_out_0#7	VSS	7.36686e-17
C681	scan_en#11	VSS	5.66754e-17
C682	scan_en#13	VSS	5.82789e-17
C683	scan_en#15	VSS	5.71049e-17
C684	scan_en#17	VSS	5.85785e-17
C685	FE_OFN1_scan_out_0#5	VSS	6.49682e-17
C686	scan_en#9	VSS	5.69638e-17
C687	FE_OFN1_scan_out_0#3	VSS	6.42267e-17
C688	scan_en#1	VSS	5.4536e-17
C689	scan_en#3	VSS	5.35619e-17
C690	scan_en#5	VSS	5.4529e-17
C691	scan_en#7	VSS	5.32003e-17
C692	FE_OFC0_q/n0#63	VSS	5.90587e-17
C693	g493__2398/n0#15	VSS	4.87986e-17
C694	\mem_reg[10]/net367#3	VSS	4.69668e-17
C695	FE_OFC0_q/n0#61	VSS	4.8808e-17
C696	g494__5107/n0#3	VSS	5.58634e-17
C697	g493__2398/n0#13	VSS	4.81852e-17
C698	\mem_reg[10]/qbint#11	VSS	5.04386e-17
C699	FE_OFC0_q/n0#59	VSS	4.43537e-17
C700	g493__2398/n0#11	VSS	5.04828e-17
C701	n_7#15	VSS	5.54002e-17
C702	n_14#3	VSS	4.07091e-17
C703	FE_OFC0_q/n0#57	VSS	4.65223e-17
C704	g493__2398/n0#9	VSS	5.71076e-17
C705	mem[10]#11	VSS	4.9494e-17
C706	addr[3]#7	VSS	3.74424e-17
C707	\mem_reg[0]/net367#3	VSS	4.33626e-17
C708	\mem_reg[2]/net367#3	VSS	4.34145e-17
C709	\mem_reg[10]/n30#3	VSS	5.46428e-17
C710	FE_OFC0_q/n0#55	VSS	4.86855e-17
C711	mem[8]#11	VSS	5.11925e-17
C712	g494__5107/S0b#3	VSS	4.31095e-17
C713	x#7	VSS	4.53166e-17
C714	\mem_reg[0]/qbint#11	VSS	4.68393e-17
C715	\mem_reg[2]/qbint#11	VSS	4.69512e-17
C716	\mem_reg[10]/qbint#9	VSS	4.21723e-17
C717	FE_OFC0_q/n0#53	VSS	4.37639e-17
C718	n_8#15	VSS	5.14279e-17
C719	addr[2]#11	VSS	6.50052e-17
C720	n_13#3	VSS	3.34162e-17
C721	FE_OFN1_scan_out_0#27	VSS	5.10413e-17
C722	FE_OFC0_q/n0#51	VSS	4.36024e-17
C723	addr[3]#5	VSS	4.39522e-17
C724	g493__2398/S0b#3	VSS	4.6537e-17
C725	\mem_reg[0]/n30#3	VSS	4.27259e-17
C726	\mem_reg[2]/n30#3	VSS	4.28384e-17
C727	\mem_reg[10]/qbint#7	VSS	4.19211e-17
C728	FE_OFC0_q/n0#49	VSS	4.78797e-17
C729	n_15#7	VSS	4.55012e-17
C730	\mem_reg[0]/qbint#9	VSS	3.79733e-17
C731	\mem_reg[2]/qbint#9	VSS	3.78686e-17
C732	\mem_reg[10]/CKb#11	VSS	5.23518e-17
C733	\mem_reg[15]/net367#3	VSS	4.559e-17
C734	mux_control_reg/net367#3	VSS	5.13508e-17
C735	FE_OFC0_q/n0#47	VSS	4.39858e-17
C736	FE_OFN1_scan_out_0#25	VSS	4.23851e-17
C737	\mem_reg[10]/CKbb#7	VSS	4.37531e-17
C738	\mem_reg[15]/qbint#11	VSS	4.91835e-17
C739	mux_control_reg/qbint#11	VSS	5.21737e-17
C740	\mem_reg[8]/net367#3	VSS	4.78517e-17
C741	FE_OFC0_q/n0#45	VSS	4.56666e-17
C742	\mem_reg[10]/n20#3	VSS	5.07482e-17
C743	\mem_reg[8]/qbint#11	VSS	4.9802e-17
C744	\mem_reg[0]/qbint#7	VSS	4.05834e-17
C745	\mem_reg[2]/qbint#7	VSS	4.02573e-17
C746	FE_OFC0_q/n0#43	VSS	4.4017e-17
C747	\mem_reg[0]/CKb#11	VSS	4.91351e-17
C748	\mem_reg[2]/CKb#11	VSS	4.8317e-17
C749	FE_OFC0_q/n0#41	VSS	4.38124e-17
C750	\mem_reg[15]/n30#3	VSS	4.50188e-17
C751	mux_control_reg/n30#3	VSS	4.94186e-17
C752	q_reg_reg/net367#3	VSS	4.86519e-17
C753	\mem_reg[0]/CKbb#7	VSS	4.09385e-17
C754	\mem_reg[2]/CKbb#7	VSS	4.12136e-17
C755	\mem_reg[10]/CKb#9	VSS	4.5008e-17
C756	FE_OFC0_q/n0#39	VSS	4.43379e-17
C757	\mem_reg[15]/qbint#9	VSS	3.8378e-17
C758	mux_control_reg/qbint#9	VSS	4.32564e-17
C759	q_reg_reg/qbint#11	VSS	4.9106e-17
C760	\mem_reg[8]/n30#3	VSS	4.34101e-17
C761	\mem_reg[0]/n20#3	VSS	4.69525e-17
C762	\mem_reg[2]/n20#3	VSS	4.67733e-17
C763	\mem_reg[10]/mout#3	VSS	4.80851e-17
C764	FE_OFC0_q/n0#37	VSS	4.43587e-17
C765	\mem_reg[8]/qbint#9	VSS	4.14691e-17
C766	\mem_reg[10]/CKbb#5	VSS	4.00339e-17
C767	FE_OFC0_q/n0#35	VSS	4.42653e-17
C768	\mem_reg[15]/qbint#7	VSS	4.3902e-17
C769	mux_control_reg/qbint#7	VSS	4.61262e-17
C770	q_reg_reg/n30#3	VSS	4.55002e-17
C771	\mem_reg[10]/CKb#7	VSS	3.62667e-17
C772	FE_OFC0_q/n0#33	VSS	4.51087e-17
C773	\mem_reg[0]/CKb#9	VSS	4.53919e-17
C774	\mem_reg[2]/CKb#9	VSS	4.53636e-17
C775	\mem_reg[15]/CKb#11	VSS	4.93388e-17
C776	mux_control_reg/CKb#11	VSS	5.17689e-17
C777	q_reg_reg/qbint#9	VSS	4.02948e-17
C778	\mem_reg[8]/qbint#7	VSS	4.15051e-17
C779	FE_OFN0_q#15	VSS	5.063e-17
C780	\mem_reg[0]/mout#3	VSS	4.82141e-17
C781	\mem_reg[2]/mout#3	VSS	4.81858e-17
C782	\mem_reg[15]/CKbb#7	VSS	4.25461e-17
C783	mux_control_reg/CKbb#7	VSS	4.46253e-17
C784	\mem_reg[8]/CKb#11	VSS	4.91912e-17
C785	FE_OFN0_q#13	VSS	4.86329e-17
C786	\mem_reg[0]/CKbb#5	VSS	4.01642e-17
C787	\mem_reg[2]/CKbb#5	VSS	4.01363e-17
C788	scan_clk#71	VSS	4.08037e-17
C789	\mem_reg[15]/n20#3	VSS	5.01799e-17
C790	mux_control_reg/n20#3	VSS	4.90545e-17
C791	\mem_reg[8]/CKbb#7	VSS	4.04554e-17
C792	q_reg_reg/qbint#7	VSS	4.35037e-17
C793	FE_OFN0_q#11	VSS	4.90695e-17
C794	\mem_reg[0]/CKb#7	VSS	3.76034e-17
C795	\mem_reg[2]/CKb#7	VSS	3.74332e-17
C796	mem[8]#9	VSS	3.77147e-17
C797	\mem_reg[8]/n20#3	VSS	5.05979e-17
C798	q_reg_reg/CKb#11	VSS	5.21751e-17
C799	FE_OFN0_q#9	VSS	5.86318e-17
C800	scan_en#143	VSS	4.41994e-17
C801	\mem_reg[15]/CKb#9	VSS	4.83487e-17
C802	mux_control_reg/CKb#9	VSS	4.75147e-17
C803	q_reg_reg/CKbb#7	VSS	3.96809e-17
C804	\mem_reg[10]/SEb#3	VSS	4.08813e-17
C805	scan_clk#67	VSS	3.8696e-17
C806	scan_clk#69	VSS	3.79703e-17
C807	\mem_reg[15]/mout#3	VSS	5.21831e-17
C808	mux_control_reg/mout#3	VSS	5.14005e-17
C809	q_reg_reg/n20#3	VSS	4.64635e-17
C810	\mem_reg[8]/CKb#9	VSS	4.59163e-17
C811	mem[10]#9	VSS	4.74217e-17
C812	mem[13]#11	VSS	3.41737e-17
C813	mem[0]#11	VSS	3.65509e-17
C814	\mem_reg[15]/CKbb#5	VSS	4.19674e-17
C815	mux_control_reg/CKbb#5	VSS	4.42017e-17
C816	\mem_reg[8]/mout#3	VSS	5.00437e-17
C817	\mem_reg[9]/net367#3	VSS	4.49698e-17
C818	scan_en#141	VSS	4.83956e-17
C819	scan_en#137	VSS	4.25199e-17
C820	scan_en#139	VSS	4.47652e-17
C821	\mem_reg[15]/CKb#7	VSS	4.42054e-17
C822	mux_control_reg/CKb#7	VSS	4.46304e-17
C823	\mem_reg[8]/CKbb#5	VSS	3.96256e-17
C824	q_reg_reg/CKb#9	VSS	4.7558e-17
C825	\mem_reg[9]/qbint#11	VSS	4.78429e-17
C826	\mem_reg[0]/SEb#3	VSS	4.02934e-17
C827	\mem_reg[2]/SEb#3	VSS	3.91235e-17
C828	\mem_reg[8]/CKb#7	VSS	4.40586e-17
C829	q_reg_reg/mout#3	VSS	5.18497e-17
C830	mem[0]#9	VSS	4.46028e-17
C831	mem[2]#11	VSS	4.43936e-17
C832	n_11#7	VSS	6.06739e-17
C833	scan_clk#63	VSS	4.02522e-17
C834	scan_clk#65	VSS	4.14043e-17
C835	q_reg_reg/CKbb#5	VSS	3.99358e-17
C836	\mem_reg[9]/n30#3	VSS	4.50283e-17
C837	scan_en#133	VSS	4.46663e-17
C838	scan_en#135	VSS	4.53925e-17
C839	n_3#3	VSS	5.07198e-17
C840	mem[7]#11	VSS	3.52669e-17
C841	mem[11]#11	VSS	3.50027e-17
C842	q_reg_reg/CKb#7	VSS	3.89543e-17
C843	scan_clk#61	VSS	3.83875e-17
C844	\mem_reg[9]/qbint#9	VSS	4.07567e-17
C845	n_9#3	VSS	5.80779e-17
C846	scan_en#129	VSS	4.41221e-17
C847	scan_en#131	VSS	4.51817e-17
C848	mem[2]#9	VSS	3.63409e-17
C849	n_12#11	VSS	5.26788e-17
C850	\mem_reg[15]/SEb#3	VSS	4.21658e-17
C851	mux_control_reg/SEb#3	VSS	4.36242e-17
C852	scan_en#127	VSS	4.28467e-17
C853	clk#3	VSS	3.97746e-17
C854	\mem_reg[13]/net367#3	VSS	5.11682e-17
C855	\mem_reg[9]/qbint#7	VSS	4.33964e-17
C856	mem[15]#11	VSS	4.65332e-17
C857	FE_OFN1_scan_out_0#23	VSS	4.63746e-17
C858	\mem_reg[8]/SEb#3	VSS	4.29142e-17
C859	n_7#13	VSS	5.63043e-17
C860	x#5	VSS	3.67832e-17
C861	\mem_reg[13]/qbint#11	VSS	5.15504e-17
C862	\mem_reg[9]/CKb#11	VSS	5.06022e-17
C863	scan_en#123	VSS	4.52914e-17
C864	scan_en#125	VSS	4.70351e-17
C865	mem[8]#7	VSS	4.61279e-17
C866	n_5#3	VSS	5.70024e-17
C867	mem[2]#7	VSS	4.98408e-17
C868	q_control#15	VSS	4.45769e-17
C869	\mem_reg[9]/CKbb#7	VSS	4.29676e-17
C870	scan_en#121	VSS	4.47623e-17
C871	addr[2]#9	VSS	5.17148e-17
C872	mem[0]#7	VSS	5.0989e-17
C873	q_reg_reg/SEb#3	VSS	4.21916e-17
C874	\mem_reg[9]/n20#3	VSS	5.38969e-17
C875	\mem_reg[13]/n30#3	VSS	5.02074e-17
C876	n_8#13	VSS	5.41663e-17
C877	n_15#5	VSS	4.64749e-17
C878	\mem_reg[13]/qbint#9	VSS	5.33738e-17
C879	\mem_reg[6]/net367#3	VSS	4.69469e-17
C880	\mem_reg[11]/net367#3	VSS	5.1022e-17
C881	q_control#13	VSS	4.81082e-17
C882	n_11#5	VSS	5.6978e-17
C883	\mem_reg[9]/CKb#9	VSS	4.83055e-17
C884	\mem_reg[6]/qbint#11	VSS	4.89717e-17
C885	\mem_reg[11]/qbint#11	VSS	5.51457e-17
C886	n_12#9	VSS	5.68595e-17
C887	n_4#3	VSS	4.96359e-17
C888	\mem_reg[9]/mout#3	VSS	5.04767e-17
C889	\mem_reg[13]/qbint#7	VSS	5.03e-17
C890	addr[0]#19	VSS	5.47658e-17
C891	n_10#3	VSS	5.76359e-17
C892	g519__1705/n0#3	VSS	5.81621e-17
C893	\mem_reg[9]/CKbb#5	VSS	4.13909e-17
C894	\mem_reg[13]/CKb#11	VSS	5.40535e-17
C895	\mem_reg[6]/n30#3	VSS	4.53005e-17
C896	\mem_reg[11]/n30#3	VSS	4.96659e-17
C897	q_control_reg/net367#3	VSS	4.82497e-17
C898	n_12#7	VSS	5.09565e-17
C899	mem[15]#9	VSS	4.26438e-17
C900	\mem_reg[9]/CKb#7	VSS	4.26579e-17
C901	\mem_reg[13]/CKbb#7	VSS	4.42836e-17
C902	\mem_reg[6]/qbint#9	VSS	4.01295e-17
C903	\mem_reg[11]/qbint#9	VSS	4.40362e-17
C904	q_control_reg/qbint#11	VSS	4.88176e-17
C905	addr[1]#31	VSS	3.87438e-17
C906	\mem_reg[13]/n20#3	VSS	5.00291e-17
C907	addr[1]#29	VSS	6.60472e-17
C908	n_6#3	VSS	5.53809e-17
C909	g519__1705/S0b#3	VSS	4.33565e-17
C910	scan_clk#59	VSS	3.87707e-17
C911	\mem_reg[6]/qbint#7	VSS	4.45779e-17
C912	\mem_reg[11]/qbint#7	VSS	4.76896e-17
C913	q_control_reg/n30#3	VSS	4.48518e-17
C914	addr[2]#7	VSS	5.44803e-17
C915	mem[13]#9	VSS	2.81862e-17
C916	mem[10]#7	VSS	3.54558e-17
C917	\mem_reg[13]/CKb#9	VSS	5.66946e-17
C918	\mem_reg[6]/CKb#11	VSS	5.11473e-17
C919	\mem_reg[11]/CKb#11	VSS	5.3249e-17
C920	q_control_reg/qbint#9	VSS	4.06005e-17
C921	addr[1]#27	VSS	4.76095e-17
C922	scan_en#119	VSS	5.38072e-17
C923	\mem_reg[13]/mout#3	VSS	5.39102e-17
C924	\mem_reg[6]/CKbb#7	VSS	4.16799e-17
C925	\mem_reg[11]/CKbb#7	VSS	4.75687e-17
C926	\mem_reg[9]/SEb#3	VSS	4.48185e-17
C927	mem[6]#11	VSS	5.74317e-17
C928	\mem_reg[13]/CKbb#5	VSS	4.13096e-17
C929	\mem_reg[6]/n20#3	VSS	5.39224e-17
C930	\mem_reg[11]/n20#3	VSS	4.49203e-17
C931	q_control_reg/qbint#7	VSS	4.21401e-17
C932	addr[1]#25	VSS	5.67136e-17
C933	mem[9]#11	VSS	4.55131e-17
C934	n_7#11	VSS	5.29238e-17
C935	\mem_reg[13]/CKb#7	VSS	4.45583e-17
C936	q_control_reg/CKb#11	VSS	5.23819e-17
C937	addr[0]#17	VSS	5.95419e-17
C938	scan_en#117	VSS	5.0582e-17
C939	g520__5122/n0#3	VSS	5.66583e-17
C940	\mem_reg[6]/CKb#9	VSS	5.38754e-17
C941	\mem_reg[11]/CKb#9	VSS	4.93727e-17
C942	q_control_reg/CKbb#7	VSS	3.98749e-17
C943	addr[0]#15	VSS	5.40627e-17
C944	mem[7]#9	VSS	4.11073e-17
C945	scan_clk#57	VSS	3.90563e-17
C946	\mem_reg[6]/mout#3	VSS	4.99724e-17
C947	\mem_reg[11]/mout#3	VSS	5.10465e-17
C948	q_control_reg/n20#3	VSS	4.56305e-17
C949	n_0#3	VSS	5.07332e-17
C950	addr[1]#23	VSS	3.83368e-17
C951	mem[15]#7	VSS	3.57039e-17
C952	\mem_reg[6]/CKbb#5	VSS	4.25666e-17
C953	\mem_reg[11]/CKbb#5	VSS	4.36601e-17
C954	mem[4]#11	VSS	4.92921e-17
C955	g520__5122/S0b#3	VSS	3.93029e-17
C956	scan_en#115	VSS	4.50785e-17
C957	mem[14]#11	VSS	5.61315e-17
C958	addr[1]#21	VSS	5.63906e-17
C959	\mem_reg[6]/CKb#7	VSS	4.29972e-17
C960	\mem_reg[11]/CKb#7	VSS	4.41721e-17
C961	q_control_reg/CKb#9	VSS	4.65081e-17
C962	n_8#11	VSS	5.1547e-17
C963	mem[5]#11	VSS	3.12694e-17
C964	\mem_reg[13]/SEb#3	VSS	4.16658e-17
C965	n_7#9	VSS	4.96732e-17
C966	mem[11]#9	VSS	4.89391e-17
C967	q_control_reg/mout#3	VSS	4.90323e-17
C968	addr[1]#19	VSS	4.35689e-17
C969	mem[13]#7	VSS	4.57268e-17
C970	mem[9]#9	VSS	4.91234e-17
C971	scan_clk#53	VSS	4.10586e-17
C972	scan_clk#55	VSS	4.16869e-17
C973	q_control_reg/CKbb#5	VSS	4.09767e-17
C974	scan_en#113	VSS	4.74902e-17
C975	n_2#11	VSS	5.0947e-17
C976	addr[0]#13	VSS	5.0847e-17
C977	mem[4]#9	VSS	3.47346e-17
C978	mem[9]#7	VSS	3.47409e-17
C979	q_control_reg/CKb#7	VSS	3.94015e-17
C980	n_1#3	VSS	4.82277e-17
C981	scan_en#109	VSS	4.51265e-17
C982	addr[1]#17	VSS	5.63278e-17
C983	scan_en#111	VSS	4.8667e-17
C984	\mem_reg[5]/net367#3	VSS	5.10988e-17
C985	mem[12]#11	VSS	4.96008e-17
C986	\mem_reg[6]/SEb#3	VSS	3.97245e-17
C987	mem[3]#11	VSS	4.99473e-17
C988	\mem_reg[11]/SEb#3	VSS	4.10927e-17
C989	scan_clk#51	VSS	4.16297e-17
C990	\mem_reg[5]/qbint#11	VSS	4.97148e-17
C991	\mem_reg[7]/net367#3	VSS	4.90241e-17
C992	n_2#9	VSS	5.19244e-17
C993	n_8#9	VSS	5.10098e-17
C994	mem[6]#9	VSS	4.64181e-17
C995	mem[1]#11	VSS	5.29321e-17
C996	mem[11]#7	VSS	4.82084e-17
C997	scan_in[0]#3	VSS	3.71412e-17
C998	\mem_reg[7]/qbint#11	VSS	5.19281e-17
C999	addr[0]#11	VSS	5.27056e-17
C1000	scan_en#105	VSS	4.64158e-17
C1001	n_2#7	VSS	5.3847e-17
C1002	scan_en#107	VSS	4.80243e-17
C1003	scan_en#103	VSS	4.44269e-17
C1004	\mem_reg[5]/n30#3	VSS	4.67163e-17
C1005	q_control_reg/SEb#3	VSS	4.25416e-17
C1006	\mem_reg[5]/qbint#9	VSS	4.11847e-17
C1007	\mem_reg[7]/n30#3	VSS	4.7169e-17
C1008	\mem_reg[14]/net367#3	VSS	4.73376e-17
C1009	q_control#11	VSS	4.62186e-17
C1010	\mem_reg[7]/qbint#9	VSS	4.60921e-17
C1011	\mem_reg[14]/qbint#11	VSS	4.84305e-17
C1012	\mem_reg[12]/net367#3	VSS	5.03788e-17
C1013	\mem_reg[4]/net367#3	VSS	4.58425e-17
C1014	\mem_reg[1]/net367#3	VSS	4.93892e-17
C1015	\mem_reg[3]/net367#3	VSS	4.96189e-17
C1016	scan_en#101	VSS	4.38442e-17
C1017	\mem_reg[12]/qbint#11	VSS	4.86589e-17
C1018	\mem_reg[4]/qbint#11	VSS	4.80087e-17
C1019	\mem_reg[1]/qbint#11	VSS	5.05126e-17
C1020	\mem_reg[3]/qbint#11	VSS	5.54669e-17
C1021	\mem_reg[5]/qbint#7	VSS	4.55792e-17
C1022	\mem_reg[5]/CKb#11	VSS	5.11688e-17
C1023	\mem_reg[7]/qbint#7	VSS	4.74872e-17
C1024	\mem_reg[14]/n30#3	VSS	4.90454e-17
C1025	FE_OFC1_scan_out_0/n0#63	VSS	5.6071e-17
C1026	\mem_reg[5]/CKbb#7	VSS	4.2551e-17
C1027	\mem_reg[7]/CKb#11	VSS	5.16787e-17
C1028	\mem_reg[14]/qbint#9	VSS	4.23755e-17
C1029	\mem_reg[12]/n30#3	VSS	4.5336e-17
C1030	\mem_reg[4]/n30#3	VSS	4.54289e-17
C1031	\mem_reg[1]/n30#3	VSS	4.75692e-17
C1032	\mem_reg[3]/n30#3	VSS	5.3478e-17
C1033	FE_OFC1_scan_out_0/n0#61	VSS	4.99721e-17
C1034	\mem_reg[5]/n20#3	VSS	4.88229e-17
C1035	\mem_reg[7]/CKbb#7	VSS	4.22665e-17
C1036	\mem_reg[12]/qbint#9	VSS	3.94263e-17
C1037	\mem_reg[4]/qbint#9	VSS	4.05256e-17
C1038	\mem_reg[1]/qbint#9	VSS	4.13274e-17
C1039	\mem_reg[3]/qbint#9	VSS	4.53753e-17
C1040	FE_OFC1_scan_out_0/n0#59	VSS	4.44309e-17
C1041	\mem_reg[7]/n20#3	VSS	5.22432e-17
C1042	\mem_reg[14]/qbint#7	VSS	4.74746e-17
C1043	FE_OFC1_scan_out_0/n0#57	VSS	4.92228e-17
C1044	\mem_reg[5]/CKb#9	VSS	4.8553e-17
C1045	\mem_reg[14]/CKb#11	VSS	5.16459e-17
C1046	\mem_reg[12]/qbint#7	VSS	4.19982e-17
C1047	\mem_reg[4]/qbint#7	VSS	4.29827e-17
C1048	\mem_reg[1]/qbint#7	VSS	4.49046e-17
C1049	\mem_reg[3]/qbint#7	VSS	5.53612e-17
C1050	FE_OFC1_scan_out_0/n0#55	VSS	4.73494e-17
C1051	\mem_reg[5]/mout#3	VSS	4.83234e-17
C1052	\mem_reg[7]/CKb#9	VSS	4.77539e-17
C1053	\mem_reg[14]/CKbb#7	VSS	4.42013e-17
C1054	\mem_reg[12]/CKb#11	VSS	4.9724e-17
C1055	\mem_reg[4]/CKb#11	VSS	5.05773e-17
C1056	\mem_reg[1]/CKb#11	VSS	5.25063e-17
C1057	\mem_reg[3]/CKb#11	VSS	5.52253e-17
C1058	FE_OFC1_scan_out_0/n0#53	VSS	4.72797e-17
C1059	\mem_reg[5]/CKbb#5	VSS	4.01167e-17
C1060	\mem_reg[7]/mout#3	VSS	5.20298e-17
C1061	\mem_reg[14]/n20#3	VSS	5.07419e-17
C1062	\mem_reg[12]/CKbb#7	VSS	4.0867e-17
C1063	\mem_reg[4]/CKbb#7	VSS	4.27108e-17
C1064	\mem_reg[1]/CKbb#7	VSS	4.35425e-17
C1065	\mem_reg[3]/CKbb#7	VSS	4.90099e-17
C1066	FE_OFC1_scan_out_0/n0#51	VSS	4.72774e-17
C1067	\mem_reg[5]/CKb#7	VSS	4.13979e-17
C1068	\mem_reg[7]/CKbb#5	VSS	4.22773e-17
C1069	\mem_reg[12]/n20#3	VSS	4.88794e-17
C1070	\mem_reg[4]/n20#3	VSS	4.83944e-17
C1071	\mem_reg[1]/n20#3	VSS	5.05837e-17
C1072	\mem_reg[3]/n20#3	VSS	5.36828e-17
C1073	FE_OFC1_scan_out_0/n0#49	VSS	5.01036e-17
C1074	\mem_reg[7]/CKb#7	VSS	4.62002e-17
C1075	\mem_reg[14]/CKb#9	VSS	5.00616e-17
C1076	FE_OFC1_scan_out_0/n0#47	VSS	4.72888e-17
C1077	scan_clk#49	VSS	4.04719e-17
C1078	\mem_reg[14]/mout#3	VSS	5.20611e-17
C1079	\mem_reg[12]/CKb#9	VSS	4.64764e-17
C1080	\mem_reg[4]/CKb#9	VSS	4.68337e-17
C1081	\mem_reg[1]/CKb#9	VSS	4.85141e-17
C1082	\mem_reg[3]/CKb#9	VSS	5.06598e-17
C1083	FE_OFC1_scan_out_0/n0#45	VSS	4.85776e-17
C1084	mem[14]#9	VSS	3.48651e-17
C1085	scan_clk#47	VSS	3.97293e-17
C1086	\mem_reg[14]/CKbb#5	VSS	4.32032e-17
C1087	\mem_reg[12]/mout#3	VSS	4.83232e-17
C1088	\mem_reg[4]/mout#3	VSS	4.96559e-17
C1089	\mem_reg[1]/mout#3	VSS	5.26857e-17
C1090	\mem_reg[3]/mout#3	VSS	5.65788e-17
C1091	FE_OFC1_scan_out_0/n0#43	VSS	4.62786e-17
C1092	scan_en#99	VSS	4.45214e-17
C1093	mem[5]#9	VSS	3.77341e-17
C1094	\mem_reg[14]/CKb#7	VSS	4.06766e-17
C1095	\mem_reg[12]/CKbb#5	VSS	3.99744e-17
C1096	\mem_reg[4]/CKbb#5	VSS	4.16064e-17
C1097	\mem_reg[1]/CKbb#5	VSS	4.35353e-17
C1098	\mem_reg[3]/CKbb#5	VSS	4.48101e-17
C1099	FE_OFC1_scan_out_0/n0#41	VSS	4.8673e-17
C1100	\mem_reg[5]/SEb#3	VSS	3.74741e-17
C1101	scan_en#97	VSS	4.47106e-17
C1102	\mem_reg[12]/CKb#7	VSS	4.287e-17
C1103	\mem_reg[4]/CKb#7	VSS	4.0117e-17
C1104	\mem_reg[1]/CKb#7	VSS	3.97193e-17
C1105	\mem_reg[3]/CKb#7	VSS	4.74803e-17
C1106	FE_OFC1_scan_out_0/n0#39	VSS	4.60391e-17
C1107	mem[5]#7	VSS	4.59359e-17
C1108	\mem_reg[7]/SEb#3	VSS	4.24534e-17
C1109	scan_clk#45	VSS	4.37953e-17
C1110	FE_OFC1_scan_out_0/n0#37	VSS	4.63293e-17
C1111	scan_en#95	VSS	4.63446e-17
C1112	mem[7]#7	VSS	4.68114e-17
C1113	mem[12]#9	VSS	3.3773e-17
C1114	scan_clk#37	VSS	3.93497e-17
C1115	scan_clk#39	VSS	3.79032e-17
C1116	scan_clk#41	VSS	4.0984e-17
C1117	scan_clk#43	VSS	3.80359e-17
C1118	FE_OFC1_scan_out_0/n0#35	VSS	4.58126e-17
C1119	scan_en#93	VSS	4.82682e-17
C1120	scan_en#91	VSS	4.21356e-17
C1121	mem[6]#7	VSS	3.73361e-17
C1122	mem[1]#9	VSS	3.41538e-17
C1123	mem[3]#9	VSS	3.53428e-17
C1124	q_control#9	VSS	3.55076e-17
C1125	FE_OFC1_scan_out_0/n0#33	VSS	4.26785e-17
C1126	\mem_reg[14]/SEb#3	VSS	3.87066e-17
C1127	scan_en#83	VSS	4.41902e-17
C1128	scan_en#85	VSS	4.46474e-17
C1129	scan_en#87	VSS	4.42994e-17
C1130	scan_en#89	VSS	4.29519e-17
C1131	FE_OFN1_scan_out_0#21	VSS	5.1286e-17
C1132	mem[14]#7	VSS	4.59533e-17
C1133	\mem_reg[12]/SEb#3	VSS	4.28953e-17
C1134	\mem_reg[4]/SEb#3	VSS	4.17486e-17
C1135	\mem_reg[1]/SEb#3	VSS	4.03907e-17
C1136	\mem_reg[3]/SEb#3	VSS	3.93462e-17
C1137	FE_OFN1_scan_out_0#19	VSS	5.02128e-17
C1138	scan_en#81	VSS	4.47225e-17
C1139	mem[12]#7	VSS	4.59607e-17
C1140	mem[4]#7	VSS	4.54022e-17
C1141	mem[1]#7	VSS	4.56748e-17
C1142	mem[3]#7	VSS	4.41644e-17
C1143	FE_OFN1_scan_out_0#17	VSS	5.19946e-17
C1144	scan_en#73	VSS	4.73086e-17
C1145	scan_en#75	VSS	4.33718e-17
C1146	scan_en#77	VSS	4.44838e-17
C1147	scan_en#79	VSS	4.43957e-17
C1148	FE_OFN1_scan_out_0#15	VSS	5.69653e-17
C1149	VDD#467	VSS	2.76427e-15
C1150	clk#14	VSS	1.41619e-15
C1151	n_13#29	VSS	2.41151e-16
C1152	mux#17	VSS	2.96476e-16
C1153	n_13#26	VSS	1.41692e-15
C1154	addr[3]#25	VSS	5.6673e-16
C1155	q#61	VSS	3.51596e-16
C1156	mem[7]#56	VSS	1.96687e-15
C1157	mem[7]#59	VSS	8.96025e-16
C1158	addr[2]#48	VSS	1.85607e-15
C1159	scan_en#384	VSS	2.10091e-15
C1160	scan_en#381	VSS	6.75572e-16
C1161	n_7#64	VSS	1.10944e-15
C1162	addr[2]#43	VSS	1.07906e-15
C1163	addr[2]#46	VSS	1.14616e-15
C1164	n_7#66	VSS	2.56549e-15
C1165	scan_clk#242	VSS	5.16419e-16
C1166	mem[6]#56	VSS	4.03351e-16
C1167	mem[6]#52	VSS	6.49875e-16
C1168	n_9#29	VSS	1.6927e-15
C1169	mem[15]#51	VSS	3.75945e-16
C1170	mem[15]#54	VSS	7.65613e-16
C1171	n_9#26	VSS	1.21045e-15
C1172	addr[1]#107	VSS	6.86173e-16
C1173	addr[1]#101	VSS	1.01714e-15
C1174	addr[1]#103	VSS	1.30114e-15
C1175	n_7#67	VSS	9.55943e-16
C1176	n_7#62	VSS	8.09332e-16
C1177	VDD#457	VSS	9.65136e-16
C1178	VDD#459	VSS	2.35843e-15
C1179	VDD#461	VSS	2.74794e-15
C1180	VDD#463	VSS	2.92392e-15
C1181	VDD#465	VSS	1.15685e-15
C1182	addr[0]#69	VSS	1.36008e-15
C1183	addr[0]#72	VSS	1.34275e-15
C1184	scan_en#378	VSS	5.3528e-16
C1185	scan_in[0]#16	VSS	3.481e-16
C1186	addr[0]#68	VSS	1.05513e-15
C1187	scan_clk#235	VSS	6.86487e-16
C1188	scan_clk#239	VSS	1.22359e-15
C1189	x#28	VSS	5.08344e-16
C1190	scan_out[0]#61	VSS	1.96081e-16
C1191	n_7#60	VSS	1.42563e-15
C1192	mux#15	VSS	3.5352e-16
C1193	n_14#27	VSS	7.36744e-16
C1194	n_5#22	VSS	1.25029e-15
C1195	n_13#24	VSS	2.75452e-16
C1196	mem[10]#47	VSS	1.01312e-15
C1197	n_12#49	VSS	1.04995e-15
C1198	mem[0]#50	VSS	3.90403e-16
C1199	mem[2]#48	VSS	7.56971e-16
C1200	n_8#61	VSS	1.62875e-15
C1201	addr[2]#41	VSS	8.0645e-16
C1202	addr[3]#23	VSS	1.92557e-15
C1203	FE_OFN1_scan_out_0#80	VSS	1.00199e-15
C1204	mem[15]#50	VSS	8.00128e-16
C1205	mem[8]#48	VSS	3.87694e-16
C1206	q#59	VSS	9.41843e-16
C1207	n_14#24	VSS	9.92866e-16
C1208	n_14#25	VSS	9.62968e-16
C1209	mem[8]#44	VSS	1.36826e-15
C1210	scan_clk#232	VSS	1.38213e-15
C1211	mem[10]#45	VSS	1.27008e-15
C1212	scan_en#377	VSS	3.60403e-16
C1213	scan_en#371	VSS	1.84019e-15
C1214	mem[9]#48	VSS	9.74314e-16
C1215	mem[0]#44	VSS	5.55287e-16
C1216	scan_en#369	VSS	4.49596e-16
C1217	mem[2]#45	VSS	8.13722e-16
C1218	mem[7]#53	VSS	1.75959e-16
C1219	n_14#23	VSS	4.90427e-16
C1220	scan_clk#230	VSS	6.11979e-16
C1221	n_3#23	VSS	1.41319e-15
C1222	n_7#58	VSS	5.93779e-16
C1223	n_9#25	VSS	7.78433e-16
C1224	n_12#47	VSS	1.22808e-15
C1225	clk#12	VSS	1.16913e-15
C1226	mem[8]#42	VSS	6.88413e-16
C1227	scan_en#375	VSS	2.4492e-16
C1228	mem[15]#46	VSS	1.82921e-15
C1229	n_5#20	VSS	1.14594e-15
C1230	addr[2]#40	VSS	2.20143e-16
C1231	n_8#59	VSS	4.51134e-16
C1232	FE_OFN1_scan_out_0#77	VSS	2.25631e-15
C1233	n_13#22	VSS	1.50305e-15
C1234	addr[0]#65	VSS	9.82947e-16
C1235	mem[11]#46	VSS	7.59244e-16
C1236	n_1#22	VSS	7.8369e-16
C1237	n_4#23	VSS	9.31159e-16
C1238	mem[6]#49	VSS	4.29979e-16
C1239	FE_OFN1_scan_out_0#76	VSS	1.78489e-15
C1240	mem[15]#47	VSS	3.05721e-16
C1241	mem[10]#42	VSS	1.47552e-15
C1242	n_2#47	VSS	9.80531e-16
C1243	addr[1]#99	VSS	5.06283e-16
C1244	q_control#58	VSS	1.24409e-15
C1245	addr[2]#38	VSS	5.31194e-16
C1246	mem[6]#47	VSS	8.73704e-16
C1247	scan_clk#223	VSS	1.2929e-15
C1248	n_8#57	VSS	9.01249e-16
C1249	mem[15]#43	VSS	2.73537e-16
C1250	addr[1]#98	VSS	6.19739e-16
C1251	scan_en#367	VSS	1.04709e-15
C1252	mem[9]#44	VSS	1.22296e-15
C1253	scan_clk#220	VSS	1.76199e-15
C1254	addr[0]#62	VSS	1.21736e-15
C1255	n_7#54	VSS	2.10076e-16
C1256	mem[7]#48	VSS	9.35276e-16
C1257	addr[1]#93	VSS	3.57018e-16
C1258	VDD#447	VSS	2.64092e-17
C1259	VDD#453	VSS	2.33165e-16
C1260	mem[4]#47	VSS	6.92918e-16
C1261	n_3#21	VSS	1.54388e-15
C1262	n_9#23	VSS	2.88497e-16
C1263	scan_en#365	VSS	1.55187e-15
C1264	n_7#53	VSS	6.38507e-16
C1265	addr[0]#64	VSS	8.0402e-16
C1266	mem[9]#42	VSS	2.73258e-16
C1267	mem[9]#46	VSS	5.41051e-16
C1268	addr[0]#60	VSS	2.76109e-16
C1269	scan_clk#212	VSS	1.1103e-15
C1270	mem[4]#46	VSS	5.55693e-16
C1271	addr[1]#91	VSS	5.49387e-16
C1272	n_2#42	VSS	7.72429e-16
C1273	scan_clk#218	VSS	5.71073e-16
C1274	n_1#19	VSS	1.13101e-15
C1275	n_4#20	VSS	1.30734e-15
C1276	mem[14]#48	VSS	8.88994e-16
C1277	mem[12]#49	VSS	7.65057e-16
C1278	scan_clk#219	VSS	1.63325e-15
C1279	mem[5]#47	VSS	7.14854e-16
C1280	n_8#52	VSS	8.75488e-16
C1281	mem[6]#45	VSS	1.23273e-15
C1282	mem[1]#49	VSS	5.41772e-16
C1283	mem[11]#44	VSS	8.2706e-16
C1284	scan_in[0]#14	VSS	1.85925e-16
C1285	mem[7]#47	VSS	7.21164e-16
C1286	addr[0]#56	VSS	2.66292e-16
C1287	addr[0]#58	VSS	3.26903e-16
C1288	scan_en#360	VSS	1.80585e-15
C1289	mem[6]#44	VSS	1.01532e-15
C1290	n_8#55	VSS	1.26705e-15
C1291	scan_en#363	VSS	1.33565e-15
C1292	mem[14]#45	VSS	7.61753e-16
C1293	q_control#55	VSS	2.29891e-15
C1294	x#25	VSS	1.64873e-15
C1295	mem[12]#46	VSS	9.02171e-16
C1296	mem[4]#44	VSS	9.04005e-16
C1297	mem[1]#46	VSS	1.13032e-15
C1298	mem[3]#47	VSS	7.07806e-16
C1299	scan_clk#209	VSS	1.51878e-16
C1300	mem[14]#44	VSS	5.93981e-16
C1301	mem[5]#44	VSS	9.96647e-16
C1302	mem[7]#45	VSS	5.04494e-16
C1303	scan_en#356	VSS	7.60618e-16
C1304	scan_clk#201	VSS	2.64619e-16
C1305	scan_clk#205	VSS	1.49268e-15
C1306	q_control#52	VSS	7.78476e-16
C1307	mem[6]#42	VSS	7.60935e-16
C1308	mem[3]#43	VSS	7.71166e-16
C1309	mem[12]#44	VSS	1.06865e-15
C1310	mem[1]#44	VSS	9.28846e-16
C1311	scan_out[0]#59	VSS	1.63325e-16
C1312	mem[14]#41	VSS	4.1942e-16
C1313	FE_OFN1_scan_out_0#72	VSS	2.13774e-15
C1314	scan_en#358	VSS	1.31084e-15
C1315	mem[4]#42	VSS	6.36098e-16
C1316	mux#10	VSS	2.13996e-16
C1317	n_15#30	VSS	1.70245e-15
C1318	n_7#50	VSS	1.45527e-16
C1319	n_14#21	VSS	3.92734e-16
C1320	n_5#17	VSS	5.29838e-16
C1321	mem[10]#40	VSS	4.1056e-16
C1322	x#22	VSS	1.04167e-15
C1323	mem[10]#38	VSS	3.91875e-16
C1324	mem[8]#41	VSS	8.77816e-16
C1325	n_12#41	VSS	3.49372e-16
C1326	mem[2]#40	VSS	1.47342e-16
C1327	addr[3]#21	VSS	1.99821e-15
C1328	n_13#21	VSS	3.137e-16
C1329	n_8#51	VSS	2.13294e-16
C1330	addr[2]#35	VSS	1.19109e-16
C1331	FE_OFN1_scan_out_0#71	VSS	5.59425e-16
C1332	n_15#27	VSS	2.50054e-15
C1333	mem[15]#41	VSS	1.23494e-16
C1334	FE_OFN1_scan_out_0#68	VSS	4.11222e-16
C1335	mem[8]#38	VSS	7.14574e-16
C1336	FE_OFN0_q#39	VSS	6.65813e-16
C1337	q#43	VSS	7.47127e-16
C1338	mem[8]#36	VSS	8.66114e-16
C1339	scan_clk#200	VSS	9.67342e-17
C1340	FE_OFN0_q#36	VSS	8.7693e-16
C1341	mem[13]#42	VSS	5.16811e-16
C1342	scan_clk#195	VSS	2.76446e-16
C1343	scan_clk#196	VSS	8.7538e-16
C1344	mem[0]#39	VSS	4.08072e-16
C1345	mem[10]#36	VSS	1.42923e-16
C1346	scan_en#347	VSS	1.19679e-15
C1347	mem[9]#40	VSS	1.38163e-16
C1348	mem[0]#37	VSS	3.70043e-16
C1349	scan_en#343	VSS	2.84853e-16
C1350	scan_en#345	VSS	2.22613e-16
C1351	mem[2]#37	VSS	2.33555e-16
C1352	n_11#30	VSS	4.97822e-16
C1353	scan_clk#190	VSS	3.43708e-16
C1354	mem[7]#42	VSS	1.93549e-16
C1355	n_14#17	VSS	4.18518e-16
C1356	mem[11]#41	VSS	5.64447e-16
C1357	scan_clk#193	VSS	1.25742e-15
C1358	mem[2]#35	VSS	4.03995e-16
C1359	scan_clk#189	VSS	3.70922e-16
C1360	n_3#18	VSS	4.36281e-16
C1361	n_7#48	VSS	8.32329e-17
C1362	n_9#21	VSS	2.43994e-16
C1363	scan_en#339	VSS	1.0708e-15
C1364	mem[15]#39	VSS	2.13959e-16
C1365	n_12#38	VSS	2.64773e-16
C1366	FE_OFN1_scan_out_0#66	VSS	2.28634e-16
C1367	scan_en#341	VSS	8.42406e-16
C1368	clk#11	VSS	2.19044e-16
C1369	x#20	VSS	2.15921e-16
C1370	mem[13]#40	VSS	2.92427e-16
C1371	mem[13]#38	VSS	6.01872e-16
C1372	n_6#17	VSS	7.4259e-16
C1373	mem[2]#33	VSS	3.68913e-16
C1374	mem[8]#33	VSS	2.89175e-16
C1375	scan_en#337	VSS	2.04602e-16
C1376	n_5#15	VSS	2.03254e-16
C1377	mem[0]#33	VSS	4.97252e-16
C1378	addr[2]#32	VSS	1.149e-16
C1379	n_8#48	VSS	1.73417e-16
C1380	q_control#50	VSS	4.32588e-16
C1381	n_15#25	VSS	8.89288e-16
C1382	n_11#24	VSS	7.30334e-16
C1383	n_11#27	VSS	3.54155e-16
C1384	n_13#17	VSS	2.88523e-16
C1385	mem[6]#39	VSS	3.63111e-16
C1386	n_12#36	VSS	2.53319e-16
C1387	mem[11]#38	VSS	4.86909e-16
C1388	n_1#18	VSS	3.09582e-16
C1389	n_4#18	VSS	3.38778e-16
C1390	addr[0]#55	VSS	4.13253e-16
C1391	mem[15]#35	VSS	7.80274e-16
C1392	n_10#22	VSS	1.0099e-15
C1393	n_12#33	VSS	9.99012e-16
C1394	n_2#40	VSS	1.46304e-16
C1395	q_control#47	VSS	2.03876e-16
C1396	n_6#15	VSS	8.13669e-16
C1397	addr[1]#89	VSS	3.25851e-16
C1398	mem[13]#36	VSS	3.8072e-16
C1399	addr[1]#86	VSS	2.48771e-16
C1400	addr[2]#29	VSS	2.02766e-16
C1401	scan_clk#186	VSS	8.43157e-16
C1402	mem[10]#33	VSS	8.66603e-16
C1403	mem[6]#37	VSS	7.03975e-17
C1404	n_8#46	VSS	4.27075e-17
C1405	addr[1]#83	VSS	9.07579e-16
C1406	n_10#19	VSS	8.40481e-16
C1407	scan_en#334	VSS	1.36717e-16
C1408	mem[9]#37	VSS	2.97907e-16
C1409	n_0#18	VSS	1.51542e-15
C1410	addr[0]#53	VSS	4.51928e-16
C1411	n_7#47	VSS	9.26964e-16
C1412	mem[7]#40	VSS	2.61182e-16
C1413	addr[0]#51	VSS	9.65043e-16
C1414	mem[15]#33	VSS	3.47694e-16
C1415	scan_clk#185	VSS	5.61288e-16
C1416	mem[14]#40	VSS	7.91794e-16
C1417	n_0#15	VSS	1.22502e-15
C1418	addr[1]#80	VSS	1.62904e-16
C1419	VDD#423	VSS	6.03985e-17
C1420	VDD#430	VSS	2.14067e-16
C1421	VDD#437	VSS	2.04057e-16
C1422	VDD#444	VSS	2.80219e-16
C1423	VDD#415	VSS	1.08263e-16
C1424	mem[4]#40	VSS	8.37453e-17
C1425	n_3#15	VSS	3.59497e-16
C1426	mem[5]#42	VSS	3.58041e-16
C1427	addr[1]#78	VSS	3.26673e-16
C1428	n_9#17	VSS	3.86409e-16
C1429	n_8#44	VSS	1.3722e-15
C1430	mem[11]#35	VSS	6.66924e-16
C1431	mem[13]#34	VSS	4.33653e-16
C1432	scan_en#332	VSS	1.50033e-16
C1433	n_7#45	VSS	3.1893e-16
C1434	addr[0]#49	VSS	1.22269e-16
C1435	scan_clk#180	VSS	3.6842e-16
C1436	mem[4]#38	VSS	1.53935e-16
C1437	addr[1]#76	VSS	1.83933e-16
C1438	n_2#37	VSS	3.29884e-16
C1439	mem[9]#33	VSS	7.32592e-16
C1440	scan_clk#182	VSS	3.1455e-16
C1441	n_1#15	VSS	4.55753e-16
C1442	n_4#15	VSS	8.36662e-16
C1443	mem[12]#42	VSS	4.68651e-16
C1444	mem[3]#40	VSS	9.04681e-16
C1445	n_7#42	VSS	9.56548e-16
C1446	mem[5]#39	VSS	3.46197e-16
C1447	n_8#42	VSS	2.18593e-16
C1448	scan_en#328	VSS	1.04917e-15
C1449	mem[6]#35	VSS	4.4805e-16
C1450	mem[1]#42	VSS	2.56769e-16
C1451	n_2#35	VSS	3.58564e-16
C1452	mem[11]#33	VSS	6.77768e-16
C1453	scan_en#330	VSS	4.90967e-16
C1454	scan_clk#178	VSS	2.19329e-16
C1455	scan_in[0]#11	VSS	2.03419e-16
C1456	mem[7]#38	VSS	3.72064e-16
C1457	n_2#33	VSS	3.6094e-16
C1458	addr[0]#47	VSS	3.31489e-16
C1459	scan_en#326	VSS	1.53284e-16
C1460	q_control#45	VSS	3.59372e-16
C1461	mem[14]#37	VSS	9.01601e-17
C1462	mem[12]#39	VSS	1.64261e-16
C1463	mem[3]#37	VSS	8.5743e-16
C1464	scan_clk#176	VSS	2.05333e-16
C1465	mem[14]#35	VSS	2.27687e-16
C1466	mem[5]#36	VSS	5.67509e-16
C1467	scan_clk#174	VSS	3.61991e-16
C1468	scan_en#323	VSS	2.07695e-16
C1469	mem[5]#34	VSS	1.1882e-16
C1470	mem[7]#33	VSS	1.86812e-16
C1471	scan_en#321	VSS	1.67259e-16
C1472	mem[12]#36	VSS	6.08705e-16
C1473	scan_clk#172	VSS	1.88995e-16
C1474	scan_clk#164	VSS	2.4146e-16
C1475	mem[6]#33	VSS	1.53702e-16
C1476	mem[1]#36	VSS	4.89934e-16
C1477	scan_clk#166	VSS	9.48421e-16
C1478	scan_clk#167	VSS	3.4823e-16
C1479	mem[3]#35	VSS	3.56432e-16
C1480	q_control#41	VSS	4.45896e-16
C1481	scan_clk#170	VSS	6.94207e-16
C1482	scan_out[0]#42	VSS	1.79219e-16
C1483	mem[14]#33	VSS	2.29426e-16
C1484	scan_en#319	VSS	7.94554e-16
C1485	scan_en#308	VSS	8.04287e-16
C1486	mem[12]#34	VSS	1.43719e-16
C1487	mem[4]#33	VSS	1.4898e-16
C1488	scan_en#311	VSS	6.52536e-16
C1489	scan_en#313	VSS	5.01296e-16
C1490	mem[1]#34	VSS	2.18445e-16
C1491	mem[3]#33	VSS	3.53133e-16
C1492	scan_en#316	VSS	2.49843e-16
C1493	FE_OFN1_scan_out_0#62	VSS	3.87287e-16
C1494	\mem_reg[10]/net367#7	VSS	2.57039e-16
C1495	FE_OFC0_q/n0#105	VSS	8.94038e-16
C1496	g493__2398/n0#27	VSS	2.30789e-16
C1497	g494__5107/n0#7	VSS	6.33632e-17
C1498	\mem_reg[10]/qbint#21	VSS	1.25056e-16
C1499	n_7#28	VSS	7.5245e-17
C1500	n_14#7	VSS	8.82968e-17
C1501	mem[10]#21	VSS	1.09217e-16
C1502	addr[3]#15	VSS	1.97953e-16
C1503	\mem_reg[0]/net367#7	VSS	2.60533e-16
C1504	\mem_reg[2]/net367#7	VSS	2.55541e-16
C1505	\mem_reg[10]/n30#7	VSS	2.70308e-16
C1506	mem[8]#21	VSS	1.22733e-16
C1507	g494__5107/S0b#7	VSS	1.31158e-16
C1508	x#14	VSS	1.37577e-16
C1509	addr[3]#14	VSS	1.85635e-16
C1510	\mem_reg[10]/qbint#20	VSS	1.56029e-16
C1511	\mem_reg[0]/qbint#21	VSS	1.25743e-16
C1512	\mem_reg[2]/qbint#21	VSS	1.25237e-16
C1513	n_13#7	VSS	6.07531e-17
C1514	g493__2398/S0b#8	VSS	2.62095e-16
C1515	n_8#28	VSS	9.81574e-17
C1516	g493__2398/S0b#7	VSS	1.92157e-16
C1517	addr[3]#13	VSS	9.13062e-17
C1518	FE_OFN1_scan_out_0#50	VSS	1.34425e-16
C1519	\mem_reg[0]/n30#7	VSS	2.79075e-16
C1520	\mem_reg[2]/n30#7	VSS	2.92499e-16
C1521	\mem_reg[10]/qbint#19	VSS	1.07047e-16
C1522	n_15#14	VSS	7.06015e-17
C1523	\mem_reg[15]/net367#7	VSS	2.63914e-16
C1524	mux_control_reg/net367#7	VSS	2.4154e-16
C1525	\mem_reg[0]/qbint#20	VSS	1.65089e-16
C1526	\mem_reg[2]/qbint#20	VSS	1.64862e-16
C1527	\mem_reg[10]/CKbb#16	VSS	6.88882e-17
C1528	\mem_reg[10]/CKb#21	VSS	1.86545e-16
C1529	FE_OFN1_scan_out_0#49	VSS	1.55893e-16
C1530	\mem_reg[8]/net367#7	VSS	2.47665e-16
C1531	\mem_reg[10]/CKbb#15	VSS	1.55232e-16
C1532	\mem_reg[15]/qbint#21	VSS	1.21193e-16
C1533	mux_control_reg/qbint#21	VSS	1.1532e-16
C1534	\mem_reg[10]/n20#7	VSS	6.95912e-17
C1535	\mem_reg[0]/qbint#19	VSS	1.13197e-16
C1536	\mem_reg[2]/qbint#19	VSS	1.16265e-16
C1537	\mem_reg[8]/qbint#21	VSS	1.25236e-16
C1538	\mem_reg[15]/n30#7	VSS	2.76604e-16
C1539	mux_control_reg/n30#7	VSS	2.63073e-16
C1540	q_reg_reg/net367#7	VSS	2.51545e-16
C1541	\mem_reg[0]/CKbb#16	VSS	6.9697e-17
C1542	\mem_reg[2]/CKbb#16	VSS	6.92727e-17
C1543	\mem_reg[0]/CKb#21	VSS	1.90215e-16
C1544	\mem_reg[2]/CKb#21	VSS	1.89858e-16
C1545	\mem_reg[10]/CKb#20	VSS	2.05759e-16
C1546	\mem_reg[8]/n30#7	VSS	2.82038e-16
C1547	\mem_reg[0]/CKbb#15	VSS	1.54784e-16
C1548	\mem_reg[2]/CKbb#15	VSS	1.5538e-16
C1549	\mem_reg[15]/qbint#20	VSS	1.54369e-16
C1550	mux_control_reg/qbint#20	VSS	1.46089e-16
C1551	q_reg_reg/qbint#21	VSS	1.23043e-16
C1552	\mem_reg[0]/n20#7	VSS	6.95973e-17
C1553	\mem_reg[2]/n20#7	VSS	6.88467e-17
C1554	\mem_reg[10]/mout#7	VSS	2.31363e-16
C1555	\mem_reg[8]/qbint#20	VSS	1.49771e-16
C1556	\mem_reg[10]/CKbb#14	VSS	1.33781e-16
C1557	q_reg_reg/n30#7	VSS	2.67405e-16
C1558	\mem_reg[15]/qbint#19	VSS	1.07138e-16
C1559	mux_control_reg/qbint#19	VSS	1.02195e-16
C1560	\mem_reg[10]/CKb#19	VSS	1.44114e-16
C1561	\mem_reg[10]/CKbb#13	VSS	1.9002e-16
C1562	\mem_reg[0]/CKb#20	VSS	2.14135e-16
C1563	\mem_reg[2]/CKb#20	VSS	2.13175e-16
C1564	\mem_reg[15]/CKbb#16	VSS	7.27523e-17
C1565	mux_control_reg/CKbb#16	VSS	6.98823e-17
C1566	q_reg_reg/qbint#20	VSS	1.57057e-16
C1567	\mem_reg[8]/qbint#19	VSS	1.06749e-16
C1568	\mem_reg[15]/CKb#21	VSS	1.92003e-16
C1569	mux_control_reg/CKb#21	VSS	1.81506e-16
C1570	\mem_reg[0]/mout#7	VSS	2.36575e-16
C1571	\mem_reg[2]/mout#7	VSS	2.31647e-16
C1572	\mem_reg[15]/CKbb#15	VSS	1.57258e-16
C1573	mux_control_reg/CKbb#15	VSS	1.51621e-16
C1574	\mem_reg[8]/CKbb#16	VSS	6.86804e-17
C1575	FE_OFN0_q#27	VSS	2.60021e-16
C1576	\mem_reg[8]/CKb#21	VSS	1.85935e-16
C1577	\mem_reg[0]/CKbb#14	VSS	1.34664e-16
C1578	\mem_reg[2]/CKbb#14	VSS	1.3463e-16
C1579	scan_clk#126	VSS	6.57679e-17
C1580	\mem_reg[15]/n20#7	VSS	6.82713e-17
C1581	mux_control_reg/n20#7	VSS	6.98134e-17
C1582	\mem_reg[8]/CKbb#15	VSS	1.53089e-16
C1583	q_reg_reg/qbint#19	VSS	1.13492e-16
C1584	\mem_reg[0]/CKb#19	VSS	1.43954e-16
C1585	\mem_reg[2]/CKb#19	VSS	1.41628e-16
C1586	\mem_reg[0]/CKbb#13	VSS	1.93199e-16
C1587	\mem_reg[2]/CKbb#13	VSS	1.93837e-16
C1588	mem[8]#20	VSS	9.41244e-17
C1589	\mem_reg[8]/n20#7	VSS	6.8439e-17
C1590	q_reg_reg/CKbb#16	VSS	6.9088e-17
C1591	q_reg_reg/CKb#21	VSS	1.85104e-16
C1592	\mem_reg[10]/SEb#8	VSS	2.15229e-16
C1593	\mem_reg[15]/CKb#20	VSS	2.15925e-16
C1594	mux_control_reg/CKb#20	VSS	2.10124e-16
C1595	q_reg_reg/CKbb#15	VSS	1.54904e-16
C1596	scan_en#252	VSS	1.01513e-16
C1597	\mem_reg[10]/SEb#7	VSS	1.42074e-16
C1598	scan_clk#124	VSS	6.86792e-17
C1599	scan_clk#125	VSS	6.07757e-17
C1600	\mem_reg[8]/CKb#20	VSS	2.12886e-16
C1601	\mem_reg[15]/mout#7	VSS	2.37374e-16
C1602	mux_control_reg/mout#7	VSS	2.33472e-16
C1603	q_reg_reg/n20#7	VSS	6.76545e-17
C1604	mem[10]#20	VSS	1.43796e-16
C1605	mem[13]#21	VSS	1.0945e-16
C1606	mem[0]#21	VSS	1.00615e-16
C1607	\mem_reg[15]/CKbb#14	VSS	1.35888e-16
C1608	mux_control_reg/CKbb#14	VSS	1.30377e-16
C1609	\mem_reg[8]/mout#7	VSS	2.3137e-16
C1610	\mem_reg[9]/net367#7	VSS	2.58381e-16
C1611	\mem_reg[15]/CKb#19	VSS	1.39201e-16
C1612	mux_control_reg/CKb#19	VSS	1.41439e-16
C1613	scan_en#251	VSS	9.38325e-17
C1614	\mem_reg[0]/SEb#8	VSS	2.13588e-16
C1615	\mem_reg[2]/SEb#8	VSS	2.15242e-16
C1616	\mem_reg[15]/CKbb#13	VSS	1.9075e-16
C1617	mux_control_reg/CKbb#13	VSS	1.93277e-16
C1618	\mem_reg[8]/CKbb#14	VSS	1.3293e-16
C1619	q_reg_reg/CKb#20	VSS	2.13893e-16
C1620	\mem_reg[9]/qbint#21	VSS	1.20769e-16
C1621	\mem_reg[8]/CKb#19	VSS	1.40191e-16
C1622	scan_en#249	VSS	9.92896e-17
C1623	scan_en#250	VSS	9.83677e-17
C1624	\mem_reg[0]/SEb#7	VSS	1.42423e-16
C1625	\mem_reg[2]/SEb#7	VSS	1.40445e-16
C1626	\mem_reg[8]/CKbb#13	VSS	1.87917e-16
C1627	q_reg_reg/mout#7	VSS	2.36324e-16
C1628	mem[0]#20	VSS	1.37287e-16
C1629	mem[2]#21	VSS	1.38429e-16
C1630	n_11#14	VSS	9.09754e-17
C1631	scan_clk#122	VSS	6.32572e-17
C1632	scan_clk#123	VSS	6.32788e-17
C1633	q_reg_reg/CKbb#14	VSS	1.34871e-16
C1634	\mem_reg[9]/n30#7	VSS	2.72929e-16
C1635	q_reg_reg/CKb#19	VSS	1.4581e-16
C1636	scan_en#247	VSS	9.73312e-17
C1637	scan_en#248	VSS	1.00328e-16
C1638	n_3#7	VSS	7.76139e-17
C1639	mem[7]#21	VSS	1.02557e-16
C1640	mem[11]#21	VSS	9.98323e-17
C1641	q_reg_reg/CKbb#13	VSS	1.93883e-16
C1642	scan_clk#121	VSS	6.70925e-17
C1643	\mem_reg[9]/qbint#20	VSS	1.55526e-16
C1644	n_9#7	VSS	9.08819e-17
C1645	\mem_reg[15]/SEb#8	VSS	2.13107e-16
C1646	mux_control_reg/SEb#8	VSS	2.14437e-16
C1647	mem[2]#20	VSS	1.02964e-16
C1648	scan_en#245	VSS	9.78115e-17
C1649	scan_en#246	VSS	1.04052e-16
C1650	\mem_reg[15]/SEb#7	VSS	1.41316e-16
C1651	n_12#21	VSS	7.9103e-17
C1652	mux_control_reg/SEb#7	VSS	1.39539e-16
C1653	\mem_reg[8]/SEb#8	VSS	2.11742e-16
C1654	clk#7	VSS	6.44445e-17
C1655	\mem_reg[13]/net367#7	VSS	2.47958e-16
C1656	\mem_reg[9]/qbint#19	VSS	1.00811e-16
C1657	scan_en#244	VSS	9.51343e-17
C1658	n_7#27	VSS	7.69915e-17
C1659	mem[15]#21	VSS	1.31347e-16
C1660	FE_OFN1_scan_out_0#48	VSS	1.46466e-16
C1661	\mem_reg[8]/SEb#7	VSS	1.38252e-16
C1662	x#13	VSS	1.016e-16
C1663	\mem_reg[9]/CKbb#16	VSS	6.91069e-17
C1664	\mem_reg[13]/qbint#21	VSS	1.20605e-16
C1665	\mem_reg[9]/CKb#21	VSS	1.82713e-16
C1666	mem[8]#19	VSS	1.36923e-16
C1667	mem[2]#19	VSS	1.14412e-16
C1668	scan_en#242	VSS	9.66155e-17
C1669	scan_en#243	VSS	9.65275e-17
C1670	q_reg_reg/SEb#8	VSS	2.18832e-16
C1671	\mem_reg[9]/CKbb#15	VSS	1.62758e-16
C1672	q_control#28	VSS	1.01212e-16
C1673	mem[0]#19	VSS	1.1807e-16
C1674	scan_en#241	VSS	9.74732e-17
C1675	addr[2]#20	VSS	8.16887e-17
C1676	q_reg_reg/SEb#7	VSS	1.40404e-16
C1677	\mem_reg[13]/n30#7	VSS	2.75492e-16
C1678	\mem_reg[9]/n20#7	VSS	6.94698e-17
C1679	n_15#13	VSS	1.33591e-16
C1680	n_8#27	VSS	9.79631e-17
C1681	\mem_reg[6]/net367#7	VSS	2.49313e-16
C1682	\mem_reg[11]/net367#7	VSS	2.48647e-16
C1683	\mem_reg[13]/qbint#20	VSS	1.51138e-16
C1684	n_11#13	VSS	7.52107e-17
C1685	q_control#27	VSS	9.82484e-17
C1686	\mem_reg[9]/CKb#20	VSS	2.0727e-16
C1687	\mem_reg[6]/qbint#21	VSS	1.17693e-16
C1688	\mem_reg[11]/qbint#21	VSS	1.14988e-16
C1689	n_12#20	VSS	1.03448e-16
C1690	n_4#7	VSS	7.15005e-17
C1691	\mem_reg[9]/mout#7	VSS	2.33028e-16
C1692	\mem_reg[13]/qbint#19	VSS	1.0087e-16
C1693	n_10#7	VSS	8.92915e-17
C1694	g519__1705/n0#7	VSS	6.35014e-17
C1695	\mem_reg[9]/CKbb#14	VSS	1.37862e-16
C1696	\mem_reg[6]/n30#7	VSS	2.76708e-16
C1697	\mem_reg[11]/n30#7	VSS	2.77628e-16
C1698	q_control_reg/net367#7	VSS	2.45786e-16
C1699	\mem_reg[13]/CKbb#16	VSS	6.85127e-17
C1700	\mem_reg[9]/CKb#19	VSS	1.38545e-16
C1701	\mem_reg[13]/CKb#21	VSS	1.98063e-16
C1702	n_12#19	VSS	7.62726e-17
C1703	\mem_reg[9]/CKbb#13	VSS	1.89984e-16
C1704	mem[15]#20	VSS	8.66897e-17
C1705	\mem_reg[13]/CKbb#15	VSS	1.55018e-16
C1706	\mem_reg[6]/qbint#20	VSS	1.52192e-16
C1707	\mem_reg[11]/qbint#20	VSS	1.52969e-16
C1708	q_control_reg/qbint#21	VSS	1.17276e-16
C1709	addr[1]#58	VSS	1.92655e-16
C1710	\mem_reg[13]/n20#7	VSS	6.77829e-17
C1711	g519__1705/S0b#7	VSS	1.24332e-16
C1712	addr[1]#56	VSS	1.77095e-16
C1713	scan_clk#120	VSS	6.29528e-17
C1714	q_control_reg/n30#7	VSS	2.69162e-16
C1715	\mem_reg[6]/qbint#19	VSS	1.04648e-16
C1716	\mem_reg[11]/qbint#19	VSS	1.02957e-16
C1717	mem[13]#20	VSS	6.1018e-17
C1718	addr[2]#19	VSS	9.18051e-17
C1719	mem[10]#19	VSS	9.76659e-17
C1720	\mem_reg[13]/CKb#20	VSS	2.42747e-16
C1721	\mem_reg[6]/CKbb#16	VSS	6.86811e-17
C1722	\mem_reg[11]/CKbb#16	VSS	6.78865e-17
C1723	q_control_reg/qbint#20	VSS	1.5138e-16
C1724	\mem_reg[6]/CKb#21	VSS	1.89077e-16
C1725	\mem_reg[11]/CKb#21	VSS	1.85829e-16
C1726	addr[1]#55	VSS	8.26369e-17
C1727	\mem_reg[9]/SEb#8	VSS	2.2757e-16
C1728	\mem_reg[13]/mout#7	VSS	2.36007e-16
C1729	\mem_reg[6]/CKbb#15	VSS	1.54161e-16
C1730	\mem_reg[11]/CKbb#15	VSS	1.52613e-16
C1731	scan_en#240	VSS	1.03115e-16
C1732	mem[6]#21	VSS	6.78053e-17
C1733	\mem_reg[9]/SEb#7	VSS	1.3991e-16
C1734	\mem_reg[13]/CKbb#14	VSS	1.3218e-16
C1735	\mem_reg[6]/n20#7	VSS	6.84061e-17
C1736	\mem_reg[11]/n20#7	VSS	6.74861e-17
C1737	q_control_reg/qbint#19	VSS	1.00686e-16
C1738	\mem_reg[13]/CKb#19	VSS	1.45396e-16
C1739	mem[9]#21	VSS	1.43219e-16
C1740	\mem_reg[13]/CKbb#13	VSS	2.03139e-16
C1741	n_7#26	VSS	6.62989e-17
C1742	q_control_reg/CKbb#16	VSS	7.03614e-17
C1743	q_control_reg/CKb#21	VSS	1.85976e-16
C1744	g520__5122/n0#7	VSS	6.37859e-17
C1745	scan_en#239	VSS	9.53539e-17
C1746	\mem_reg[6]/CKb#20	VSS	2.19038e-16
C1747	\mem_reg[11]/CKb#20	VSS	2.12429e-16
C1748	q_control_reg/CKbb#15	VSS	1.54377e-16
C1749	addr[0]#33	VSS	9.68878e-17
C1750	mem[7]#20	VSS	9.01052e-17
C1751	scan_clk#119	VSS	6.41636e-17
C1752	\mem_reg[6]/mout#7	VSS	2.34242e-16
C1753	\mem_reg[11]/mout#7	VSS	2.29122e-16
C1754	q_control_reg/n20#7	VSS	7.01849e-17
C1755	n_0#7	VSS	9.03503e-17
C1756	addr[1]#53	VSS	1.95998e-16
C1757	mem[15]#19	VSS	1.07762e-16
C1758	\mem_reg[6]/CKbb#14	VSS	1.31777e-16
C1759	\mem_reg[11]/CKbb#14	VSS	1.29496e-16
C1760	\mem_reg[6]/CKb#19	VSS	1.45063e-16
C1761	\mem_reg[11]/CKb#19	VSS	1.36193e-16
C1762	mem[14]#21	VSS	6.96523e-17
C1763	mem[4]#21	VSS	8.67796e-17
C1764	addr[1]#52	VSS	7.27958e-17
C1765	g520__5122/S0b#7	VSS	1.28739e-16
C1766	addr[1]#51	VSS	1.71698e-16
C1767	\mem_reg[13]/SEb#8	VSS	2.11841e-16
C1768	\mem_reg[6]/CKbb#13	VSS	1.83992e-16
C1769	\mem_reg[11]/CKbb#13	VSS	1.86515e-16
C1770	q_control_reg/CKb#20	VSS	2.07788e-16
C1771	scan_en#238	VSS	1.04934e-16
C1772	mem[5]#21	VSS	6.09901e-17
C1773	mem[11]#20	VSS	1.09621e-16
C1774	\mem_reg[13]/SEb#7	VSS	1.39182e-16
C1775	n_8#26	VSS	8.48595e-17
C1776	n_7#25	VSS	6.61629e-17
C1777	q_control_reg/mout#7	VSS	2.30197e-16
C1778	mem[13]#19	VSS	1.4277e-16
C1779	mem[9]#20	VSS	1.16416e-16
C1780	addr[1]#50	VSS	8.54644e-17
C1781	scan_clk#117	VSS	6.62798e-17
C1782	scan_clk#118	VSS	6.40034e-17
C1783	q_control_reg/CKbb#14	VSS	1.30083e-16
C1784	q_control_reg/CKb#19	VSS	1.32278e-16
C1785	addr[0]#32	VSS	9.94321e-17
C1786	scan_en#237	VSS	9.36632e-17
C1787	n_2#21	VSS	9.5959e-17
C1788	mem[4]#20	VSS	1.05528e-16
C1789	mem[9]#19	VSS	1.04026e-16
C1790	q_control_reg/CKbb#13	VSS	1.92316e-16
C1791	addr[1]#49	VSS	7.31423e-17
C1792	n_1#7	VSS	8.67482e-17
C1793	\mem_reg[6]/SEb#8	VSS	2.22652e-16
C1794	\mem_reg[11]/SEb#8	VSS	2.12561e-16
C1795	\mem_reg[5]/net367#7	VSS	2.65599e-16
C1796	scan_en#235	VSS	9.5839e-17
C1797	scan_en#236	VSS	1.04183e-16
C1798	mem[12]#21	VSS	8.63521e-17
C1799	mem[3]#21	VSS	1.05703e-16
C1800	\mem_reg[6]/SEb#7	VSS	1.41971e-16
C1801	\mem_reg[11]/SEb#7	VSS	1.43716e-16
C1802	scan_clk#116	VSS	6.97036e-17
C1803	\mem_reg[7]/net367#7	VSS	2.53875e-16
C1804	\mem_reg[5]/qbint#21	VSS	1.21704e-16
C1805	mem[6]#20	VSS	1.36103e-16
C1806	mem[1]#21	VSS	1.20203e-16
C1807	mem[11]#19	VSS	1.43908e-16
C1808	n_8#25	VSS	8.61521e-17
C1809	scan_in[0]#7	VSS	1.06135e-16
C1810	\mem_reg[7]/qbint#21	VSS	1.22276e-16
C1811	scan_en#233	VSS	9.41459e-17
C1812	n_2#19	VSS	9.96801e-17
C1813	scan_en#234	VSS	9.5739e-17
C1814	q_control_reg/SEb#8	VSS	2.24982e-16
C1815	\mem_reg[5]/n30#7	VSS	2.82276e-16
C1816	scan_en#232	VSS	1.04004e-16
C1817	q_control_reg/SEb#7	VSS	1.46548e-16
C1818	\mem_reg[7]/n30#7	VSS	2.76952e-16
C1819	\mem_reg[14]/net367#7	VSS	2.51465e-16
C1820	\mem_reg[5]/qbint#20	VSS	1.58806e-16
C1821	q_control#26	VSS	1.40478e-16
C1822	\mem_reg[12]/net367#7	VSS	2.59661e-16
C1823	\mem_reg[4]/net367#7	VSS	2.65551e-16
C1824	\mem_reg[1]/net367#7	VSS	2.62233e-16
C1825	\mem_reg[3]/net367#7	VSS	2.39772e-16
C1826	\mem_reg[7]/qbint#20	VSS	1.53734e-16
C1827	\mem_reg[14]/qbint#21	VSS	1.19852e-16
C1828	scan_en#231	VSS	9.80785e-17
C1829	\mem_reg[5]/qbint#19	VSS	1.0287e-16
C1830	\mem_reg[12]/qbint#21	VSS	1.19456e-16
C1831	\mem_reg[4]/qbint#21	VSS	1.21954e-16
C1832	\mem_reg[1]/qbint#21	VSS	1.20589e-16
C1833	\mem_reg[3]/qbint#21	VSS	1.13279e-16
C1834	\mem_reg[14]/n30#7	VSS	2.71702e-16
C1835	\mem_reg[5]/CKbb#16	VSS	7.0595e-17
C1836	\mem_reg[7]/qbint#19	VSS	1.01418e-16
C1837	\mem_reg[5]/CKb#21	VSS	1.86617e-16
C1838	\mem_reg[12]/n30#7	VSS	2.80243e-16
C1839	\mem_reg[4]/n30#7	VSS	2.84821e-16
C1840	\mem_reg[1]/n30#7	VSS	2.85892e-16
C1841	\mem_reg[3]/n30#7	VSS	2.7077e-16
C1842	\mem_reg[5]/CKbb#15	VSS	1.50641e-16
C1843	\mem_reg[7]/CKbb#16	VSS	7.2203e-17
C1844	\mem_reg[14]/qbint#20	VSS	1.55156e-16
C1845	FE_OFC1_scan_out_0/n0#105	VSS	8.95322e-16
C1846	\mem_reg[7]/CKb#21	VSS	1.83624e-16
C1847	\mem_reg[5]/n20#7	VSS	7.03251e-17
C1848	\mem_reg[7]/CKbb#15	VSS	1.49139e-16
C1849	\mem_reg[12]/qbint#20	VSS	1.50322e-16
C1850	\mem_reg[4]/qbint#20	VSS	1.5116e-16
C1851	\mem_reg[1]/qbint#20	VSS	1.51721e-16
C1852	\mem_reg[3]/qbint#20	VSS	1.46116e-16
C1853	\mem_reg[7]/n20#7	VSS	7.20191e-17
C1854	\mem_reg[14]/qbint#19	VSS	1.06532e-16
C1855	\mem_reg[5]/CKb#20	VSS	2.06478e-16
C1856	\mem_reg[14]/CKbb#16	VSS	7.36334e-17
C1857	\mem_reg[12]/qbint#19	VSS	1.01147e-16
C1858	\mem_reg[4]/qbint#19	VSS	9.50574e-17
C1859	\mem_reg[1]/qbint#19	VSS	1.03781e-16
C1860	\mem_reg[3]/qbint#19	VSS	1.03408e-16
C1861	\mem_reg[14]/CKb#21	VSS	1.92185e-16
C1862	\mem_reg[7]/CKb#20	VSS	2.08489e-16
C1863	\mem_reg[5]/mout#7	VSS	2.32725e-16
C1864	\mem_reg[14]/CKbb#15	VSS	1.53381e-16
C1865	\mem_reg[12]/CKbb#16	VSS	6.67424e-17
C1866	\mem_reg[4]/CKbb#16	VSS	6.97099e-17
C1867	\mem_reg[1]/CKbb#16	VSS	6.96282e-17
C1868	\mem_reg[3]/CKbb#16	VSS	6.83896e-17
C1869	\mem_reg[12]/CKb#21	VSS	1.91709e-16
C1870	\mem_reg[4]/CKb#21	VSS	1.89392e-16
C1871	\mem_reg[1]/CKb#21	VSS	1.89317e-16
C1872	\mem_reg[3]/CKb#21	VSS	1.84029e-16
C1873	\mem_reg[5]/CKbb#14	VSS	1.31991e-16
C1874	\mem_reg[7]/mout#7	VSS	2.35836e-16
C1875	\mem_reg[14]/n20#7	VSS	7.3908e-17
C1876	\mem_reg[12]/CKbb#15	VSS	1.56509e-16
C1877	\mem_reg[4]/CKbb#15	VSS	1.55404e-16
C1878	\mem_reg[1]/CKbb#15	VSS	1.53411e-16
C1879	\mem_reg[3]/CKbb#15	VSS	1.53642e-16
C1880	\mem_reg[5]/CKb#19	VSS	1.36435e-16
C1881	\mem_reg[5]/CKbb#13	VSS	1.94112e-16
C1882	\mem_reg[7]/CKbb#14	VSS	1.31502e-16
C1883	\mem_reg[12]/n20#7	VSS	6.65983e-17
C1884	\mem_reg[4]/n20#7	VSS	6.95647e-17
C1885	\mem_reg[1]/n20#7	VSS	6.95667e-17
C1886	\mem_reg[3]/n20#7	VSS	6.81588e-17
C1887	\mem_reg[7]/CKb#19	VSS	1.41954e-16
C1888	\mem_reg[7]/CKbb#13	VSS	1.96855e-16
C1889	\mem_reg[14]/CKb#20	VSS	2.26069e-16
C1890	scan_clk#115	VSS	6.98742e-17
C1891	\mem_reg[12]/CKb#20	VSS	2.20213e-16
C1892	\mem_reg[4]/CKb#20	VSS	2.13864e-16
C1893	\mem_reg[1]/CKb#20	VSS	2.13944e-16
C1894	\mem_reg[3]/CKb#20	VSS	2.07964e-16
C1895	\mem_reg[14]/mout#7	VSS	2.35356e-16
C1896	mem[14]#20	VSS	1.06523e-16
C1897	scan_clk#114	VSS	6.58725e-17
C1898	\mem_reg[14]/CKbb#14	VSS	1.30252e-16
C1899	\mem_reg[12]/mout#7	VSS	2.36541e-16
C1900	\mem_reg[4]/mout#7	VSS	2.36541e-16
C1901	\mem_reg[1]/mout#7	VSS	2.36541e-16
C1902	\mem_reg[3]/mout#7	VSS	2.33314e-16
C1903	\mem_reg[14]/CKb#19	VSS	1.50951e-16
C1904	\mem_reg[5]/SEb#8	VSS	2.15102e-16
C1905	mem[5]#20	VSS	9.57565e-17
C1906	\mem_reg[14]/CKbb#13	VSS	1.92566e-16
C1907	\mem_reg[12]/CKbb#14	VSS	1.33446e-16
C1908	\mem_reg[4]/CKbb#14	VSS	1.35395e-16
C1909	\mem_reg[1]/CKbb#14	VSS	1.32252e-16
C1910	\mem_reg[3]/CKbb#14	VSS	1.30007e-16
C1911	\mem_reg[12]/CKb#19	VSS	1.50292e-16
C1912	\mem_reg[4]/CKb#19	VSS	1.48322e-16
C1913	\mem_reg[1]/CKb#19	VSS	1.44715e-16
C1914	\mem_reg[3]/CKb#19	VSS	1.41548e-16
C1915	scan_en#230	VSS	9.27977e-17
C1916	\mem_reg[5]/SEb#7	VSS	1.44653e-16
C1917	\mem_reg[7]/SEb#8	VSS	2.23882e-16
C1918	\mem_reg[12]/CKbb#13	VSS	1.92854e-16
C1919	\mem_reg[4]/CKbb#13	VSS	1.88477e-16
C1920	\mem_reg[1]/CKbb#13	VSS	1.88656e-16
C1921	\mem_reg[3]/CKbb#13	VSS	1.93076e-16
C1922	scan_en#229	VSS	9.86438e-17
C1923	mem[5]#19	VSS	1.26835e-16
C1924	\mem_reg[7]/SEb#7	VSS	1.41658e-16
C1925	scan_clk#113	VSS	6.55948e-17
C1926	mem[7]#19	VSS	1.37259e-16
C1927	scan_en#228	VSS	8.77861e-17
C1928	mem[12]#20	VSS	9.80972e-17
C1929	scan_clk#109	VSS	6.86247e-17
C1930	scan_clk#110	VSS	6.60196e-17
C1931	scan_clk#111	VSS	6.13793e-17
C1932	scan_clk#112	VSS	6.85859e-17
C1933	scan_en#227	VSS	9.43518e-17
C1934	\mem_reg[14]/SEb#8	VSS	2.27207e-16
C1935	mem[6]#19	VSS	1.07619e-16
C1936	mem[1]#20	VSS	9.55562e-17
C1937	mem[3]#20	VSS	1.0856e-16
C1938	q_control#25	VSS	9.46186e-17
C1939	scan_en#226	VSS	9.60556e-17
C1940	\mem_reg[14]/SEb#7	VSS	1.47617e-16
C1941	\mem_reg[12]/SEb#8	VSS	2.21558e-16
C1942	\mem_reg[4]/SEb#8	VSS	2.1742e-16
C1943	\mem_reg[1]/SEb#8	VSS	2.21219e-16
C1944	\mem_reg[3]/SEb#8	VSS	2.14357e-16
C1945	FE_OFN1_scan_out_0#45	VSS	2.68421e-16
C1946	scan_en#222	VSS	9.5865e-17
C1947	scan_en#223	VSS	9.83932e-17
C1948	scan_en#224	VSS	9.97339e-17
C1949	scan_en#225	VSS	9.9948e-17
C1950	mem[14]#19	VSS	1.32043e-16
C1951	\mem_reg[12]/SEb#7	VSS	1.47841e-16
C1952	\mem_reg[4]/SEb#7	VSS	1.47175e-16
C1953	\mem_reg[1]/SEb#7	VSS	1.40764e-16
C1954	\mem_reg[3]/SEb#7	VSS	1.41936e-16
C1955	mem[12]#19	VSS	1.2874e-16
C1956	mem[4]#19	VSS	1.33046e-16
C1957	mem[1]#19	VSS	1.26445e-16
C1958	mem[3]#19	VSS	1.41447e-16
C1959	scan_en#221	VSS	9.13964e-17
C1960	scan_en#217	VSS	8.93463e-17
C1961	scan_en#218	VSS	9.3437e-17
C1962	scan_en#219	VSS	8.98529e-17
C1963	scan_en#220	VSS	8.7603e-17
C1964	VDD#270	VSS	7.73644e-16
C1965	VDD#174	VSS	5.87717e-16
C1966	VDD#3	VSS	4.99673e-16
C1967	VDD#106	VSS	4.16063e-16
C1968	UNCONNECTED15#2	VSS	4.40194e-16
C1969	UNCONNECTED15#3	VSS	2.78668e-16
C1970	q#30	VSS	2.57626e-16
C1971	q#32	VSS	2.97388e-16
C1972	n_15#19	VSS	2.34926e-16
C1973	n_15#22	VSS	7.07025e-17
C1974	mux#5	VSS	2.87414e-16
C1975	mux#8	VSS	3.14743e-16
C1976	n_7#39	VSS	2.4876e-16
C1977	g494__5107/n0#13	VSS	2.06642e-16
C1978	g510__3680/net89#6	VSS	3.16856e-16
C1979	mem[10]#29	VSS	4.67141e-16
C1980	mem[10]#30	VSS	2.64973e-16
C1981	n_14#14	VSS	1.77298e-16
C1982	x#17	VSS	2.49579e-16
C1983	q#26	VSS	2.07232e-16
C1984	q#28	VSS	2.85223e-16
C1985	mux#1	VSS	1.24722e-16
C1986	mux#4	VSS	3.39905e-16
C1987	n_5#12	VSS	1.4638e-16
C1988	VDD#356	VSS	2.02008e-16
C1989	mem[10]#26	VSS	2.40823e-16
C1990	UNCONNECTED9#2	VSS	3.36183e-16
C1991	UNCONNECTED9#3	VSS	2.00949e-16
C1992	UNCONNECTED1#2	VSS	2.78566e-16
C1993	UNCONNECTED1#3	VSS	2.46632e-16
C1994	\mem_reg[10]/qbint#23	VSS	5.00693e-16
C1995	\mem_reg[10]/qbint#28	VSS	2.59644e-16
C1996	n_5#10	VSS	4.11307e-16
C1997	g510__3680/net89#4	VSS	1.94027e-16
C1998	g494__5107/n0#8	VSS	1.1891e-16
C1999	g494__5107/n0#10	VSS	2.18978e-16
C2000	g493__2398/n0#35	VSS	7.29661e-17
C2001	mem[8]#30	VSS	2.09638e-16
C2002	q#22	VSS	2.45081e-16
C2003	q#24	VSS	2.67137e-16
C2004	n_12#28	VSS	2.5205e-16
C2005	n_12#31	VSS	3.75993e-16
C2006	mem[0]#29	VSS	4.50844e-16
C2007	mem[0]#30	VSS	2.99447e-16
C2008	mem[2]#29	VSS	3.56948e-16
C2009	mem[2]#30	VSS	2.18194e-16
C2010	\mem_reg[10]/net367#9	VSS	3.02149e-16
C2011	\mem_reg[10]/net367#13	VSS	3.93474e-16
C2012	\mem_reg[10]/net367#10	VSS	1.71433e-16
C2013	addr[3]#17	VSS	2.96675e-16
C2014	n_13#14	VSS	1.1347e-16
C2015	n_8#39	VSS	1.75162e-16
C2016	addr[2]#27	VSS	2.4811e-16
C2017	g510__3680/net89#2	VSS	3.00143e-16
C2018	g493__2398/n0#30	VSS	2.44333e-16
C2019	g493__2398/n0#33	VSS	2.18551e-16
C2020	FE_OFN1_scan_out_0#59	VSS	3.63476e-16
C2021	\mem_reg[0]/qbint#23	VSS	4.11061e-16
C2022	\mem_reg[0]/qbint#28	VSS	2.47518e-16
C2023	\mem_reg[2]/qbint#28	VSS	2.54708e-16
C2024	\mem_reg[2]/qbint#22	VSS	4.17504e-16
C2025	q#18	VSS	2.31556e-16
C2026	q#20	VSS	3.09023e-16
C2027	g494__5107/S0b#8	VSS	1.80753e-16
C2028	g494__5107/S0b#13	VSS	2.78443e-16
C2029	g494__5107/S0b#10	VSS	1.73908e-16
C2030	VDD#185	VSS	1.36881e-15
C2031	VDD#14	VSS	1.1286e-15
C2032	\mem_reg[10]/CKbb#23	VSS	2.93801e-16
C2033	\mem_reg[10]/CKbb#25	VSS	3.17416e-16
C2034	n_15#17	VSS	2.12489e-16
C2035	UNCONNECTED7#2	VSS	3.66904e-16
C2036	UNCONNECTED7#3	VSS	3.30832e-16
C2037	UNCONNECTED2#2	VSS	4.01088e-16
C2038	UNCONNECTED2#3	VSS	2.45211e-16
C2039	\mem_reg[0]/net367#9	VSS	2.91356e-16
C2040	\mem_reg[0]/net367#13	VSS	3.74518e-16
C2041	\mem_reg[0]/net367#10	VSS	1.38264e-16
C2042	\mem_reg[2]/net367#9	VSS	2.46211e-16
C2043	\mem_reg[2]/net367#13	VSS	3.86285e-16
C2044	\mem_reg[2]/net367#10	VSS	1.61886e-16
C2045	\mem_reg[10]/n30#9	VSS	2.51464e-16
C2046	\mem_reg[10]/n30#13	VSS	4.82554e-16
C2047	\mem_reg[10]/n30#10	VSS	2.30981e-16
C2048	UNCONNECTED14#2	VSS	4.4589e-16
C2049	UNCONNECTED14#3	VSS	2.41412e-16
C2050	q#14	VSS	2.16537e-16
C2051	q#16	VSS	2.65944e-16
C2052	g493__2398/S0b#10	VSS	2.31676e-16
C2053	g493__2398/S0b#15	VSS	3.95557e-16
C2054	g493__2398/S0b#11	VSS	1.08585e-16
C2055	\mem_reg[10]/mout#9	VSS	2.69178e-16
C2056	\mem_reg[10]/mout#12	VSS	4.18511e-16
C2057	\mem_reg[10]/mout#10	VSS	2.03717e-16
C2058	mem[15]#29	VSS	3.63113e-16
C2059	mem[15]#30	VSS	3.62943e-16
C2060	FE_OFN1_scan_out_0#56	VSS	4.38296e-16
C2061	FE_OFN1_scan_out_0#57	VSS	3.20496e-16
C2062	\mem_reg[10]/n20#13	VSS	5.08438e-16
C2063	mem[8]#27	VSS	4.40105e-16
C2064	mem[8]#28	VSS	3.00927e-16
C2065	VDD#363	VSS	6.99646e-16
C2066	VDD#111	VSS	3.00504e-16
C2067	\mem_reg[0]/CKbb#23	VSS	2.65001e-16
C2068	\mem_reg[0]/CKbb#25	VSS	2.7857e-16
C2069	\mem_reg[2]/CKbb#25	VSS	2.78895e-16
C2070	\mem_reg[2]/CKbb#23	VSS	2.77369e-16
C2071	q#10	VSS	1.87756e-16
C2072	q#12	VSS	2.69422e-16
C2073	\mem_reg[15]/qbint#28	VSS	2.72649e-16
C2074	\mem_reg[15]/qbint#22	VSS	4.11556e-16
C2075	mux_control_reg/qbint#23	VSS	4.26071e-16
C2076	mux_control_reg/qbint#28	VSS	2.60192e-16
C2077	UNCONNECTED#2	VSS	3.59909e-16
C2078	UNCONNECTED#3	VSS	3.4575e-16
C2079	\mem_reg[0]/n30#9	VSS	2.42859e-16
C2080	\mem_reg[0]/n30#13	VSS	4.8776e-16
C2081	\mem_reg[0]/n30#10	VSS	2.22575e-16
C2082	\mem_reg[2]/n30#9	VSS	3.07991e-16
C2083	\mem_reg[2]/n30#13	VSS	5.14689e-16
C2084	\mem_reg[2]/n30#10	VSS	1.3422e-16
C2085	\mem_reg[10]/CKbb#18	VSS	1.94932e-16
C2086	\mem_reg[10]/CKbb#21	VSS	2.93946e-16
C2087	\mem_reg[10]/CKbb#27	VSS	1.95701e-16
C2088	\mem_reg[10]/CKbb#19	VSS	7.73876e-17
C2089	\mem_reg[8]/qbint#23	VSS	4.5981e-16
C2090	\mem_reg[8]/qbint#28	VSS	2.63054e-16
C2091	\mem_reg[0]/mout#9	VSS	2.70681e-16
C2092	\mem_reg[0]/mout#12	VSS	3.99027e-16
C2093	\mem_reg[0]/mout#10	VSS	1.95503e-16
C2094	\mem_reg[2]/mout#9	VSS	2.80867e-16
C2095	\mem_reg[2]/mout#12	VSS	4.4591e-16
C2096	\mem_reg[2]/mout#10	VSS	1.60616e-16
C2097	q#6	VSS	1.84547e-16
C2098	q#8	VSS	3.21929e-16
C2099	\mem_reg[15]/net367#9	VSS	2.58898e-16
C2100	\mem_reg[15]/net367#13	VSS	3.85625e-16
C2101	\mem_reg[15]/net367#10	VSS	1.78821e-16
C2102	mux_control_reg/net367#9	VSS	3.42374e-16
C2103	mux_control_reg/net367#13	VSS	4.07295e-16
C2104	mux_control_reg/net367#10	VSS	1.84913e-16
C2105	FE_OFN0_q#33	VSS	3.75457e-16
C2106	FE_OFN0_q#34	VSS	4.26872e-16
C2107	\mem_reg[0]/n20#13	VSS	4.96959e-16
C2108	\mem_reg[2]/n20#13	VSS	4.98301e-16
C2109	\mem_reg[8]/net367#9	VSS	2.97235e-16
C2110	\mem_reg[8]/net367#13	VSS	3.72245e-16
C2111	\mem_reg[8]/net367#10	VSS	1.48564e-16
C2112	q_reg_reg/qbint#28	VSS	2.82103e-16
C2113	q_reg_reg/qbint#22	VSS	4.57233e-16
C2114	\mem_reg[10]/n20#9	VSS	6.87443e-16
C2115	\mem_reg[10]/n20#10	VSS	2.94263e-16
C2116	q#2	VSS	2.30148e-16
C2117	q#4	VSS	1.57886e-16
C2118	\mem_reg[0]/CKbb#18	VSS	1.90609e-16
C2119	\mem_reg[0]/CKbb#21	VSS	2.93062e-16
C2120	\mem_reg[0]/CKbb#27	VSS	1.88637e-16
C2121	\mem_reg[0]/CKbb#19	VSS	6.72888e-17
C2122	\mem_reg[2]/CKbb#18	VSS	1.23938e-16
C2123	\mem_reg[2]/CKbb#27	VSS	1.92615e-16
C2124	\mem_reg[2]/CKbb#21	VSS	2.98944e-16
C2125	\mem_reg[2]/CKbb#19	VSS	1.04762e-16
C2126	\mem_reg[15]/CKbb#25	VSS	2.96127e-16
C2127	\mem_reg[15]/CKbb#23	VSS	2.75599e-16
C2128	mux_control_reg/CKbb#23	VSS	2.65331e-16
C2129	mux_control_reg/CKbb#25	VSS	2.87399e-16
C2130	\mem_reg[10]/Db#6	VSS	6.67328e-16
C2131	\mem_reg[10]/Db#7	VSS	3.56524e-16
C2132	\mem_reg[15]/n30#9	VSS	3.21841e-16
C2133	\mem_reg[15]/n30#13	VSS	5.34854e-16
C2134	\mem_reg[15]/n30#10	VSS	1.56464e-16
C2135	mux_control_reg/n30#9	VSS	2.74955e-16
C2136	mux_control_reg/n30#13	VSS	5.2866e-16
C2137	mux_control_reg/n30#10	VSS	2.77856e-16
C2138	q_reg_reg/net367#9	VSS	3.00623e-16
C2139	q_reg_reg/net367#13	VSS	4.31717e-16
C2140	q_reg_reg/net367#10	VSS	1.83057e-16
C2141	VDD#292	VSS	1.224e-15
C2142	\mem_reg[8]/CKbb#23	VSS	2.61954e-16
C2143	\mem_reg[8]/CKbb#25	VSS	2.89615e-16
C2144	FE_OFC0_q/n0#119	VSS	2.51026e-16
C2145	FE_OFC0_q/n0#123	VSS	2.3788e-16
C2146	FE_OFC0_q/n0#121	VSS	3.35594e-16
C2147	\mem_reg[10]/CKb#23	VSS	2.68809e-16
C2148	\mem_reg[10]/CKb#29	VSS	3.03086e-16
C2149	\mem_reg[10]/CKb#24	VSS	1.75755e-16
C2150	\mem_reg[15]/mout#9	VSS	3.09587e-16
C2151	\mem_reg[15]/mout#12	VSS	4.42404e-16
C2152	\mem_reg[15]/mout#10	VSS	1.69105e-16
C2153	mux_control_reg/mout#9	VSS	2.74016e-16
C2154	mux_control_reg/mout#12	VSS	4.31576e-16
C2155	mux_control_reg/mout#10	VSS	2.10089e-16
C2156	\mem_reg[8]/n30#9	VSS	2.48752e-16
C2157	\mem_reg[8]/n30#13	VSS	4.89006e-16
C2158	\mem_reg[8]/n30#10	VSS	2.31271e-16
C2159	\mem_reg[0]/n20#9	VSS	6.31619e-16
C2160	\mem_reg[0]/n20#10	VSS	2.64332e-16
C2161	\mem_reg[2]/n20#9	VSS	3.5364e-16
C2162	\mem_reg[2]/n20#10	VSS	5.03227e-16
C2163	\mem_reg[15]/n20#13	VSS	5.56042e-16
C2164	mux_control_reg/n20#13	VSS	5.00851e-16
C2165	\mem_reg[8]/mout#9	VSS	2.91978e-16
C2166	\mem_reg[8]/mout#12	VSS	4.40904e-16
C2167	\mem_reg[8]/mout#10	VSS	2.27874e-16
C2168	mem[8]#24	VSS	1.35485e-16
C2169	scan_clk#162	VSS	3.46289e-16
C2170	FE_OFN0_q#31	VSS	1.3963e-16
C2171	VDD#117	VSS	7.54082e-16
C2172	q_reg_reg/CKbb#25	VSS	2.84558e-16
C2173	q_reg_reg/CKbb#23	VSS	2.7897e-16
C2174	FE_OFC0_q/n0#115	VSS	2.89945e-16
C2175	FE_OFC0_q/n0#117	VSS	3.17496e-16
C2176	\mem_reg[0]/Db#6	VSS	6.23455e-16
C2177	\mem_reg[0]/Db#7	VSS	2.9322e-16
C2178	\mem_reg[2]/Db#6	VSS	3.52065e-16
C2179	\mem_reg[2]/Db#7	VSS	5.56668e-16
C2180	\mem_reg[8]/n20#13	VSS	5.07121e-16
C2181	FE_OFC0_q/n0#124	VSS	3.61763e-16
C2182	\mem_reg[15]/CKbb#18	VSS	1.45534e-16
C2183	\mem_reg[15]/CKbb#27	VSS	2.40967e-16
C2184	\mem_reg[15]/CKbb#21	VSS	3.24267e-16
C2185	\mem_reg[15]/CKbb#19	VSS	1.16327e-16
C2186	mux_control_reg/CKbb#18	VSS	1.97914e-16
C2187	mux_control_reg/CKbb#21	VSS	3.18615e-16
C2188	mux_control_reg/CKbb#27	VSS	2.32707e-16
C2189	mux_control_reg/CKbb#19	VSS	7.68753e-17
C2190	q_reg_reg/n30#9	VSS	3.21458e-16
C2191	q_reg_reg/n30#13	VSS	4.97423e-16
C2192	q_reg_reg/n30#10	VSS	1.41751e-16
C2193	\mem_reg[10]/Db#2	VSS	6.42264e-16
C2194	\mem_reg[10]/Db#3	VSS	3.02129e-16
C2195	\mem_reg[0]/CKb#23	VSS	2.99e-16
C2196	\mem_reg[0]/CKb#29	VSS	2.73495e-16
C2197	\mem_reg[0]/CKb#24	VSS	1.55319e-16
C2198	\mem_reg[2]/CKb#23	VSS	2.24667e-16
C2199	\mem_reg[2]/CKb#29	VSS	3.52155e-16
C2200	\mem_reg[2]/CKb#24	VSS	1.92888e-16
C2201	q_reg_reg/mout#9	VSS	2.94645e-16
C2202	q_reg_reg/mout#12	VSS	4.35681e-16
C2203	q_reg_reg/mout#10	VSS	1.56626e-16
C2204	\mem_reg[8]/CKbb#18	VSS	1.99607e-16
C2205	\mem_reg[8]/CKbb#21	VSS	3.0816e-16
C2206	\mem_reg[8]/CKbb#27	VSS	2.46217e-16
C2207	\mem_reg[8]/CKbb#19	VSS	7.64977e-17
C2208	q_reg_reg/n20#13	VSS	5.26842e-16
C2209	mem[13]#30	VSS	1.20047e-16
C2210	scan_clk#158	VSS	1.75474e-16
C2211	scan_clk#160	VSS	3.25791e-16
C2212	mem[0]#26	VSS	9.92654e-17
C2213	mem[10]#24	VSS	1.66401e-16
C2214	scan_en#305	VSS	1.67502e-16
C2215	UNCONNECTED3#2	VSS	3.60398e-16
C2216	UNCONNECTED3#3	VSS	3.08754e-16
C2217	\mem_reg[15]/n20#9	VSS	3.96902e-16
C2218	\mem_reg[15]/n20#10	VSS	5.63248e-16
C2219	mux_control_reg/n20#9	VSS	6.81282e-16
C2220	mux_control_reg/n20#10	VSS	3.1629e-16
C2221	q_reg_reg/CKbb#18	VSS	1.24965e-16
C2222	q_reg_reg/CKbb#27	VSS	2.01273e-16
C2223	q_reg_reg/CKbb#21	VSS	3.2833e-16
C2224	q_reg_reg/CKbb#19	VSS	1.14519e-16
C2225	\mem_reg[10]/SEb#10	VSS	1.61435e-16
C2226	\mem_reg[10]/SEb#16	VSS	3.15684e-16
C2227	\mem_reg[10]/SEb#14	VSS	2.53658e-16
C2228	\mem_reg[10]/SEb#11	VSS	9.74038e-17
C2229	\mem_reg[0]/Db#2	VSS	6.18069e-16
C2230	\mem_reg[0]/Db#3	VSS	2.38727e-16
C2231	\mem_reg[2]/Db#2	VSS	3.5998e-16
C2232	\mem_reg[2]/Db#3	VSS	4.75011e-16
C2233	\mem_reg[15]/Db#6	VSS	3.94536e-16
C2234	\mem_reg[15]/Db#7	VSS	5.95736e-16
C2235	mux_control_reg/Db#6	VSS	6.80644e-16
C2236	mux_control_reg/Db#7	VSS	3.29125e-16
C2237	\mem_reg[8]/n20#9	VSS	6.67651e-16
C2238	\mem_reg[8]/n20#10	VSS	2.8475e-16
C2239	mem[9]#29	VSS	4.85576e-16
C2240	mem[9]#30	VSS	3.42021e-16
C2241	VDD#194	VSS	1.18948e-15
C2242	VDD#26	VSS	1.21157e-15
C2243	\mem_reg[8]/Db#6	VSS	6.38441e-16
C2244	\mem_reg[8]/Db#7	VSS	3.29702e-16
C2245	g495__6260/n0#6	VSS	2.67421e-16
C2246	\mem_reg[15]/CKb#23	VSS	2.38327e-16
C2247	\mem_reg[15]/CKb#29	VSS	3.24482e-16
C2248	\mem_reg[15]/CKb#24	VSS	1.75225e-16
C2249	mux_control_reg/CKb#23	VSS	2.96008e-16
C2250	mux_control_reg/CKb#29	VSS	3.02447e-16
C2251	mux_control_reg/CKb#24	VSS	1.83921e-16
C2252	mem[0]#24	VSS	2.92311e-16
C2253	scan_en#299	VSS	1.94529e-16
C2254	scan_en#302	VSS	2.03599e-16
C2255	mem[2]#26	VSS	1.07098e-16
C2256	n_11#21	VSS	1.91695e-16
C2257	\mem_reg[9]/qbint#28	VSS	2.65736e-16
C2258	\mem_reg[9]/qbint#22	VSS	4.47253e-16
C2259	q_reg_reg/n20#9	VSS	3.81054e-16
C2260	q_reg_reg/n20#10	VSS	6.05234e-16
C2261	\mem_reg[8]/CKb#23	VSS	2.67684e-16
C2262	\mem_reg[8]/CKb#29	VSS	2.78699e-16
C2263	\mem_reg[8]/CKb#24	VSS	1.67249e-16
C2264	scan_clk#154	VSS	3.29087e-16
C2265	mem[7]#30	VSS	9.27624e-17
C2266	mem[11]#30	VSS	1.43618e-16
C2267	scan_clk#156	VSS	1.93097e-16
C2268	\mem_reg[0]/SEb#10	VSS	1.90192e-16
C2269	\mem_reg[0]/SEb#16	VSS	2.78514e-16
C2270	\mem_reg[0]/SEb#14	VSS	2.62634e-16
C2271	\mem_reg[0]/SEb#11	VSS	9.47628e-17
C2272	\mem_reg[2]/SEb#10	VSS	1.30747e-16
C2273	\mem_reg[2]/SEb#14	VSS	2.83794e-16
C2274	\mem_reg[2]/SEb#16	VSS	2.85946e-16
C2275	\mem_reg[2]/SEb#11	VSS	7.16854e-17
C2276	n_14#10	VSS	4.16731e-16
C2277	g495__6260/n0#4	VSS	3.5622e-16
C2278	q_reg_reg/Db#6	VSS	3.69525e-16
C2279	q_reg_reg/Db#7	VSS	5.76797e-16
C2280	\mem_reg[9]/net367#9	VSS	2.8174e-16
C2281	\mem_reg[9]/net367#13	VSS	3.93332e-16
C2282	\mem_reg[9]/net367#10	VSS	1.67262e-16
C2283	mem[2]#24	VSS	1.20142e-16
C2284	scan_clk#152	VSS	1.78013e-16
C2285	n_3#13	VSS	3.23022e-16
C2286	g495__6260/n1#3	VSS	2.65276e-16
C2287	\mem_reg[15]/Db#2	VSS	3.64212e-16
C2288	\mem_reg[15]/Db#3	VSS	4.56265e-16
C2289	mux_control_reg/Db#2	VSS	5.97457e-16
C2290	mux_control_reg/Db#3	VSS	2.9251e-16
C2291	q_reg_reg/CKb#23	VSS	2.3217e-16
C2292	q_reg_reg/CKb#29	VSS	2.98382e-16
C2293	q_reg_reg/CKb#24	VSS	1.70978e-16
C2294	VDD#368	VSS	2.37619e-16
C2295	n_7#37	VSS	2.83345e-16
C2296	n_9#15	VSS	3.09931e-16
C2297	UNCONNECTED5#2	VSS	4.20539e-16
C2298	UNCONNECTED5#3	VSS	2.46424e-16
C2299	g495__6260/n0#2	VSS	3.2944e-16
C2300	\mem_reg[8]/Db#2	VSS	5.79943e-16
C2301	\mem_reg[8]/Db#3	VSS	2.58224e-16
C2302	g509__6783/net89#6	VSS	3.17615e-16
C2303	scan_en#293	VSS	1.63098e-16
C2304	mem[15]#26	VSS	3.06436e-16
C2305	n_12#27	VSS	2.78879e-16
C2306	FE_OFN1_scan_out_0#53	VSS	3.16786e-16
C2307	scan_en#296	VSS	1.95172e-16
C2308	clk#9	VSS	1.79308e-16
C2309	x#15	VSS	1.49027e-16
C2310	\mem_reg[9]/CKbb#25	VSS	2.84631e-16
C2311	\mem_reg[9]/CKbb#23	VSS	2.64318e-16
C2312	n_14#8	VSS	1.70605e-16
C2313	n_6#12	VSS	4.46995e-16
C2314	mem[13]#27	VSS	4.0355e-16
C2315	mem[13]#28	VSS	1.30546e-16
C2316	\mem_reg[9]/n30#9	VSS	3.16687e-16
C2317	\mem_reg[9]/n30#13	VSS	4.87925e-16
C2318	\mem_reg[9]/n30#10	VSS	1.31354e-16
C2319	mem[2]#22	VSS	2.44446e-16
C2320	mem[8]#22	VSS	1.03423e-16
C2321	scan_en#290	VSS	1.80594e-16
C2322	n_5#9	VSS	3.45049e-16
C2323	\mem_reg[15]/SEb#10	VSS	1.54901e-16
C2324	\mem_reg[15]/SEb#14	VSS	2.75874e-16
C2325	\mem_reg[15]/SEb#16	VSS	2.80392e-16
C2326	\mem_reg[15]/SEb#11	VSS	8.64693e-17
C2327	mux_control_reg/SEb#10	VSS	1.60578e-16
C2328	mux_control_reg/SEb#16	VSS	2.98325e-16
C2329	mux_control_reg/SEb#14	VSS	2.86802e-16
C2330	mux_control_reg/SEb#11	VSS	8.50414e-17
C2331	n_6#10	VSS	3.9006e-16
C2332	g509__6783/net89#4	VSS	1.91118e-16
C2333	q_reg_reg/Db#2	VSS	3.32729e-16
C2334	q_reg_reg/Db#3	VSS	5.20287e-16
C2335	\mem_reg[9]/mout#9	VSS	2.92753e-16
C2336	\mem_reg[9]/mout#12	VSS	4.2069e-16
C2337	\mem_reg[9]/mout#10	VSS	1.57322e-16
C2338	mem[0]#22	VSS	1.27369e-16
C2339	addr[2]#24	VSS	1.91861e-16
C2340	\mem_reg[13]/qbint#23	VSS	3.98417e-16
C2341	\mem_reg[13]/qbint#28	VSS	2.9608e-16
C2342	\mem_reg[8]/SEb#10	VSS	1.67574e-16
C2343	\mem_reg[8]/SEb#16	VSS	2.96604e-16
C2344	\mem_reg[8]/SEb#14	VSS	2.91215e-16
C2345	\mem_reg[8]/SEb#11	VSS	1.02234e-16
C2346	\mem_reg[9]/n20#13	VSS	5.20501e-16
C2347	n_8#37	VSS	2.89775e-16
C2348	q_control#38	VSS	1.45023e-16
C2349	n_15#15	VSS	3.02369e-16
C2350	UNCONNECTED12#2	VSS	3.90919e-16
C2351	UNCONNECTED12#3	VSS	3.74962e-16
C2352	UNCONNECTED16#2	VSS	4.29285e-16
C2353	UNCONNECTED16#3	VSS	2.82927e-16
C2354	g509__6783/net89#2	VSS	3.42731e-16
C2355	g496__4319/n0#6	VSS	2.68816e-16
C2356	\mem_reg[13]/net367#9	VSS	2.73348e-16
C2357	\mem_reg[13]/net367#13	VSS	3.743e-16
C2358	\mem_reg[13]/net367#10	VSS	1.99641e-16
C2359	\mem_reg[9]/CKbb#18	VSS	1.26813e-16
C2360	\mem_reg[9]/CKbb#27	VSS	2.14887e-16
C2361	\mem_reg[9]/CKbb#21	VSS	2.97023e-16
C2362	\mem_reg[9]/CKbb#19	VSS	1.05187e-16
C2363	n_11#19	VSS	2.29798e-16
C2364	q_reg_reg/SEb#10	VSS	1.40133e-16
C2365	q_reg_reg/SEb#14	VSS	2.97053e-16
C2366	q_reg_reg/SEb#16	VSS	2.63209e-16
C2367	q_reg_reg/SEb#11	VSS	1.0052e-16
C2368	n_11#18	VSS	3.95935e-16
C2369	n_13#12	VSS	1.53055e-16
C2370	mem[6]#29	VSS	4.46958e-16
C2371	mem[6]#30	VSS	3.15568e-16
C2372	mem[11]#27	VSS	3.37038e-16
C2373	mem[11]#28	VSS	2.83931e-16
C2374	n_12#24	VSS	2.17914e-16
C2375	n_11#15	VSS	2.07316e-16
C2376	n_13#10	VSS	3.49764e-16
C2377	g496__4319/n0#4	VSS	3.54242e-16
C2378	n_1#10	VSS	3.58255e-16
C2379	n_1#13	VSS	1.27301e-16
C2380	VDD#374	VSS	5.55574e-16
C2381	n_4#13	VSS	3.21036e-16
C2382	addr[0]#45	VSS	4.31702e-16
C2383	VDD#122	VSS	3.96501e-16
C2384	\mem_reg[13]/CKbb#23	VSS	2.58263e-16
C2385	\mem_reg[13]/CKbb#25	VSS	3.17747e-16
C2386	g519__1705/n0#13	VSS	2.75589e-16
C2387	\mem_reg[6]/qbint#28	VSS	2.66129e-16
C2388	\mem_reg[6]/qbint#22	VSS	4.50524e-16
C2389	\mem_reg[11]/qbint#23	VSS	4.5771e-16
C2390	\mem_reg[11]/qbint#28	VSS	2.9253e-16
C2391	UNCONNECTED11#2	VSS	3.83012e-16
C2392	UNCONNECTED11#3	VSS	2.82503e-16
C2393	g496__4319/n1#3	VSS	2.69041e-16
C2394	\mem_reg[9]/n20#9	VSS	3.67056e-16
C2395	\mem_reg[9]/n20#10	VSS	5.61607e-16
C2396	\mem_reg[13]/n30#9	VSS	2.39953e-16
C2397	\mem_reg[13]/n30#13	VSS	4.79229e-16
C2398	\mem_reg[13]/n30#10	VSS	2.68614e-16
C2399	mem[15]#24	VSS	3.02352e-16
C2400	n_10#15	VSS	4.5182e-16
C2401	g496__4319/n0#2	VSS	3.19078e-16
C2402	\mem_reg[9]/Db#6	VSS	3.59418e-16
C2403	\mem_reg[9]/Db#7	VSS	5.29837e-16
C2404	\mem_reg[13]/mout#9	VSS	2.66612e-16
C2405	\mem_reg[13]/mout#12	VSS	4.09613e-16
C2406	\mem_reg[13]/mout#10	VSS	2.422e-16
C2407	\mem_reg[6]/net367#9	VSS	2.96472e-16
C2408	\mem_reg[6]/net367#13	VSS	4.04675e-16
C2409	\mem_reg[6]/net367#10	VSS	2.2455e-16
C2410	\mem_reg[11]/net367#9	VSS	3.18296e-16
C2411	\mem_reg[11]/net367#13	VSS	3.96787e-16
C2412	\mem_reg[11]/net367#10	VSS	1.51468e-16
C2413	q_control#34	VSS	3.49292e-16
C2414	q_control#35	VSS	3.92704e-16
C2415	n_12#23	VSS	2.94276e-16
C2416	n_2#28	VSS	3.88786e-16
C2417	n_2#31	VSS	4.36461e-16
C2418	n_13#8	VSS	1.86751e-16
C2419	g519__1705/n0#8	VSS	2.09306e-16
C2420	g519__1705/n0#10	VSS	1.17251e-16
C2421	\mem_reg[13]/n20#13	VSS	4.9028e-16
C2422	\mem_reg[9]/CKb#23	VSS	2.57958e-16
C2423	\mem_reg[9]/CKb#29	VSS	3.08277e-16
C2424	\mem_reg[9]/CKb#24	VSS	1.70692e-16
C2425	n_6#9	VSS	3.6287e-16
C2426	addr[1]#70	VSS	2.16053e-16
C2427	q_control_reg/qbint#28	VSS	2.85361e-16
C2428	q_control_reg/qbint#22	VSS	4.24811e-16
C2429	\mem_reg[13]/CKbb#18	VSS	1.9131e-16
C2430	\mem_reg[13]/CKbb#21	VSS	3.07207e-16
C2431	\mem_reg[13]/CKbb#27	VSS	2.79985e-16
C2432	\mem_reg[13]/CKbb#19	VSS	8.91238e-17
C2433	mem[13]#24	VSS	1.1929e-16
C2434	addr[1]#72	VSS	2.21562e-16
C2435	addr[2]#22	VSS	1.91298e-16
C2436	VDD#205	VSS	1.20673e-15
C2437	scan_clk#150	VSS	3.64963e-16
C2438	mem[10]#22	VSS	7.93923e-17
C2439	VDD#37	VSS	1.34234e-15
C2440	\mem_reg[6]/CKbb#25	VSS	3.2765e-16
C2441	\mem_reg[6]/CKbb#23	VSS	2.94843e-16
C2442	\mem_reg[11]/CKbb#23	VSS	3.02181e-16
C2443	\mem_reg[11]/CKbb#25	VSS	2.89426e-16
C2444	\mem_reg[6]/n30#9	VSS	3.36576e-16
C2445	\mem_reg[6]/n30#13	VSS	5.03603e-16
C2446	\mem_reg[6]/n30#10	VSS	1.51974e-16
C2447	\mem_reg[11]/n30#9	VSS	2.56581e-16
C2448	\mem_reg[11]/n30#13	VSS	5.03661e-16
C2449	\mem_reg[11]/n30#10	VSS	2.42511e-16
C2450	q_control_reg/net367#9	VSS	2.80063e-16
C2451	q_control_reg/net367#13	VSS	3.88299e-16
C2452	q_control_reg/net367#10	VSS	1.87373e-16
C2453	mem[6]#26	VSS	2.13587e-16
C2454	g519__1705/S0b#8	VSS	2.13606e-16
C2455	g519__1705/S0b#13	VSS	2.43559e-16
C2456	g519__1705/S0b#10	VSS	1.71834e-16
C2457	\mem_reg[9]/Db#2	VSS	3.57612e-16
C2458	\mem_reg[9]/Db#3	VSS	5.27147e-16
C2459	g497__8428/net118#5	VSS	1.21333e-16
C2460	n_10#12	VSS	7.80022e-17
C2461	\mem_reg[6]/mout#9	VSS	3.25803e-16
C2462	\mem_reg[6]/mout#12	VSS	4.55249e-16
C2463	\mem_reg[6]/mout#10	VSS	1.67143e-16
C2464	\mem_reg[11]/mout#9	VSS	2.76554e-16
C2465	\mem_reg[11]/mout#12	VSS	4.00669e-16
C2466	\mem_reg[11]/mout#10	VSS	1.98941e-16
C2467	n_8#35	VSS	4.75784e-16
C2468	n_10#11	VSS	4.94947e-16
C2469	\mem_reg[13]/n20#9	VSS	6.56391e-16
C2470	\mem_reg[13]/n20#10	VSS	3.04084e-16
C2471	\mem_reg[6]/n20#13	VSS	5.38007e-16
C2472	\mem_reg[11]/n20#13	VSS	5.2621e-16
C2473	scan_en#287	VSS	2.1545e-16
C2474	mem[9]#26	VSS	1.64359e-16
C2475	VDD#128	VSS	4.90363e-16
C2476	addr[1]#68	VSS	3.3231e-16
C2477	q_control_reg/CKbb#25	VSS	2.79274e-16
C2478	q_control_reg/CKbb#23	VSS	2.66113e-16
C2479	n_8#34	VSS	5.33694e-16
C2480	g497__8428/net118#3	VSS	2.02629e-16
C2481	\mem_reg[13]/Db#6	VSS	6.45495e-16
C2482	\mem_reg[13]/Db#7	VSS	3.39608e-16
C2483	n_0#10	VSS	3.38842e-16
C2484	n_0#13	VSS	1.36777e-16
C2485	\mem_reg[6]/CKbb#18	VSS	1.42762e-16
C2486	\mem_reg[6]/CKbb#27	VSS	2.42694e-16
C2487	\mem_reg[6]/CKbb#21	VSS	3.59482e-16
C2488	\mem_reg[6]/CKbb#19	VSS	1.20302e-16
C2489	\mem_reg[11]/CKbb#18	VSS	1.90061e-16
C2490	\mem_reg[11]/CKbb#21	VSS	3.22759e-16
C2491	\mem_reg[11]/CKbb#27	VSS	2.0183e-16
C2492	\mem_reg[11]/CKbb#19	VSS	6.75687e-17
C2493	q_control_reg/n30#9	VSS	3.20557e-16
C2494	q_control_reg/n30#13	VSS	4.70944e-16
C2495	q_control_reg/n30#10	VSS	1.39303e-16
C2496	addr[0]#43	VSS	3.63421e-16
C2497	n_7#36	VSS	4.49583e-16
C2498	g520__5122/n0#13	VSS	2.39435e-16
C2499	\mem_reg[9]/SEb#10	VSS	1.39863e-16
C2500	\mem_reg[9]/SEb#14	VSS	2.69729e-16
C2501	\mem_reg[9]/SEb#16	VSS	2.71428e-16
C2502	\mem_reg[9]/SEb#11	VSS	8.08149e-17
C2503	g497__8428/net122#5	VSS	1.8588e-16
C2504	\mem_reg[13]/CKb#23	VSS	2.55076e-16
C2505	\mem_reg[13]/CKb#29	VSS	3.37653e-16
C2506	\mem_reg[13]/CKb#24	VSS	1.92282e-16
C2507	q_control_reg/mout#9	VSS	2.9139e-16
C2508	q_control_reg/mout#12	VSS	4.08579e-16
C2509	q_control_reg/mout#10	VSS	1.62533e-16
C2510	mem[7]#28	VSS	1.69826e-16
C2511	addr[0]#40	VSS	2.03512e-16
C2512	q_control_reg/n20#13	VSS	5.12805e-16
C2513	mem[15]#22	VSS	9.62546e-17
C2514	scan_clk#148	VSS	1.72955e-16
C2515	mem[14]#30	VSS	1.87575e-16
C2516	n_0#8	VSS	2.37637e-16
C2517	addr[1]#65	VSS	2.86196e-16
C2518	g497__8428/net122#3	VSS	2.15167e-16
C2519	n_10#9	VSS	3.82459e-16
C2520	g520__5122/n0#8	VSS	2.00552e-16
C2521	g520__5122/n0#10	VSS	1.25686e-16
C2522	g498__5526/net118#5	VSS	1.29284e-16
C2523	n_9#12	VSS	6.96861e-17
C2524	g522__2802/net89#6	VSS	3.27579e-16
C2525	\mem_reg[6]/n20#9	VSS	3.84049e-16
C2526	\mem_reg[6]/n20#10	VSS	5.56862e-16
C2527	\mem_reg[11]/n20#9	VSS	6.54225e-16
C2528	\mem_reg[11]/n20#10	VSS	2.6746e-16
C2529	q_control_reg/CKbb#18	VSS	1.25895e-16
C2530	q_control_reg/CKbb#27	VSS	2.02006e-16
C2531	q_control_reg/CKbb#21	VSS	2.91492e-16
C2532	q_control_reg/CKbb#19	VSS	1.15281e-16
C2533	mem[4]#30	VSS	3.50085e-16
C2534	\mem_reg[13]/Db#2	VSS	5.58561e-16
C2535	\mem_reg[13]/Db#3	VSS	2.85559e-16
C2536	n_9#11	VSS	4.99686e-16
C2537	n_3#10	VSS	3.21851e-16
C2538	\mem_reg[6]/Db#6	VSS	3.66186e-16
C2539	\mem_reg[6]/Db#7	VSS	5.27822e-16
C2540	\mem_reg[11]/Db#6	VSS	6.26138e-16
C2541	\mem_reg[11]/Db#7	VSS	2.87858e-16
C2542	mem[5]#30	VSS	1.08236e-16
C2543	addr[1]#62	VSS	2.1751e-16
C2544	VDD#324	VSS	2.25526e-15
C2545	n_8#31	VSS	1.64044e-16
C2546	mem[11]#24	VSS	2.97581e-16
C2547	g498__5526/net118#3	VSS	1.65726e-16
C2548	n_3#8	VSS	1.60738e-16
C2549	g522__2802/net89#4	VSS	1.94762e-16
C2550	\mem_reg[6]/CKb#23	VSS	2.57407e-16
C2551	\mem_reg[6]/CKb#29	VSS	3.15697e-16
C2552	\mem_reg[6]/CKb#24	VSS	1.8087e-16
C2553	\mem_reg[11]/CKb#23	VSS	2.75208e-16
C2554	\mem_reg[11]/CKb#29	VSS	2.85141e-16
C2555	\mem_reg[11]/CKb#24	VSS	1.53603e-16
C2556	mem[13]#22	VSS	2.85585e-16
C2557	scan_en#284	VSS	1.84799e-16
C2558	n_7#34	VSS	4.57376e-16
C2559	mem[9]#24	VSS	3.76678e-16
C2560	g520__5122/S0b#8	VSS	2.29843e-16
C2561	g520__5122/S0b#13	VSS	2.40751e-16
C2562	g520__5122/S0b#10	VSS	1.82862e-16
C2563	g498__5526/net122#5	VSS	1.76005e-16
C2564	q_control_reg/n20#9	VSS	3.87689e-16
C2565	q_control_reg/n20#10	VSS	5.41741e-16
C2566	addr[0]#38	VSS	2.38381e-16
C2567	scan_clk#144	VSS	1.84198e-16
C2568	mem[4]#28	VSS	1.37925e-16
C2569	addr[1]#59	VSS	2.3143e-16
C2570	n_2#26	VSS	1.42864e-16
C2571	mem[9]#22	VSS	1.15998e-16
C2572	scan_clk#146	VSS	1.65828e-16
C2573	\mem_reg[13]/SEb#10	VSS	1.51586e-16
C2574	\mem_reg[13]/SEb#16	VSS	2.61157e-16
C2575	\mem_reg[13]/SEb#14	VSS	2.44895e-16
C2576	\mem_reg[13]/SEb#11	VSS	9.7733e-17
C2577	g522__2802/net89#2	VSS	3.08845e-16
C2578	g521__1617/net89#5	VSS	1.47005e-16
C2579	q_control_reg/Db#6	VSS	4.02664e-16
C2580	q_control_reg/Db#7	VSS	5.47021e-16
C2581	n_1#8	VSS	2.31816e-16
C2582	UNCONNECTED0#2	VSS	3.46539e-16
C2583	UNCONNECTED0#3	VSS	3.32481e-16
C2584	g498__5526/net122#3	VSS	2.02279e-16
C2585	n_9#9	VSS	3.80844e-16
C2586	\mem_reg[6]/Db#2	VSS	3.66813e-16
C2587	\mem_reg[6]/Db#3	VSS	5.35905e-16
C2588	n_4#11	VSS	1.0591e-16
C2589	\mem_reg[11]/Db#2	VSS	6.06719e-16
C2590	\mem_reg[11]/Db#3	VSS	2.4704e-16
C2591	q_control_reg/CKb#23	VSS	2.69044e-16
C2592	q_control_reg/CKb#29	VSS	2.72519e-16
C2593	q_control_reg/CKb#24	VSS	1.85693e-16
C2594	VDD#379	VSS	2.47915e-16
C2595	mem[12]#30	VSS	2.19941e-16
C2596	mem[3]#30	VSS	3.05735e-16
C2597	UNCONNECTED13#2	VSS	3.75756e-16
C2598	UNCONNECTED13#3	VSS	2.47761e-16
C2599	n_7#32	VSS	5.49052e-16
C2600	n_4#9	VSS	3.67794e-16
C2601	mem[5]#27	VSS	2.80028e-16
C2602	mem[5]#28	VSS	2.71716e-16
C2603	n_8#29	VSS	1.68969e-16
C2604	scan_en#278	VSS	1.91934e-16
C2605	mem[6]#24	VSS	1.6279e-16
C2606	mem[1]#30	VSS	2.18047e-16
C2607	mem[11]#22	VSS	1.3186e-16
C2608	scan_en#281	VSS	1.85235e-16
C2609	scan_clk#142	VSS	1.74589e-16
C2610	scan_in[0]#8	VSS	8.84741e-17
C2611	n_2#25	VSS	4.60208e-16
C2612	n_7#29	VSS	3.76983e-16
C2613	mem[7]#25	VSS	4.63212e-16
C2614	mem[7]#26	VSS	2.66102e-16
C2615	n_2#22	VSS	1.86919e-16
C2616	addr[0]#37	VSS	2.16689e-16
C2617	\mem_reg[5]/qbint#28	VSS	2.71818e-16
C2618	\mem_reg[5]/qbint#22	VSS	4.37678e-16
C2619	\mem_reg[6]/SEb#10	VSS	1.48675e-16
C2620	\mem_reg[6]/SEb#14	VSS	2.63878e-16
C2621	\mem_reg[6]/SEb#16	VSS	2.93804e-16
C2622	\mem_reg[6]/SEb#11	VSS	9.57738e-17
C2623	\mem_reg[11]/SEb#10	VSS	1.62611e-16
C2624	\mem_reg[11]/SEb#16	VSS	2.60582e-16
C2625	\mem_reg[11]/SEb#14	VSS	2.48533e-16
C2626	\mem_reg[11]/SEb#11	VSS	1.14427e-16
C2627	q_control_reg/Db#2	VSS	3.38052e-16
C2628	q_control_reg/Db#3	VSS	4.77629e-16
C2629	\mem_reg[7]/qbint#23	VSS	4.02538e-16
C2630	\mem_reg[7]/qbint#28	VSS	2.6955e-16
C2631	UNCONNECTED6#2	VSS	4.47861e-16
C2632	UNCONNECTED6#3	VSS	2.54093e-16
C2633	\mem_reg[5]/net367#9	VSS	2.78617e-16
C2634	\mem_reg[5]/net367#13	VSS	3.98151e-16
C2635	\mem_reg[5]/net367#10	VSS	1.78395e-16
C2636	VDD#211	VSS	4.13436e-16
C2637	VDD#44	VSS	5.09083e-16
C2638	scan_en#275	VSS	2.01374e-16
C2639	q_control#31	VSS	3.10945e-16
C2640	UNCONNECTED4#2	VSS	3.86085e-16
C2641	UNCONNECTED4#3	VSS	3.16581e-16
C2642	UNCONNECTED17#2	VSS	3.99805e-16
C2643	UNCONNECTED17#3	VSS	2.68047e-16
C2644	UNCONNECTED10#2	VSS	3.62997e-16
C2645	UNCONNECTED10#3	VSS	2.66476e-16
C2646	UNCONNECTED8#2	VSS	3.94208e-16
C2647	UNCONNECTED8#3	VSS	2.71396e-16
C2648	\mem_reg[7]/net367#9	VSS	2.84129e-16
C2649	\mem_reg[7]/net367#13	VSS	3.71339e-16
C2650	\mem_reg[7]/net367#10	VSS	1.41232e-16
C2651	mem[14]#27	VSS	4.46984e-16
C2652	mem[14]#28	VSS	3.43562e-16
C2653	q_control_reg/SEb#10	VSS	1.46506e-16
C2654	q_control_reg/SEb#14	VSS	2.63391e-16
C2655	q_control_reg/SEb#16	VSS	2.74371e-16
C2656	q_control_reg/SEb#11	VSS	8.44821e-17
C2657	mem[12]#27	VSS	2.99442e-16
C2658	mem[12]#28	VSS	2.69834e-16
C2659	mem[4]#25	VSS	4.07048e-16
C2660	mem[4]#26	VSS	3.22305e-16
C2661	mem[1]#27	VSS	3.94362e-16
C2662	mem[1]#28	VSS	4.43933e-16
C2663	mem[3]#27	VSS	4.08693e-16
C2664	mem[3]#28	VSS	3.36614e-16
C2665	\mem_reg[5]/CKbb#25	VSS	2.78558e-16
C2666	\mem_reg[5]/CKbb#23	VSS	2.51174e-16
C2667	\mem_reg[14]/qbint#23	VSS	4.18323e-16
C2668	\mem_reg[14]/qbint#28	VSS	2.59037e-16
C2669	\mem_reg[5]/n30#9	VSS	3.09816e-16
C2670	\mem_reg[5]/n30#13	VSS	4.74771e-16
C2671	\mem_reg[5]/n30#10	VSS	1.33456e-16
C2672	VDD#385	VSS	6.55748e-16
C2673	VDD#133	VSS	3.51545e-16
C2674	\mem_reg[7]/CKbb#23	VSS	2.56942e-16
C2675	\mem_reg[7]/CKbb#25	VSS	2.78815e-16
C2676	\mem_reg[12]/qbint#28	VSS	2.54844e-16
C2677	\mem_reg[12]/qbint#22	VSS	4.07211e-16
C2678	\mem_reg[4]/qbint#23	VSS	4.04581e-16
C2679	\mem_reg[4]/qbint#28	VSS	2.53237e-16
C2680	\mem_reg[1]/qbint#28	VSS	2.50428e-16
C2681	\mem_reg[1]/qbint#22	VSS	3.96047e-16
C2682	\mem_reg[3]/qbint#23	VSS	4.30996e-16
C2683	\mem_reg[3]/qbint#28	VSS	2.4679e-16
C2684	scan_out[0]#30	VSS	2.52933e-16
C2685	scan_out[0]#32	VSS	2.92104e-16
C2686	\mem_reg[5]/mout#9	VSS	2.81539e-16
C2687	\mem_reg[5]/mout#12	VSS	4.14364e-16
C2688	\mem_reg[5]/mout#10	VSS	1.56616e-16
C2689	\mem_reg[7]/n30#9	VSS	2.3952e-16
C2690	\mem_reg[7]/n30#13	VSS	4.7668e-16
C2691	\mem_reg[7]/n30#10	VSS	2.25624e-16
C2692	\mem_reg[14]/net367#9	VSS	2.79418e-16
C2693	\mem_reg[14]/net367#13	VSS	3.67504e-16
C2694	\mem_reg[14]/net367#10	VSS	1.39302e-16
C2695	\mem_reg[5]/n20#13	VSS	4.8443e-16
C2696	\mem_reg[7]/mout#9	VSS	2.65908e-16
C2697	\mem_reg[7]/mout#12	VSS	4.10838e-16
C2698	\mem_reg[7]/mout#10	VSS	1.98886e-16
C2699	\mem_reg[12]/net367#9	VSS	2.52556e-16
C2700	\mem_reg[12]/net367#13	VSS	3.74014e-16
C2701	\mem_reg[12]/net367#10	VSS	1.61728e-16
C2702	\mem_reg[4]/net367#9	VSS	2.79581e-16
C2703	\mem_reg[4]/net367#13	VSS	3.74098e-16
C2704	\mem_reg[4]/net367#10	VSS	1.41105e-16
C2705	\mem_reg[1]/net367#9	VSS	2.51105e-16
C2706	\mem_reg[1]/net367#13	VSS	3.64996e-16
C2707	\mem_reg[1]/net367#10	VSS	1.48903e-16
C2708	\mem_reg[3]/net367#9	VSS	3.07051e-16
C2709	\mem_reg[3]/net367#13	VSS	3.69501e-16
C2710	\mem_reg[3]/net367#10	VSS	1.32244e-16
C2711	scan_out[0]#26	VSS	1.93585e-16
C2712	scan_out[0]#28	VSS	2.44365e-16
C2713	\mem_reg[7]/n20#13	VSS	4.88218e-16
C2714	\mem_reg[5]/CKbb#18	VSS	1.26279e-16
C2715	\mem_reg[5]/CKbb#27	VSS	2.01276e-16
C2716	\mem_reg[5]/CKbb#21	VSS	2.88121e-16
C2717	\mem_reg[5]/CKbb#19	VSS	1.00046e-16
C2718	\mem_reg[14]/CKbb#23	VSS	2.64388e-16
C2719	\mem_reg[14]/CKbb#25	VSS	2.77705e-16
C2720	\mem_reg[7]/CKbb#18	VSS	1.90274e-16
C2721	\mem_reg[7]/CKbb#21	VSS	2.95575e-16
C2722	\mem_reg[7]/CKbb#27	VSS	1.98831e-16
C2723	\mem_reg[7]/CKbb#19	VSS	7.48558e-17
C2724	\mem_reg[14]/n30#9	VSS	2.38243e-16
C2725	\mem_reg[14]/n30#13	VSS	4.78421e-16
C2726	\mem_reg[14]/n30#10	VSS	2.14875e-16
C2727	VDD#344	VSS	1.11806e-15
C2728	VDD#217	VSS	1.34005e-15
C2729	VDD#50	VSS	1.30204e-15
C2730	VDD#157	VSS	5.15692e-16
C2731	\mem_reg[12]/CKbb#25	VSS	2.84647e-16
C2732	\mem_reg[12]/CKbb#23	VSS	2.57955e-16
C2733	\mem_reg[4]/CKbb#23	VSS	2.54289e-16
C2734	\mem_reg[4]/CKbb#25	VSS	2.76117e-16
C2735	\mem_reg[1]/CKbb#25	VSS	2.75694e-16
C2736	\mem_reg[1]/CKbb#23	VSS	2.52438e-16
C2737	\mem_reg[3]/CKbb#23	VSS	2.57105e-16
C2738	\mem_reg[3]/CKbb#25	VSS	2.78595e-16
C2739	scan_out[0]#22	VSS	1.98445e-16
C2740	scan_out[0]#24	VSS	2.46821e-16
C2741	\mem_reg[14]/mout#9	VSS	2.66767e-16
C2742	\mem_reg[14]/mout#12	VSS	4.08736e-16
C2743	\mem_reg[14]/mout#10	VSS	1.94857e-16
C2744	\mem_reg[12]/n30#9	VSS	3.0885e-16
C2745	\mem_reg[12]/n30#13	VSS	4.86717e-16
C2746	\mem_reg[12]/n30#10	VSS	1.38536e-16
C2747	\mem_reg[4]/n30#9	VSS	2.41142e-16
C2748	\mem_reg[4]/n30#13	VSS	4.81346e-16
C2749	\mem_reg[4]/n30#10	VSS	2.21027e-16
C2750	\mem_reg[1]/n30#9	VSS	3.16477e-16
C2751	\mem_reg[1]/n30#13	VSS	4.81634e-16
C2752	\mem_reg[1]/n30#10	VSS	1.31013e-16
C2753	\mem_reg[3]/n30#9	VSS	2.40383e-16
C2754	\mem_reg[3]/n30#13	VSS	4.89921e-16
C2755	\mem_reg[3]/n30#10	VSS	2.213e-16
C2756	\mem_reg[5]/n20#9	VSS	3.58477e-16
C2757	\mem_reg[5]/n20#10	VSS	5.136e-16
C2758	\mem_reg[14]/n20#13	VSS	4.84012e-16
C2759	\mem_reg[12]/mout#9	VSS	2.83599e-16
C2760	\mem_reg[12]/mout#12	VSS	4.24526e-16
C2761	\mem_reg[12]/mout#10	VSS	1.58732e-16
C2762	\mem_reg[4]/mout#9	VSS	2.70859e-16
C2763	\mem_reg[4]/mout#12	VSS	4.14925e-16
C2764	\mem_reg[4]/mout#10	VSS	1.94634e-16
C2765	\mem_reg[1]/mout#9	VSS	2.88412e-16
C2766	\mem_reg[1]/mout#12	VSS	4.10575e-16
C2767	\mem_reg[1]/mout#10	VSS	1.58221e-16
C2768	\mem_reg[3]/mout#9	VSS	2.6915e-16
C2769	\mem_reg[3]/mout#12	VSS	4.27055e-16
C2770	\mem_reg[3]/mout#10	VSS	1.98733e-16
C2771	scan_out[0]#18	VSS	1.99435e-16
C2772	scan_out[0]#20	VSS	2.45303e-16
C2773	\mem_reg[5]/Db#6	VSS	3.69044e-16
C2774	\mem_reg[5]/Db#7	VSS	5.2585e-16
C2775	\mem_reg[7]/n20#9	VSS	6.37014e-16
C2776	\mem_reg[7]/n20#10	VSS	2.76483e-16
C2777	\mem_reg[12]/n20#13	VSS	5.09883e-16
C2778	\mem_reg[4]/n20#13	VSS	5.02753e-16
C2779	\mem_reg[1]/n20#13	VSS	5.16629e-16
C2780	\mem_reg[3]/n20#13	VSS	5.01827e-16
C2781	\mem_reg[14]/CKbb#18	VSS	1.86568e-16
C2782	\mem_reg[14]/CKbb#21	VSS	2.83031e-16
C2783	\mem_reg[14]/CKbb#27	VSS	1.88271e-16
C2784	\mem_reg[14]/CKbb#19	VSS	7.56825e-17
C2785	\mem_reg[7]/Db#6	VSS	6.30672e-16
C2786	\mem_reg[7]/Db#7	VSS	2.99042e-16
C2787	\mem_reg[5]/CKb#23	VSS	2.29002e-16
C2788	\mem_reg[5]/CKb#29	VSS	2.76707e-16
C2789	\mem_reg[5]/CKb#24	VSS	1.63217e-16
C2790	\mem_reg[12]/CKbb#18	VSS	1.25112e-16
C2791	\mem_reg[12]/CKbb#27	VSS	2.07009e-16
C2792	\mem_reg[12]/CKbb#21	VSS	2.83608e-16
C2793	\mem_reg[12]/CKbb#19	VSS	1.06829e-16
C2794	\mem_reg[4]/CKbb#18	VSS	1.92269e-16
C2795	\mem_reg[4]/CKbb#21	VSS	2.83017e-16
C2796	\mem_reg[4]/CKbb#27	VSS	1.93584e-16
C2797	\mem_reg[4]/CKbb#19	VSS	6.82175e-17
C2798	\mem_reg[1]/CKbb#18	VSS	1.33654e-16
C2799	\mem_reg[1]/CKbb#27	VSS	1.96687e-16
C2800	\mem_reg[1]/CKbb#21	VSS	2.87692e-16
C2801	\mem_reg[1]/CKbb#19	VSS	1.04689e-16
C2802	\mem_reg[3]/CKbb#18	VSS	1.93092e-16
C2803	\mem_reg[3]/CKbb#21	VSS	2.96613e-16
C2804	\mem_reg[3]/CKbb#27	VSS	2.10186e-16
C2805	\mem_reg[3]/CKbb#19	VSS	6.78592e-17
C2806	scan_out[0]#14	VSS	1.98212e-16
C2807	scan_out[0]#16	VSS	2.45629e-16
C2808	\mem_reg[7]/CKb#23	VSS	2.65741e-16
C2809	\mem_reg[7]/CKb#29	VSS	2.82619e-16
C2810	\mem_reg[7]/CKb#24	VSS	1.86003e-16
C2811	scan_clk#140	VSS	1.69078e-16
C2812	mem[14]#24	VSS	1.24591e-16
C2813	\mem_reg[14]/n20#9	VSS	6.32659e-16
C2814	\mem_reg[14]/n20#10	VSS	2.69017e-16
C2815	mem[5]#24	VSS	7.7289e-17
C2816	scan_clk#138	VSS	1.89656e-16
C2817	scan_out[0]#10	VSS	2.00043e-16
C2818	scan_out[0]#12	VSS	2.44898e-16
C2819	\mem_reg[5]/Db#2	VSS	3.54634e-16
C2820	\mem_reg[5]/Db#3	VSS	4.56867e-16
C2821	\mem_reg[14]/Db#6	VSS	6.23525e-16
C2822	\mem_reg[14]/Db#7	VSS	2.80138e-16
C2823	\mem_reg[12]/n20#9	VSS	3.65061e-16
C2824	\mem_reg[12]/n20#10	VSS	5.41685e-16
C2825	\mem_reg[4]/n20#9	VSS	6.26569e-16
C2826	\mem_reg[4]/n20#10	VSS	2.67338e-16
C2827	\mem_reg[1]/n20#9	VSS	3.53875e-16
C2828	\mem_reg[1]/n20#10	VSS	5.03991e-16
C2829	\mem_reg[3]/n20#9	VSS	6.28075e-16
C2830	\mem_reg[3]/n20#10	VSS	2.77835e-16
C2831	\mem_reg[7]/Db#2	VSS	5.58553e-16
C2832	\mem_reg[7]/Db#3	VSS	2.65864e-16
C2833	\mem_reg[12]/Db#6	VSS	3.64477e-16
C2834	\mem_reg[12]/Db#7	VSS	5.44837e-16
C2835	\mem_reg[4]/Db#6	VSS	6.52411e-16
C2836	\mem_reg[4]/Db#7	VSS	2.87541e-16
C2837	\mem_reg[1]/Db#6	VSS	3.54201e-16
C2838	\mem_reg[1]/Db#7	VSS	5.79727e-16
C2839	\mem_reg[3]/Db#6	VSS	6.3479e-16
C2840	\mem_reg[3]/Db#7	VSS	2.93616e-16
C2841	\mem_reg[14]/CKb#23	VSS	2.63944e-16
C2842	\mem_reg[14]/CKb#29	VSS	2.91247e-16
C2843	\mem_reg[14]/CKb#24	VSS	1.61649e-16
C2844	scan_out[0]#6	VSS	2.06581e-16
C2845	scan_out[0]#8	VSS	2.37388e-16
C2846	scan_en#272	VSS	1.98069e-16
C2847	mem[5]#22	VSS	2.96189e-16
C2848	\mem_reg[12]/CKb#23	VSS	2.47106e-16
C2849	\mem_reg[12]/CKb#29	VSS	2.97275e-16
C2850	\mem_reg[12]/CKb#24	VSS	2.02692e-16
C2851	\mem_reg[4]/CKb#23	VSS	2.83013e-16
C2852	\mem_reg[4]/CKb#29	VSS	3.14054e-16
C2853	\mem_reg[4]/CKb#24	VSS	1.63834e-16
C2854	\mem_reg[1]/CKb#23	VSS	2.1481e-16
C2855	\mem_reg[1]/CKb#29	VSS	2.92406e-16
C2856	\mem_reg[1]/CKb#24	VSS	1.88982e-16
C2857	\mem_reg[3]/CKb#23	VSS	2.89673e-16
C2858	\mem_reg[3]/CKb#29	VSS	2.86803e-16
C2859	\mem_reg[3]/CKb#24	VSS	1.77722e-16
C2860	mem[7]#22	VSS	9.84204e-17
C2861	scan_en#269	VSS	1.77956e-16
C2862	mem[12]#24	VSS	8.03335e-17
C2863	scan_clk#136	VSS	2.04798e-16
C2864	\mem_reg[5]/SEb#10	VSS	1.36545e-16
C2865	\mem_reg[5]/SEb#14	VSS	2.94041e-16
C2866	\mem_reg[5]/SEb#16	VSS	2.50372e-16
C2867	\mem_reg[5]/SEb#11	VSS	6.7792e-17
C2868	scan_out[0]#2	VSS	2.45462e-16
C2869	scan_out[0]#4	VSS	1.57793e-16
C2870	scan_clk#128	VSS	1.73151e-16
C2871	mem[6]#22	VSS	1.34262e-16
C2872	mem[1]#24	VSS	8.86319e-17
C2873	scan_clk#130	VSS	1.67587e-16
C2874	scan_clk#132	VSS	3.30734e-16
C2875	mem[3]#24	VSS	1.49944e-16
C2876	q_control#29	VSS	7.65029e-17
C2877	scan_clk#134	VSS	1.86799e-16
C2878	\mem_reg[7]/SEb#10	VSS	1.37266e-16
C2879	\mem_reg[7]/SEb#16	VSS	2.45353e-16
C2880	\mem_reg[7]/SEb#14	VSS	2.20191e-16
C2881	\mem_reg[7]/SEb#11	VSS	8.21929e-17
C2882	\mem_reg[14]/Db#2	VSS	6.2147e-16
C2883	\mem_reg[14]/Db#3	VSS	2.89078e-16
C2884	\mem_reg[12]/Db#2	VSS	3.63515e-16
C2885	\mem_reg[12]/Db#3	VSS	4.92449e-16
C2886	\mem_reg[4]/Db#2	VSS	6.07067e-16
C2887	\mem_reg[4]/Db#3	VSS	2.65848e-16
C2888	\mem_reg[1]/Db#2	VSS	3.5928e-16
C2889	\mem_reg[1]/Db#3	VSS	5.43972e-16
C2890	\mem_reg[3]/Db#2	VSS	6.57926e-16
C2891	\mem_reg[3]/Db#3	VSS	2.88783e-16
C2892	FE_OFC1_scan_out_0/n0#119	VSS	3.05973e-16
C2893	FE_OFC1_scan_out_0/n0#123	VSS	2.1949e-16
C2894	FE_OFC1_scan_out_0/n0#121	VSS	3.30717e-16
C2895	mem[14]#22	VSS	1.67894e-16
C2896	scan_en#266	VSS	1.60037e-16
C2897	scan_en#254	VSS	1.86336e-16
C2898	mem[12]#22	VSS	3.21831e-16
C2899	mem[4]#22	VSS	1.58126e-16
C2900	scan_en#257	VSS	1.64956e-16
C2901	scan_en#260	VSS	1.91154e-16
C2902	mem[1]#22	VSS	3.16574e-16
C2903	mem[3]#22	VSS	1.67925e-16
C2904	scan_en#263	VSS	2.01118e-16
C2905	FE_OFN1_scan_out_0#52	VSS	3.56218e-16
C2906	\mem_reg[14]/SEb#10	VSS	1.44409e-16
C2907	\mem_reg[14]/SEb#16	VSS	2.41156e-16
C2908	\mem_reg[14]/SEb#14	VSS	2.19371e-16
C2909	\mem_reg[14]/SEb#11	VSS	7.43356e-17
C2910	FE_OFC1_scan_out_0/n0#115	VSS	3.07925e-16
C2911	FE_OFC1_scan_out_0/n0#117	VSS	2.88981e-16
C2912	FE_OFC1_scan_out_0/n0#124	VSS	3.84942e-16
C2913	\mem_reg[12]/SEb#10	VSS	1.24699e-16
C2914	\mem_reg[12]/SEb#14	VSS	2.28707e-16
C2915	\mem_reg[12]/SEb#16	VSS	2.64962e-16
C2916	\mem_reg[12]/SEb#11	VSS	8.20328e-17
C2917	\mem_reg[4]/SEb#10	VSS	1.5035e-16
C2918	\mem_reg[4]/SEb#16	VSS	2.61398e-16
C2919	\mem_reg[4]/SEb#14	VSS	2.50095e-16
C2920	\mem_reg[4]/SEb#11	VSS	8.19039e-17
C2921	\mem_reg[1]/SEb#10	VSS	1.28767e-16
C2922	\mem_reg[1]/SEb#14	VSS	2.26122e-16
C2923	\mem_reg[1]/SEb#16	VSS	2.60034e-16
C2924	\mem_reg[1]/SEb#11	VSS	8.12246e-17
C2925	\mem_reg[3]/SEb#10	VSS	1.4842e-16
C2926	\mem_reg[3]/SEb#16	VSS	2.59708e-16
C2927	\mem_reg[3]/SEb#14	VSS	2.48027e-16
C2928	\mem_reg[3]/SEb#11	VSS	8.73542e-17
C2929	VDD#1	VSS	1.54574e-16
C2930	VDD#103	VSS	1.50853e-16
C2931	mux#7	VSS	2.49745e-20
C2932	n_15#20	VSS	1.15465e-16
C2933	VDD#222	VSS	1.99584e-16
C2934	VDD#55	VSS	2.07042e-16
C2935	VDD#273	VSS	1.90372e-16
C2936	VDD#135	VSS	2.22354e-16
C2937	g510__3680/net89#5	VSS	2.93785e-19
C2938	mem[10]#28	VSS	7.78297e-20
C2939	mux#3	VSS	3.35854e-19
C2940	n_5#13	VSS	1.90157e-16
C2941	UNCONNECTED1#4	VSS	9.78601e-17
C2942	\mem_reg[10]/qbint#22	VSS	1.46037e-20
C2943	VDD#56	VSS	2.04842e-16
C2944	VDD#136	VSS	1.98647e-16
C2945	g510__3680/net89#3	VSS	1.47115e-18
C2946	g494__5107/n0#9	VSS	1.11195e-16
C2947	VDD#390	VSS	1.94469e-16
C2948	VDD#274	VSS	1.94346e-16
C2949	VDD#223	VSS	1.14407e-16
C2950	q#23	VSS	1.78481e-19
C2951	VDD#224	VSS	2.238e-16
C2952	n_12#29	VSS	1.01427e-16
C2953	mem[2]#31	VSS	1.47692e-16
C2954	VDD#58	VSS	1.97381e-16
C2955	VDD#57	VSS	1.57665e-16
C2956	VDD#275	VSS	1.30042e-16
C2957	g493__2398/n0#32	VSS	1.54249e-16
C2958	VDD#225	VSS	1.02256e-16
C2959	q#19	VSS	2.35147e-19
C2960	g494__5107/S0b#9	VSS	8.61383e-17
C2961	VDD#391	VSS	1.06163e-16
C2962	VDD#278	VSS	1.07284e-16
C2963	UNCONNECTED7#4	VSS	1.2227e-16
C2964	VDD#59	VSS	2.01856e-16
C2965	VDD#137	VSS	1.6368e-16
C2966	\mem_reg[2]/net367#11	VSS	1.19468e-16
C2967	VDD#226	VSS	2.0345e-16
C2968	VDD#60	VSS	1.96274e-16
C2969	q#15	VSS	1.06744e-19
C2970	g493__2398/S0b#12	VSS	7.29698e-17
C2971	mem[15]#31	VSS	1.48601e-16
C2972	VDD#277	VSS	2.03284e-16
C2973	VDD#61	VSS	2.1965e-16
C2974	VDD#392	VSS	9.87163e-17
C2975	VDD#281	VSS	1.01094e-16
C2976	VDD#227	VSS	1.3592e-16
C2977	q#11	VSS	1.15129e-19
C2978	UNCONNECTED#4	VSS	1.26536e-16
C2979	\mem_reg[0]/n30#8	VSS	1.09438e-20
C2980	\mem_reg[2]/n30#11	VSS	1.09624e-16
C2981	VDD#63	VSS	2.04052e-16
C2982	VDD#228	VSS	1.11051e-16
C2983	VDD#62	VSS	1.0927e-16
C2984	VDD#138	VSS	2.02471e-16
C2985	\mem_reg[2]/mout#11	VSS	1.15682e-16
C2986	VDD#229	VSS	1.53295e-16
C2987	q#7	VSS	1.97577e-20
C2988	\mem_reg[15]/net367#11	VSS	1.19213e-16
C2989	mux_control_reg/net367#8	VSS	5.3943e-21
C2990	FE_OFN0_q#35	VSS	1.479e-16
C2991	VDD#280	VSS	1.11165e-16
C2992	VDD#393	VSS	1.36671e-16
C2993	VDD#285	VSS	1.31632e-16
C2994	VDD#64	VSS	2.09799e-16
C2995	\mem_reg[8]/net367#8	VSS	3.23102e-19
C2996	VDD#230	VSS	1.05279e-16
C2997	VDD#65	VSS	1.0362e-16
C2998	\mem_reg[10]/n20#8	VSS	2.00643e-19
C2999	q#3	VSS	3.9968e-19
C3000	\mem_reg[2]/CKbb#20	VSS	9.06194e-17
C3001	VDD#139	VSS	1.14021e-16
C3002	VDD#284	VSS	1.03127e-16
C3003	\mem_reg[10]/Db#5	VSS	1.50703e-19
C3004	VDD#66	VSS	1.9894e-16
C3005	VDD#394	VSS	1.49584e-16
C3006	VDD#289	VSS	1.52243e-16
C3007	\mem_reg[15]/n30#11	VSS	1.10598e-16
C3008	mux_control_reg/n30#8	VSS	3.23138e-20
C3009	q_reg_reg/net367#11	VSS	1.20518e-16
C3010	FE_OFC0_q/n0#120	VSS	5.59491e-19
C3011	\mem_reg[15]/mout#11	VSS	1.10974e-16
C3012	mux_control_reg/mout#8	VSS	7.10482e-20
C3013	\mem_reg[8]/n30#8	VSS	2.41731e-19
C3014	VDD#140	VSS	1.07045e-16
C3015	VDD#67	VSS	2.03395e-16
C3016	\mem_reg[2]/n20#11	VSS	1.10382e-16
C3017	VDD#231	VSS	1.39609e-16
C3018	VDD#232	VSS	1.33824e-16
C3019	VDD#68	VSS	1.34142e-16
C3020	\mem_reg[8]/mout#8	VSS	4.5251e-20
C3021	FE_OFC0_q/n0#116	VSS	6.01845e-19
C3022	\mem_reg[2]/Db#8	VSS	9.65049e-17
C3023	VDD#288	VSS	1.35363e-16
C3024	q_reg_reg/n30#11	VSS	1.13805e-16
C3025	\mem_reg[15]/CKbb#20	VSS	8.78612e-17
C3026	VDD#69	VSS	1.54062e-16
C3027	\mem_reg[2]/CKb#25	VSS	9.39605e-17
C3028	VDD#233	VSS	1.53792e-16
C3029	VDD#70	VSS	1.54533e-16
C3030	q_reg_reg/mout#11	VSS	1.1224e-16
C3031	VDD#395	VSS	1.30914e-16
C3032	VDD#297	VSS	1.38189e-16
C3033	VDD#141	VSS	1.32874e-16
C3034	VDD#291	VSS	1.52673e-16
C3035	UNCONNECTED3#4	VSS	1.1923e-16
C3036	VDD#234	VSS	1.3804e-16
C3037	\mem_reg[15]/n20#11	VSS	1.11484e-16
C3038	mux_control_reg/n20#8	VSS	7.10482e-20
C3039	q_reg_reg/CKbb#20	VSS	9.49186e-17
C3040	VDD#71	VSS	2.02117e-16
C3041	\mem_reg[10]/SEb#9	VSS	3.18385e-19
C3042	\mem_reg[2]/Db#4	VSS	1.0984e-16
C3043	\mem_reg[15]/Db#8	VSS	9.65507e-17
C3044	VDD#142	VSS	1.58611e-16
C3045	mem[9]#31	VSS	1.45542e-16
C3046	VDD#235	VSS	2.10768e-16
C3047	\mem_reg[15]/CKb#25	VSS	9.24704e-17
C3048	VDD#396	VSS	1.40928e-16
C3049	VDD#298	VSS	1.31322e-16
C3050	VDD#236	VSS	1.28359e-16
C3051	VDD#72	VSS	1.33366e-16
C3052	q_reg_reg/n20#11	VSS	1.11269e-16
C3053	VDD#73	VSS	1.13841e-16
C3054	\mem_reg[0]/SEb#9	VSS	5.10682e-19
C3055	\mem_reg[2]/SEb#12	VSS	7.07538e-17
C3056	n_14#11	VSS	1.88042e-16
C3057	q_reg_reg/Db#8	VSS	9.76325e-17
C3058	VDD#296	VSS	1.37072e-16
C3059	\mem_reg[9]/net367#11	VSS	1.18302e-16
C3060	\mem_reg[15]/Db#4	VSS	1.09881e-16
C3061	q_reg_reg/CKb#25	VSS	9.75998e-17
C3062	VDD#74	VSS	1.04282e-16
C3063	VDD#237	VSS	2.07782e-16
C3064	g509__6783/net89#5	VSS	6.06746e-19
C3065	VDD#143	VSS	1.42168e-16
C3066	VDD#397	VSS	2.01345e-16
C3067	VDD#238	VSS	1.40158e-16
C3068	VDD#75	VSS	1.34211e-16
C3069	n_14#9	VSS	1.89989e-16
C3070	n_6#13	VSS	1.96886e-16
C3071	\mem_reg[9]/n30#11	VSS	1.14374e-16
C3072	\mem_reg[15]/SEb#12	VSS	7.94602e-17
C3073	mux_control_reg/SEb#9	VSS	9.59241e-20
C3074	VDD#300	VSS	1.33239e-16
C3075	g509__6783/net89#3	VSS	1.50413e-18
C3076	q_reg_reg/Db#4	VSS	1.11084e-16
C3077	\mem_reg[9]/mout#11	VSS	1.17379e-16
C3078	\mem_reg[8]/SEb#9	VSS	1.31231e-18
C3079	VDD#239	VSS	1.54137e-16
C3080	VDD#304	VSS	2.18483e-16
C3081	VDD#76	VSS	1.3232e-16
C3082	VDD#398	VSS	1.04637e-16
C3083	UNCONNECTED12#4	VSS	1.24668e-16
C3084	VDD#144	VSS	1.41971e-16
C3085	VDD#303	VSS	2.03345e-16
C3086	\mem_reg[9]/CKbb#20	VSS	8.98415e-17
C3087	VDD#240	VSS	1.97094e-16
C3088	VDD#77	VSS	2.04482e-16
C3089	q_reg_reg/SEb#12	VSS	7.89172e-17
C3090	VDD#241	VSS	1.79777e-16
C3091	VDD#78	VSS	1.52716e-16
C3092	mem[6]#31	VSS	1.47037e-16
C3093	VDD#399	VSS	9.87897e-17
C3094	n_11#16	VSS	1.80265e-16
C3095	n_13#11	VSS	1.86555e-16
C3096	UNCONNECTED11#4	VSS	1.23765e-16
C3097	VDD#242	VSS	1.61499e-16
C3098	VDD#308	VSS	1.8468e-16
C3099	\mem_reg[9]/n20#11	VSS	1.11165e-16
C3100	\mem_reg[13]/n30#8	VSS	1.09438e-20
C3101	VDD#243	VSS	1.1836e-16
C3102	VDD#79	VSS	1.15027e-16
C3103	VDD#145	VSS	2.00726e-16
C3104	VDD#307	VSS	2.07975e-16
C3105	\mem_reg[9]/Db#8	VSS	9.69146e-17
C3106	\mem_reg[6]/net367#11	VSS	1.17857e-16
C3107	\mem_reg[11]/net367#8	VSS	3.12518e-19
C3108	q_control#36	VSS	1.48046e-16
C3109	n_2#29	VSS	1.179e-16
C3110	n_13#9	VSS	1.89495e-16
C3111	g519__1705/n0#11	VSS	1.10846e-16
C3112	VDD#400	VSS	1.36671e-16
C3113	\mem_reg[9]/CKb#25	VSS	9.23704e-17
C3114	VDD#244	VSS	1.01932e-16
C3115	VDD#80	VSS	1.06227e-16
C3116	q_control_reg/qbint#23	VSS	1.9942e-19
C3117	VDD#245	VSS	1.52662e-16
C3118	VDD#81	VSS	1.36701e-16
C3119	VDD#146	VSS	1.17521e-16
C3120	VDD#312	VSS	1.61062e-16
C3121	VDD#315	VSS	1.27834e-16
C3122	VDD#401	VSS	1.50996e-16
C3123	\mem_reg[6]/n30#11	VSS	1.10272e-16
C3124	\mem_reg[11]/n30#8	VSS	4.46454e-19
C3125	q_control_reg/net367#11	VSS	1.20619e-16
C3126	g519__1705/S0b#11	VSS	8.89791e-17
C3127	\mem_reg[9]/Db#4	VSS	1.11099e-16
C3128	g497__8428/net118#6	VSS	1.68804e-16
C3129	\mem_reg[6]/mout#11	VSS	1.11966e-16
C3130	\mem_reg[11]/mout#8	VSS	4.07849e-19
C3131	VDD#147	VSS	1.05601e-16
C3132	n_8#36	VSS	4.19537e-19
C3133	n_10#10	VSS	1.0145e-18
C3134	VDD#246	VSS	1.31952e-16
C3135	VDD#82	VSS	1.33443e-16
C3136	VDD#83	VSS	1.38627e-16
C3137	g497__8428/net118#4	VSS	1.77662e-16
C3138	n_0#12	VSS	9.39972e-19
C3139	q_control_reg/n30#11	VSS	1.14408e-16
C3140	\mem_reg[6]/CKbb#20	VSS	9.26259e-17
C3141	VDD#314	VSS	1.73805e-16
C3142	\mem_reg[9]/SEb#12	VSS	7.30137e-17
C3143	VDD#321	VSS	1.8843e-16
C3144	g497__8428/net122#6	VSS	1.99879e-16
C3145	VDD#247	VSS	1.53686e-16
C3146	VDD#84	VSS	1.51222e-16
C3147	q_control_reg/mout#11	VSS	1.1675e-16
C3148	g497__8428/net118#2	VSS	2.00621e-16
C3149	VDD#402	VSS	1.32477e-16
C3150	VDD#148	VSS	1.30808e-16
C3151	g497__8428/net122#4	VSS	1.96756e-16
C3152	g520__5122/n0#11	VSS	1.0925e-16
C3153	g498__5526/net118#6	VSS	1.65883e-16
C3154	g522__2802/net89#5	VSS	2.18153e-19
C3155	\mem_reg[6]/n20#11	VSS	1.09816e-16
C3156	\mem_reg[11]/n20#8	VSS	1.42647e-19
C3157	q_control_reg/CKbb#20	VSS	9.10558e-17
C3158	VDD#248	VSS	2.15565e-16
C3159	n_9#10	VSS	1.66972e-18
C3160	n_3#11	VSS	1.88538e-16
C3161	\mem_reg[6]/Db#8	VSS	9.60628e-17
C3162	VDD#149	VSS	1.52282e-16
C3163	g497__8428/net122#2	VSS	1.36204e-16
C3164	VDD#322	VSS	1.25488e-16
C3165	g498__5526/net118#4	VSS	1.65759e-16
C3166	g522__2802/net89#3	VSS	1.16665e-18
C3167	\mem_reg[6]/CKb#25	VSS	9.03357e-17
C3168	g520__5122/S0b#11	VSS	8.66759e-17
C3169	VDD#403	VSS	1.32481e-16
C3170	VDD#85	VSS	2.17018e-16
C3171	g498__5526/net122#6	VSS	1.85561e-16
C3172	VDD#249	VSS	1.36859e-16
C3173	VDD#86	VSS	1.38896e-16
C3174	q_control_reg/n20#11	VSS	1.10725e-16
C3175	g498__5526/net118#2	VSS	2.02163e-16
C3176	q_control_reg/Db#8	VSS	9.7335e-17
C3177	g521__1617/net89#6	VSS	1.68537e-16
C3178	UNCONNECTED0#4	VSS	1.2366e-16
C3179	g498__5526/net122#4	VSS	2.08444e-16
C3180	\mem_reg[6]/Db#4	VSS	1.10525e-16
C3181	n_4#10	VSS	1.89071e-16
C3182	q_control_reg/CKb#25	VSS	9.79091e-17
C3183	VDD#323	VSS	2.00388e-16
C3184	n_7#31	VSS	3.49325e-19
C3185	VDD#320	VSS	2.16903e-16
C3186	g521__1617/net89#4	VSS	1.90035e-16
C3187	VDD#150	VSS	1.45092e-16
C3188	mem[5]#29	VSS	1.46831e-16
C3189	VDD#404	VSS	1.98197e-16
C3190	g498__5526/net122#2	VSS	1.4961e-16
C3191	VDD#251	VSS	1.39982e-16
C3192	VDD#250	VSS	2.19432e-16
C3193	VDD#87	VSS	1.41988e-16
C3194	VDD#88	VSS	1.73325e-16
C3195	\mem_reg[6]/SEb#12	VSS	7.80733e-17
C3196	g521__1617/net89#2	VSS	1.2372e-16
C3197	q_control_reg/Db#4	VSS	1.1031e-16
C3198	VDD#328	VSS	1.1595e-16
C3199	\mem_reg[5]/net367#11	VSS	1.19404e-16
C3200	VDD#405	VSS	1.06163e-16
C3201	VDD#327	VSS	2.12521e-16
C3202	UNCONNECTED4#4	VSS	1.23176e-16
C3203	UNCONNECTED10#4	VSS	1.1376e-16
C3204	VDD#151	VSS	1.39208e-16
C3205	VDD#253	VSS	2.0741e-16
C3206	VDD#252	VSS	2.08469e-16
C3207	VDD#90	VSS	1.9729e-16
C3208	VDD#89	VSS	2.00858e-16
C3209	VDD#332	VSS	1.00622e-16
C3210	q_control_reg/SEb#12	VSS	8.37935e-17
C3211	mem[12]#29	VSS	1.46211e-16
C3212	mem[1]#29	VSS	1.47512e-16
C3213	VDD#406	VSS	1.00893e-16
C3214	\mem_reg[14]/qbint#22	VSS	2.3252e-20
C3215	VDD#152	VSS	1.58272e-16
C3216	\mem_reg[5]/n30#11	VSS	1.09594e-16
C3217	VDD#331	VSS	1.11196e-16
C3218	scan_out[0]#31	VSS	3.62973e-19
C3219	\mem_reg[5]/mout#11	VSS	1.089e-16
C3220	\mem_reg[7]/n30#8	VSS	2.15999e-19
C3221	VDD#255	VSS	1.11632e-16
C3222	VDD#254	VSS	1.15738e-16
C3223	VDD#92	VSS	1.15802e-16
C3224	VDD#91	VSS	1.16335e-16
C3225	VDD#163	VSS	2.09443e-16
C3226	VDD#336	VSS	1.30744e-16
C3227	\mem_reg[7]/mout#8	VSS	2.05056e-19
C3228	\mem_reg[12]/net367#11	VSS	1.17695e-16
C3229	\mem_reg[1]/net367#11	VSS	1.17965e-16
C3230	\mem_reg[3]/net367#8	VSS	1.33744e-19
C3231	VDD#335	VSS	1.02852e-16
C3232	scan_out[0]#27	VSS	2.38899e-19
C3233	VDD#407	VSS	1.33126e-16
C3234	\mem_reg[5]/CKbb#20	VSS	8.87036e-17
C3235	VDD#257	VSS	1.01377e-16
C3236	VDD#256	VSS	1.08308e-16
C3237	VDD#94	VSS	1.0621e-16
C3238	VDD#93	VSS	1.06283e-16
C3239	VDD#164	VSS	2.11162e-16
C3240	VDD#340	VSS	1.46759e-16
C3241	scan_out[0]#23	VSS	1.15129e-19
C3242	VDD#408	VSS	1.49584e-16
C3243	\mem_reg[12]/n30#11	VSS	1.09659e-16
C3244	\mem_reg[4]/n30#8	VSS	9.09923e-19
C3245	\mem_reg[1]/n30#11	VSS	1.12531e-16
C3246	\mem_reg[3]/n30#8	VSS	3.36961e-19
C3247	VDD#165	VSS	2.1152e-16
C3248	\mem_reg[5]/n20#11	VSS	1.09815e-16
C3249	VDD#339	VSS	1.35669e-16
C3250	\mem_reg[12]/mout#11	VSS	1.11045e-16
C3251	\mem_reg[4]/mout#8	VSS	9.09923e-19
C3252	\mem_reg[1]/mout#11	VSS	1.13958e-16
C3253	\mem_reg[3]/mout#8	VSS	4.46454e-19
C3254	scan_out[0]#19	VSS	1.15129e-19
C3255	\mem_reg[5]/Db#8	VSS	9.57241e-17
C3256	VDD#259	VSS	1.33927e-16
C3257	VDD#258	VSS	1.35596e-16
C3258	VDD#96	VSS	1.33058e-16
C3259	VDD#95	VSS	1.36412e-16
C3260	VDD#166	VSS	2.11345e-16
C3261	\mem_reg[5]/CKb#25	VSS	9.04207e-17
C3262	VDD#342	VSS	1.49399e-16
C3263	\mem_reg[12]/CKbb#20	VSS	8.76838e-17
C3264	\mem_reg[4]/CKbb#17	VSS	3.28134e-19
C3265	\mem_reg[1]/CKbb#20	VSS	8.93408e-17
C3266	scan_out[0]#15	VSS	1.15129e-19
C3267	VDD#349	VSS	1.37704e-16
C3268	VDD#261	VSS	1.54251e-16
C3269	VDD#260	VSS	1.54979e-16
C3270	VDD#98	VSS	1.52132e-16
C3271	VDD#97	VSS	1.46833e-16
C3272	VDD#167	VSS	2.12413e-16
C3273	VDD#409	VSS	1.33771e-16
C3274	\mem_reg[14]/n20#8	VSS	6.38193e-20
C3275	scan_out[0]#11	VSS	1.15129e-19
C3276	\mem_reg[5]/Db#4	VSS	1.09229e-16
C3277	\mem_reg[12]/n20#11	VSS	1.09919e-16
C3278	\mem_reg[4]/n20#8	VSS	9.34521e-19
C3279	\mem_reg[1]/n20#11	VSS	1.10069e-16
C3280	\mem_reg[3]/n20#8	VSS	2.58751e-19
C3281	VDD#168	VSS	2.07237e-16
C3282	\mem_reg[12]/Db#8	VSS	9.60418e-17
C3283	\mem_reg[4]/Db#5	VSS	8.17292e-19
C3284	\mem_reg[1]/Db#8	VSS	9.65317e-17
C3285	\mem_reg[3]/Db#5	VSS	3.16919e-20
C3286	VDD#350	VSS	1.4084e-16
C3287	VDD#348	VSS	1.31232e-16
C3288	VDD#169	VSS	1.98268e-16
C3289	\mem_reg[12]/CKb#25	VSS	9.47225e-17
C3290	\mem_reg[1]/CKb#25	VSS	9.37411e-17
C3291	\mem_reg[5]/SEb#12	VSS	6.31058e-17
C3292	VDD#410	VSS	1.33497e-16
C3293	VDD#263	VSS	1.39726e-16
C3294	VDD#262	VSS	1.42392e-16
C3295	VDD#100	VSS	1.38465e-16
C3296	VDD#99	VSS	1.40917e-16
C3297	VDD#170	VSS	2.11006e-16
C3298	\mem_reg[12]/Db#4	VSS	1.09309e-16
C3299	\mem_reg[1]/Db#4	VSS	1.09309e-16
C3300	FE_OFC1_scan_out_0/n0#120	VSS	9.01039e-19
C3301	VDD#352	VSS	1.36201e-16
C3302	VDD#171	VSS	2.08389e-16
C3303	VDD#265	VSS	1.40626e-16
C3304	VDD#264	VSS	1.43828e-16
C3305	VDD#102	VSS	1.39464e-16
C3306	VDD#101	VSS	1.33877e-16
C3307	FE_OFC1_scan_out_0/n0#116	VSS	1.46009e-19
C3308	\mem_reg[12]/SEb#12	VSS	8.08048e-17
C3309	\mem_reg[4]/SEb#9	VSS	6.47236e-18
C3310	\mem_reg[1]/SEb#12	VSS	8.03457e-17
C3311	\mem_reg[3]/SEb#9	VSS	6.53225e-18
C3312	VDD#162	VSS	1.09719e-16
C3313	VDD#357	VSS	1.40746e-16
C3314	VDD#271	VSS	2.08417e-16
C3315	VDD#175	VSS	9.73532e-17
C3316	VDD#4	VSS	1.59092e-16
C3317	VDD#107	VSS	2.2599e-17
C3318	UNCONNECTED15#4	VSS	7.70106e-17
C3319	mux#6	VSS	1.11562e-16
C3320	mem[10]#31	VSS	1.13543e-16
C3321	mux#2	VSS	1.39982e-16
C3322	UNCONNECTED9#4	VSS	7.76964e-17
C3323	\mem_reg[10]/qbint#29	VSS	5.23758e-17
C3324	n_5#11	VSS	1.47156e-16
C3325	g494__5107/n0#11	VSS	8.18237e-17
C3326	q#21	VSS	2.24547e-18
C3327	n_12#30	VSS	1.05923e-20
C3328	mem[0]#31	VSS	1.11543e-16
C3329	\mem_reg[10]/net367#11	VSS	1.1155e-16
C3330	g493__2398/n0#31	VSS	1.0981e-16
C3331	\mem_reg[0]/qbint#29	VSS	5.06365e-17
C3332	\mem_reg[2]/qbint#29	VSS	5.05512e-17
C3333	g494__5107/S0b#11	VSS	5.44007e-17
C3334	UNCONNECTED2#4	VSS	9.38814e-17
C3335	\mem_reg[0]/net367#11	VSS	1.10075e-16
C3336	\mem_reg[10]/n30#11	VSS	8.84381e-17
C3337	UNCONNECTED14#4	VSS	9.27222e-17
C3338	g493__2398/S0b#9	VSS	4.83694e-19
C3339	\mem_reg[10]/mout#11	VSS	8.99144e-17
C3340	FE_OFN1_scan_out_0#58	VSS	1.12606e-16
C3341	mem[8]#29	VSS	1.13903e-16
C3342	\mem_reg[15]/qbint#29	VSS	5.15679e-17
C3343	mux_control_reg/qbint#29	VSS	5.40575e-17
C3344	\mem_reg[0]/n30#11	VSS	8.75994e-17
C3345	\mem_reg[10]/CKbb#20	VSS	5.65597e-17
C3346	\mem_reg[8]/qbint#29	VSS	5.27755e-17
C3347	\mem_reg[0]/mout#11	VSS	8.76586e-17
C3348	mux_control_reg/net367#11	VSS	1.11513e-16
C3349	FE_OFN0_q#32	VSS	9.54882e-19
C3350	\mem_reg[8]/net367#11	VSS	1.12137e-16
C3351	q_reg_reg/qbint#29	VSS	5.3161e-17
C3352	\mem_reg[10]/n20#11	VSS	8.11631e-17
C3353	\mem_reg[0]/CKbb#20	VSS	5.44719e-17
C3354	\mem_reg[10]/Db#8	VSS	6.52545e-17
C3355	mux_control_reg/n30#11	VSS	8.88887e-17
C3356	q_reg_reg/net367#8	VSS	1.16143e-18
C3357	\mem_reg[10]/CKb#25	VSS	7.01336e-17
C3358	mux_control_reg/mout#11	VSS	8.87824e-17
C3359	\mem_reg[8]/n30#11	VSS	8.78051e-17
C3360	\mem_reg[0]/n20#11	VSS	8.11841e-17
C3361	\mem_reg[8]/mout#11	VSS	8.9713e-17
C3362	\mem_reg[0]/Db#8	VSS	6.33904e-17
C3363	mux_control_reg/CKbb#20	VSS	5.41034e-17
C3364	\mem_reg[10]/Db#4	VSS	8.66634e-17
C3365	\mem_reg[0]/CKb#25	VSS	6.90146e-17
C3366	\mem_reg[8]/CKbb#20	VSS	5.6423e-17
C3367	\mem_reg[15]/n20#8	VSS	1.58082e-19
C3368	mux_control_reg/n20#11	VSS	8.13073e-17
C3369	q_reg_reg/CKbb#17	VSS	2.50816e-22
C3370	\mem_reg[10]/SEb#12	VSS	5.86775e-17
C3371	\mem_reg[0]/Db#4	VSS	8.1394e-17
C3372	mux_control_reg/Db#8	VSS	6.47362e-17
C3373	\mem_reg[8]/n20#11	VSS	8.00804e-17
C3374	\mem_reg[8]/Db#8	VSS	6.41368e-17
C3375	mux_control_reg/CKb#25	VSS	6.96205e-17
C3376	\mem_reg[9]/qbint#29	VSS	5.03968e-17
C3377	n_14#13	VSS	1.38056e-16
C3378	\mem_reg[8]/CKb#25	VSS	7.14172e-17
C3379	\mem_reg[0]/SEb#12	VSS	5.70514e-17
C3380	\mem_reg[2]/SEb#9	VSS	1.33026e-18
C3381	g495__6260/n0#3	VSS	2.05697e-19
C3382	g495__6260/n1#4	VSS	1.41452e-16
C3383	mux_control_reg/Db#4	VSS	8.0406e-17
C3384	UNCONNECTED5#4	VSS	9.55795e-17
C3385	\mem_reg[8]/Db#4	VSS	8.26759e-17
C3386	mem[13]#29	VSS	1.10272e-16
C3387	mux_control_reg/SEb#12	VSS	5.35707e-17
C3388	g495__6260/n1#2	VSS	1.35848e-16
C3389	n_6#11	VSS	1.47451e-16
C3390	\mem_reg[13]/qbint#29	VSS	5.36756e-17
C3391	\mem_reg[8]/SEb#12	VSS	5.92072e-17
C3392	UNCONNECTED16#4	VSS	9.59937e-17
C3393	\mem_reg[13]/net367#11	VSS	1.11361e-16
C3394	n_11#17	VSS	2.68379e-20
C3395	n_13#13	VSS	1.37297e-16
C3396	mem[11]#29	VSS	1.13111e-16
C3397	n_1#11	VSS	8.04003e-17
C3398	\mem_reg[6]/qbint#29	VSS	5.02572e-17
C3399	\mem_reg[11]/qbint#29	VSS	5.37173e-17
C3400	g496__4319/n1#4	VSS	1.41156e-16
C3401	\mem_reg[13]/n30#11	VSS	8.89672e-17
C3402	\mem_reg[13]/mout#11	VSS	8.95489e-17
C3403	\mem_reg[11]/net367#11	VSS	1.11418e-16
C3404	g519__1705/n0#9	VSS	8.12629e-17
C3405	q_control_reg/qbint#29	VSS	5.20094e-17
C3406	g496__4319/n1#2	VSS	1.37801e-16
C3407	\mem_reg[13]/CKbb#20	VSS	5.63841e-17
C3408	\mem_reg[11]/n30#11	VSS	8.95344e-17
C3409	q_control_reg/net367#8	VSS	2.21688e-19
C3410	n_10#13	VSS	8.26678e-17
C3411	\mem_reg[11]/mout#11	VSS	8.85926e-17
C3412	\mem_reg[13]/n20#11	VSS	8.17015e-17
C3413	n_8#33	VSS	1.51579e-20
C3414	\mem_reg[13]/Db#8	VSS	6.34556e-17
C3415	n_0#11	VSS	8.67268e-17
C3416	\mem_reg[11]/CKbb#20	VSS	5.6214e-17
C3417	\mem_reg[13]/CKb#25	VSS	7.35162e-17
C3418	g520__5122/n0#9	VSS	7.97703e-17
C3419	n_9#13	VSS	8.19233e-17
C3420	\mem_reg[6]/n20#8	VSS	6.82853e-20
C3421	\mem_reg[11]/n20#11	VSS	8.2872e-17
C3422	\mem_reg[13]/Db#4	VSS	8.07019e-17
C3423	\mem_reg[11]/Db#8	VSS	6.46136e-17
C3424	n_3#9	VSS	1.48841e-16
C3425	\mem_reg[11]/CKb#25	VSS	7.05847e-17
C3426	q_control_reg/n20#8	VSS	2.2661e-20
C3427	\mem_reg[13]/SEb#12	VSS	5.25827e-17
C3428	\mem_reg[11]/Db#4	VSS	8.3425e-17
C3429	UNCONNECTED13#4	VSS	9.59198e-17
C3430	n_4#8	VSS	1.49494e-16
C3431	n_7#30	VSS	1.59854e-16
C3432	mem[7]#27	VSS	1.10371e-16
C3433	\mem_reg[5]/qbint#29	VSS	5.05808e-17
C3434	\mem_reg[6]/SEb#9	VSS	3.41396e-20
C3435	\mem_reg[11]/SEb#12	VSS	5.35501e-17
C3436	\mem_reg[7]/qbint#29	VSS	5.62512e-17
C3437	UNCONNECTED6#4	VSS	9.23299e-17
C3438	UNCONNECTED17#4	VSS	9.12903e-17
C3439	UNCONNECTED8#4	VSS	9.54833e-17
C3440	\mem_reg[7]/net367#11	VSS	1.10965e-16
C3441	mem[14]#29	VSS	1.12663e-16
C3442	q_control_reg/SEb#9	VSS	3.14609e-18
C3443	mem[4]#27	VSS	1.10639e-16
C3444	mem[1]#26	VSS	2.2198e-20
C3445	mem[3]#29	VSS	1.12905e-16
C3446	\mem_reg[14]/qbint#29	VSS	5.06493e-17
C3447	\mem_reg[5]/n30#8	VSS	1.04968e-19
C3448	\mem_reg[12]/qbint#29	VSS	5.08054e-17
C3449	\mem_reg[4]/qbint#29	VSS	4.91481e-17
C3450	\mem_reg[1]/qbint#29	VSS	5.09298e-17
C3451	\mem_reg[3]/qbint#29	VSS	5.27286e-17
C3452	\mem_reg[5]/mout#8	VSS	1.71025e-19
C3453	\mem_reg[7]/n30#11	VSS	8.75419e-17
C3454	\mem_reg[14]/net367#11	VSS	1.09449e-16
C3455	\mem_reg[7]/mout#11	VSS	8.88039e-17
C3456	\mem_reg[4]/net367#11	VSS	1.09182e-16
C3457	\mem_reg[1]/net367#8	VSS	2.19313e-20
C3458	\mem_reg[3]/net367#11	VSS	1.11351e-16
C3459	scan_out[0]#25	VSS	2.61687e-19
C3460	\mem_reg[14]/n30#11	VSS	9.10807e-17
C3461	\mem_reg[7]/CKbb#20	VSS	5.6302e-17
C3462	scan_out[0]#21	VSS	1.15129e-19
C3463	\mem_reg[14]/mout#11	VSS	8.95514e-17
C3464	\mem_reg[4]/n30#11	VSS	8.82781e-17
C3465	\mem_reg[1]/n30#8	VSS	1.08992e-18
C3466	\mem_reg[3]/n30#11	VSS	9.13398e-17
C3467	\mem_reg[4]/mout#11	VSS	8.83419e-17
C3468	\mem_reg[1]/mout#8	VSS	1.17618e-18
C3469	\mem_reg[3]/mout#11	VSS	9.1061e-17
C3470	scan_out[0]#17	VSS	1.15129e-19
C3471	\mem_reg[7]/n20#11	VSS	8.16057e-17
C3472	\mem_reg[14]/CKbb#20	VSS	5.72488e-17
C3473	\mem_reg[7]/Db#8	VSS	6.33664e-17
C3474	\mem_reg[4]/CKbb#20	VSS	5.52626e-17
C3475	\mem_reg[1]/CKbb#17	VSS	8.73885e-19
C3476	\mem_reg[3]/CKbb#20	VSS	5.68107e-17
C3477	scan_out[0]#13	VSS	1.15129e-19
C3478	\mem_reg[7]/CKb#25	VSS	7.22918e-17
C3479	\mem_reg[14]/n20#11	VSS	8.83982e-17
C3480	scan_out[0]#9	VSS	1.15129e-19
C3481	\mem_reg[14]/Db#8	VSS	6.42534e-17
C3482	\mem_reg[4]/n20#11	VSS	8.20795e-17
C3483	\mem_reg[1]/n20#8	VSS	1.17618e-18
C3484	\mem_reg[3]/n20#11	VSS	8.46652e-17
C3485	\mem_reg[7]/Db#4	VSS	8.14633e-17
C3486	\mem_reg[4]/Db#8	VSS	6.42366e-17
C3487	\mem_reg[1]/Db#5	VSS	3.58501e-19
C3488	\mem_reg[3]/Db#8	VSS	6.51525e-17
C3489	\mem_reg[14]/CKb#25	VSS	7.00334e-17
C3490	scan_out[0]#5	VSS	1.15129e-19
C3491	\mem_reg[12]/CKb#22	VSS	9.77905e-21
C3492	\mem_reg[4]/CKb#25	VSS	6.90872e-17
C3493	\mem_reg[1]/CKb#22	VSS	1.40125e-19
C3494	\mem_reg[3]/CKb#25	VSS	7.48554e-17
C3495	\mem_reg[5]/SEb#9	VSS	3.6815e-18
C3496	scan_out[0]#1	VSS	1.15129e-19
C3497	\mem_reg[7]/SEb#12	VSS	4.83616e-17
C3498	\mem_reg[14]/Db#4	VSS	8.43238e-17
C3499	\mem_reg[4]/Db#4	VSS	8.12148e-17
C3500	\mem_reg[3]/Db#4	VSS	8.19465e-17
C3501	FE_OFC1_scan_out_0/n0#118	VSS	4.57542e-19
C3502	\mem_reg[14]/SEb#12	VSS	4.86374e-17
C3503	\mem_reg[4]/SEb#12	VSS	4.83185e-17
C3504	\mem_reg[3]/SEb#12	VSS	5.33504e-17
C3505	\mem_reg[12]/qbint#25	VSS	8.99135e-17
C3506	\mem_reg[4]/qbint#25	VSS	8.99296e-17
C3507	\mem_reg[1]/qbint#25	VSS	9.1919e-17
C3508	\mem_reg[3]/qbint#25	VSS	8.93763e-17
C3509	\mem_reg[14]/qbint#25	VSS	9.35036e-17
C3510	\mem_reg[7]/qbint#25	VSS	9.10515e-17
C3511	\mem_reg[5]/qbint#25	VSS	9.51476e-17
C3512	q_control_reg/qbint#25	VSS	8.98224e-17
C3513	\mem_reg[6]/qbint#25	VSS	9.19741e-17
C3514	\mem_reg[11]/qbint#25	VSS	8.38892e-17
C3515	\mem_reg[13]/qbint#25	VSS	8.54775e-17
C3516	\mem_reg[9]/qbint#25	VSS	9.26788e-17
C3517	q_reg_reg/qbint#25	VSS	9.14382e-17
C3518	\mem_reg[8]/qbint#25	VSS	9.62566e-17
C3519	\mem_reg[15]/qbint#25	VSS	8.96181e-17
C3520	mux_control_reg/qbint#25	VSS	8.81153e-17
C3521	\mem_reg[0]/qbint#25	VSS	9.91548e-17
C3522	\mem_reg[2]/qbint#25	VSS	9.82359e-17
C3523	\mem_reg[10]/qbint#25	VSS	1.05426e-16
C3524	VDD#2	VSS	3.13204e-17
C3525	VDD#6	VSS	4.40176e-17
C3526	VDD#7	VSS	5.5656e-17
C3527	VDD#8	VSS	3.02188e-17
C3528	VDD#9	VSS	2.24648e-17
C3529	VDD#10	VSS	3.73512e-17
C3530	VDD#11	VSS	5.05213e-17
C3531	VDD#13	VSS	3.58694e-17
C3532	VDD#16	VSS	3.10414e-17
C3533	VDD#18	VSS	5.48742e-17
C3534	VDD#20	VSS	5.73236e-17
C3535	VDD#21	VSS	7.60679e-17
C3536	VDD#23	VSS	6.95339e-17
C3537	VDD#24	VSS	4.14166e-17
C3538	VDD#25	VSS	3.21756e-17
C3539	VDD#28	VSS	2.20298e-17
C3540	VDD#29	VSS	2.71362e-17
C3541	VDD#30	VSS	3.94124e-17
C3542	VDD#31	VSS	6.64425e-17
C3543	VDD#32	VSS	3.27345e-17
C3544	VDD#33	VSS	5.97068e-17
C3545	VDD#34	VSS	5.65464e-17
C3546	VDD#35	VSS	2.31901e-17
C3547	VDD#36	VSS	3.11388e-17
C3548	VDD#40	VSS	1.04221e-16
C3549	VDD#41	VSS	6.97322e-17
C3550	VDD#42	VSS	2.14121e-17
C3551	VDD#43	VSS	5.62027e-17
C3552	VDD#46	VSS	2.9199e-17
C3553	VDD#47	VSS	5.81662e-17
C3554	VDD#48	VSS	7.06235e-17
C3555	VDD#49	VSS	5.21001e-17
C3556	VDD#52	VSS	1.09021e-16
C3557	VDD#53	VSS	8.3028e-17
C3558	VDD#104	VSS	3.67178e-17
C3559	VDD#109	VSS	3.6373e-17
C3560	VDD#110	VSS	2.98665e-17
C3561	VDD#113	VSS	1.3648e-17
C3562	VDD#114	VSS	2.82143e-17
C3563	VDD#115	VSS	3.59534e-17
C3564	VDD#116	VSS	2.66672e-17
C3565	VDD#119	VSS	5.5672e-17
C3566	VDD#120	VSS	4.08872e-17
C3567	VDD#121	VSS	3.00523e-17
C3568	VDD#124	VSS	1.35803e-17
C3569	VDD#125	VSS	2.76825e-17
C3570	VDD#126	VSS	3.38213e-17
C3571	VDD#127	VSS	2.48161e-17
C3572	VDD#130	VSS	5.38085e-17
C3573	VDD#131	VSS	3.95555e-17
C3574	VDD#132	VSS	7.07423e-18
C3575	VDD#153	VSS	2.09194e-17
C3576	VDD#154	VSS	2.10718e-17
C3577	VDD#155	VSS	2.09614e-17
C3578	VDD#156	VSS	2.1003e-17
C3579	VDD#159	VSS	1.98975e-17
C3580	VDD#160	VSS	2.11709e-17
C3581	VDD#161	VSS	3.43984e-17
C3582	VDD#173	VSS	4.54104e-17
C3583	VDD#178	VSS	4.83194e-17
C3584	VDD#179	VSS	2.68843e-17
C3585	VDD#180	VSS	5.46841e-17
C3586	VDD#181	VSS	4.20234e-17
C3587	VDD#182	VSS	2.25764e-17
C3588	VDD#183	VSS	2.47821e-17
C3589	VDD#184	VSS	2.69869e-17
C3590	VDD#187	VSS	7.54115e-17
C3591	VDD#188	VSS	6.22457e-17
C3592	VDD#189	VSS	5.93083e-17
C3593	VDD#190	VSS	3.45571e-17
C3594	VDD#191	VSS	6.26465e-17
C3595	VDD#192	VSS	5.65125e-17
C3596	VDD#193	VSS	3.35335e-17
C3597	VDD#196	VSS	3.46983e-17
C3598	VDD#197	VSS	1.65622e-17
C3599	VDD#198	VSS	3.17517e-17
C3600	VDD#199	VSS	3.03071e-17
C3601	VDD#200	VSS	1.78058e-17
C3602	VDD#202	VSS	6.63108e-17
C3603	VDD#203	VSS	8.72773e-17
C3604	VDD#204	VSS	6.23658e-17
C3605	VDD#207	VSS	8.21709e-17
C3606	VDD#208	VSS	8.31449e-17
C3607	VDD#209	VSS	1.00918e-16
C3608	VDD#210	VSS	5.59769e-17
C3609	VDD#213	VSS	2.77324e-17
C3610	VDD#214	VSS	5.35912e-17
C3611	VDD#215	VSS	7.65174e-17
C3612	VDD#216	VSS	5.4891e-17
C3613	VDD#219	VSS	1.07094e-16
C3614	VDD#220	VSS	8.30935e-17
C3615	VDD#267	VSS	4.35642e-17
C3616	VDD#268	VSS	5.54386e-17
C3617	VDD#269	VSS	3.23362e-17
C3618	VDD#276	VSS	5.67131e-17
C3619	VDD#279	VSS	4.97598e-17
C3620	VDD#282	VSS	2.09413e-17
C3621	VDD#283	VSS	2.32984e-17
C3622	VDD#286	VSS	5.40453e-17
C3623	VDD#287	VSS	7.36581e-17
C3624	VDD#290	VSS	5.83637e-17
C3625	VDD#294	VSS	3.14565e-17
C3626	VDD#295	VSS	6.49441e-17
C3627	VDD#299	VSS	7.84786e-17
C3628	VDD#301	VSS	2.16564e-17
C3629	VDD#302	VSS	4.14054e-17
C3630	VDD#305	VSS	6.01054e-17
C3631	VDD#306	VSS	4.25703e-17
C3632	VDD#311	VSS	9.21237e-17
C3633	VDD#316	VSS	1.08151e-16
C3634	VDD#317	VSS	1.10833e-16
C3635	VDD#319	VSS	9.00613e-17
C3636	VDD#326	VSS	2.03326e-17
C3637	VDD#329	VSS	3.43865e-17
C3638	VDD#330	VSS	5.05366e-17
C3639	VDD#334	VSS	3.74683e-17
C3640	VDD#337	VSS	5.40599e-17
C3641	VDD#338	VSS	5.8561e-17
C3642	VDD#341	VSS	3.79372e-17
C3643	VDD#343	VSS	5.26351e-17
C3644	VDD#347	VSS	7.04913e-17
C3645	VDD#351	VSS	5.00872e-17
C3646	VDD#355	VSS	2.85081e-17
C3647	VDD#359	VSS	1.31937e-17
C3648	VDD#360	VSS	2.7885e-17
C3649	VDD#361	VSS	3.62295e-17
C3650	VDD#362	VSS	2.62816e-17
C3651	VDD#365	VSS	5.11127e-17
C3652	VDD#366	VSS	4.00205e-17
C3653	VDD#367	VSS	2.88235e-17
C3654	VDD#370	VSS	1.29066e-17
C3655	VDD#371	VSS	2.8025e-17
C3656	VDD#372	VSS	3.60364e-17
C3657	VDD#373	VSS	2.62816e-17
C3658	VDD#376	VSS	5.01955e-17
C3659	VDD#377	VSS	4.05648e-17
C3660	VDD#378	VSS	3.0168e-17
C3661	VDD#381	VSS	1.31937e-17
C3662	VDD#382	VSS	2.90599e-17
C3663	VDD#383	VSS	3.72134e-17
C3664	VDD#384	VSS	2.62816e-17
C3665	VDD#387	VSS	5.091e-17
C3666	VDD#388	VSS	3.95035e-17
C3667	VDD	VSS	2.20328e-17
C3668	VDD	VSS	2.20328e-17
C3669	scan_out[0]#34	VSS	1.39873e-16
C3670	scan_out[0]#35	VSS	1.03033e-16
C3671	scan_out[0]#36	VSS	1.15235e-16
C3672	scan_out[0]#37	VSS	1.16764e-16
C3673	scan_out[0]#38	VSS	1.18634e-16
C3674	scan_out[0]#39	VSS	1.17705e-16
C3675	scan_out[0]#40	VSS	1.17429e-16
C3676	scan_out[0]#41	VSS	1.1621e-16
C3677	g521__1617/net89#8	VSS	2.61108e-16
C3678	g522__2802/net89#8	VSS	2.70811e-16
C3679	g520__5122/n0#15	VSS	3.08975e-16
C3680	n_10#17	VSS	4.07191e-16
C3681	g519__1705/n0#15	VSS	3.11264e-16
C3682	g509__6783/net89#8	VSS	2.17089e-16
C3683	g493__2398/n0#37	VSS	2.91075e-16
C3684	g510__3680/net89#8	VSS	2.55137e-16
C3685	g494__5107/n0#15	VSS	3.83266e-16
C3686	q#34	VSS	1.42385e-16
C3687	q#35	VSS	1.57628e-16
C3688	q#36	VSS	1.49576e-16
C3689	q#37	VSS	2.10759e-16
C3690	q#38	VSS	1.30849e-16
C3691	q#39	VSS	1.16126e-16
C3692	q#40	VSS	1.43758e-16
C3693	q#41	VSS	1.86336e-16
C3694	scan_clk#202	VSS	6.63626e-16
C3695	scan_en#349	VSS	5.76122e-16
C3696	scan_en#354	VSS	7.11286e-16
C3697	scan_clk#215	VSS	2.13736e-16
C3698	scan_clk#216	VSS	4.94933e-16
C3699	mem[7]#49	VSS	4.95315e-16
C3700	addr[1]#95	VSS	6.11287e-16
C3701	mem[13]#44	VSS	8.9691e-16
C3702	FE_OFN1_scan_out_0#75	VSS	4.8846e-16
C3703	n_12#44	VSS	3.40791e-16
C3704	n_12#45	VSS	1.14816e-15
C3705	n_11#32	VSS	8.87896e-16
C3706	mem[2]#43	VSS	3.51928e-16
C3707	mem[0]#46	VSS	7.34253e-16
C3708	scan_clk#225	VSS	1.47469e-15
C3709	scan_clk#226	VSS	1.20605e-15
C3710	x#24	VSS	1.85288e-15
C3711	mem[4]#50	VSS	7.39442e-16
C3712	n_8#63	VSS	2.35333e-15
C3713	VDD	VSS	7.65904e-16
C3714	VDD	VSS	9.76588e-16
*
*
.ENDS clb
*

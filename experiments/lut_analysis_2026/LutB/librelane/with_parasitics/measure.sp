* These are based on the input pattern (scan_data) and the order of connection
* from the scan chain to the muxes (mux order).
*
* TODO(aryap): the std-cell comparison model needs the bypass mux at the output
* (and before the output register).

.option measure use_lttm=1

* Have to count rises across MUX_OUT to figure this one:
.param first=5

*.measure tran d0 trig v(xtop:a0)={half_vdd} fall=1 targ v(xtop:xlut:z)={half_vdd} fall={first}
*.measure tran d1 trig v(xtop:a0)={half_vdd} fall=2 targ v(xtop:xlut:z)={half_vdd} rise={first+1}
*.measure tran d2 trig v(xtop:a0)={half_vdd} rise=4 targ v(xtop:xlut:z)={half_vdd} fall={first+1}

*.measure tran  a0 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:xlut:n_1)=0.9 fall=1 from={test_start}
*.measure tran  a1 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:xlut:n_2)=0.9 rise=1 from={test_start}

.measure tran  d0 trig v(xtop:a0)=0.9 cross=1 targ v(xtop:mux_out)=0.9 cross=1 from={test_start}
.measure tran  d1 trig v(xtop:a0)=0.9 cross=2 targ v(xtop:mux_out)=0.9 cross=2 from={test_start}
.measure tran  d2 trig v(xtop:a0)=0.9 cross=3 targ v(xtop:mux_out)=0.9 cross=3 from={test_start}
.measure tran  d3 trig v(xtop:a0)=0.9 cross=4 targ v(xtop:mux_out)=0.9 cross=4 from={test_start}
.measure tran  d4 trig v(xtop:a0)=0.9 cross=5 targ v(xtop:mux_out)=0.9 cross=5 from={test_start}
.measure tran  d5 trig v(xtop:a0)=0.9 cross=6 targ v(xtop:mux_out)=0.9 cross=6 from={test_start}
.measure tran  d6 trig v(xtop:a0)=0.9 cross=7 targ v(xtop:mux_out)=0.9 cross=7 from={test_start}
.measure tran  d7 trig v(xtop:a0)=0.9 cross=8 targ v(xtop:mux_out)=0.9 cross=8 from={test_start}
.measure tran  d8 trig v(xtop:a0)=0.9 cross=9 targ v(xtop:mux_out)=0.9 cross=9 from={test_start}
.measure tran  d9 trig v(xtop:a0)=0.9 cross=10 targ v(xtop:mux_out)=0.9 cross=10 from={test_start}
.measure tran  d10 trig v(xtop:a0)=0.9 cross=11 targ v(xtop:mux_out)=0.9 cross=11 from={test_start}
.measure tran  d11 trig v(xtop:a0)=0.9 cross=12 targ v(xtop:mux_out)=0.9 cross=12 from={test_start}
.measure tran  d12 trig v(xtop:a0)=0.9 cross=13 targ v(xtop:mux_out)=0.9 cross=13 from={test_start}
.measure tran  d13 trig v(xtop:a0)=0.9 cross=14 targ v(xtop:mux_out)=0.9 cross=14 from={test_start}
.measure tran  d14 trig v(xtop:a0)=0.9 cross=15 targ v(xtop:mux_out)=0.9 cross=15 from={test_start}
.measure tran  d15 trig v(xtop:a0)=0.9 cross=16 targ v(xtop:mux_out)=0.9 cross=16 from={test_start}

*.param z_rise=4
*.param z_fall=4
*.measure tran  z0 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:xlut:z)=0.9 rise={z_rise+1}
*.measure tran  z1 trig v(xtop:a0)=0.9 fall=1 targ v(xtop:xlut:z)=0.9 fall={z_fall+1}
*.measure tran  z2 trig v(xtop:a0)=0.9 rise=2 targ v(xtop:xlut:z)=0.9 rise={z_rise+2}
*.measure tran  z3 trig v(xtop:a0)=0.9 fall=2 targ v(xtop:xlut:z)=0.9 fall={z_fall+2}
*.measure tran  z4 trig v(xtop:a0)=0.9 rise=3 targ v(xtop:xlut:z)=0.9 rise={z_rise+3}
*.measure tran  z5 trig v(xtop:a0)=0.9 fall=3 targ v(xtop:xlut:z)=0.9 fall={z_fall+3}
*.measure tran  z6 trig v(xtop:a0)=0.9 rise=4 targ v(xtop:xlut:z)=0.9 rise={z_rise+4}
*.measure tran  z7 trig v(xtop:a0)=0.9 fall=4 targ v(xtop:xlut:z)=0.9 fall={z_fall+4}
*.measure tran  z8 trig v(xtop:a0)=0.9 rise=5 targ v(xtop:xlut:z)=0.9 rise={z_rise+5}
*.measure tran  z9 trig v(xtop:a0)=0.9 fall=5 targ v(xtop:xlut:z)=0.9 fall={z_fall+5}
*.measure tran z10 trig v(xtop:a0)=0.9 rise=6 targ v(xtop:xlut:z)=0.9 rise={z_rise+6}
*.measure tran z11 trig v(xtop:a0)=0.9 fall=6 targ v(xtop:xlut:z)=0.9 fall={z_fall+6}
*.measure tran z12 trig v(xtop:a0)=0.9 rise=7 targ v(xtop:xlut:z)=0.9 rise={z_rise+7}
*.measure tran z13 trig v(xtop:a0)=0.9 fall=7 targ v(xtop:xlut:z)=0.9 fall={z_fall+7}
*.measure tran z14 trig v(xtop:a0)=0.9 rise=8 targ v(xtop:xlut:z)=0.9 rise={z_rise+8}
*.measure tran z15 trig v(xtop:a0)=0.9 fall=8 targ v(xtop:xlut:z)=0.9 fall={z_fall+8}

* These are based on the input pattern (scan_data) and the order of connection
* from the scan chain to the muxes (mux order).
*
* TODO(aryap): the std-cell comparison model needs the bypass mux at the output
* (and before the output register).

** Have to count rises across MUX_OUT to figure this one:
*.param first_rise=4
*.param first_fall=5
*
**.measure tran d0 trig v(xtop:a0)={half_vdd} fall=1 targ v(xtop:xlut:z)={half_vdd} fall={first}
**.measure tran d1 trig v(xtop:a0)={half_vdd} fall=2 targ v(xtop:xlut:z)={half_vdd} rise={first+1}
**.measure tran d2 trig v(xtop:a0)={half_vdd} rise=4 targ v(xtop:xlut:z)={half_vdd} fall={first+1}
*
*.measure tran  a0 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:xlut:n_1)=0.9 fall=1 from={test_start}
*.measure tran  a1 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:xlut:n_2)=0.9 rise=1 from={test_start}
*
*.measure tran  d0 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:mux_out)=0.9 rise={first_rise+1}
*.measure tran  d1 trig v(xtop:a0)=0.9 fall=1 targ v(xtop:mux_out)=0.9 fall={first_fall+1}
*.measure tran  d2 trig v(xtop:a0)=0.9 rise=2 targ v(xtop:mux_out)=0.9 rise={first_rise+2}
*.measure tran  d3 trig v(xtop:a0)=0.9 fall=2 targ v(xtop:mux_out)=0.9 fall={first_fall+2}
*.measure tran  d4 trig v(xtop:a0)=0.9 rise=3 targ v(xtop:mux_out)=0.9 rise={first_rise+3}
*.measure tran  d5 trig v(xtop:a0)=0.9 fall=3 targ v(xtop:mux_out)=0.9 fall={first_fall+3}
*.measure tran  d6 trig v(xtop:a0)=0.9 rise=4 targ v(xtop:mux_out)=0.9 rise={first_rise+4}
*.measure tran  d7 trig v(xtop:a0)=0.9 fall=4 targ v(xtop:mux_out)=0.9 fall={first_fall+4}
*.measure tran  d8 trig v(xtop:a0)=0.9 rise=5 targ v(xtop:mux_out)=0.9 rise={first_rise+5}
*.measure tran  d9 trig v(xtop:a0)=0.9 fall=5 targ v(xtop:mux_out)=0.9 fall={first_fall+5}
*.measure tran d10 trig v(xtop:a0)=0.9 rise=6 targ v(xtop:mux_out)=0.9 rise={first_rise+6}
*.measure tran d11 trig v(xtop:a0)=0.9 fall=6 targ v(xtop:mux_out)=0.9 fall={first_fall+6}
*.measure tran d12 trig v(xtop:a0)=0.9 rise=7 targ v(xtop:mux_out)=0.9 rise={first_rise+7}
*.measure tran d13 trig v(xtop:a0)=0.9 fall=7 targ v(xtop:mux_out)=0.9 fall={first_fall+7}
*.measure tran d14 trig v(xtop:a0)=0.9 rise=8 targ v(xtop:mux_out)=0.9 rise={first_rise+8}
*.measure tran d15 trig v(xtop:a0)=0.9 fall=8 targ v(xtop:mux_out)=0.9 fall={first_fall+8}

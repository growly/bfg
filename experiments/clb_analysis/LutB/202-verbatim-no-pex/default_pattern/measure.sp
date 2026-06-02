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

*.measure tran  a0  when v(xtop:a0)=0.9 cross=1  from={test_start}
*.measure tran  a1  when v(xtop:a0)=0.9 cross=2  from={test_start}
*.measure tran  a2  when v(xtop:a0)=0.9 cross=3  from={test_start}
*.measure tran  a3  when v(xtop:a0)=0.9 cross=4  from={test_start}
*.measure tran  a4  when v(xtop:a0)=0.9 cross=5  from={test_start}
*.measure tran  a5  when v(xtop:a0)=0.9 cross=6  from={test_start}
*.measure tran  a6  when v(xtop:a0)=0.9 cross=7  from={test_start}
*.measure tran  a7  when v(xtop:a0)=0.9 cross=8  from={test_start}
*.measure tran  a8  when v(xtop:a0)=0.9 cross=9  from={test_start}
*.measure tran  a9  when v(xtop:a0)=0.9 cross=10 from={test_start}
*.measure tran  a10 when v(xtop:a0)=0.9 cross=11 from={test_start}
*.measure tran  a11 when v(xtop:a0)=0.9 cross=12 from={test_start}
*.measure tran  a12 when v(xtop:a0)=0.9 cross=13 from={test_start}
*.measure tran  a13 when v(xtop:a0)=0.9 cross=14 from={test_start}
*.measure tran  a14 when v(xtop:a0)=0.9 cross=15 from={test_start}
*.measure tran  a15 when v(xtop:a0)=0.9 cross=16 from={test_start}
*
*.measure tran  b0  when v(xtop:mux_out)=0.9 cross=1 from={test_start}
*.measure tran  b1  when v(xtop:mux_out)=0.9 cross=2 from={test_start}
*.measure tran  b2  when v(xtop:mux_out)=0.9 cross=3 from={test_start}
*.measure tran  b3  when v(xtop:mux_out)=0.9 cross=4 from={test_start}
*.measure tran  b4  when v(xtop:mux_out)=0.9 cross=5 from={test_start}
*.measure tran  b5  when v(xtop:mux_out)=0.9 cross=6 from={test_start}
*.measure tran  b6  when v(xtop:mux_out)=0.9 cross=7 from={test_start}
*.measure tran  b7  when v(xtop:mux_out)=0.9 cross=8 from={test_start}
*.measure tran  b8  when v(xtop:mux_out)=0.9 cross=9 from={test_start}
*.measure tran  b9  when v(xtop:mux_out)=0.9 cross=10 from={test_start}
*.measure tran  b10 when v(xtop:mux_out)=0.9 cross=11 from={test_start}
*.measure tran  b11 when v(xtop:mux_out)=0.9 cross=12 from={test_start}
*.measure tran  b12 when v(xtop:mux_out)=0.9 cross=13 from={test_start}
*.measure tran  b13 when v(xtop:mux_out)=0.9 cross=14 from={test_start}
*.measure tran  b14 when v(xtop:mux_out)=0.9 cross=15 from={test_start}
*.measure tran  b15 when v(xtop:mux_out)=0.9 cross=16 from={test_start}

.measure tran  iall avg i(xtop:vpower)
.measure tran itest avg i(xtop:vpower) from={test_start} to={test_end}
*.measure tran  i0 avg i(xtop:vpower) from={3.955000e-08} to={4.036240e-08}

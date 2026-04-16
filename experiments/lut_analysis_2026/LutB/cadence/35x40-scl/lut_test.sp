; test harness for luts

.include testbench.sp

.print tran format=csv v(*) i(*)

* These are based on the input pattern (scan_data) and the order of connection
* from the scan chain to the muxes (mux order).
*
* TODO(aryap): the std-cell comparison model needs the bypass mux at the output
* (and before the output register).

* Have to count rises across MUX_OUT to figure this one:
.param first=4

*.measure tran d0 trig v(xtop:a0)={half_vdd} fall=1 targ v(xtop:xlut:z)={half_vdd} fall={first}
*.measure tran d1 trig v(xtop:a0)={half_vdd} fall=2 targ v(xtop:xlut:z)={half_vdd} rise={first+1}
*.measure tran d2 trig v(xtop:a0)={half_vdd} rise=4 targ v(xtop:xlut:z)={half_vdd} fall={first+1}

.measure tran  d0 trig v(xtop:a0)=0.9 rise=1 targ v(xtop:mux_out)=0.9 rise={first+1}
.measure tran  d1 trig v(xtop:a0)=0.9 fall=1 targ v(xtop:mux_out)=0.9 fall={first+1}
.measure tran  d2 trig v(xtop:a0)=0.9 rise=2 targ v(xtop:mux_out)=0.9 rise={first+2}
.measure tran  d3 trig v(xtop:a0)=0.9 fall=2 targ v(xtop:mux_out)=0.9 fall={first+2}
.measure tran  d4 trig v(xtop:a0)=0.9 rise=3 targ v(xtop:mux_out)=0.9 rise={first+3}
.measure tran  d5 trig v(xtop:a0)=0.9 fall=3 targ v(xtop:mux_out)=0.9 fall={first+3}
.measure tran  d6 trig v(xtop:a0)=0.9 rise=4 targ v(xtop:mux_out)=0.9 rise={first+4}
.measure tran  d7 trig v(xtop:a0)=0.9 fall=4 targ v(xtop:mux_out)=0.9 fall={first+4}
.measure tran  d8 trig v(xtop:a0)=0.9 rise=5 targ v(xtop:mux_out)=0.9 rise={first+5}
.measure tran  d9 trig v(xtop:a0)=0.9 fall=5 targ v(xtop:mux_out)=0.9 fall={first+5}
.measure tran d10 trig v(xtop:a0)=0.9 rise=6 targ v(xtop:mux_out)=0.9 rise={first+6}
.measure tran d11 trig v(xtop:a0)=0.9 fall=6 targ v(xtop:mux_out)=0.9 fall={first+6}
.measure tran d12 trig v(xtop:a0)=0.9 rise=7 targ v(xtop:mux_out)=0.9 rise={first+7}
.measure tran d13 trig v(xtop:a0)=0.9 fall=7 targ v(xtop:mux_out)=0.9 fall={first+7}
.measure tran d14 trig v(xtop:a0)=0.9 rise=8 targ v(xtop:mux_out)=0.9 rise={first+8}
.measure tran d15 trig v(xtop:a0)=0.9 fall=8 targ v(xtop:mux_out)=0.9 fall={first+8}

.end

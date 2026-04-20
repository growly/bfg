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

.include measure.sp

.end

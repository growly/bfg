; test harness for luts

.include params.sp
.include testbench.sp

*.print tran format=csv v(*) i(*)

.include measure.sp

.end

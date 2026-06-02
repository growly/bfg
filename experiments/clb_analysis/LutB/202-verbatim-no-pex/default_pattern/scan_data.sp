* NOTE: The first bit has to be 0 so that the last register in the scan chain
* gets loaded with 0: this sets the combinational output selection mux to the
* internal mux output.
vscan_data
+ scan_in VSS
*+ pat ({1.8} {0} {0} {50p} {50p} {scan_clock_period} b010011001100110010)
+ pat ({1.8} {0} {0} {50p} {50p} {scan_clock_period} b001100110011001100)

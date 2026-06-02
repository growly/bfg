###############################################################################
# Created by write_sdc
###############################################################################
current_design clb
###############################################################################
# Timing Constraints
###############################################################################
create_clock -name scan_clk -period 30.0000 [get_ports {scan_clk}]
set_clock_transition 0.1500 [get_clocks {scan_clk}]
set_clock_uncertainty 0.2500 scan_clk
set_propagated_clock [get_clocks {scan_clk}]
set_input_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {addr[0]}]
set_input_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {addr[1]}]
set_input_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {addr[2]}]
set_input_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {addr[3]}]
set_input_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {clk}]
set_input_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {scan_en}]
set_input_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {scan_in}]
set_input_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {x}]
set_output_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {mux}]
set_output_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {q}]
set_output_delay 6.0000 -clock [get_clocks {scan_clk}] -add_delay [get_ports {scan_out}]
###############################################################################
# Environment
###############################################################################
set_load -pin_load 0.0334 [get_ports {mux}]
set_load -pin_load 0.0334 [get_ports {q}]
set_load -pin_load 0.0334 [get_ports {scan_out}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {clk}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {scan_clk}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {scan_en}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {scan_in}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {x}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {addr[3]}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {addr[2]}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {addr[1]}]
set_driving_cell -lib_cell sky130_fd_sc_hd__inv_2 -pin {Y} -input_transition_rise 0.0000 -input_transition_fall 0.0000 [get_ports {addr[0]}]
###############################################################################
# Design Rules
###############################################################################
set_max_transition 0.7500 [current_design]
set_max_capacitance 0.2000 [current_design]
set_max_fanout 10.0000 [current_design]

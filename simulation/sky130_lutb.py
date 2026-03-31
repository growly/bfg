#!/usr/bin/env python3

import sys
sys.path.extend(['../vlsir/VlsirTools', '../vlsir/bindings/python'])

import os
from hdl21.prefix import m, u, n, p, f
import hdl21 as h
import hdl21.primitives as primitives
import hdl21.sim as sim
import sky130_hdl21 as sky130
import vlsir.circuit_pb2 as vlsir_circuit
import vlsirtools.spice as spice
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import util

PDK_PATH = Path(os.environ.get("PDK_ROOT")) / "sky130A"
#PDK_PATH = "/home/arya/src/pdk-root/sky130A_dan"
sky130.install = sky130.Install(
    pdk_path=PDK_PATH,
    lib_path="libs.tech/ngspice/sky130.lib.spice",
    model_ref=""    # NOTE(aryap): This seems unused.
)

package = vlsir_circuit.Package()
with open('../build/LutB.package.pb', 'rb') as package_file:
    package.ParseFromString(package_file.read())
ns = h.from_proto(package)
lut_b = ns.LutB

RUN_DIR = "./scratch_lutb"

HIGH = 1.8  # volts
LOW = 0.0   # volts

SCAN_CLK_PERIOD = 1*n
SCAN_START_DELAY = 50*p

CLK_PERIOD = 500*p
CLK_START_DELAY = 20*SCAN_CLK_PERIOD

# TODO(aryap): Using Hdl21 limits us because certain very-useful voltage sources
# are not yet available, namely PATTERN. We want to be able to load the
# configuration registers then turn the scan clock off.

# FIXME(aryap): If I set the scan clock to 10ns instead of 1s, the scan chain
# glitches. What the hell? The only thing I can think of causing this is the
# automatic timesteps Xyce is taking. But that raises questions about whether
# the circuits are robust enough (they aren't).

@sim.sim
class LutBSim:
    """The LutB testbench proceeds as follows.

    A simulated scan-chain clock is used to load a configuration bitstream. The
    number of bits will depend on the parameters used to generate the LutB.

    Inputs are then varied to read the contents of the LutB.

    A simulated application clock is used to test whether the register output
    works.

    """

    @h.module
    class Tb:   # required to be named 'Tb'?
        # Seems to be a requirement of a 'Tb' to have a "single, scalar port".
        VSS = h.Port()
        vpwr = h.Signal();

        # Inputs.
        s0, s1, s2, s3 = h.Signals(4)
        scan_in = h.Signal()
        scan_clk, app_clk = h.Signals(2)

        # Outputs.
        reg_out, mux_out, scan_out = h.Signals(3)

        power = primitives.DcVoltageSource(dc=HIGH)(p=vpwr, n=VSS)

        # 2 fF
        reg_load = primitives.IdealCapacitor(c=2*f)(p=reg_out, n=VSS)
        mux_load = primitives.IdealCapacitor(c=2*f)(p=mux_out, n=VSS)

        dut = lut_b()(S0=s0,
                      S1=s1,
                      S2=s2,
                      S3=s3,
                      CONFIG_IN=scan_in,
                      CONFIG_OUT=scan_out,
                      CLK=scan_clk,   # FIXME(aryap)
                      Q=reg_out,
                      MUX=mux_out,
                      VPWR_0=vpwr,
                      VGND_0=VSS,
                      VPWR_1=vpwr,
                      VGND_1=VSS)

        scan_clk_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=SCAN_START_DELAY,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=SCAN_CLK_PERIOD,
            width=0.5*SCAN_CLK_PERIOD)(p=scan_clk, n=VSS)

        scan_in_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=SCAN_START_DELAY + 0.5*SCAN_CLK_PERIOD,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=2*SCAN_CLK_PERIOD,
            width=SCAN_CLK_PERIOD)(p=scan_in, n=VSS)

        s0_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=CLK_START_DELAY,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=CLK_PERIOD,
            width=0.5*CLK_PERIOD)(p=s0, n=VSS)

        s1_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=CLK_START_DELAY,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=2*CLK_PERIOD,
            width=CLK_PERIOD)(p=s1, n=VSS)

        s2_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=CLK_START_DELAY,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=4*CLK_PERIOD,
            width=2*CLK_PERIOD)(p=s2, n=VSS)

        s3_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=CLK_START_DELAY,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=8*CLK_PERIOD,
            width=4*CLK_PERIOD)(p=s3, n=VSS)


    includes = sky130.install.include(h.pdk.Corner.TYP)
    tran = sim.Tran(tstop=20*SCAN_CLK_PERIOD + 17*CLK_PERIOD, tstep=1*p)


def plot(tran_result: spice.sim_data.TranResult):
    data = tran_result.data

    lines = [
        #"I(XTOP:CMUX_LOAD)",
        #"I(XTOP:CREG_LOAD)",
        #"I(XTOP:VPOWER)",
        #"I(XTOP:VS0_DRIVER)",
        #"I(XTOP:VS1_DRIVER)",
        #"I(XTOP:VS2_DRIVER)",
        #"I(XTOP:VS3_DRIVER)",
        #"I(XTOP:VSCAN_CLK_DRIVER)",
        #"I(XTOP:VSCAN_IN_DRIVER)",
        #"TIME",

        #"V(XTOP:MUX_OUT)",
        #"V(XTOP:REG_OUT)",
        #"V(XTOP:S0)",
        #"V(XTOP:S1)",
        #"V(XTOP:S2)",
        #"V(XTOP:S3)",

        #"V(XTOP:SCAN_CLK)",
        "V(XTOP:SCAN_IN)",

        #"V(XTOP:VPWR)",

        #"V(XTOP:XDUT:APP_CLK)",

        "V(XTOP:XDUT:CLK_0)",
        #"V(XTOP:XDUT:CLK_1)",
        "V(XTOP:XDUT:CLK_I_0)",
        #"V(XTOP:XDUT:CLK_I_1)",

        "V(XTOP:XDUT:REGISTER_SELECT_CONFIG_DFXTP_I.Q)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:B)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:C)",

        "V(XTOP:XDUT:LUT_DFXTP_0_0_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_0_1_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_0_2_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_0_3_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_0_4_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_0_5_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_0_6_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_0_7_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_1_0_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_1_1_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_1_2_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_1_3_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_1_4_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_1_5_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_1_6_I.Q)",
        "V(XTOP:XDUT:LUT_DFXTP_1_7_I.Q)",
        "V(XTOP:SCAN_OUT)",

        #"V(XTOP:XDUT:N_0)",
        #"V(XTOP:XDUT:N_1)",
        #"V(XTOP:XDUT:N_2)",
        #"V(XTOP:XDUT:N_3)",
        #"V(XTOP:XDUT:N_4)",
        #"V(XTOP:XDUT:N_5)",
        #"V(XTOP:XDUT:N_6)",
        #"V(XTOP:XDUT:N_7)",
        #"V(XTOP:XDUT:N_8)",
        #"V(XTOP:XDUT:N_9)",
        #"V(XTOP:XDUT:REG_FLOP_IN)",
        #"V(XTOP:XDUT:VGND)",
        #"V(XTOP:XDUT:VPWR)",
        #"V(XTOP:XDUT:X)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:A)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:B)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:C)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:E)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:F)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:H)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:I)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:J)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_CONFIG_DFXTP_I:Q_B)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_SELECT_HD_MUX2_1_I:I0)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_SELECT_HD_MUX2_1_I:I1)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_SELECT_HD_MUX2_1_I:I2)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_SELECT_HD_MUX2_1_I:I3)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_SELECT_HD_MUX2_1_I:S_B)",
        #"V(XTOP:XDUT:XCOMBINATIONAL_SELECT_HD_MUX2_1_I:X_B)",
        #"V(XTOP:XDUT:XHD_MUX2_1_0:I0)",
        #"V(XTOP:XDUT:XHD_MUX2_1_0:I1)",
        #"V(XTOP:XDUT:XHD_MUX2_1_0:I2)",
        #"V(XTOP:XDUT:XHD_MUX2_1_0:I3)",
        #"V(XTOP:XDUT:XHD_MUX2_1_0:S_B)",
        #"V(XTOP:XDUT:XHD_MUX2_1_0:X_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_0_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_1_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_2_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_3_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_4_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_5_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_6_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_0_7_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_0_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_1_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_2_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_3_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_4_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_5_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_6_I:Q_B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:A)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:B)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:C)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:E)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:F)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:H)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:I)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:J)",
        #"V(XTOP:XDUT:XLUT_DFXTP_1_7_I:Q_B)",
        #"V(XTOP:XDUT:XMUX_0:A0)",
        #"V(XTOP:XDUT:XMUX_0:A1)",
        #"V(XTOP:XDUT:XMUX_0:A2)",
        #"V(XTOP:XDUT:XMUX_0:A3)",
        #"V(XTOP:XDUT:XMUX_0:LOWER_LEFT__A0)",
        #"V(XTOP:XDUT:XMUX_0:LOWER_LEFT__A1)",
        #"V(XTOP:XDUT:XMUX_0:LOWER_RIGHT__A0)",
        #"V(XTOP:XDUT:XMUX_0:LOWER_RIGHT__A1)",
        #"V(XTOP:XDUT:XMUX_0:UPPER_LEFT__A0)",
        #"V(XTOP:XDUT:XMUX_0:UPPER_LEFT__A1)",
        #"V(XTOP:XDUT:XMUX_0:UPPER_RIGHT__A0)",
        #"V(XTOP:XDUT:XMUX_0:UPPER_RIGHT__A1)",
        #"V(XTOP:XDUT:XMUX_1:A0)",
        #"V(XTOP:XDUT:XMUX_1:A1)",
        #"V(XTOP:XDUT:XMUX_1:A2)",
        #"V(XTOP:XDUT:XMUX_1:A3)",
        #"V(XTOP:XDUT:XMUX_1:LOWER_LEFT__A0)",
        #"V(XTOP:XDUT:XMUX_1:LOWER_LEFT__A1)",
        #"V(XTOP:XDUT:XMUX_1:LOWER_RIGHT__A0)",
        #"V(XTOP:XDUT:XMUX_1:LOWER_RIGHT__A1)",
        #"V(XTOP:XDUT:XMUX_1:UPPER_LEFT__A0)",
        #"V(XTOP:XDUT:XMUX_1:UPPER_LEFT__A1)",
        #"V(XTOP:XDUT:XMUX_1:UPPER_RIGHT__A0)",
        #"V(XTOP:XDUT:XMUX_1:UPPER_RIGHT__A1)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:A)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:B)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:C)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:CLKI)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:E)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:F)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:H)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:I)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:J)",
        #"V(XTOP:XDUT:XREGISTER_DFXTP_I:Q_B)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:A)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:C)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:E)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:F)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:H)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:I)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:J)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_CONFIG_DFXTP_I:Q_B)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_HD_MUX2_1_I:I0)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_HD_MUX2_1_I:I1)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_HD_MUX2_1_I:I2)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_HD_MUX2_1_I:I3)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_HD_MUX2_1_I:S_B)",
        #"V(XTOP:XDUT:XREGISTER_SELECT_HD_MUX2_1_I:X_B)",
        #"V(XTOP:XDUT:Z)",
    ]

    fig, subs = plt.subplots(len(lines))

    for i, line in enumerate(lines):
        sub = subs[i]
        sub.plot(data["TIME"], data[line], label=line)
        sub.set_title(line)
        #sub.set_ylabel('V')
        sub.legend()
        sub.set_ylim(-0.1, 2.0)
        sub.set_yticks(np.arange(-0.1, 2.0, step=0.4))
        sub.tick_params(axis='y', labelsize=4)
        sub.tick_params(axis='x', labelsize=4)

    plt.ylabel('V')
    plt.xlabel('Time (s)')
    plt.savefig('plot.sky130_lutb.png', dpi=300)
        

def main():
    options = spice.SimOptions(
        simulator=spice.SupportedSimulators.XYCE,
        fmt=spice.ResultFormat.SIM_DATA,
        rundir=RUN_DIR
    )
    #if not spice.xyce.available():
    #    print("spice is not available!")
    #    return

    util.scale_params(lut_b)

    results = LutBSim.run(options)

    tran_results = None
    for analysis in results.an:
        if isinstance(analysis, spice.sim_data.TranResult):
            tran_results = analysis

    if tran_results:
        plot(tran_results)


if __name__ == "__main__":
    main()

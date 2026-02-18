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
with open('../build/sky130_mux.package.pb', 'rb') as package_file:
    package.ParseFromString(package_file.read())
ns = h.from_proto(package)
mux = ns.sky130_mux

RUN_DIR = "./scratch_mux"

HIGH = 1.8  # volts
LOW = 0.0   # volts


@sim.sim
class Sky130MuxSim:

    @h.module
    class Tb:   # required to be named 'Tb'?
        # Seems to be a requirement of a 'Tb' to have a "single, scalar port".
        VSS = h.Port()

        s0, s1, s2 = h.Signals(3)
        s0_b, s1_b, s2_b = h.Signals(3)
        x0, x1, x2, x3, x4, x5, x6, x7 = h.Signals(8)
        z = h.Signal()

        dut = mux()(S0=s0,
                    S0_B=s0_b,
                    S1=s1,
                    S1_B=s1_b,
                    S2=s2,
                    S2_B=s2_b,
                    input_0=x0,
                    input_1=x1,
                    input_2=x2,
                    input_3=x3,
                    input_4=x4,
                    input_5=x5,
                    input_6=x6,
                    input_7=x7,
                    Z=z,
                    VNB=VSS)
                    #VGND=VSS)

        # Does the mux have a power source?
        power = primitives.DcVoltageSource(dc=HIGH)(p=dut.VPB, n=VSS)

        fake_load = primitives.IdealCapacitor(c=2*f)(p=z, n=VSS)

        init_x0 = primitives.DcVoltageSource(dc=LOW)(p=x0, n=VSS)
        init_x1 = primitives.DcVoltageSource(dc=HIGH)(p=x1, n=VSS)
        init_x2 = primitives.DcVoltageSource(dc=LOW)(p=x2, n=VSS)
        init_x3 = primitives.DcVoltageSource(dc=HIGH)(p=x3, n=VSS)
        init_x4 = primitives.DcVoltageSource(dc=LOW)(p=x4, n=VSS)
        init_x5 = primitives.DcVoltageSource(dc=HIGH)(p=x5, n=VSS)
        init_x6 = primitives.DcVoltageSource(dc=LOW)(p=x6, n=VSS)
        init_x7 = primitives.DcVoltageSource(dc=HIGH)(p=x7, n=VSS)

        s0_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=50*p,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=1*n,
            width=500*p)(p=s0, n=VSS)

        s0_b_driver = primitives.PulseVoltageSource(
            v1=HIGH,
            delay=50*p,
            v2=LOW,
            rise=50*p,
            fall=50*p,
            period=1*n,
            width=500*p)(p=s0_b, n=VSS)

        s1_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=50*p,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=2*n,
            width=1*n)(p=s1, n=VSS)

        s1_b_driver = primitives.PulseVoltageSource(
            v1=HIGH,
            delay=50*p,
            v2=LOW,
            rise=50*p,
            fall=50*p,
            period=2*n,
            width=1*n)(p=s1_b, n=VSS)

        s2_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=50*p,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=4*n,
            width=2*n)(p=s2, n=VSS)

        s2_b_driver = primitives.PulseVoltageSource(
            v1=HIGH,
            delay=50*p,
            v2=LOW,
            rise=50*p,
            fall=50*p,
            period=4*n,
            width=2*n)(p=s2_b, n=VSS)


    includes = sky130.install.include(h.pdk.Corner.TYP)
    tran = sim.Tran(tstop=4*n, tstep=1*p)


def plot(tran_result: spice.sim_data.TranResult):
    data = tran_result.data

    lines = [
        "V(XTOP:S0)",
        "V(XTOP:S0_B)",
        "V(XTOP:S1)",
        "V(XTOP:S1_B)",
        "V(XTOP:S2)",
        "V(XTOP:S2_B)",

        #"V(XTOP:XDUT:UPPER_LEFT__A0)",
        #"V(XTOP:XDUT:UPPER_LEFT__A1)",
        #"V(XTOP:XDUT:UPPER_RIGHT__A0)",
        #"V(XTOP:XDUT:UPPER_RIGHT__A1)",
        #"V(XTOP:XDUT:LOWER_LEFT__A0)",
        #"V(XTOP:XDUT:LOWER_LEFT__A1)",
        #"V(XTOP:XDUT:LOWER_RIGHT__A0)",
        #"V(XTOP:XDUT:LOWER_RIGHT__A1)",

        #"V(XTOP:DUT_VPWR)",

        "V(XTOP:X0)",
        "V(XTOP:X1)",
        #"V(XTOP:X2)",
        #"V(XTOP:X3)",
        #"V(XTOP:X4)",
        #"V(XTOP:X5)",
        #"V(XTOP:X6)",
        #"V(XTOP:X7)",

        #"V(XTOP:XDUT:A0)",
        #"V(XTOP:XDUT:A1)",
        #"V(XTOP:XDUT:A2)",
        #"V(XTOP:XDUT:A3)",

        "V(XTOP:Z)"
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
    plt.savefig('plot.png', dpi=300)
        

def main():
    options = spice.SimOptions(
        simulator=spice.SupportedSimulators.XYCE,
        fmt=spice.ResultFormat.SIM_DATA,
        rundir=RUN_DIR
    )
    #if not spice.xyce.available():
    #    print("spice is not available!")
    #    return

    util.scale_params(mux)

    results = Sky130MuxSim.run(options)

    tran_results = None
    for analysis in results.an:
        if isinstance(analysis, spice.sim_data.TranResult):
            tran_results = analysis

    if tran_results:
        plot(tran_results)


if __name__ == "__main__":
    main()

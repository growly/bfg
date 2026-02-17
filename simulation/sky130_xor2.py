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


PDK_PATH = Path(os.environ.get("PDK_ROOT")) / "sky130A"
#PDK_PATH = "/home/arya/src/pdk-root/sky130A_dan"
sky130.install = sky130.Install(
    pdk_path=PDK_PATH,
    lib_path="libs.tech/ngspice/sky130.lib.spice",
    model_ref=""    # NOTE(aryap): This seems unused.
)

package = vlsir_circuit.Package()
with open('../build/Sky130Xor2.package.pb', 'rb') as package_file:
    package.ParseFromString(package_file.read())
ns = h.from_proto(package)
xor2 = ns.Sky130Xor2


def scale_params(module: h.Module):
    SCALE = 1E6
    for name, instance in module.instances.items():
        for param, value in instance.of.params.items():
            new_value = (value * SCALE).scale(m)
            instance.of.params[param] = new_value


HIGH = 1.8  # volts
LOW = 0.0   # volts


@sim.sim
class Sky130Xor2Sim:

    @h.module
    class Tb:   # required to be named 'Tb'?
        # Seems to be a requirement of a 'Tb' to have a "single, scalar port".
        VSS = h.Port()

        vpwr = h.Signal();

        a, b = h.Signals(2)
        x = h.Signal()

        # Does the mux have a power source?
        power = primitives.DcVoltageSource(dc=HIGH)(p=vpwr, n=VSS)

        fake_load = primitives.IdealCapacitor(c=2*f)(p=x, n=VSS)

        dut = xor2()(A=a,
                     B=b,
                     X=x,
                     VPWR=vpwr,
                     VGND=VSS,
                     VPB=vpwr,
                     VNB=VSS)

        a_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=50*p,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=1*n,
            width=500*p)(p=a, n=VSS)

        b_driver = primitives.PulseVoltageSource(
            v1=LOW,
            delay=50*p,
            v2=HIGH,
            rise=50*p,
            fall=50*p,
            period=2*n,
            width=1000*p)(p=b, n=VSS)


    includes = sky130.install.include(h.pdk.Corner.TYP)
    tran = sim.Tran(tstop=4*n, tstep=1*p)


def plot(tran_result: spice.sim_data.TranResult):
    data = tran_result.data

    lines = [
        "V(XTOP:A)",
        "V(XTOP:B)",
        "V(XTOP:X)",

        #"V(XTOP:DUT_VPWR)",

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
    plt.savefig('plot.sky130_xor2.png', dpi=300)
        

def main():
    options = spice.SimOptions(
        simulator=spice.SupportedSimulators.XYCE,
        fmt=spice.ResultFormat.SIM_DATA,
        rundir="./scratch_xor2"
    )
    #if not spice.xyce.available():
    #    print("spice is not available!")
    #    return

    scale_params(xor2)

    results = Sky130Xor2Sim.run(options)

    tran_results = None
    for analysis in results.an:
        if isinstance(analysis, spice.sim_data.TranResult):
            tran_results = analysis

    if tran_results:
        plot(tran_results)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3

import sys
sys.path.extend(['../vlsir/VlsirTools', '../vlsir/bindings/python'])

import os
#from hdl21.prefix import m, u, n, p, f
#import hdl21 as h
#import hdl21.primitives as primitives
#import hdl21.sim as sim
#import sky130_hdl21 as sky130
import vlsir.circuit_pb2 as circuit_pb
#import vlsirtools.spice as spice
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from vlsirtools.netlist.spice import SpiceNetlister
from vlsirtools.netlist.spectre import SpectreNetlister
from vlsirtools.netlist.spice import NgspiceNetlister
from vlsirtools.netlist.spice import XyceNetlister

#PDK_PATH = Path(os.environ.get("PDK_ROOT")) / "sky130A"
##PDK_PATH = "/home/arya/src/pdk-root/sky130A_dan"
#sky130.install = sky130.Install(
#    pdk_path=PDK_PATH,
#    lib_path="libs.tech/ngspice/sky130.lib.spice",
#    model_ref=""    # NOTE(aryap): This seems unused.
#)


def main():
    #options = spice.SimOptions(
    #    simulator=spice.SupportedSimulators.XYCE,
    #    fmt=spice.ResultFormat.SIM_DATA,
    #    rundir="./scratch"
    #)

    in_file_name = '../build/package.pb'
    out_file_name = 'package.sp'

    package_pb = circuit_pb.Package()
    with open(in_file_name, 'rb') as in_file:
      package_pb.ParseFromString(in_file.read())
    with open(out_file_name, 'w') as out_file:
      #netlister = SpectreNetlister(out_file)
      #netlister = NgspiceNetlister(out_file)
      #netlister = SpiceNetlister(out_file)
      netlister = XyceNetlister(out_file)
      netlister.write_package(package_pb)
    

if __name__ == "__main__":
    main()

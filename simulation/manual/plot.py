#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

data = pd.read_csv('tran.sp.csv')

#data.plot("TIME", [
#           ],
#          subplots=True)

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

    "V(XTOP:DUT_VPWR)",

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

    "V(XTOP:Y)"
]

fig, subs = plt.subplots(len(lines))

for i, line in enumerate(lines):
    sub = subs[i]
    sub.plot(data["TIME"], data[line], label=line)
    #sub.set_title(line)
    #sub.set_ylabel('V')
    sub.legend()
    sub.set_ylim(-0.1, 2.0)
    sub.set_yticks(np.arange(-0.1, 2.0, step=0.4))
    sub.tick_params(axis='y', labelsize=4)
    sub.tick_params(axis='x', labelsize=4)

plt.ylabel('V')
plt.xlabel('Time (s)')
plt.savefig('plot.png', dpi=300)

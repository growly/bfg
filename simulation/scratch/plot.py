#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv('tran.sp.csv')
data.plot("TIME", [
           "V(XTOP:S0)",
           #"V(XTOP:S0_B)",
           "V(XTOP:S1)",
           #"V(XTOP:S1_B)",
           "V(XTOP:S2)",
           #"V(XTOP:S2_B)",

           #"V(XTOP:X0)",
           "V(XTOP:Z)",

           "V(XTOP:XDUT:UPPER_LEFT__A0)",
           "V(XTOP:XDUT:UPPER_LEFT__A1)",
           "V(XTOP:XDUT:UPPER_RIGHT__A0)",
           "V(XTOP:XDUT:UPPER_RIGHT__A1)",
           #"V(XTOP:XDUT:LOWER_LEFT__A0)",
           #"V(XTOP:XDUT:LOWER_LEFT__A1)",
           #"V(XTOP:XDUT:LOWER_RIGHT__A0)",
           #"V(XTOP:XDUT:LOWER_RIGHT__A1)",

           #"V(XTOP:X6)",
           #"V(XTOP:X3)",
           #"V(XTOP:X1)",
           #"V(XTOP:X4)",
           #"V(XTOP:X5)",
           #"V(XTOP:X7)",
           #"V(XTOP:X2)",
           #"V(XTOP:DUT_VPWR)",

           "V(XTOP:XDUT:A0)",
           "V(XTOP:XDUT:A1)",
           "V(XTOP:XDUT:A2)",
           "V(XTOP:XDUT:A3)"
           ],
          subplots=True,
          figsize=(8,11))
plt.savefig('plot.png', dpi=100)

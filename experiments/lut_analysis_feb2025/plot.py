#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv('lut_test.sp.csv')

all_columns = data.columns

headers = [
   "V(XTOP:CONFIG_CLK)",
   #"V(XTOP:CONFIG_EN)",
   "V(XTOP:CONFIG_IN)"]

internal_vars = [
   # When there are no parasitics:
   "V(XTOP:XLUT:MEM[0])",
   "V(XTOP:XLUT:MEM[1])",
   "V(XTOP:XLUT:MEM[2])",
   "V(XTOP:XLUT:MEM[3])",
   "V(XTOP:XLUT:MEM[4])",
   "V(XTOP:XLUT:MEM[5])",
   "V(XTOP:XLUT:MEM[6])",
   "V(XTOP:XLUT:MEM[7])",
   "V(XTOP:XLUT:MEM[8])",
   "V(XTOP:XLUT:MEM[9])",
   "V(XTOP:XLUT:MEM[10])",
   "V(XTOP:XLUT:MEM[11])",
   "V(XTOP:XLUT:MEM[12])",
   "V(XTOP:XLUT:MEM[13])",
   "V(XTOP:XLUT:MEM[14])",
   #"V(XTOP:XLUT:MEM[15])",

   "V(XTOP:XLUT:LUT_DFXTP_0_0.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_1.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_2.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_3.Q)",

   "V(XTOP:XLUT:CLK_0)",
   "V(XTOP:XLUT:CLK_1)",
   "V(XTOP:XLUT:CLK_I_0)",
   "V(XTOP:XLUT:CLK_I_1)",
]
for column in internal_vars:
    if column in all_columns:
        headers.append(column)

headers += [
   "V(XTOP:A0)",
   "V(XTOP:A1)",
   "V(XTOP:A2)",
   "V(XTOP:A3)",

   "V(XTOP:OUT)",
]

headers = [
   "V(XTOP:CONFIG_IN)",
   "V(XTOP:XLUT:CLK_0)",
   "V(XTOP:XLUT:CLK_I_0)",

   "V(XTOP:XLUT:XLUT_DFXTP_0_0:VPWR)",
   "V(XTOP:XLUT:LUT_DFXTP_0_0.Q)",
   "V(XTOP:XLUT:XLUT_DFXTP_0_0:I)",
   "V(XTOP:XLUT:XLUT_DFXTP_0_0:Q_N)",
   "V(XTOP:XLUT:XLUT_DFXTP_0_0:F)",
   "V(XTOP:XLUT:XLUT_DFXTP_0_0:B)",
   "V(XTOP:XLUT:XLUT_DFXTP_0_0:A)",
   "V(XTOP:XLUT:XLUT_DFXTP_0_0:H)",
   "V(XTOP:XLUT:XLUT_DFXTP_0_0:E)",
   "V(XTOP:XLUT:XLUT_DFXTP_0_0:C)",
]

headers = [
   "V(XTOP:CONFIG_IN)",
   "V(XTOP:CONFIG_CLK)",

   "V(XTOP:XLUT:1)",
   "V(XTOP:XLUT:2)",
   "V(XTOP:XLUT:3)",
   "V(XTOP:XLUT:4)",
   "V(XTOP:XLUT:5)",
   "V(XTOP:XLUT:6)",
   "V(XTOP:XLUT:7)",
   "V(XTOP:XLUT:8)",
   "V(XTOP:XLUT:9)",
   "V(XTOP:XLUT:10)",
   "V(XTOP:XLUT:11)",
   "V(XTOP:XLUT:12)",
   "V(XTOP:XLUT:13)",
   "V(XTOP:XLUT:14)",
   "V(XTOP:XLUT:15)",

   #"V(XTOP:XLUT:N_1)",
   #"V(XTOP:XLUT:N_2)",
   #"V(XTOP:XLUT:N_3)",
   #"V(XTOP:XLUT:N_4)",
   #"V(XTOP:XLUT:N_5)",

   "V(XTOP:A0)",
   "V(XTOP:A1)",
   "V(XTOP:A2)",
   "V(XTOP:A3)",

   "V(XTOP:XLUT:N_6)",
   "V(XTOP:XLUT:N_7)",
   "V(XTOP:XLUT:N_8)",

   "V(XTOP:OUT)",
]

headers = [
   "V(XTOP:A0)",
   "V(XTOP:A1)",
   "V(XTOP:A2)",
   "V(XTOP:A3)",

   "V(XTOP:OUT)",
]

data.plot("TIME", headers,
          subplots=True,
          figsize=(8,11))

plt.xlim(3.5e-8, 4.1e-8)
plt.savefig('plot.png', dpi=300)

#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd

from collections import namedtuple

data = pd.read_csv('lut_test.sp.csv')

all_columns = data.columns

LutOrder = namedtuple('LutOrder', 'mux_order name scan_order')

# FIXME(aryap): This is outdated.
# In the earlier 2025 test, the registers were not loaded in order, and instead
# it was more convenient to test that mux outputs coincided to an input pattern
# in the specified order. However, since the input order to the muxes is
# not contiguous, we have to specify it manually here when comparing outputs.

# mux order                  scan order
bfg_luts = [
    LutOrder( 0, "LUT_DFXTP_0_1_I.Q",  2),
    LutOrder( 1, "LUT_DFXTP_0_0_I.Q",  1),
    LutOrder( 2, "LUT_DFXTP_0_2_I.Q",  3),
    LutOrder( 3, "LUT_DFXTP_0_3_I.Q",  4),
    LutOrder( 4, "LUT_DFXTP_1_5_I.Q", 14),
    LutOrder( 5, "LUT_DFXTP_1_4_I.Q", 13),
    LutOrder( 6, "LUT_DFXTP_1_6_I.Q", 15),
    LutOrder( 7, "LUT_DFXTP_1_7_I.Q", 16),
    LutOrder( 8, "LUT_DFXTP_1_2_I.Q", 11),
    LutOrder( 9, "LUT_DFXTP_1_3_I.Q", 12),
    LutOrder(10, "LUT_DFXTP_1_1_I.Q", 10),
    LutOrder(11, "LUT_DFXTP_1_0_I.Q",  9),
    LutOrder(12, "LUT_DFXTP_0_6_I.Q",  7),
    LutOrder(13, "LUT_DFXTP_0_7_I.Q",  8),
    LutOrder(14, "LUT_DFXTP_0_5_I.Q",  6),
    LutOrder(15, "LUT_DFXTP_0_4_I.Q",  5),
]

headers = [
   "V(XTOP:SCAN_CLK)",
   #"V(XTOP:CONFIG_EN)",
   "V(XTOP:SCAN_IN)"]

internal_vars = [
   # When there are no parasitics:
   #"V(XTOP:XLUT:MEM[0])",
   #"V(XTOP:XLUT:MEM[1])",
   #"V(XTOP:XLUT:MEM[2])",
   #"V(XTOP:XLUT:MEM[3])",
   #"V(XTOP:XLUT:MEM[4])",
   #"V(XTOP:XLUT:MEM[5])",
   #"V(XTOP:XLUT:MEM[6])",
   #"V(XTOP:XLUT:MEM[7])",
   #"V(XTOP:XLUT:MEM[8])",
   #"V(XTOP:XLUT:MEM[9])",
   #"V(XTOP:XLUT:MEM[10])",
   #"V(XTOP:XLUT:MEM[11])",
   #"V(XTOP:XLUT:MEM[12])",
   #"V(XTOP:XLUT:MEM[13])",
   #"V(XTOP:XLUT:MEM[14])",
   #"V(XTOP:XLUT:MEM[15])",

   "V(XTOP:XLUT:LUT_DFXTP_0_0_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_1_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_2_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_3_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_4_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_5_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_6_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_0_7_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_1_0_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_1_1_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_1_2_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_1_3_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_1_4_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_1_5_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_1_6_I.Q)",
   "V(XTOP:XLUT:LUT_DFXTP_1_7_I.Q)",

   "V(XTOP:XLUT:CLK_0)",
   "V(XTOP:XLUT:CLK_1)",
   "V(XTOP:XLUT:CLK_I_0)",
   "V(XTOP:XLUT:CLK_I_1)",

   "V(XTOP:MUX_OUT)",
]
for column in internal_vars:
    if column in all_columns:
        headers.append(column)

def plot_lut_readout():
    headers = [
       "V(XTOP:SCAN_CLK)",
       #"V(XTOP:CONFIG_EN)",

       #"V(XTOP:SCAN_IN)",

       "V(XTOP:A0)",
       "V(XTOP:A1)",
       "V(XTOP:A2)",
       "V(XTOP:A3)",

       "V(XTOP:A0)",
       "V(XTOP:XLUT:N_1)",
       "V(XTOP:XLUT:N_2)",

       "V(XTOP:A1)",
       "V(XTOP:XLUT:N_3)",
       "V(XTOP:XLUT:N_0)",

       "V(XTOP:A2)",
       "V(XTOP:XLUT:N_4)",
       "V(XTOP:XLUT:N_5)",

       "V(XTOP:A3)",
       "V(XTOP:XLUT:N_9)",
       "V(XTOP:XLUT:N_6)",

       "V(XTOP:XLUT:Z)",
       "V(XTOP:BYPASS)",

       "V(XTOP:MUX_OUT)"
    ]
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    plt.xlim(3.7e-8, 4.5e-8)
    #plt.xlim(0, 4.1e-8)
    plt.savefig('plot.png', dpi=300)

def plot_lut_presentation():
    headers = [
       "V(XTOP:SCAN_CLK)",
       #"V(XTOP:CONFIG_EN)",
       "V(XTOP:SCAN_IN)",
    ]
    for lut_order in bfg_luts:
        headers.append(f"V(XTOP:XLUT:{lut_order.name})")
    headers += [
       "V(XTOP:A0)",
       "V(XTOP:A1)",
       "V(XTOP:A2)",
       "V(XTOP:A3)",

       "V(XTOP:MUX_OUT)"
    ]
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    #plt.xlim(3.4e-8, 4.1e-8)
    plt.xlim(0, 4.1e-8)
    plt.savefig('plot.png', dpi=300)


def plot_register_scan_chain_load():
    headers = [
       "V(XTOP:SCAN_CLK)",

       "V(XTOP:XLUT:CLK_0)",
       "V(XTOP:XLUT:CLK_I_0)",

       "V(XTOP:SCAN_IN)",

       "V(XTOP:XLUT:REGISTER_SELECT_CONFIG_DFXTP_I.Q)",


    ]
    bfg_luts.sort(key=lambda x: x.scan_order)
    for lut_order in bfg_luts:
        headers.append(f"V(XTOP:XLUT:{lut_order.name})")
    headers.append("V(XTOP:SCAN_OUT)")

    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    plt.xlim(0, 35e-9)
    plt.savefig('plot.png', dpi=300)

def plot_fake_lut_register_scan_chain_load():
    headers = [
       "V(XTOP:SCAN_CLK)",

       "V(XTOP:XLUT:CLK_0)",
       "V(XTOP:XLUT:CLK_I_0)",

       "V(XTOP:SCAN_IN)",
       
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
       "V(XTOP:XLUT:15)"
    ]
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    plt.savefig('plot.png', dpi=300)


def plot_fake_lut_x1_debug():
    headers = [
       "V(XTOP:CONFIG_CLK)",

       "V(XTOP:XLUT:CLK_0)",
       "V(XTOP:XLUT:CLK_I_0)",

       "V(XTOP:CONFIG_IN)",
       "V(XTOP:XLUT:X0:A)",
       "V(XTOP:XLUT:X0:B)",
       "V(XTOP:XLUT:X0:C)",
       #"V(XTOP:XLUT:X0:E)",
       "V(XTOP:XLUT:X0:F)",
       #"V(XTOP:XLUT:X0:H)",
       #"V(XTOP:XLUT:X0:I)",
       "V(XTOP:XLUT:X0:Q_B)",
       "V(XTOP:XLUT:1)"
    ]
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))

    plt.xlim(0, 1e-8)
    plt.savefig('plot.png', dpi=300)

if __name__ == '__main__':
    #print(data.columns.values)
    #plot_register_scan_chain_load()
    plot_lut_readout()
    #plot_lut_presentation()

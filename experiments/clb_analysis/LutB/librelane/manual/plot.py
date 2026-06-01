#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd
import sys
from pathlib import Path

from collections import namedtuple

subfolder = sys.argv[1] if len(sys.argv) > 1 else None

in_csv_file = Path('lut_test.sp.csv')

complete_csv_path = Path(subfolder) / in_csv_file if subfolder else in_csv_file

if not complete_csv_path.exists():
  print('error: file not found:', complete_csv_path)
  sys.exit(1)

data = pd.read_csv(complete_csv_path)
all_columns = data.columns

LutOrder = namedtuple('LutOrder', 'mux_order name scan_order')

# In the earlier 2025 test, the registers were not loaded in order, and instead
# it was more convenient to test that mux outputs coincided to an input pattern
# in the specified order. However, since the input order to the muxes is
# not contiguous, we have to specify it manually here when comparing outputs.

# From BFG (grep "Mux order: " output.log):
# lut_dfxtp_0_1_i -> mux_0.input_0
# lut_dfxtp_0_0_i -> mux_0.input_1
# lut_dfxtp_0_2_i -> mux_0.input_2
# lut_dfxtp_0_3_i -> mux_0.input_3
# lut_dfxtp_1_5_i -> mux_0.input_4
# lut_dfxtp_1_4_i -> mux_0.input_5
# lut_dfxtp_1_6_i -> mux_0.input_6
# lut_dfxtp_1_7_i -> mux_0.input_7
# lut_dfxtp_1_2_i -> mux_1.input_0
# lut_dfxtp_1_3_i -> mux_1.input_1
# lut_dfxtp_1_1_i -> mux_1.input_2
# lut_dfxtp_1_0_i -> mux_1.input_3
# lut_dfxtp_0_6_i -> mux_1.input_4
# lut_dfxtp_0_7_i -> mux_1.input_5
# lut_dfxtp_0_5_i -> mux_1.input_6
# lut_dfxtp_0_4_i -> mux_1.input_7

# mux order                  scan order
bfg_luts = [
    LutOrder( 0, "_66_/Q",  9),
    LutOrder( 1, "_65_/Q", 10),
    LutOrder( 2, "_64_/Q", 11),
    LutOrder( 3, "_63_/Q", 12),
    LutOrder( 4, "_62_/Q", 13),
    LutOrder( 5, "_61_/Q", 14),
    LutOrder( 6, "_60_/Q", 15),
    LutOrder( 7, "_77_/Q", 16),
    LutOrder( 8, "_76_/Q", 17),
    LutOrder( 9, "_59_/Q", 18),
    LutOrder(10, "_75_/Q",  1),
    LutOrder(11, "_74_/Q",  0),
    LutOrder(12, "_73_/Q",  2),
    LutOrder(13, "_72_/Q",  3),
    LutOrder(14, "_71_/Q",  4),
    LutOrder(15, "_70_/Q",  5),
    LutOrder(16, "_69_/Q",  6),
    LutOrder(17, "_68_/Q",  7),
    LutOrder(18, "_67_/Q",  8),
]

PLOT_COUNT = 0

def get_plot_file_name():
  global PLOT_COUNT
  name = f'plot_{PLOT_COUNT}.png'
  PLOT_COUNT += 1
  if subfolder:
    return Path(subfolder) / Path(name)
  else:
    return Path(name)

headers = [
   "V(XTOP:SCAN_CLK)",
   #"V(XTOP:CONFIG_EN)",
   "V(XTOP:SCAN_IN)"]

def plot_input_buffers():
    headers = [
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
    ]
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    plt.xlim(3.9e-8, 4.3e-8)
    #plt.xlim(3.7e-8, 5.2e-8)
    #plt.xlim(0, 5.2e-8)
    plt.savefig(get_plot_file_name(), dpi=300)

def plot_mux_propagation():
    headers = [
       "V(XTOP:A0)",

       #"V(XTOP:XLUT:XBUF_0:1)",
       #"V(XTOP:XLUT:XBUF_0:2)",

       "V(XTOP:XLUT:N_1)",
       "V(XTOP:XLUT:N_2)",

       #"V(XTOP:A1)",
       #"V(XTOP:XLUT:N_3)",
       #"V(XTOP:XLUT:N_0)",

       #"V(XTOP:A2)",
       #"V(XTOP:XLUT:N_4)",
       #"V(XTOP:XLUT:N_5)",

       #"V(XTOP:A3)",
       #"V(XTOP:XLUT:N_9)",
       #"V(XTOP:XLUT:N_6)",

       "V(XTOP:XLUT:LUT_DFXTP_0_1_I.Q)",
       "V(XTOP:XLUT:XMUX_0:UPPER_RIGHT__A1)",

       "V(XTOP:XLUT:N_7)"
    ]
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    #lt.xlim(3.9e-8, 4.3e-8)
    plt.xlim(3.7e-8, 5.2e-8)
    #plt.xlim(0, 5.2e-8)
    #plt.xlim(3.8e-9, 4e-9)
    plt.savefig(get_plot_file_name(), dpi=300)

def plot_lut_readout(start=3e-9,zoom=False):
    headers = [
       #"V(XTOP:CONFIG_EN)",

       #"V(XTOP:SCAN_IN)",

       "V(XTOP:A0)",
       #"V(XTOP:XLUT:N_1)",
       #"V(XTOP:XLUT:N_2)",

       "V(XTOP:A1)",
       #"V(XTOP:XLUT:N_3)",
       #"V(XTOP:XLUT:N_0)",

       "V(XTOP:A2)",
       #"V(XTOP:XLUT:N_4)",
       #"V(XTOP:XLUT:N_5)",

       "V(XTOP:A3)",
       #"V(XTOP:XLUT:N_9)",
       #"V(XTOP:XLUT:N_6)",
    ]
    if not zoom:
      for lut_order in sorted(bfg_luts, key=lambda x: x.mux_order):
          headers.append(f"V(XTOP:XLUT:{lut_order.name})")

    headers.extend([
       #"V(XTOP:XLUT:Z)",
       
       #"V(XTOP:BYPASS)",

       "V(XTOP:XLUT:_38_/X)",
       "V(XTOP:MUX_OUT)"
    ])
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    plt.xlim(start, 5.2e-8)
    plt.savefig(get_plot_file_name(), dpi=300)

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
    plt.savefig(get_plot_file_name(), dpi=300)


def plot_fake_lut_register_scan_chain_load():
    headers = [
    ]
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    plt.savefig(get_plot_file_name(), dpi=300)

def plot_register_scan_chain_load_spef():
    headers = [
       "V(XTOP:SCAN_CLK)",

       "V(XTOP:SCAN_EN)",

       "V(XTOP:SCAN_IN)",
    ]
    bfg_luts.sort(key=lambda x: x.scan_order)
    for lut_order in bfg_luts:
        headers.append(f"V(XTOP:XLUT:{lut_order.name})")
    headers.append("V(XTOP:SCAN_OUT)")

    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    plt.xlim(0, 41e-9)
    plt.savefig(get_plot_file_name(), dpi=300)

def plot_register_scan_chain_load():
    headers = [
       "V(XTOP:SCAN_CLK)",

       "V(XTOP:SCAN_EN)",
       "V(XTOP:SCAN_IN)",
       
    ]
    bfg_luts.sort(key=lambda x: x.scan_order)
    for lut_order in bfg_luts:
        headers.append(f"V(XTOP:XLUT:{lut_order.name})")
    headers.append("V(XTOP:SCAN_OUT)")

    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    plt.xlim(0, 5.2e-8)
    plt.savefig(get_plot_file_name(), dpi=300)


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
    plt.savefig(get_plot_file_name(), dpi=300)

if __name__ == '__main__':
    #print(data.columns.values)
    #plot_register_scan_chain_load_spef()
    plot_register_scan_chain_load()
    plot_lut_readout(start=3.8e-8, zoom=True)
    plot_lut_readout(start=0, zoom=False)
    #plot_input_buffers()
    #plot_mux_propagation()
    #plot_lut_presentation()

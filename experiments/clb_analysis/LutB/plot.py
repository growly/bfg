#!/usr/bin/env python3

import re
import argparse
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import pandas as pd
import sys
from pathlib import Path

from collections import namedtuple
from collections import defaultdict

parser = argparse.ArgumentParser()
parser.add_argument('subfolder', nargs='?', default=None)
parser.add_argument('--x', default=None, help='x-axis source')
parser.add_argument('--x-label', default=None, help='x-axis label')
parser.add_argument('--y', default=None, help='y-axis source')
parser.add_argument('--y-label', default=None, help='y-axis label')
parser.add_argument('--surface-plot', default=None, action='store')
args = parser.parse_args()

SUB_DIR_RE = re.compile(r'run_(\d+)_(\d+)')

data = pd.DataFrame()
all_columns = []

subfolder = args.subfolder
in_csv_file = Path('lut_test.sp.csv')
if in_csv_file.exists():
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

def plot_lut_readout():
    headers = [
       "V(XTOP:SCAN_CLK)",
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
    for lut_order in sorted(bfg_luts, key=lambda x: x.mux_order):
        headers.append(f"V(XTOP:XLUT:{lut_order.name})")

    headers.extend([
       "V(XTOP:XLUT:Z)",
       
       #"V(XTOP:BYPASS)",

       "V(XTOP:MUX_OUT)"
    ])
    data.plot("TIME", headers,
              subplots=True,
              figsize=(8,11))
    #plt.xlim(3.7e-8, 5.2e-8)
    plt.xlim(0, 5.2e-8)
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
    plt.xlim(0, 41e-9)
    plt.savefig(get_plot_file_name(), dpi=300)

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

def to_number(with_units):
  number = None
  scale = 1e0
  if with_units.endswith('m'):
    number = int(with_units[:-1])
    # TODO(aryap): Ok but actually, here, because of the spectre/Xyce scaling
    # issues, 'm' means 1e-6 also.
    scale = 1e-3
  elif with_units.endswith('n'):
    number = int(with_units[:-1])
    scale = 1e-9
  elif with_units.endswith('u'):
    number = int(with_units[:-1])
    scale = 1e-6
  else:
    number = int(with_units)
  return number*scale

def plot_max_d_vs_xy_merged_runs(
    csv_file, x_source, x_label, y_source, y_label):
  in_csv_file = Path(csv_file)
  data = pd.read_csv(in_csv_file)

  # Maybe this can be split into two separate dataframes and then merged in the
  # pandas-blessed way? Central problem is merging on multiple indices:
  # (sub_dir, x label, y label).

  runs = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: None)))

  for _, row in data.iterrows():
    sub_dir = row['sub_dir']
    if not isinstance(sub_dir, str):
      continue
    matches = SUB_DIR_RE.match(sub_dir)
    key = matches.group(1)
    x = to_number(row[x_source])
    y = to_number(row[y_source])
    d_cols = [c for c in data.columns if c in [f'D{i}' for i in range(16)]]
    z = max(row[d_cols])

    if runs[key][x][y] is None:
      runs[key][x][y] = z
    else:
      runs[key][x][y] = max(z, runs[key][x][y])

  merged = pd.DataFrame.from_records(
      [(k, x, y, z)
      for k, inner in runs.items()
      for x, innner in inner.items()
      for y, z in innner.items()],
      columns=['sub_dir', x_source, y_source, 'z'])

  z = merged['z'] * 1e12
  x = merged[x_source]
  y = merged[y_source]

  print(merged.sort_values(by=['z'], axis=0, ascending=True))

  fig = plt.figure()
  ax = fig.add_subplot(111, projection='3d')
  ax.plot_trisurf(x, y, z, cmap='coolwarm')
  #ax.scatter(x, y, z)
  plt.title(f'{x_label}, {y_label}')
  ax.set_xlabel(x_label, fontsize=10)
  ax.set_ylabel(y_label, fontsize=10)
  ax.set_zlabel('Read delay (ps)')
  #plt.show()
  plt.savefig(get_plot_file_name(), dpi=400)

def plot_pd_vs_xy_merged_runs(
    csv_file, x_source, x_label, y_source, y_label):
  in_csv_file = Path(csv_file)
  data = pd.read_csv(in_csv_file)

  # Maybe this can be split into two separate dataframes and then merged in the
  # pandas-blessed way? Central problem is merging on multiple indices:
  # (sub_dir, x label, y label).

  runs = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: None)))

  for _, row in data.iterrows():
    sub_dir = row['sub_dir']
    if not isinstance(sub_dir, str):
      continue
    matches = SUB_DIR_RE.match(sub_dir)
    key = matches.group(1)
    x = to_number(row[x_source])
    y = to_number(row[y_source])
    d_cols = [c for c in data.columns if c in [f'D{i}' for i in range(16)]]
    z = max(row[d_cols])
    p = -1 * row['ITEST'] * 1.8  # average power during test, in Watts

    if runs[key][x][y] is None:
      runs[key][x][y] = z * p
    else:
      runs[key][x][y] = max(z * p, runs[key][x][y])

  merged = pd.DataFrame.from_records(
      [(k, x, y, z)
      for k, inner in runs.items()
      for x, innner in inner.items()
      for y, z in innner.items()],
      columns=['sub_dir', x_source, y_source, 'z'])

  z = merged['z'] / max(merged['z'])
  x = merged[x_source]
  y = merged[y_source]

  print(merged.sort_values(by=['z'], axis=0, ascending=True))

  fig = plt.figure()
  ax = fig.add_subplot(111, projection='3d')
  ax.plot_trisurf(x, y, z, cmap='coolwarm')
  #ax.scatter(x, y, z)
  plt.title(f'{x_label}, {y_label}')
  ax.set_xlabel(x_label, fontsize=10)
  ax.set_ylabel(y_label, fontsize=10)
  ax.set_zlabel('Norm. Power-Delay Product')
  #plt.show()
  plt.savefig(get_plot_file_name(), dpi=400)

def plot_max_d_vs_xy(
    csv_file, suffix, x_source, x_label, y_source, y_label):
  in_csv_file = Path(csv_file)
  raw_data = pd.read_csv(in_csv_file)
  data = raw_data.loc[lambda df: df['sub_dir'].apply(
    lambda x: isinstance(x, str) and x.endswith(suffix))]

  d_cols = [c for c in data.columns if c in [f'D{i}' for i in range(16)]]
  z = data[d_cols].max(axis=1)
  x = data[x_source].apply(to_number)
  y = data[y_source].apply(to_number)

  fig = plt.figure()
  ax = fig.add_subplot(111, projection='3d')
  ax.plot_trisurf(x, y, z, cmap='coolwarm')
  #ax.scatter(x, y, z)
  ax.set_xlabel(x_label)
  ax.set_ylabel(y_label)
  ax.set_zlabel('max delay (ps)')
  plt.savefig(get_plot_file_name(), dpi=300)


if __name__ == '__main__':
  #print(data.columns.values)
  if args.surface_plot:
    #plot_max_d_vs_xy(args.surface_plot, '0', args.x, args.x_label, args.y, args.y_label)
    #plot_max_d_vs_xy(args.surface_plot, '1', args.x, args.x_label, args.y, args.y_label)
    plot_max_d_vs_xy_merged_runs(
        args.surface_plot, args.x, args.x_label, args.y, args.y_label)
    plot_pd_vs_xy_merged_runs(
        args.surface_plot, args.x, args.x_label, args.y, args.y_label)
  else:
    plot_register_scan_chain_load()
    plot_lut_readout()
    plot_input_buffers()
    plot_mux_propagation()
    #plot_lut_presentation()

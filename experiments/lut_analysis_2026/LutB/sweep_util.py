#!/usr/bin/env python3
from pathlib import Path
import glob
import shutil

PATTERNS = [
  '0110011001100110',
  '1001100110011001'
]

def make_sim_bundle(params, input_pattern, destination):
  if destination.exists():
    if not destination.is_dir():
      print(f'error: already exists but is not a directory: {destination}')
      return
  else:
    destination.mkdir()

  # Copy starting data
  starting_files = glob.glob('*.sp')
  for path in starting_files:
    shutil.copy2(path, destination)

  # Write scan_data.sp
  scan_data_path = destination / Path('scan_data.sp')
  with open(scan_data_path, 'w') as f:
    f.write('''* input written by sweep.py
vscan_data
+ scan_in VSS
+ pat ({1.8} {0} {0} {50p} {50p} {scan_clock_period} b0''' +
input_pattern + '''0)
''')

  # Write params.sp
  param_path = destination / Path('params.sp')
  with open(param_path, 'w') as f:
    f.write('* param file written by sweep.py\n')
    f.writelines(f'.param {k}={v}\n' for k, v in params.items())


def generate_all_params():
  for mux_nfet_width in [640]:
    for mux_pfet_width in [480]:
      for reg_out_nfet_width in [650]:
        for reg_out_pfet_width in [1000]:
          for s0s1_nfet_width in range(460,1000,20):
            for s0s1_pfet_width in range(460, 1600 - s0s1_nfet_width, 20):
              params = {
                  #'s2_nfet_width': f'{s2_nfet_width}m',
                  #'s2_pfet_width': f'{s2_pfet_width}m',
                  's0s1_nfet_width': f'{s0s1_nfet_width}m',
                  's0s1_pfet_width': f'{s0s1_pfet_width}m',
                  'mux_nfet_width': f'{mux_nfet_width}m',
                  'mux_pfet_width': f'{mux_pfet_width}m',
                  'reg_out_nfet_width': f'{reg_out_nfet_width}m',
                  'reg_out_pfet_width': f'{reg_out_pfet_width}m',
              }
              yield params

def generate_test_params():
  params = {
      's2_nfet_width': '510m',
      's2_pfet_width': '900m',
      's0s1_nfet_width': '440m',
      's0s1_pfet_width': '760m',
      'mux_nfet_width': '580m',
      'mux_pfet_width': '460m',
      'reg_out_nfet_width': '640m',
      'reg_out_pfet_width': '540m'
  }
  yield params


def main(param_maker=generate_all_params, patterns=PATTERNS):
  commands_path = Path('commands.sh')
  param_count = 0
  with open(commands_path, 'w') as c:
    for params in param_maker():
      pattern_count = 0
      for pattern in patterns:
        path = Path(f'run_{param_count}_{pattern_count}')
        make_sim_bundle(params, pattern, path)
        c.write(f'bash -c "cd {path}; Xyce lut_test.sp"\n')
        pattern_count += 1
      param_count += 1
  

if __name__ == '__main__':
  main()

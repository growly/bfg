#!/usr/bin/env python3
import sweep_util

PATTERNS = [
  '0110011001100110',
  '1001100110011001'
]

def generate_test_params():
  params = {
      's1s2_nfet_width': '510m',
      's1s2_pfet_width': '900m',
      's0_nfet_width': '440m',
      's0_pfet_width': '760m',
      'mux_nfet_width': '580m',
      'mux_pfet_width': '460m',
      'reg_out_nfet_width': '640m',
      'reg_out_pfet_width': '540m'
  }
  yield params

def main():
  sweep_util.main(param_maker=generate_test_params, patterns=PATTERNS)
  

if __name__ == '__main__':
  main()

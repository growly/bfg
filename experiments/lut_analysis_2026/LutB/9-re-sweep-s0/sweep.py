#!/usr/bin/env python3
import sweep_util

PATTERNS = [
  '0110011001100110',
  '1001100110011001'
]

def generate_parmesan():
  params = {
      's1s2_nfet_width': '510m',
      's1s2_pfet_width': '900m',
      'mux_nfet_width': '580m',
      'mux_pfet_width': '460m',
      'reg_out_nfet_width': '640m',
      'reg_out_pfet_width': '540m'
  }
  for s0_nfet_width in range(400, 640, 10):
    for s0_pfet_width in range(650, 1000, 20):
      params.update({
          's0_nfet_width': f'{s0_nfet_width}m',
          's0_pfet_width': f'{s0_pfet_width}m',
      })
      yield params

def main():
  sweep_util.main(param_maker=generate_parmesan, patterns=PATTERNS)
  

if __name__ == '__main__':
  main()

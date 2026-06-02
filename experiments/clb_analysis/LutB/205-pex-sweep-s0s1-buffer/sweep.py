#!/usr/bin/env python3
import sweep_util

SCAN_DATA_PATTERNS = [
  '0110011001100110',
  '1001100110011001'
]

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


def main():
  sweep_util.main(param_maker=generate_all_params, patterns=SCAN_DATA_PATTERNS)
  

if __name__ == '__main__':
  main()

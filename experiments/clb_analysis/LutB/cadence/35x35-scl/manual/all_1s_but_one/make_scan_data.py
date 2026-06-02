#!/usr/bin/env python3

import sys

i = int(sys.argv[1])

pattern = 'B0'
for j in range(i):
    pattern += '1'
pattern += '0'
for j in range(i, 15):
    pattern += '1'
pattern += '0'

statement= f'''vscan_data
+ scan_in VSS
+ pat ("1.8" "0" "0" "50p" "50p" "scan_clock_period" {pattern})
'''

print(statement)

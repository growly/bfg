#!/usr/bin/env python3

import csv
import collections
from pathlib import Path
import sys
import tabulate

def read_measure_file(path):
  results = {}
  with open(path) as f:
    for line in f:
      parts = line.split('=')
      if not parts or len(parts) != 2:
        continue
      variable = parts[0].strip()
      value = parts[1].strip()
      results[variable] = value
  return results

def read_params_file(path):
  results = {}
  with open(path) as f:
    for line in f:
      if not parts or len(parts) != 2:
        continue
      parts = line.split('=')
      variable = parts[0].strip()
      value = parts[1].strip()
      results[variable] = value
  return results

results_by_sub_dir = collections.defaultdict(lambda: {})

for globbed in Path().rglob("*"):
  if (globbed.suffix == ".mt0"):
    results = read_measure_file(globbed)
    results_by_sub_dir[globbed.parent].update(results)
    #print('read', globbed)
  elif (globbed.name == "params.sp"):
    results = read_measure_file(globbed)
    results_by_sub_dir[globbed.parent].update(results)
    #print('read', globbed)

headers = ['sub_dir']
rows = []

for sub_dir, result in results_by_sub_dir.items():
  row = [sub_dir.name] + [''] * (len(headers) - 1)
  for key, value in result.items():
    if key not in headers:
      index = len(headers)
      headers.append(key)
      row.append(value)
      continue
    index = headers.index(key)
    row[index] = value

  print('read', row)

  rows.append(row)

print(tabulate.tabulate(rows, headers=headers))
with open('collected.csv', 'w') as csvfile:
  writer = csv.writer(csvfile)
  writer.writerow(headers)
  for row in rows:
    writer.writerow(row)

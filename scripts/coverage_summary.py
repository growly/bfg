#!/usr/bin/env python3
"""Summarises gcov JSON output by source file.

Usage: coverage_summary.py <src_dir> <gcov_json_dir> [--functions FILE...]

Prints line and function coverage for every non-test source file under
src_dir, least covered first, then the overall totals. With --functions, also
lists the unexecuted functions in the given files (relative to src_dir),
largest first.
"""

import collections
import glob
import gzip
import json
import os
import sys


def load(src_dir, gcov_dir):
  files = collections.defaultdict(lambda: {'lines': {}, 'functions': {}})
  for path in glob.glob(os.path.join(gcov_dir, '*.gcov.json.gz')):
    with gzip.open(path) as f:
      data = json.load(f)
    cwd = data.get('current_working_directory', '')
    for entry in data['files']:
      source = os.path.realpath(os.path.join(cwd, entry['file']))
      if (not source.startswith(src_dir + os.sep) or
          source.endswith('_test.cc') or '.pb.' in source):
        continue
      record = files[os.path.relpath(source, src_dir)]
      # Headers are compiled into many objects, so sum their counts.
      for line in entry['lines']:
        number = line['line_number']
        record['lines'][number] = (
            record['lines'].get(number, 0) + line['count'])
      for function in entry['functions']:
        name = function['demangled_name']
        count, _, _ = record['functions'].get(name, (0, 0, 0))
        record['functions'][name] = (
            count + function['execution_count'],
            function['start_line'],
            function['end_line'])
  return files


def percent(hit, total):
  return 100.0 * hit / total if total else 100.0


def main(argv):
  if len(argv) < 3:
    print(__doc__, file=sys.stderr)
    return 1
  src_dir = os.path.realpath(argv[1])
  files = load(src_dir, argv[2])
  detail = argv[argv.index('--functions') + 1:] if '--functions' in argv else []

  rows = []
  for name, record in files.items():
    lines = record['lines']
    functions = record['functions']
    rows.append((
        name,
        sum(1 for count in lines.values() if count), len(lines),
        sum(1 for f in functions.values() if f[0]), len(functions)))
  rows.sort(key=lambda r: (percent(r[1], r[2]), -r[2]))

  for name, line_hit, line_total, fn_hit, fn_total in rows:
    print(f'{name:60s} lines {line_hit:5d}/{line_total:5d} '
          f'({percent(line_hit, line_total):5.1f}%)  '
          f'functions {fn_hit:4d}/{fn_total:4d}')

  line_hit = sum(r[1] for r in rows)
  line_total = sum(r[2] for r in rows)
  fn_hit = sum(r[3] for r in rows)
  fn_total = sum(r[4] for r in rows)
  print(f'\nTotal: lines {line_hit}/{line_total} '
        f'({percent(line_hit, line_total):.1f}%), '
        f'functions {fn_hit}/{fn_total} ({percent(fn_hit, fn_total):.1f}%)')

  for name in detail:
    print(f'\n== Unexecuted functions in {name}')
    functions = files.get(name, {'functions': {}})['functions']
    for function, (count, start, end) in sorted(
        functions.items(), key=lambda kv: kv[1][1] - kv[1][2]):
      if count == 0:
        print(f'  {end - start + 1:4d} lines  L{start:<5d} {function[:100]}')
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))

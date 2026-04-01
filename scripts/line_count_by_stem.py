#!/usr/bin/env python3

import sys
from pathlib import Path
import collections
import tabulate

counts_by_root = collections.defaultdict(int)

for line in sys.stdin:
  # 'line' includes the trailing newline '\n'
  entries = line.strip().split(' ')

  count = entries[0]
  rest = ' '.join(entries[1:])

  path = Path(rest)

  counts_by_root[path.stem] += int(count)

rows = []
for entry in counts_by_root.items():
  rows.append(entry)

print(tabulate.tabulate(rows))

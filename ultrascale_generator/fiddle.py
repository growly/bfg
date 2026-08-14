#!/usr/bin/env python3

import collections
import csv
import itertools
import argparse
import re
import sys
import pygtrie
import tabulate
import natsort

parser = argparse.ArgumentParser()
parser.add_argument('--edges', action='store',
          default='../xczu3eg_INT_X20Y100.csv')
parser.add_argument('--labels', action='store',
          default='../xczu3eg_INT_X20Y100.nodes.csv')
parser.add_argument('--human_sort', '-H', action='store_true', default=True)

parser.add_argument('--show_node', action='append', default=[])
parser.add_argument('--show_ble', action='store')
parser.add_argument('--fan_out', action='store')

parser.add_argument('--distinguish_remote_nodes', action='store_true',
          default=True)

parser.add_argument('--write_adjacency_matrices_with_prefix', '-m', action='store',
          default=None)

parser.add_argument('--custom_matrix', action='store_true', default=False)

# Using 'add_driver' or 'add_driven' adds the fan-outs (and fan-ins,
# respectively) of each node that matches.
parser.add_argument('--add_driven', action='append', default=[])
parser.add_argument('--add_driver', action='append', default=[])

# Using 'add_symmetric' adds nodes to both the list of drivers and the list of
# drivens, but doesn't expand on their fan outs or fan ins.
parser.add_argument('--add_symmetric', action='append', default=[])

parser.add_argument('--summarise_some_ble_outputs', action='store_true',
          default=False)
parser.add_argument('--summarise_iib_layer_1_to_2', action='store_true',
          default=False)
parser.add_argument('--summarise_iib_layer_2_inputs', action='store_true',
          default=False)

parser.add_argument('--sort_columns', action='store_true')
parser.add_argument('--sort_column_pattern', action='store', default=None)

# Some nodes differ only in the suffix "OUT0" or "OUT1". If the driver set is
# the same and this argument is set, they will be merged to a single node.
parser.add_argument('--merge_outs', action='store_true', default=False)

DELETE_NODES = set([
   'INT/VCC_WIRE'
])


def make_ok_dot_token(string: str) -> str:
  #return string.replace(
  #    '/', '__').replace('.', '__').replace('(','__').replace(')', '__')
  return f'"{string}"'


class Node:
  def __init__(self, name, kind):
    self.name = name
    self.kind = kind
    self.dx = 0
    self.dy = 0
    self.index = None
    self.label = None
    self.ble = None
    self.drives = set()
    self.driven_by = set()

  def dot_name(self):
    return make_ok_dot_token(self.name)

  def is_diagonal(self):
    return self.dx != 0 and self.dy != 0


InOutNode = collections.namedtuple('InOutNode', ['node', 'in_or_out'])


edges = []
nodes_by_name = pygtrie.CharTrie()


def load_edges(file_name, distinguish_remote_nodes=False):
  '''Read a CSV file with one row per edge marking driving node, driven node,
  their types, the x/y tile differences spanned by the connection, and
  possibly a final couple of columns with annotating labels. For example:

  ,fromType,fromInst,toType,toInst,deltaX,deltaY,delay,,comment,ble
  CONN,NODE_LOCAL,INT/INT_NODE_IMUX_1_INT_OUT0,NODE_PINBOUNCE,INT/BYPASS_E4,0,0,0,#,CLEL_R.A_I,CLEL_R.A
  CONN,NODE_LOCAL,INT/INT_NODE_IMUX_3_INT_OUT0,NODE_PINBOUNCE,INT/BYPASS_E4,0,0,0,#,CLEL_R.A_I,CLEL_R.A
  CONN,NODE_LOCAL,INT/INT_NODE_IMUX_30_INT_OUT0,NODE_PINBOUNCE,INT/BYPASS_E4,0,0,0,#,CLEL_R.A_I,CLEL_R.A
  CONN,NODE_LOCAL,INT/INT_NODE_IMUX_5_INT_OUT1,NODE_PINBOUNCE,INT/BYPASS_E4,0,0,0,#,CLEL_R.A_I,CLEL_R.A
  CONN,NODE_LOCAL,INT/INT_NODE_IMUX_7_INT_OUT1,NODE_PINBOUNCE,INT/BYPASS_E4,0,0,0,#,CLEL_R.A_I,CLEL_R.A
  CONN,NODE_LOCAL,INT/INT_NODE_IMUX_9_INT_OUT1,NODE_PINBOUNCE,INT/BYPASS_E4,0,0,0,#,CLEL_R.A_I,CLEL_R.A
  '''
  with open(file_name, 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
      if len(row) < 5:
        continue
      source_type = row['fromType']
      source_node = row['fromInst']
      target_type = row['toType']
      target_node = row['toInst']
      deltaX = row['deltaX']
      deltaY = row['deltaY']

      if distinguish_remote_nodes:
        # By eyeballing the edge list RapidWright spits out, I have come
        # to believe that the deltaX and deltaY numbers tells where the
        # source driver is relative to the driven node. So for example
        # INT/WW4_E_BEG4 having deltaX=2 and deltaY=0 makes sense
        # because we expect it to be the west-ward 4-unit wire from the
        # east side of the interconnect tile 2 tiles to the right (in
        # the x dimension).
        source_node += f'({deltaX},{deltaY})'
        target_node += '(0,0)'

      for kind, name in (
          (source_type, source_node), (target_type, target_node)):
        if name not in nodes_by_name:
          node = Node(name, kind)
          nodes_by_name[name] = node
      
      source = nodes_by_name[source_node]
      target = nodes_by_name[target_node]

      if source.name in DELETE_NODES or target.name in DELETE_NODES:
        continue

      source.dx = deltaX
      source.dy = deltaY

      source.drives.add(target)
      target.driven_by.add(source)

      if row['comment']:
        commented = row['comment']
        if commented not in nodes_by_name:
          node = Node(commented, 'COMMENTED_GUESS')
          nodes_by_name[commented] = node
        else:
          node = nodes_by_name[commented]
        target.drives.add(node)
        node.driven_by.add(target)

        if row['ble']:
          node.ble = row['ble']


def get_nodes_by_label(label):
  return [node for node in nodes_by_name.values() if node.label == label]


def get_nodes_by_prefix(prefix):
  return nodes_by_name.values(prefix)


nodes = []

def load_labels(file_name):
  ''' Read a CSV file with labels for each node, e.g.:

  $ head -n 7 xczu3eg_INT_X20Y100.nodes.csv
  name,label
  CLEL_R.A1,CLE_INPUT
  CLEL_R.A2,CLE_INPUT
  CLEL_R.A3,CLE_INPUT
  CLEL_R.A4,CLE_INPUT
  CLEL_R.A5,CLE_INPUT
  CLEL_R.A6,CLE_INPUT
  '''
  with open(file_name, 'r') as f:
    reader = csv.DictReader(f)

    for row in reader:
      try:
        name = row['name']
        label = row['label']
      except KeyError:
        continue

      for node in get_nodes_by_prefix(name):
        node.label = label

  for i, name in enumerate(sorted(nodes_by_name.keys())):
    node = nodes_by_name[name]
    node.index = i
    nodes.append(node)


def human_sort_nodes(nodes):
  # We will human (natural) sort the nodes!
  natural_keys = lambda s: [int(c) if c.lstrip('-').isdigit() else c
                for c in re.split(r'(-?\d+)', s.name)]

  nodes.sort(key=natural_keys)


#with open('nodes.csv', 'w') as f:
#  f.writelines([n.name + '\n' for n in nodes])

# 2-d adjacency matrix. Entry (i, j) pertains to row i, column j, and contains a
# 1 iff node i drives node j.
def write_adjacency_matrix(
    file_name, nodes, sort_columns=False, sort_column_pattern=None):
  adjacency = [];

  for i, source in enumerate(nodes):
    row = [source.name]
    for j, target in enumerate(nodes):
      if target in source.drives:
        row.append(1)
      else:
        row.append(0)
    adjacency.append(row)

  if sort_columns:
    columns = []
    if sort_column_pattern:
      matcher = re.compile(sort_column_pattern)
      for i, node in enumerate(nodes):
        match = matcher.search(node.name)
        if match:
          columns.append(i)
    min_index = min(columns) + 1 if columns else 1
    max_index = max(columns) + 2 if columns else len(nodes)
    adjacency.sort(key=lambda r: r[min_index:max_index], reverse=True)

  with open(file_name, 'w') as f:
    writer = csv.writer(f)
    header = [''] + [node.name for node in nodes]
    writer.writerow(header)
    for i, row in enumerate(adjacency):
      writer.writerow(row)

#for name, node in nodes_by_name.items():
#  #print(f'{node.kind} drives {len(node.drives)}')
#  #print(f'{node.name}: {node.kind} ({node.dx}, {node.dy}) drives {len(node.drives)}')
#  if name.find("INT/IMUX_") != -1:
#    print(f'{node.name}: {node.kind} drives {len(node.drives)}')

RANK_BY_LABEL = {
  'IIB_LAYER_1': 1,
  'IIB_LAYER_2': 2,
  'CLE_INPUT': 3,
  'CLE_OUTPUT': 4,
  'OIB_LAYER_2': 5,
  'OIB_LAYER_1': 6,
}
def write_ranked_columns(nodes, rank_by_label):
  nodes_by_rank = collections.defaultdict(list)
  edges = set()
  for node in nodes:
    if node.label not in rank_by_label:
      continue
    nodes_by_rank[rank_by_label[node.label]].append(node)

    for in_node in node.driven_by:
      edges.add((in_node.dot_name(), node.dot_name()))
    for out_node in node.drives:
      edges.add((node.dot_name(), out_node.dot_name()))

  # see https://stackoverflow.com/questions/50812267/increasing-rank-spacing-in-graphviz
  # can also add 
  #  size = "16.66,8.33!"; // 1200x600 at 72px/in, "!" to force
  #  ratio = "fill"; // see https://graphviz.gitlab.io/_pages/doc/info/attrs.html#d:ratio
  print('digraph {')
  print('  graph [rankdir="LR"]')
  print('  splines="ortho"')
  for key in sorted(nodes_by_rank.keys()):
    print('  {')
    print('  rank=same')
    for node in nodes_by_rank[key]:
      print(f'  {node.dot_name()}')
    print('  }')

  for edge in edges:
    source, sink = edge
    print(f'  {source} -> {sink}')
  print('}')

def nodes_from_names(names):
  nodes = set()
  for name in names:
    node = nodes_by_name[name]
    nodes.add(node)
  return nodes

# dump the fan-in and fan-out of one node
def focus_nodes(nodes):
  fan_in = set()
  fan_out = set()
  for node in nodes:
    fan_in.update(node.driven_by)
    fan_out.update(node.drives)

  print('digraph {')
  print('  graph [rankdir="LR"]')
  print('  {')
  print('  rank=same')
  for node in fan_in:
    print(f'  {node.dot_name()} [shape="box"]')
  print('  }')
  print('  {')
  print('  rank=same')
  for node in nodes:
    print(f'  {node.dot_name()} [shape="oval"]')
  print('  }')
  print('  {')
  print('  rank=same')
  for node in (fan_out - fan_in):
    print(f'  {node.dot_name()} [shape="box"]')
  print('  }')

  for node in nodes:
    for driven_by in node.driven_by:
      print(f'  {driven_by.dot_name()} -> {node.dot_name()}')
    for drives in node.drives:
      print(f'  {node.dot_name()} -> {drives.dot_name()}')
  print('}')


def focus_ble(ble_name):
  drivers = set()
  for node in nodes:
    if node.ble == ble_name:
      drivers.update(node.driven_by)
  focus_nodes(list(drivers))

class InOutNode:
  def __init__(self, node, in_or_out):
    self.node = node
    self.in_or_out = in_or_out

  def __str__(self):
    #return f'({self.node.name}, {self.in_or_out})'
    return f'({self.node}, {self.in_or_out})'

  def __repr__(self):
    return self.__str__()


def find_longest_chain(connectivity_pairs, node, in_or_out, visited):
  visited = set(visited)
  paths = [[InOutNode(node, in_or_out)]]
  chains = [[]]

  in_by_out = collections.defaultdict(list)
  out_by_in = collections.defaultdict(list)
  for pair in connectivity_pairs:
    in_node, out_node = pair
    in_by_out[out_node].append(in_node)
    out_by_in[in_node].append(out_node)

  while paths:
    path = paths.pop()
    last_entry = path[-1]
    search = (in_by_out[last_entry.node]
          if last_entry.in_or_out == 'out'
          else out_by_in[last_entry.node])
    flip = 'in' if last_entry.in_or_out == 'out' else 'out'
    #import pdb
    #pdb.set_trace()
    for node in search:
      if last_entry.in_or_out == 'out':
        seen_pair = (node, last_entry.node)
      else:
        seen_pair = (last_entry.node, node)
      if seen_pair in visited:
        continue
      new_path = path + [InOutNode(node, flip)]
      chains.append(new_path)
      paths.append(new_path)
      visited.add(seen_pair)
  return chains


def find_longest_chain_in_mux_pair(connectivity_pairs, pair):
  in_node, out_node = pair
  out_chains = find_longest_chain(connectivity_pairs, out_node, 'out', [pair])
  in_chains = find_longest_chain(connectivity_pairs, in_node, 'in', [pair])

  chains = []
  for in_chain, out_chain in itertools.product(in_chains, out_chains):
    if not in_chain and not out_chain:
      chains.append([
        InOutNode(in_node, 'in'), InOutNode(out_node, 'out')])
    else:
      chains.append(list(reversed(in_chain)) + out_chain)

  chains.sort(key=lambda x: len(x), reverse=True)
  return chains[0]

# Assume input of muxes, with fan-in and fan-out. Find a cover over the space of
# (in, out) pairs that maximised source/drain sharing.
def generate_mux_pairs(muxes):
  pairs = set()
  for mux in muxes:
    for in_node in mux.driven_by:
      pairs.add((in_node.name, mux.name + '_O'))

  return pairs


def write_adjacency_matrices(
    prefix='', sort_columns=False, sort_column_pattern=None):
  # only include edges touching these labels in the graph
  allowed_labels = set([
    'INTERCONNECT', 'OIB_LAYER_1', 'OIB_LAYER_2', 'CLE_OUTPUT'])
  filtered = list(filter(lambda x: x.label in allowed_labels, nodes))
  write_adjacency_matrix(
    f'{prefix}oib.adjacency.csv', filtered, sort_columns,
    sort_column_pattern)

  allowed_labels = set([
    'INTERCONNECT', 'IIB_LAYER_1', 'IIB_LAYER_2', 'CLE_INPUT'])
  filtered = list(filter(lambda x: x.label in allowed_labels, nodes))
  write_adjacency_matrix(
    f'{prefix}iib.adjacency.csv', filtered, sort_columns,
    sort_column_pattern)

  allowed_labels = set(['INTERCONNECT'])
  filtered = list(filter(lambda x: x.label in allowed_labels, nodes))
  write_adjacency_matrix(
    f'{prefix}interconnect.adjacency.csv', filtered, sort_columns,
    sort_column_pattern)


def pairs_in_chain(chain):
  pairs = set()
  for i in range(len(chain) - 1):
    left = chain[i]
    right = chain[i + 1]

    if left.in_or_out == 'in':
      pair = (left.node, right.node)
    else:
      pair = (right.node, left.node)
    pairs.add(pair)
  return pairs


def print_chain_as_transistors(chain):
  #print(' | '.join(f'{c.node} [{c.in_or_out}]' for c in chain))
  print(' | '.join(c.node for c in chain))


# num gates, including dummies, required for chain and separation on one side.
def chain_width(chain):
  return len(chain) + 1


def find_cover_ordered(pairs):
  num_pairs_removed = 0
  max_width = 0
  total_width = 0
  chains = []
  while pairs:
    pair = pairs.pop()
    pairs.append(pair)
    #print(f'---- {pair[0]} {pair[1]}')
    chain = find_longest_chain_in_mux_pair(set(pairs), pair)
    chains.append(chain)
    width = chain_width(chain)
    max_width = max(max_width, width)
    total_width += width
    chain_pairs = pairs_in_chain(chain)
    #print(len(pairs), len(chain_pairs), chain)
    #print_chain_as_transistors(chain)
    for pair in pairs_in_chain(chain):
      pairs.remove(pair)
      num_pairs_removed += 1

  #print(f'{total_width=}')
  #print(f'{max_width=}')
  #print(f'{num_pairs_removed=}')
  return (chains, max_width, total_width)


def dumb_way_of_finding_covers():
  pairs = generate_mux_pairs(nodes_from_names(sys.argv[1:]))

  pairs = set([
    ('A', '1'),
    ('B', '1'),
    ('C', '1'),
    ('D', '1'),
    ('E', '1'),
    ('F', '1'),
    ('A', '3'),
    ('B', '3'),
    ('C', '3'),
    ('D', '3'),
    ('G', '3'),
    ('H', '3'),
    ('D', '5'),
    ('E', '5'),
    ('F', '5'),
    ('I', '5'),
    ('J', '5')
  ])

  least_width = None
  best = None
  for permuted in itertools.permutations(pairs):
    #print(permuted)
    chains, max_width, total_width = find_cover_ordered(list(permuted))
    if best is None:
      print('initial')
      best = chains
      least_width = total_width
    elif total_width < least_width:
      print('improvement')
      least_width = total_width
      best = chains

  print(f'best {least_width=}')
  for chain in chains:
    print_chain_as_transistors(chain)


def merge_differing_by_out_suffix():
  result = merge_differing_by_out_suffix_once()
  while result is not None:
    result = merge_differing_by_out_suffix_once()


# FIXME(aryap): Rewrite this to factor how these are probably interdigitated
# muxes.
def merge_differing_by_out_suffix_once():
  for node in nodes:
    match = re.match(r'(.*_OUT)\d[\(\),\d-]*$', node.name)
    if not match:
      continue

    common_prefix = match.groups()[0]
    
    # Use trie to find others with starting name:
    matching_nodes = get_nodes_by_prefix(common_prefix)

    #print(f'found {len(matching_nodes)} match(es) for {common_prefix}')
    if not matching_nodes:
      # Huh?
      continue

    first = matching_nodes[0]
    same_inputs = [first]
    for node in matching_nodes[1:]:
      if node.driven_by == first.driven_by:
        same_inputs.append(node)
      #else:
      #  print(', '.join(n.name for n in node.driven_by))
      #  print(', '.join(n.name for n in first.driven_by))

    if len(same_inputs) < 2:
      continue

    print(f"merging {', '.join(n.name for n in same_inputs)}")

    del nodes_by_name[first.name]
    first.name = common_prefix
    nodes_by_name[first.name] = first

    for node in same_inputs[1:]:
      nodes.remove(node)
      del nodes_by_name[node.name]

      first.drives.update(node.drives)

      for drives in node.drives:
        drives.driven_by.remove(node)
        drives.driven_by.add(first)

      for driven_by in node.driven_by:
        driven_by.drives.remove(node)
        # should be redundant!
        #driven_by.drives.add(first)

    return first
  return None


def summarise_some_luts_to_iib_layer_2():
  pattern = re.compile(r'INT/INT_NODE_IMUX_(\d+)_INT_OUT[01]')
  interconnect_pattern = re.compile(r'INT/([NESW]{2}\d+).*_BEG(\d+).*')
  
  filtered = list(filter(
    lambda x: x.label in ('INTERCONNECT') if x.label else False, nodes))

  iib_layer_1_by_driver = collections.defaultdict(list)
  for node in filtered:
    int_match = interconnect_pattern.match(node.name)
    if not int_match:
      continue
    name = f'{int_match.group(1)}\t{int_match.group(2)}'

    for driven in node.drives:
      match = pattern.match(driven.name)
      if not match:
        continue
      iib_layer_1_by_driver[name].append(match.group(1))

  for node, driven in iib_layer_1_by_driver.items():
    driven_list = ', '.join(sorted(driven))
    set_list = ', '.join(sorted(list(set(driven))))
    print(f'{node}\t{driven_list}\t{set_list}')

def summarise_some_interconnect_to_iib_layer_2():
  pattern = re.compile(r'INT/INT_NODE_IMUX_(\d+)_INT_OUT[01]')
  interconnect_pattern = re.compile(r'INT/([NESW]{2}\d+).*_BEG(\d+).*')
  
  filtered = list(filter(
    lambda x: x.label in ('INTERCONNECT') if x.label else False, nodes))

  iib_layer_1_by_driver = collections.defaultdict(list)
  for node in filtered:
    int_match = interconnect_pattern.match(node.name)
    if not int_match:
      continue
    name = f'{int_match.group(1)}\t{int_match.group(2)}'

    for driven in node.drives:
      match = pattern.match(driven.name)
      if not match:
        continue
      iib_layer_1_by_driver[name].append(match.group(1))

  for node, driven in iib_layer_1_by_driver.items():
    driven_list = ', '.join(sorted(driven))
    set_list = ', '.join(sorted(list(set(driven))))
    print(f'{node}\t{driven_list}\t{set_list}')


def summarise_some_interconnect_to_oib_layer_2():
  pattern = re.compile(r'INT/INT_NODE_SDQ_(\d+)_INT_OUT[01]')
  interconnect_pattern = re.compile(r'INT/([NESW]{2}\d+).*_BEG(\d+).*')
  
  filtered = list(filter(
    lambda x: x.label in ('INTERCONNECT') if x.label else False, nodes))

  sdq_numbers_by_driver = collections.defaultdict(list)
  for node in filtered:
    int_match = interconnect_pattern.match(node.name)
    if not int_match:
      continue
    name = f'{int_match.group(1)}\t{int_match.group(2)}'

    for driven in node.drives:
      match = pattern.match(driven.name)
      if not match:
        continue
      sdq_numbers_by_driver[name].append(match.group(1))

  for node, driven in sdq_numbers_by_driver.items():
    driven_list = ', '.join(sorted(driven))
    set_list = ', '.join(sorted(list(set(driven))))
    print(f'{node}\t{driven_list}\t{set_list}')


def summarise_iib_layer_1_to_2():
  group_A = get_nodes_by_prefix('INT/BOUNCE_E')
  group_B = get_nodes_by_prefix('INT/BYPASS_E')
  group_C_D = get_nodes_by_prefix('INT/IMUX_E')

  print('NOTE: INT/INT_NODE_IMUX_(number)_INT_OUT(out) are reduced to ordinal '
      '2 * number + out')

  groups = ['A', 'B', 'C', 'D']

  imux_pattern = re.compile(r'INT/IMUX_E(\d+)')
  def extract_range(nodes, start, end):
    result = []
    for node in nodes:
      match = imux_pattern.search(node.name)
      if not match:
        continue
      number = int(match.group(1))
      if number >= start and number < end:
        result.append(node)
    return result

  group_C_D_boundary = int(len(group_C_D) / 3)
  group_C = extract_range(group_C_D, 0, group_C_D_boundary)
  group_D = extract_range(group_C_D, group_C_D_boundary, len(group_C_D))
 
  inode_imux_numbers_by_drivee = collections.defaultdict(list)
  inode_e_numbers_by_drivee = collections.defaultdict(list)

  inode_e_pattern = re.compile(r'INT/INODE_E_(\d+)_FT[01]')
  inode_imux_pattern = re.compile(r'INT/INT_NODE_IMUX_(\d+)_INT_OUT([01])')
  for node in group_A + group_B + group_C_D:
    for driver in node.driven_by:
      inode_imux_match = inode_imux_pattern.search(driver.name)
      if inode_imux_match:
        number = inode_imux_match.group(1)
        out = inode_imux_match.group(2)
        ordinal = 2 * int(number) + int(out)
        inode_imux_numbers_by_drivee[node.name].append(ordinal)
        continue

      inode_e_match = inode_e_pattern.search(driver.name)
      if inode_e_match:
        number = inode_e_match.group(1)
        inode_e_numbers_by_drivee[node.name].append(int(number))

  i = 0
  header = ['Index',
        'IIB Layer 2 Node',
        'INT/INODE_E Drivers',
        'INT/INT_NODE_IMUX Drivers']
  for group in [group_A, group_B, group_C, group_D]:
    print(f'GROUP {groups[i]}: {len(group)} nodes')

    rows = [header]
    sorted_names = natsort.natsorted(n.name for n in group)
    j = 0
    for node_name in sorted_names:
      row = [j, node_name]

      drivers = inode_e_numbers_by_drivee[node_name]
      driver_list = ', '.join([str(x) for x in sorted(drivers)])
      row.append(driver_list)

      drivers = inode_imux_numbers_by_drivee[node_name]
      driver_list = ', '.join([str(x) for x in sorted(drivers)])
      row.append(driver_list)
      rows.append(row)
      j += 1

    print(tabulate.tabulate(rows, headers="firstrow"))
    print()
    i += 1

  # TODO(aryap): I think I'm doing this a dumb way but my brain isn't working.
  group_by_name = {}
  for node in group_A:
    group_by_name[node.name] = 'A'
  for node in group_B:
    group_by_name[node.name] = 'B'
  for node in group_C:
    group_by_name[node.name] = 'C'
  for node in group_D:
    group_by_name[node.name] = 'D'

  # How many muxes from each group does each layer 1 mux drive?
  groups_driven_by_driver = collections.defaultdict(
    lambda : collections.defaultdict(int))
  for node_name, driver_numbers in inode_imux_numbers_by_drivee.items():
    for number in driver_numbers:
      count = groups_driven_by_driver[number][group_by_name[node_name]]
      groups_driven_by_driver[number][group_by_name[node_name]] = count + 1

  print('Nodes driven per group')
  rows = [['INT/INT_NODE_IMUX', 'A', 'B', 'C', 'D']]
  for driver_number in natsort.natsorted(groups_driven_by_driver.keys()):
    counts = groups_driven_by_driver[driver_number]
    row = [driver_number, counts['A'], counts['B'], counts['C'], counts['D']]
    rows.append(row)
  print(tabulate.tabulate(rows, headers='firstrow', tablefmt='tsv'))


# This is only BLE -> OIB S2
def summarise_some_ble_outputs():
  pattern = re.compile(r'INT/INT_NODE_SDQ_(\d+)_INT_OUT[01]')
  cle_pattern = re.compile(
    r'CLEL_R/CLE_CLE_L_SITE_0_([A-H])(_O|Q2|MUX|Q)\(0,0\)')
  
  filtered = get_nodes_by_prefix('CLEL_R/CLE_CLE_L')

  sdq_numbers_by_driver = collections.defaultdict(list)
  for node in filtered:
    cle_match = cle_pattern.match(node.name)
    if not cle_match:
      continue
    cle_string = f'{cle_match.group(1)}\t{cle_match.group(2)}'

    for driven in node.drives:
      match = pattern.match(driven.name)
      if not match:
        continue
      sdq_numbers_by_driver[cle_string].append(match.group(1))

  for node, driven in sdq_numbers_by_driver.items():
    driven_list = ', '.join(sorted(driven))
    set_list = ', '.join(sorted(list(set(driven))))
    print(f'{node}\t{driven_list}\t{set_list}')


def best_guess_layout():
  #                             Interconnect
  #  Interco. IIB S1   IIB S2   BLE in   BLE out  OIB S2  OIB S1   Wire
  #   |    |    |    |    |    |    |     |
  #   |    |    |    |    |    |    |     |
  #   |    |    |    |    |    |    |     |
  #   |    |    |    |    |    |    |     |
  #   |    |    |    |    |    |    |     |
  #   |    |    |    |    |    |    |     |
  #   |    |    |    |    |    |    |     |
  #   |    |    |    |    |    |    |     |
  #
  # These are the columns we will group the picture into, visually (they are
  # also the logical groups I believe best describe the architecture.)
  #
  # TODO(aryap): Refactor this out when you know what it's supposed to do.

  interconnect_in = []
  interconnect_out = []
  for node in nodes:
    if node.label != 'INTERCONNECT':
      continue
    if node.dx == 0 and node.dy == 0:
      interconnect_out.append(node)
    else:
      interconnect_in.append(node)

  iib_s1 = get_nodes_by_label('IIB_LAYER_1')
  iib_s2 = get_nodes_by_label('IIB_LAYER_2')
  ble_in = get_nodes_by_label('CLE_INPUT')
  ble_out = get_nodes_by_label('CLE_OUTPUT')
  oib_s2 = get_nodes_by_label('OIB_LAYER_2')
  oib_s1 = interconnect_out

  node_groups = [
    interconnect_in, iib_s1, iib_s2, ble_in, ble_out, oib_s2, oib_s1]

  edges = set()
  for group in node_groups:
    for node in group:
      for in_node in node.driven_by:
        edges.add((in_node.dot_name(), node.dot_name()))
      for out_node in node.drives:
        edges.add((node.dot_name(), out_node.dot_name()))

  print('digraph {')
  print('  graph [rankdir="LR"]')
  print('  splines="ortho"')
  for i, group in enumerate(node_groups):
    print('  subgraph ' + f'cluster_{i}' + ' {')
    print('  rank=same')
    for node in group:
      print(f'  {node.dot_name()}')
    print('  }')

  for edge in edges:
    source, sink = edge
    print(f'  {source} -> {sink}')
  print('}')
  


def main():
  global nodes

  args = parser.parse_args()
  load_edges(args.edges, args.distinguish_remote_nodes)
  load_labels(args.labels)
  if args.human_sort:
    human_sort_nodes(nodes)
  if args.merge_outs:
    merge_differing_by_out_suffix()

  if args.show_node:
    all_nodes = set()
    for show_node in args.show_node:
      nodes = get_nodes_by_prefix(show_node)
      all_nodes.update(nodes)
      if not nodes:
        print(KeyError(f'No matching nodes: {node_name}'))
    focus_nodes(list(all_nodes))
  elif args.show_ble:
    focus_ble(args.show_ble)
  elif args.fan_out:
    pass
  elif args.write_adjacency_matrices_with_prefix:
    write_adjacency_matrices(
      args.write_adjacency_matrices_with_prefix,
      args.sort_columns,
      args.sort_column_pattern)
  elif args.summarise_some_ble_outputs:
    summarise_some_interconnect_to_oib_layer_2()
    summarise_some_ble_outputs()
  elif args.summarise_iib_layer_1_to_2:
    summarise_iib_layer_1_to_2()
  elif args.summarise_iib_layer_2_inputs:
    summarise_some_interconnect_to_iib_layer_2()
  elif args.custom_matrix:
    fan_in = set()
    fan_out = set()
    for name in args.add_driver:
      nodes = get_nodes_by_prefix(name)
      if nodes is not None:
        fan_in.update(nodes)
        fan_out.update(nodes)
    for name in args.add_driven:
      nodes = get_nodes_by_prefix(name)
      if nodes is None:
        continue
      fan_out.update(nodes)
      for node in nodes:
        fan_in.update(node.driven_by)
    for name in args.add_driver:
      nodes = get_nodes_by_prefix(name)
      if nodes is None:
        continue
      fan_in.update(nodes)
      for node in nodes:
        fan_out.update(node.drives)
    # Not actually what I wanted:
    nodes = natsort.natsorted(list(fan_in | fan_out), key=lambda n: n.name)
    write_adjacency_matrix(
        'custom_matrix.csv',
        nodes,
        args.sort_columns,
        args.sort_column_pattern)
  else:
    best_guess_layout()

if __name__ == '__main__':
  main()

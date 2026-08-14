import csv
import natsort
import collections

# Perhaps counter-intuitively, inputs drive outputs here:
def write_adjacency_matrix(
    file_name, inputs, outputs,
    sort_by_group=False,
    sort_by_type=False,
    skip_empties=False):
  rows = [];

  try:
    columns = natsort.natsorted(outputs.values(), key=lambda n: n.name)
  except Exception as err:
    print(err, 'from: ', outputs.values())

  def by_name():
    for in_node in natsort.natsorted(
      inputs.values(), lambda n: n.name):
      row = [in_node.name] + list(
        1 if in_node.has_out_node(out) else 0
        for out in columns)
      if skip_empties and sum(row[1:]) == 0:
        continue
      rows.append(row)

  # TODO(aryap): These could very well be merged.
  def by_group():
    nodes_by_group = collections.defaultdict(list)
    for node in inputs.values():
      nodes_by_group[node.group].append(node)

    for _, nodes in sorted(nodes_by_group.items(), key=lambda n: n[0]):
      for node in nodes:
        row = [node.name]
        for target in columns:
          if node.has_out_node(target):
            row.append(1)
          else:
            row.append(0)
        rows.append(row)

  def by_type():
    nodes_by_type = collections.defaultdict(list)
    for node in inputs.values():
      nodes_by_type[node.type].append(node)

    for _, nodes in sorted(
        nodes_by_type.items(), key=lambda n: n[0], reverse=True):
      for node in nodes:
        row = [node.name]
        for target in columns:
          if node.has_out_node(target):
            row.append(1)
          else:
            row.append(0)
        rows.append(row)

  if sort_by_group:
    by_group()
  elif sort_by_type:
    by_type()
  else:
    by_name()

  if skip_empties:
    to_delete = []
    for i, target in enumerate(columns):
      num_inputs = sum(row[i+1] for row in rows)
      if num_inputs == 0:
        to_delete.append(i)
    for i in reversed(sorted(to_delete)):
      del columns[i]
      for row in rows:
        del row[i+1]

  with open(file_name, 'w') as f:
    writer = csv.writer(f)
    header = [''] + list(n.name for n in columns)
    writer.writerow(header)
    for row in rows:
      writer.writerow(row)


def write_bfg_edge_list(file_name, inputs, outputs):
  edges = []
  for start in natsort.natsorted(inputs.values(), lambda n: n.name):
    start_port = ''
    start_name = start.name
    if '/' in start.name:
      pieces = start.name.split('/')
      start_port = pieces[1]
      start_name = pieces[0]
    
    for end in natsort.natsorted(start.get_out_nodes(), lambda n: n.name):
      end_port = ''
      end_name = end.name
      if '/' in end.name:
        pieces = end.name.split('/')
        end_port = pieces[1]
        end_name = pieces[0]

      edges.append((start_name, start_port, end_name, end_port))

  with open(file_name, 'w') as f:
    writer = csv.writer(f)
    #header = ['source', 'source_port', 'dest', 'dest_port']
    #writer.writerow(header)
    for edge in edges:
      writer.writerow(list(edge))


def write_basic_edge_list(file_name, edges):
  with open(file_name, 'w') as f:
    writer = csv.writer(f)
    header = ['source', 'dest']
    writer.writerow(header)
    for edge in edges:
      writer.writerow([str(edge[0]), str(edge[1])])

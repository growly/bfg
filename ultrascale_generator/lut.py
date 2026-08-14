from constants import *
from node import *
import functools

class Lut:
  def __init__(self, ordinal, size, num_bypass):
    self.name = 'LUT_' + chr(ord('A') + ordinal) #f'LUT_{ordinal}'
    self.ordinal = ordinal
    self.port_order = list(chr(ord('A') + i) for i in range(size))
    self.bypass_order = list(
      'X_' + chr(ord('A') + size + i) for i in range(num_bypass))
    self.registered_bypass_out_order = list(
        'Q_' + chr(ord('A') + size + i) for i in range(num_bypass))
    self.output_order = ['O'] + self.registered_bypass_out_order + ['MUX']

    self.inputs = {p: None for p in self.port_order + self.bypass_order}
    self.outputs = {p: set() for p in self.output_order}

    self.input_nodes = {}
    for port_name in self.port_order + self.bypass_order:
      node = LutNode()
      node.lut_parent = self
      node.lut_port_name = port_name
      node.type = NodeType.LUT
      node.name = f'{self.name}/{port_name}'
      self.input_nodes[port_name] = node

    self.output_nodes = {}
    for port_name in self.output_order:
      node = LutNode()
      node.lut_parent = self
      node.lut_port_name = port_name
      node.type = NodeType.LUT
      node.name = f'{self.name}/{port_name}'
      self.output_nodes[port_name] = node

  def __repr__(self):
    return f'LUT "{self.name}"'

  def num_free_ports(self):
    # Does not include bypass inputs.
    count = 0
    for p in self.port_order:
      if p not in self.inputs or self.inputs[p] is None:
        count += 1
    return count

  # Returns all nodes fanning in across all inputs:
  def get_in_nodes(self):
    in_nodes = set()
    for node in self.input_nodes.values():
      in_nodes.update(node.get_in_nodes())
    return in_nodes

  # Returns all nodes fanning out across all outputs:
  def get_out_nodes(self):
    out_nodes = set()
    for node in self.output_nodes.values():
      out_nodes.update(node.get_out_nodes())
    return out_nodes

  def connect_first_bypass(self, node):
    for port_name in self.bypass_order:
      if self.inputs[port_name] is None:
        self.inputs[port_name] = node
        connect_nodes(node, self.input_nodes[port_name])
        return
    print(
        f'warning: no free bypass port on {self}, leaving disconnected: {node}')
    raise KeyError('no free bypass port')

  def connect_first_input(self, node):
    for port_name in self.port_order:
      if self.inputs[port_name] is None:
        self.inputs[port_name] = node
        connect_nodes(node, self.input_nodes[port_name])
        return
    print(
        f'warning: no free input port on {self}, leaving disconnected: {node}')
    raise KeyError('no free input port')

  def connect_output(self, port_name, node, skip_exceeded_fanout=True):
    port_node = self.output_nodes[port_name]
    if len(port_node.get_out_nodes()) >= LUT_FAN_OUT:
      if skip_exceeded_fanout:
        return
      else:
        print(f'warning: {self.name}/{port_name} fan_out already exceeds'
              f'{LUT_FAN_OUT}')
    node.connect_input(self.output_nodes[port_name])
    print(f'connected {self.name}/{port_name} -> {node.name}')
    self.outputs[port_name].add(node)
    self.output_nodes[port_name].connect_output(node)

  def outputs_by_node_name(self):
    qualified = {node.name: node for node in self.output_nodes.values()}
    return qualified


def list_by_fanout(nodes):
  sort = sorted(nodes, key=lambda n: len(n.get_out_nodes()), reverse=True)
  rows = [['node', 'fan out']] + list(
    [node.name, len(node.get_out_nodes())] for node in sort)
  print(tabulate.tabulate(rows, headers='firstrow'))

def list_by_fanin(nodes):
  sort = sorted(nodes, key=lambda n: len(n.get_in_nodes()), reverse=True)
  rows = [['node', 'fan in']] + list(
    [node.name, len(node.get_in_nodes())] for node in sort)
  print(tabulate.tabulate(rows, headers='firstrow'))

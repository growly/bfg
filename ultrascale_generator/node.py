import sys
from enum import IntEnum, Enum

class NoFreeInputs(Exception):
  pass

class NodeType(IntEnum):
  WIRE_IN = 0
  WIRE_OUT = 1
  MUX = 2
  SHARED_MUX = 3
  SHARED_MUX_PARTITION = 4
  LUT = 5


NODE_TYPE_TO_NAME = {
  NodeType.WIRE_IN: "WIRE_IN",
  NodeType.WIRE_OUT: "WIRE_OUT",
  NodeType.MUX: "MUX",
  NodeType.SHARED_MUX: "SHARED_MUX",
  NodeType.SHARED_MUX_PARTITION: "MUX_PARTITION",
  NodeType.LUT: "LUT"
}


# We need a slightly more sophisticated model for this bit.
class Node:
  def __init__(self):
    self.name = -1
    self.group = -1
    self.ordinal = None
    self.netlist_name = None
    self.type = None

    self.max_num_inputs = 0
    self.max_num_lut_inputs = 0

    self._out_nodes = set()
    self._in_nodes = set()

  def get_out_nodes(self):
    return self._out_nodes

  def has_out_node(self, node):
    return node in self._out_nodes

  def get_in_nodes(self):
    return self._in_nodes

  def has_in_node(self, node):
    return node in self._in_nodes

  def fan_out(self):
    return len(self._out_nodes)

  def fan_in(self):
    return len(self._in_nodes)

  def connect_input(self, node):
    self._in_nodes.add(node)

  def connect_output(self, node):
    self._out_nodes.add(node)

  def disconnect_input(self, node):
    if node not in self._in_nodes:
      return
    self._in_nodes.remove(node)

  def disconnect_output(self, node):
    if node not in self._out_nodes:
      return
    self._out_nodes.remove(node)

  def inputs_of_type(self, node_type):
    return [node for node in self._in_nodes if node.type == node_type]

  def num_inputs_of_type(self, node_type):
    return len(self.inputs_of_type(node_type))

  def num_lut_inputs(self):
    return self.num_inputs_of_type(NodeType.LUT)

  def ok_for_lut_connection(self):
    #return self.fan_in() < self.max_num_inputs and (
    #    self.num_lut_inputs() < self.max_num_lut_inputs)
    return self.fan_in() < self.max_num_inputs

  def __str__(self):
    return self.name

  def __repr__(self):
    return f'{NODE_TYPE_TO_NAME[self.type]} "{self.name}"'


class LutNode(Node):
  def __init__(self):
    super().__init__()
    self.type = NodeType.LUT
    self.lut_parent = None
    self.lut_port_name = None

  def disconnect_output(self, node):
    super().disconnect_output(node)
    try:
      self.lut_parent.outputs[self.lut_port_name].remove(node)
    except:
      pass


class WireInNode(Node):
  def __init__(self):
    super().__init__()
    self.type = NodeType.WIRE_IN

class MuxNode(Node):
  def __init__(self):
    super().__init__()
    self.type = NodeType.MUX

  def num_unique_inputs_available(self):
    pass


# Represents both the A and B MUX nodes.
class SharedMuxNode(MuxNode):
  def __init__(self, name, ordinal, num_inputs=4, num_shared_inputs=3):
    super().__init__()
    self.name = name
    self.type = NodeType.SHARED_MUX
    self.ordinal = ordinal

    # Another mux.
    self.num_shared_inputs = 0
    self.num_outputs = 2
    self.num_inputs = num_inputs
    self.num_shared_inputs = num_shared_inputs
    self.num_unique_inputs_per_port = self.num_inputs - self.num_shared_inputs

    self.out_nodes_per_output = {
        chr(ord('A') + i): set() for i in range(self.num_outputs)
    }

    # Here the output port name acts as the key for the unique inputs driving
    # that port. The inputs are not named.
    self.unique_inputs_per_output = {
        chr(ord('A') + i): set() for i in range(self.num_outputs)
    }

    self.shared_inputs = set()

    self.partitions = {
        output: SharedMuxPartitionNode(self, output)
        for output in self.out_nodes_per_output.keys()
    }
    for outer in self.partitions.values():
      for inner in self.partitions.values():
        if outer == inner:
          continue
        outer.input_sharers.add(inner)
      outer.ordinal = ordinal
      outer.max_num_inputs = num_inputs

  def report_inputs(self):
    report = 'shared: ' + ', '.join(repr(n) for n in self.shared_inputs) + ' '
    for output, unique_inputs in self.unique_inputs_per_output.items():
      report += (
          f'only {output}: ' + ', '.join(str(n) for n in unique_inputs) + ' ')
    return report

  def has_available_inputs(self, output=None):
    # TODO(aryap): Do we need this, or do we just maek the connection function
    # fail in a useful way?
    pass

  def get_lowest_fan_out_partition(self):
    min_fan_out = None
    partition = None
    for partition in self.partitions.values():
      fan_out = partition.fan_out()

      if min_fan_out is None or fan_out < min_fan_out:
        min_fan_out = fan_out
        partition = partition
    return partition

  def disconnect_input(self, node, output=None):
    if output is None:
      for port, unique_inputs in self.unique_inputs_per_output.items():
        try:
          unique_inputs.remove(node)
        except KeyError as err:
          pass
    else:
      try:
        unique_inputs = self.unique_inputs_per_output[output]
        unique_inputs.remove(node)
      except KeyError as err:
        pass

    try:
      self.shared_inputs.remove(node)
      partitions = ', '.join(mux.name for mux in self.partitions.values())
      print(f'removing {node} from shared inputs to {partitions}')
    except KeyError as err:
      pass
    self._in_nodes.remove(node)

  # If 'output' is the name of an output port, the node is connected to the
  # input driving that port (or a shared input, see below). If it is None, it
  # can only be connected to a shared input driving all output ports.
  #
  # We accept new inputs per output as long as we can re-arrange existing
  # inputs to make space for them.
  #
  # 1. If the node is already an input among the shared or unique inputs, we're
  # done.
  # 2. If the node is a unique input for a _different_ output and we have space
  # to promote it to a shared input, we're done.
  # 3. If are unique inputs free for this output, assign it there.
  # 4. If there are shared inputs free, assign it there.
  def connect_input(self, node, output=None):
    if node in self.shared_inputs:
      # Nothing to do.
      #print(f'{self} shared input already connected to {node}')
      return

    if output is None:
      if len(self.shared_inputs) >= self.num_shared_inputs:
        print(f'{self.name} has already exhausted shared inputs')
        sys.exit(1)
        return

    unique_to_port = self.unique_inputs_per_output[output]
    if node in unique_to_port:
      # Nothing to do.
      #print(f'{self} unique input already connected to {node} for output '
      #      f'{output}')
      return

    # If there's room among shared inputs to promote this input to a shared
    # one, check if its a unique input for any other output first and then do
    # so:
    if len(self.shared_inputs) < self.num_shared_inputs:
      for other_port, unique_inputs in self.unique_inputs_per_output.items():
        if other_port == output:
          continue
        if node in unique_inputs:
          #print(f'{self} unique input {node} promoted to shared input')
          # The node is a unique input to another port, so we promote it to
          # shared:
          unique_inputs.remove(node)
          self.shared_inputs.add(node)
          self._in_nodes.add(node)
          return

    if len(unique_to_port) < self.num_unique_inputs_per_port:
      # Connect to the unique inputs for this port.
      #print(f'{self} unique input connected to {node} for output '
      #      f'{output}')
      unique_to_port.add(node)
      self._in_nodes.add(node)
      return

    if len(self.shared_inputs) >= self.num_shared_inputs:
      # There is no room for this node among the shared inputs. Even if it is a
      # unique node to another port already, we can't promote that to a shared
      # input because there are no pins left.
      message = f'{self} has no room for another input from: {node}; existing: '
      message += self.report_inputs()
      raise NoFreeInputs(message)

    # Lastly, if there is still room in the shared inputs, add the new
    # connection there:
    #print(f'{self} shared input conneted to {node}')
    self.shared_inputs.add(node)
    self._in_nodes.add(node)


# This class is meant to represent the view of a SharedMuxNode pertaining to a
# given output port, meaning controlling that output alone and all of the
# inputs meant for that output (as well those which are shared). It should be
# used like a MuxNode.
class SharedMuxPartitionNode(MuxNode):
  def __init__(self, parent_shared_mux, corresponding_port):
    self.name = f'{parent_shared_mux.name}{corresponding_port}'

    self.ordinal = parent_shared_mux.ordinal
    self.netlist_name = None
    self.type = parent_shared_mux.type
    self.group = parent_shared_mux.group

    self.input_sharers = set()
    self.parent = parent_shared_mux
    self.corresponding_port = corresponding_port

  def get_out_nodes(self):
    return self.parent.out_nodes_per_output[self.corresponding_port]

  def get_in_nodes(self):
    return self.parent.unique_inputs_per_output[
        self.corresponding_port] | self.parent.shared_inputs

  def fan_in(self):
    return len(self.get_in_nodes())

  def fan_out(self):
    return len(self.get_out_nodes())

  def connect_input(self, node):
    self.parent.connect_input(node, output=self.corresponding_port)

  def connect_output(self, node):
    self.get_out_nodes().add(node)

  def disconnect_input(self, node):
    self.parent.disconnect_input(node, output=self.corresponding_port)

  def has_out_node(self, node):
    return node in self.get_out_nodes()

  def disconnect_output(self, node):
    self.get_out_nodes().remove(node)

  def inputs_of_type(self, node_type):
    return self.parent.inputs_of_type(node_type)

  def ok_for_lut_connection(self):
    return self.fan_in() < self.max_num_inputs


def maybe_connect_nodes(driver, driven):
  try:
    connect_nodes(driver, driven)
  except Exception as err:
    # Try to roll back to avoid any inconsistent state between driver and
    # drivee:
    try:
      disconnect_nodes(driver, driven)
    except Exception as inner_err:
      pass


def connect_nodes(driver, driven):
  if driver is None:
    print(f'warning: driver was None')
    return
  if driven is None:
    print(f'warning: driven was None')
    return
  driver.connect_output(driven)
  driven.connect_input(driver)


def maybe_disconnect_nodes(driver, driven):
  try:
    disconnect_nodes(driver, driven)
  except Exception as err:
    pass


def disconnect_nodes(driver, driven):
  try:
    driver.disconnect_output(driven)
    driven.disconnect_input(driver)
  except KeyError as err:
    print(f'error: {driver.name} does not drive {driven.name}')
    raise err

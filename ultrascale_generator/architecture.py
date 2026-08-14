import math
import tabulate
import functools
import natsort
import itertools

import io_tools
from constants import *
from common import *
from node import *
from lut import *

# FIXME(aryap): Qualify these names better. This is OIB S2 only. There is an
# equivalent for IIB S1 I have reimplemented because I am stupid.
def get_side_of_tile_by_cumulative_index(index):
  for bounds, side in SIDE_OF_TILE_BY_BOUNDS:
    beginning, end = bounds
    if index >= beginning and index < end:
      return side
  return None

def get_index_relative_to_side(index):
  return index % HALF_N_OIB_S2


def increment_over_exclusions(
    i: int, min_increment: int, exclusions: set) -> int:
  if min_increment == 0 and i in exclusions:
    min_increment += 1
  i_inc = i + min_increment
  for special in exclusions:
    if special > i and special <= i_inc:
      min_increment += 1
  return i + min_increment


def increment_over_exclusions_with_rollover(
    i: int, min_increment: int, boundary: int, exclusions: set) -> int:
  if min_increment == 0 and i in exclusions:
    min_increment += 1
  i_inc = i + min_increment
  if i_inc >= boundary:
    # Split into two separate ranges to check:
    residual = i_inc % boundary
    for special in exclusions:
      if special >= 0 and special <= residual:
        min_increment += 1
      elif special > i and special < boundary:
        min_increment += 1
  else:
    for special in exclusions:
      if special > i and special <= i_inc:
        min_increment += 1
  return (i + min_increment) % boundary



class Architecture:
  def __init__(self):
    self.interconnect_by_name = {}
    self.iib_s1_by_name = {}
    self.iib_s2_by_name = {}
    self.oib_s2_by_name = {}
    self.oib_s1_by_name = {}

    # These become pins. The 'output' of each node in the list is the pin.
    self.input_wires = []
    self.output_wires = []

    # TODO(aryap): HM!
    self.clocks = []
    self.globals = []
    
    # The interconnect wires comin in ("interconnect") and wires going out
    # ("OIB S2") are collected together here, even though they are logically
    # distinct portions of the design.
    self.interconnect_wires = collections.defaultdict(list)
    self.interconnect_specials = None
    #self.oib_s1 = None

    self.iib_s1_shared_masters = []
    self.iib_s1_regular = collections.defaultdict(
        lambda : collections.defaultdict(list))
    self.iib_s1_regular_list = []
    self.iib_s1_regular_indices = None
    self.iib_s1_north_feeders = []
    self.iib_s1_south_feeders = []
    self.iib_s1_local_feeders = []
    self.iib_s1_groups = []
    self.iib_s1_unallocated_special = None

    self.iib_s2 = None
    self.iib_s2_mux_indices = None
    self.iib_s2_groups = None
    self.iib_s2_group_indices = None
    self.iib_s2_bounce = None
    self.iib_s2_bypass = None

    # FIXME(aryap): Need a bank of LUTs per side as well.
    self.luts = None
    self.luts_by_name = {}

    self.oib_s2_shared_masters = []
    self.oib_s2_regular = collections.defaultdict(
        lambda : collections.defaultdict(list))
    self.oib_s2_regular_list = None

    self.oib_s2_bounce = None
    self.oib_s2_bounce_list = None
    self.oib_s2_bounce_groups = None
    self.oib_s2_bounce_special_indices = None
    self.oib_s2_bounce_specials = None

    # Generate nonlocal bounce muxes to represent those local to this tile and
    # also those in all immediate neighbours. In tile there are two sides, and
    # on each side there are two parts. The local tile is repsented by the
    # 'None' neighbour direction.

    # A list of two lists, since there are two parts:
    self.oib_s2_nonlocal_bounce = collections.defaultdict(
        lambda : collections.defaultdict(
            lambda : collections.defaultdict(list)))
    self.oib_s2_nonlocal_bounce_list = []

    # These nonlocal-bounce nodes do not drive anything in this tile.
    self.oib_s2_nonlocal_bounce_no_drive = set()

  def get_lut_ports_by_name(self):
    lut_ports = {}
    for _, lut in self.luts_by_name.items():
      for _, node in lut.input_nodes.items():
        lut_ports[node.name] = node
      for _, node in lut.output_nodes.items():
        lut_ports[node.name] = node
    return lut_ports

  def build_all(self):
    # Interconnect includes OIB S1 muxes.
    self.build_interconnect()

    self.build_iib_s1_muxes()
    self.build_iib_s2_muxes()
    self.build_oib_s2_muxes()
    self.build_oib_s2_nonlocal_bounce()
    self.build_oib_s2_bounce()
    self.build_luts()

    self.assemble_iib_s1_groups()

  def build_luts(self):
    self.luts = []
    for i in range(NUM_LUTS):
      lut = Lut(i, K, BYPASS_INPUTS)
      self.luts.append(lut)
      self.luts_by_name[lut.name] = lut

  def build_interconnect(self):
    # 1-unit east-travelling wires are only on the east side, and
    # 1-unit west-travelling wires are only on the west side.
    for direction in DIRECTIONS:
      for size in INTERCONNECT_WIRE_SIZES:

        if size == 1 and direction == Direction.EAST:
          sides = [Direction.EAST]
        elif size == 1 and direction == Direction.WEST:
          sides = [Direction.WEST]
        # Treat the largest size differently.
        elif size == INTERCONNECT_WIRE_SIZES[-1]:
          sides = [None]
        else:
          sides = SIDES_OF_TILE

        for side in sides:
          for i in range(N_INTERCONNECT_PAIRS):
            for io in IO_DIRECTIONS:
              dir_str = DIRECTIONS[direction]
              side_str = '' if side is None else SIDES_OF_TILE[side]

              node = WireInNode() if io == 'IN' else MuxNode()

              #node.type = NodeType.WIRE_IN if io == 'IN' else NodeType.MUX
              node.name = f'{dir_str}{size}_{side_str}_{i}_{io}'
              key = WireKey(io, direction, size, side)
              self.interconnect_wires[key].append(node)

              if io == 'IN':
                self.interconnect_by_name[node.name] = node
                self.input_wires.append(node)
              elif io == 'OUT':
                self.oib_s1_by_name[node.name] = node
                self.output_wires.append(node)

    # There are two special wires:
    # A 12-unit wire travelling eastward and up, and a 12-unit wire travelling
    # eastward and down.
    # However in general, it's not a 12-unit wire, we just do this to whatever
    # is the longest-span wire type.
    direction = Direction.EAST
    dir_str = DIRECTIONS[direction]
    size = INTERCONNECT_WIRE_SIZES[-1]
    io = 'IN'
    side_str = ''
    self.interconnect_specials = []
    for i in range(2):
      j = N_INTERCONNECT_PAIRS + i
      node = WireInNode() if io == 'IN' else MuxNode()
      node.type = NodeType.WIRE_IN if io == 'IN' else NodeType.MUX
      node.name = f'{dir_str}{size}_{side_str}_{j}_{io}'
      key = WireKey(io, direction, size, None)
      self.interconnect_wires[key].append(node)
      self.interconnect_specials.append(node)

  def build_iib_s1_muxes(self):
    self.iib_s1_regular_list = []
    self.iib_s1_regular_indices = [x for x in range(0, HALF_N_IIB_S1_REGULAR)]
    for k, side in enumerate(IIB_S1_MUX_SIDES_INCREASING_INDEX):
      for i in self.iib_s1_regular_indices:
        index = k * HALF_N_IIB_S1_REGULAR + i
        nodes = self.make_iib_s1_shared('', index)
        for output, mux in nodes.items():
          if output is None:
            self.iib_s1_shared_masters.append(mux)
            continue
          self.iib_s1_regular[side][output].append(mux)
          self.iib_s1_by_name[mux.name] = mux
          self.iib_s1_regular_list.append(mux)

    for i in range(N_FEEDERS_NORTH):
      mux = self.make_iib_s1('FEEDER', i, suffix='_NORTH')
      self.iib_s1_by_name[mux.name] = mux
      self.iib_s1_north_feeders.append(mux)

    for i in range(N_FEEDERS_SOUTH):
      mux = self.make_iib_s1('FEEDER', i, suffix='_SOUTH')
      self.iib_s1_by_name[mux.name] = mux
      self.iib_s1_south_feeders.append(mux)

    for i in range(N_FEEDERS_LOCAL):
      mux = self.make_iib_s1('FEEDER', i, suffix='_LOCAL')
      self.iib_s1_by_name[mux.name] = mux
      self.iib_s1_local_feeders.append(mux)

  def assemble_iib_s1_groups(self):
    # Allocate the IIB S1 muxes to groups.
    group_size = int(IIB_S2_MAX_NUM_INPUTS / 2)
    M = HALF_N_IIB_S1_REGULAR
    N = HALF_N_IIB_S2

    unallocated = list(self.iib_s1_regular_indices)
    R = len(unallocated) #M - 9

    def make_special_groups(group_size, source_muxes):
      specials = []
      for i in range(group_size):
        entry = i * int(math.ceil(len(source_muxes) / group_size))
        specials.append(entry)
      # ?!
      specials.extend([source_muxes[-4], source_muxes[-1]])

      included = [False] * len(specials)
      groups = []
      for i in range(len(specials)):
        # Try to make a group of `group_size` many indices 2 apart. If you
        # can't make a whole group, those remain unallocated:
        if included[i]:
          continue
        group_by_index = []
        for j in range(group_size):
          index = i + 2 * j
          if index >= len(specials):
            break
          group_by_index.append(index)

        if len(group_by_index) == group_size:
          group = tuple(specials[i] for i in group_by_index)
          for index in group_by_index:
            included[index] = True
          groups.append(group)

      unallocated = list(specials[i] for i in range(len(included)) if not included[i])
      return groups, unallocated

    special_groups_ord, unallocated_special_ord = make_special_groups(
        group_size, self.iib_s1_regular_indices)

    special_groups = []
    for group in special_groups_ord:
      for x in group:
        unallocated.remove(x)
      special_groups.append(tuple(
          self.get_iib_s1_regular_mux(lut_side, x, 'A')
          for x in group))

    for ordinal in unallocated_special_ord:
      try:
        unallocated.remove(ordinal)
      except:
        print(f'warning: {ordinal} not in unallocated list')
        pass
    unallocated_special = list(
        self.get_iib_s1_regular_mux(lut_side, x, 'A')
        for x in unallocated_special_ord)
    self.iib_s1_unallocated_special = unallocated_special

    i = -2
    regular_group_ords = []
    while unallocated:
      group = []
      if len(unallocated) <= group_size:
        regular_group_ords.append(tuple(unallocated))
        unallocated.clear()
        break

      def get_next_from(k):
        count = 0
        while k % R not in unallocated or (
            len(group) == group_size - 1 and k % R == min(unallocated)):
          k += 1
          count += 1

          if count == len(unallocated):
            return None
        return k % R

      k = i
      while len(group) < group_size:
        ordinal = get_next_from(k)
        if not ordinal:
          break

        unallocated.remove(ordinal)
        group.append(ordinal)
        k = ordinal + 2

      if group:
        regular_group_ords.append(tuple(group))
      i += 1

    regular_groups_out0 = []
    regular_groups_out1 = []
    for i, group in enumerate(regular_group_ords):
      first_output_group = tuple(self.get_iib_s1_regular_mux(lut_side, x, 'A')
                                 for x in group)
      second_output_group = tuple(list(n.input_sharers)[0] for n in first_output_group)
      regular_groups_out0.append(first_output_group)

      # The last group does not have its second output connected.
      if i != len(regular_group_ords) - 1:
        regular_groups_out1.append(second_output_group)

    def linearly_make_input_group(size, muxes):
      unallocated = collections.deque(muxes)
      groups = []
      i = 0
      while unallocated:
        if len(unallocated) < size:
          break
        group = tuple(unallocated.popleft() for _ in range(0, size))
        groups.append(group)
      return (groups, list(unallocated))

    groups_north, unallocated_north = linearly_make_input_group(
        group_size, self.iib_s1_north_feeders)
    groups_local, unallocated_local = linearly_make_input_group(
        group_size, self.iib_s1_local_feeders)
    groups_south, unallocated_south = linearly_make_input_group(
        group_size, self.iib_s1_south_feeders)

    self.iib_s1_groups = groups_local + groups_north

    def max_and_argmax(some_list):
      if not some_list:
        return None
      max_value = some_list[0]
      max_index = 0
      for i in range(1, len(some_list)):
        if some_list[i] > max_value:
          max_value = some_list[i]
          max_index = i
      return max_value, max_index

    # updateable priority queue might be nice here.
    TRIGGER = 3
    inclusion_clock = [0] * len(regular_groups_out0)
    on_clock = set()
    def preempt_from_clock():
      max_value, max_index = max_and_argmax(inclusion_clock)
      if max_value >= TRIGGER:
        inclusion_clock[max_index] = 0
        on_clock.remove(max_index)
        return max_index
      return None

    def tick_those_on_clock():
      for i in on_clock:
        inclusion_clock[i] += 1

    # I can't figure out why this goes here.
    if len(special_groups) > 1:
      print('warning: not sure what to do with this many special groups')
    unallocated_special_groups = collections.deque(special_groups)
    s = 0
    def preempt_from_specials(current_index):
      if not unallocated_special_groups:
        return None
      # Here 16 is how often they get inserted - it's made up, but roughly feels
      # like 1/2 the span of regular muxes.
      if (current_index - 4) % 16 == 0:
        return unallocated_special_groups.popleft()
      return None


    i = 0
    unmatched = set(x for x in range(len(regular_groups_out1)))
    while i < len(regular_groups_out0):
      preempt = preempt_from_specials(len(self.iib_s1_groups))
      if preempt is not None:
        self.iib_s1_groups.append(preempt)
        tick_those_on_clock()
        continue

      preempt = preempt_from_clock()
      if preempt is not None and preempt < len(regular_groups_out1):
        print('!!', regular_groups_out1[preempt])
        self.iib_s1_groups.append(regular_groups_out1[preempt])
        unmatched.remove(preempt)
        tick_those_on_clock()
        continue

      group = regular_groups_out0[i]
      print(group)
      self.iib_s1_groups.append(group)

      tick_those_on_clock()

      on_clock.add(i)
      i += 1

    while unmatched:
      index = min(unmatched)
      self.iib_s1_groups.append(regular_groups_out1[index])
      unmatched.remove(index)

    self.iib_s1_groups += groups_south

    # Note that at this point there are at least 3 collections of free, unallocated
    # stage 1 muxes:
    stage_1_unallocated = unallocated_special + (
        unallocated_north + unallocated_local + unallocated_south)

    print('Stage 1 unallocated:')
    print(stage_1_unallocated)
    print()

    print('Stage 1 groups:')
    for group in self.iib_s1_groups:
      print(group)
    print()

    print('\nUnallocated so far:')
    print(unallocated_special)

    # These muxes are stand-ins for input-ports to the interconnect.
    print(unallocated_north)
    print(unallocated_south)

    # These muxes also feed external ports. They are the 'north' and 'south'
    # feeders for adjacent tiles.
    print(unallocated_local)


  @staticmethod
  def make_iib_s2(group_name, i, out, suffix=''):
    node = MuxNode()
    group_name = f'_{group_name}' if group_name else ''
    node.name = f'IIB_S2{group_name}_{i}{suffix}{out}'
    node.type = NodeType.MUX
    node.ordinal = i
    return node


  def build_iib_s2_muxes(self):
    self.iib_s2_mux_indices = list(x for x in range(0, HALF_N_IIB_S2))

    # Lists of the indices of the members of each group for a single side.
    self.iib_s2_group_indices = []

    # Idea: Add perturbations to mix up the r
    # Rotate all groups by subtracting 1 from the indices included (this is the
    # same as starting with i = -1):
    #for i, group in enumerate(stage_1_groups):
    #  rotated = tuple((x - 1) % M for x in group)
    #  stage_1_groups[i] = rotated
    #
    #def get_iib_s2_mux(side, index, port)
    #  return None

    i = 0
    j = 0
    while len(self.iib_s2_group_indices) < len(IIB_S2_GROUP_SIZES):
      group = []
      while len(group) < IIB_S2_GROUP_SIZES[i]:
        if j >= len(self.iib_s2_mux_indices):
          break
        group.append(self.iib_s2_mux_indices[j])
        j += 1
      # If the we didn't get a clean division of 17 into N, there will be
      # leftovers, which we add to the last group:
      while i == len(IIB_S2_GROUP_SIZES) - 1 and j < len(self.iib_s2_mux_indices):
        group.append(self.iib_s2_mux_indices[j])
        j += 1
      self.iib_s2_group_indices.append(group)
      i += 1

    self.iib_s2_by_name = {}
    self.iib_s2 = {}
    self.iib_s2_groups = collections.defaultdict(list)

    # group A are the BOUNCE muxes, group B are BYPASS muxes, and groups C, D are
    # regular muxes.
    self.iib_s2_bounce = []
    self.iib_s2_bypass = []
    self.iib_s2_regular = []

    group_mapping = [
        (0, 'BOUNCE', self.iib_s2_bounce),
        (1, 'BYPASS', self.iib_s2_bypass),
        (2, '', self.iib_s2_regular),
        (3, '', self.iib_s2_regular),
    ]
    rows = [['i', 'name', 'tag', 'count']]
    for i, tag, _ in group_mapping:
      rows.append([i, IIB_S2_GROUP_NAMES[i], tag, IIB_S2_GROUP_SIZES[i]])
    print(tabulate.tabulate(rows, headers='firstrow'))

    for group_index, group_name, container in group_mapping:
      for index in self.iib_s2_group_indices[group_index]:
        mux = self.make_iib_s2(group_name, index, '', '')
        container.append(mux)
        self.iib_s2[index] = mux
        self.iib_s2_by_name[mux.name] = mux
        self.iib_s2_groups[group_index].append(mux)

    print('Stage 2 groups:')
    for group in self.iib_s2_group_indices:
      print(group)
    print()

  def get_iib_s2_mux_by_ordinal(self, ordinal):
    if ordinal in self.iib_s2:
      return self.iib_s2[ordinal]
    return None

  def get_iib_s2_mux_by_name(self, name):
    if name in self.iib_s2_by_name:
      return self.iib_s2_by_name[name]
    return None

  @staticmethod
  def make_iib_s1(group_name, i, suffix=''):
    node = MuxNode()
    group_name = f'_{group_name}' if group_name else ''
    node.name = f'IIB_S1{group_name}_{i}{suffix}'
    node.type = NodeType.LUT
    node.ordinal = i
    return node

  @staticmethod
  def make_iib_s1_shared(group_name, index, suffix=''):
    group_name = f'_{group_name}' if group_name else ''
    name = f'IIB_S1{group_name}_{index}{suffix}'
    shared_mux = SharedMuxNode(
        name, index, IIB_S1_MAX_NUM_INPUTS, IIB_S1_NUM_SHARED_INPUTS)

    nodes = {
        None: shared_mux
    }
    nodes.update(shared_mux.partitions)
    return nodes

  def get_iib_s1_side_and_index_from_flat(self, index):
    if index >= HALF_N_IIB_S1_REGULAR:
      return (IIB_S1_MUX_SIDES_INCREASING_INDEX[1],
              index - HALF_N_IIB_S1_REGULAR)
    return (IIB_S1_MUX_SIDES_INCREASING_INDEX[0], index)


  def get_iib_s1_regular_mux(self, side, index, out):
    if side is None:
      # Treat the index as flat, across all nodes:
      side, index = self.get_iib_s1_side_and_index_from_flat(index)
    try:
      mux = self.iib_s1_regular[side][out][index]
    except Exception as err:
      print(f'{side=} {out=} {index=}')
      return None
    return mux

  def get_iib_s1_mux_by_name(name):
    try:
      mux = self.iib_s1_muxes_by_name[name]
    except Exception as err:
      raise err
    return mux

  def build_oib_s2_muxes(self):
    self.oib_s2_regular_list = []
    self.oib_s2_shared_masters = []
    for side in OIB_S2_MUX_SIDES_INCREASING_INDEX:
      bounds = BOUNDS_BY_SIDE_OF_TILE[side]
      for i in range(bounds[0], bounds[1]):
        nodes = self.make_oib_s2_shared('', i)
        for output, mux in nodes.items():
          if output is None:
            self.oib_s2_shared_masters.append(mux)
            continue
          self.oib_s2_regular[side][output].append(mux)
          self.oib_s2_by_name[mux.name] = mux
          self.oib_s2_regular_list.append(mux)

  def build_oib_s2_bounce(self):
    N_A_OUT = int(math.ceil(N_OIB_S2_BOUNCE / 2))
    HALF_N_A_OUT = int(N_A_OUT / 2)
    REST_N_A_OUT = N_A_OUT - HALF_N_A_OUT
    N_B_OUT = N_OIB_S2_BOUNCE - N_A_OUT

    self.oib_s2_bounce = collections.defaultdict(list)
    for i in range(N_OIB_S2_BOUNCE):
      nodes = self.make_oib_s2_shared('BOUNCE', i)
      for output, mux in nodes.items():
        if output is None:
          self.oib_s2_shared_masters.append(mux)
          continue
        self.oib_s2_bounce[output].append(mux)
        self.oib_s2_by_name[mux.name] = mux
    # Now we can look up bounce muxes with the getter functions.

    self.oib_s2_bounce_groups = []
    ordered_muxes = []
    group = []
    for i in range(HALF_N_A_OUT):
      index = i
      mux = self.get_oib_s2_bounce(index, 'A')
      group.append(mux)
      ordered_muxes.append(mux)
    self.oib_s2_bounce_groups.append(group)
    group = []
    last = len(ordered_muxes)
    for i in range(N_B_OUT):
      index = last + i
      mux = self.get_oib_s2_bounce(index, 'B')
      ordered_muxes.append(mux)
      group.append(mux)
    self.oib_s2_bounce_groups.append(group)
    group = []
    last = len(ordered_muxes)
    for i in range(REST_N_A_OUT):
      index = last + i
      mux = self.get_oib_s2_bounce(index, 'A')
      ordered_muxes.append(mux)
      group.append(mux)
    self.oib_s2_bounce_groups.append(group)

    flank_ordinal = int(N_OIB_S2_BOUNCE / 4)
    left_special = int(N_OIB_S2_BOUNCE / 2)
    right_special = N_OIB_S2_BOUNCE - flank_ordinal - 1
    self.oib_s2_bounce_special_indices = [left_special, right_special]
    self.oib_s2_bounce_specials = [
        ordered_muxes[i] for i in self.oib_s2_bounce_special_indices]
    self.oib_s2_bounce_list = ordered_muxes

    print()
    print('Physical-esque arrangement of OIB S2 bounce muxes:')
    for mux in ordered_muxes:
      print(mux.name)


  @staticmethod
  def make_oib_s2(group_name, index, suffix=''):
    group_name = f'_{group_name}' if group_name else ''
    name = f'OIB_S2{group_name}_{index}{suffix}'
    mux = MuxNode()
    mux.name = name
    mux.type = NodeType.MUX
    mux.ordinal = index
    return mux

  @staticmethod
  def make_oib_s2_shared(group_name, index, suffix=''):
    group_name = f'_{group_name}' if group_name else ''
    name = f'OIB_S2{group_name}_{index}{suffix}'
    mux = SharedMuxNode(
        name, index, OIB_S2_MAX_NUM_INPUTS, OIB_S2_NUM_SHARED_INPUTS)
    nodes = { None: mux }
    nodes.update(mux.partitions)
    return nodes

  def get_oib_s2_side_and_index_from_flat(self, index):
    if index >= HALF_N_OIB_S2:
      return (OIB_S2_MUX_SIDES_INCREASING_INDEX[1],
              index - HALF_N_OIB_S2)
    return (OIB_S2_MUX_SIDES_INCREASING_INDEX[0], index)

  def get_oib_s2_mux(self, side, index, out):
    if side is None:
      # Treat the index as flat, across all nodes:
      side, index = self.get_oib_s2_side_and_index_from_flat(index)
    try:
      mux = self.oib_s2_regular[side][out][index]
    except Exception as err:
      print(f'{side=} {out=} {index=}')
      return None
    return mux

  def get_oib_s2_bounce(self, index, out):
    try:
      mux = self.oib_s2_bounce[out][index]
    except Exception as err:
      print(f'{out=} {index=}')
      raise err
    return mux

  def get_oib_s2_side_of_tile_by_cumulative_index(self, index):
    for bounds, side in side_of_tile_by_bounds:
      beginning, end = bounds
      if index >= beginning and index < end:
        return side
    return None

  def get_oib_s2_mux_ordinal_bounds(self, side_of_tile):
    return BOUNDS_BY_SIDE_OF_TILE[side_of_tile]

  def get_oib_s2_mux_with_rollover(self, j, out):
    side = get_oib_s2_side_of_tile_by_cumulative_index(j)
    index = j % HALF_N_OIB_S2
    return self.oib_s2_regular[side][out][index]

  def build_oib_s2_nonlocal_bounce(self):
    for neighbour in [Direction.NORTH, None, Direction.SOUTH]:
      for side_of_tile in SIDES_OF_TILE:
        for i in range(HALF_N_OIB_S2_NONLOCAL_BOUNCE):
          source_name = f'_FROM_{DIRECTIONS[neighbour][0]}' if neighbour else ''
          side = SIDES_OF_TILE[side_of_tile]

          mux = MuxNode()
          mux.name = f'OIB_S2_NONLOCAL_BOUNCE_{side}_{i}{source_name}'
          mux.type = NodeType.MUX
          mux.max_num_lut_inputs = OIB_S2_MAX_NUM_LUT_INPUTS
          mux.max_num_inputs = OIB_S2_MAX_NUM_INPUTS
          mux.ordinal = i

          part = 0 if i < int(OIB_S2_NONLOCAL_SPLIT) else 1

          self.oib_s2_nonlocal_bounce[neighbour][side_of_tile][part].append(mux)
          self.oib_s2_nonlocal_bounce_list.append(mux)

          self.oib_s2_by_name[mux.name] = mux

          if neighbour is None:
            self.output_wires.append(mux)
            if i in OIB_S2_NONLOCAL_NO_DRIVE_INDICES:
              self.oib_s2_nonlocal_bounce_no_drive.add(mux)
          else:
            self.input_wires.append(mux)


  def get_oib_s2_nonlocal_bounce_muxes(
      self, neighbour_direction, side_of_tile, part):
    this_side_and_source = self.oib_s2_nonlocal_bounce[neighbour_direction][
        side_of_tile]
    if part is None:
      return functools.reduce(
          lambda x, y: x + y, this_side_and_source.values(), [])
    return this_side_and_source[part]

  def get_interconnect_wire(self, io_direction, wire_direction, size, i, side):
    if side is None:
      # Figure side based on other inputs.
      if size == MAX_SIZE:
        side = None
      elif wire_direction == Direction.WEST and size == 1:
        side = Direction.WEST
      elif wire_direction == Direction.EAST and size == 1:
        side = Direction.EAST
      else:
        j = int(i / N_INTERCONNECT_PAIRS) % len(
            INTERCONNECT_SIDE_INCREASING_INDEX)
        try: 
          side = INTERCONNECT_SIDE_INCREASING_INDEX[j]
        except Exception as err:
          print(f'{j=}', f'{size=}', f'{i=}', err)
          sys.exit(1)
    key = (io_direction, wire_direction, size, side)
    wires = self.interconnect_wires[key]
    if not wires:
      print(f'empty wires! {io_direction=} {wire_direction=} {size=} {i=} {side=}')
      return None
    assert len(wires) >= N_INTERCONNECT_PAIRS, (
        f'{len(wires)=} vs {N_INTERCONNECT_PAIRS=}')
    return wires[i % N_INTERCONNECT_PAIRS]

  def get_interconnect_wire_in(self, direction, size, i, side=None):
    return self.get_interconnect_wire('IN', direction, size, i, side)


  def get_interconnect_wire_out(self, direction, size, i, side=None):
    return self.get_interconnect_wire('OUT', direction, size, i, side)

  def get_interconnect_wire_out_bundle(self, side_of_tile, wire_direction, wire_size):
    out_wires = [
        (j, self.get_interconnect_wire_out(
            wire_direction, wire_size, j, side_of_tile))
        for j in range(N_INTERCONNECT_PAIRS)]
    return out_wires

  def get_lut(self, index):
    return self.luts[index]

  def num_luts(self):
    return len(self.luts)

  def get_all_nodes(self):
    all_nodes = []
    for d in [self.interconnect_by_name,
              self.iib_s1_by_name,
              self.iib_s2_by_name,
              dict(functools.reduce(lambda x, y: x + y, [
                  list((node.name, node)
                       for lut in self.luts
                       for node in lut.input_nodes.values())], [])),
              self.oib_s1_by_name,
              self.oib_s2_by_name]:
      all_nodes.extend(
          natsort.natsorted(d.items(), key=lambda x: x[1].name))
    return all_nodes

  def print_lut_ports(self):
    input_order = self.luts[0].port_order + self.luts[0].bypass_order
    output_order = self.luts[0].output_order
    rows = [['LUT'] + list(input_order) + ['-'] + list(output_order)]
    for lut in self.luts:
      row = [lut.name]
      for port in input_order:
        node = lut.inputs[port]
        row.append(node.netlist_name or node.name if node else 'None')
      row.append('-')
      for port in output_order:
        node = lut.outputs[port]
        row.append(node.netlist_name or node.name if node else 'None')
      rows.append(row)
    print(tabulate.tabulate(rows, headers='firstrow'))

  def print_all(self):
    all_nodes = self.get_all_nodes()
    rows = []
    for name, node in all_nodes:
      rows.append([name, node])
    print(tabulate.tabulate(rows))

  def print_stats(self):
    rows = [
        ['Interconnect in', len(self.interconnect_by_name)],
        ['IIB S1 MUXes', len(self.iib_s1_by_name)],
        ['IIB S2 MUXes', len(self.iib_s2_by_name)],
        ['OIB S2 MUXes', len(self.oib_s2_by_name)],
        ['OIB S1 MUXes', len(self.oib_s1_by_name)],
        ['LUTs', len(self.luts)]
    ]

    print('===================================================================')
    print('Node stats')
    print(tabulate.tabulate(rows))
    print('===================================================================')

  def report(self):
    all_nodes = self.get_all_nodes()
    rows = [['Node', 'Fan In', 'Fan Out']]
    for name, node in all_nodes:
      rows.append([name, node.fan_in(), node.fan_out()])
    print(tabulate.tabulate(rows, headers='firstrow'))

  def report_io(self):
    rows = [['IN', 'OUT']]
    for row in itertools.zip_longest(
        self.input_wires, self.output_wires, fillvalue=None):
      rows.append(row)
    rows.append([f'Total: {len(self.input_wires)}',
                 f'Total: {len(self.output_wires)}'])
    print('===================================================================')
    print('I/O report')
    print(tabulate.tabulate(rows, headers='firstrow'))
    print('===================================================================')
    
  def sectional_summary(self):
    sections = [
        ('Interconnect -> IIB S1', self.interconnect_by_name,
             self.iib_s1_by_name),
        ('Interconnect -> OIB S2', self.interconnect_by_name,
             self.oib_s2_by_name),
        ('IIB S1 -> IIB S2', self.iib_s1_by_name, self.iib_s2_by_name),
        ('IIB S2 -> LUTs', self.iib_s2_by_name, self.luts_by_name),
        ('LUTs -> OIB S2', self.luts_by_name, self.oib_s2_by_name),
        ('OIB S2 -> OIB S1', self.oib_s2_by_name, self.oib_s1_by_name)
    ]
    rows = [['Section', 'Num Nodes', 'Num Connections']]
    for name, senders, receivers in sections:
      outgoing_connections = [
          (node, driven)
          for node in senders.values()
          for driven in node.get_out_nodes()]
      incoming_connections = [
          (node, driven)
          for node in receivers.values()
          for driven in node.get_in_nodes()]
      rows.append([
          name,
          f'{len(senders)} -> {len(receivers)}',
          f'{len(outgoing_connections)} -> {len(incoming_connections)}'
          ])
    print('===================================================================')
    print('Routing section summary')
    print(tabulate.tabulate(rows, headers='firstrow'))
    print('===================================================================')


  def write_all(self):
    all_nodes = dict(self.get_all_nodes())
    io_tools.write_adjacency_matrix(
        'all.csv', all_nodes, all_nodes)

    io_tools.write_bfg_edge_list(
        'bfg_edges.csv', all_nodes, all_nodes)

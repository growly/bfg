import io_tools
from node import connect_nodes, disconnect_nodes
from common import Direction
from constants import *

# Interconnect to IIB Layer/Stage 1.
# ----------------------------------
#

sizes = INTERCONNECT_WIRE_SIZES

def connect_wire_iib_s1_regular(
  arch, in_direction, in_size, in_index,
  out_index, out_port, out_side,
  in_side=None):
  if in_size is None:
    return
  in_wire = arch.get_interconnect_wire_in(in_direction, in_size, in_index, side=in_side)
  if in_wire is None:
    return
  out_mux = arch.get_iib_s1_regular_mux(out_side, out_index, out_port)
  if out_mux is None:
    return
  print(f'connecting {in_wire.name} {in_size=} i={in_index} to j={out_index} '
        f'{out_port} {out_mux.name}')
  connect_nodes(in_wire, out_mux)

initial_i_j_span_by_direction = {
  Direction.SOUTH: (1, 1, 8),
  Direction.EAST: (1, 1, 8),
  Direction.WEST: (1, 0, 8),
  Direction.NORTH: (1, 1, 8),
}

def increment_over_specials(j, min_increment):
  j_inc = j + min_increment
  # For every special index that appears in the range we're incrementing over,
  # increase by an additional 1.
  for special in set(IIB_S1_SPECIAL_INDICES):
    if special > j and special <= j_inc:
      min_increment += 1
  return j + min_increment

# There are N_INTERCONNECT_PAIRS = 8 wires per bundle, and one bundle per side
# of the tile, for a given direction of wires. Sometimes we need the index e.g.
# i = 5 + 3 = 8 to refer to the 0th wire in the first bundle, as opposed to the
# 0th wire in the second bundle. Likewise, i = 14 + 2 = 16 might need to index
# 8, the first wire in the second bundle, instead of index 0, the first wire in
# the first bundle.
NUM_BUNDLES = len(INTERCONNECT_SIDE_INCREASING_INDEX)
def wrap_interconnect_index_within_bundle(index, increment):
  base_of_bundle = int(index / N_INTERCONNECT_PAIRS) * N_INTERCONNECT_PAIRS
  if index >= NUM_BUNDLES * N_INTERCONNECT_PAIRS:
    base_of_bundle -= N_INTERCONNECT_PAIRS
  wrapped = base_of_bundle + ((index + increment) % N_INTERCONNECT_PAIRS)
  return wrapped

def connect(arch):
  # SOUTH wires -> IIB S1
  # ---------------------
  dirn = Direction.SOUTH
  i, initial_j, span = initial_i_j_span_by_direction[dirn]
  pattern_count = 0
  for side in IIB_S1_MUX_SIDES_INCREASING_INDEX:
    j = initial_j
    while j < HALF_N_IIB_S1_REGULAR - span:
      print(f'{DIRECTIONS[dirn]} pattern {pattern_count}')
      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'A', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'B', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'B', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 2, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 2, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 2, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 2, j, 'B', side)

      j = increment_over_specials(j, 1)
      pattern_count += 1
      i += 2

    # Final stage here.
    j = increment_over_specials(j, 1)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)
    j = increment_over_specials(j, 1)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

    j = increment_over_specials(j, 1)
    i_offset = wrap_interconnect_index_within_bundle(i, 2)
    connect_wire_iib_s1_regular(arch, dirn, sizes[0], i_offset, j, 'A', side)
    connect_wire_iib_s1_regular(arch, dirn, sizes[0], i_offset, j, 'B', side)

    j = increment_over_specials(j, 1)
    connect_wire_iib_s1_regular(arch, dirn, sizes[0], i_offset, j, 'A', side)

    # TODO(aryap): This next connection is actually from (0, 2) not (0, 1) like
    # the others.
    j = 0
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'A', side)

    i += 2

  # EAST wires -> IIB S1
  # --------------------
  dirn = Direction.EAST
  i, initial_j, span = initial_i_j_span_by_direction[dirn]
  pattern_count = 0
  for side in IIB_S1_MUX_SIDES_INCREASING_INDEX:
    j = initial_j
    while j < HALF_N_IIB_S1_REGULAR - span:
      print(f'{DIRECTIONS[dirn]} pattern {pattern_count}')
      if side == Direction.WEST:
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'A', side)
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)

      j = increment_over_specials(j, 1)
      if side == Direction.WEST:
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'A', side)
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)

      j = increment_over_specials(j, 2)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'A', side)

      j = increment_over_specials(j, 1)
      if side == Direction.WEST:
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'A', side)
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'B', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 2, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 2, j, 'B', side)

      j = increment_over_specials(j, 1)
      if side == Direction.WEST:
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'A', side)
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'B', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 2, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 2, j, 'B', side)

      i += 2
      j = increment_over_specials(j, 1)
      pattern_count += 1

    # Final stage here.

    #j = HALF_N_IIB_S1 - 2
    #i = wrap_interconnect_index_within_bundle(i - 2, 2)
    i_offset = wrap_interconnect_index_within_bundle(i, 1)
    if side == Direction.WEST:
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)

    j = increment_over_specials(j, 1)
    if side == Direction.WEST:
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)

    j = increment_over_specials(j, 2)
    if side == Direction.WEST:
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i_offset, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i_offset, j, 'B', side)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i_offset, j, 'A', side)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i_offset, j, 'B', side)

    j = increment_over_specials(j, 1)
    if side == Direction.WEST:
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i_offset, j, 'A', side)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i_offset, j, 'A', side)

    i += 2

  # WEST wires -> IIB S1
  # --------------------
  dirn = Direction.WEST
  i, initial_j, span = initial_i_j_span_by_direction[dirn]
  pattern_count = 0
  for side in IIB_S1_MUX_SIDES_INCREASING_INDEX:
    j = initial_j
    while j < HALF_N_IIB_S1_REGULAR - span:
      print(f'{DIRECTIONS[dirn]} pattern {pattern_count}')
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'A', side)

      j = increment_over_specials(j, 1)
      if side == Direction.EAST:
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'A', side)
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      if side == Direction.EAST:
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'A', side)
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)

      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      if side == Direction.EAST:
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'A', side)
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      if side == Direction.EAST:
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'A', side)
        connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'B', side)

      j = increment_over_specials(j, 2)
      i += 2
      pattern_count += 1

    # Final stage here.
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

    j = increment_over_specials(j, 1)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

    j = increment_over_specials(j, 2)
    if side == Direction.EAST:
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)

    i_offset = wrap_interconnect_index_within_bundle(i, 1)

    # FIXME(aryap): This actually connects to the WW1_W wire from (1, -1), not (1,
    # 0) as in the other cases.
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i_offset, j, 'B', side)

    j = 0
    if side == Direction.EAST:
      # FIXME(aryap): This actually connects to the WW1_W wire from (1, -1), not (1,
      # 0) as in the other cases.
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'A', side)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i_offset, j, 'A', side)
    i += 2



  # NORTH wires -> IIB S1
  # ---------------------
  dirn = Direction.NORTH
  i, initial_j, span = initial_i_j_span_by_direction[dirn]
  pattern_count = 0
  for side in IIB_S1_MUX_SIDES_INCREASING_INDEX:
    j = initial_j
    while j < HALF_N_IIB_S1_REGULAR - span:
      print(f'{DIRECTIONS[dirn]} pattern {pattern_count}')
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[0], i + 1, j, 'B', side)

      j = increment_over_specials(j, 2)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'A', side)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      connect_wire_iib_s1_regular(arch, dirn, sizes[1], i + 1, j, 'A', side)

      j = increment_over_specials(j, 1)
      i += 2
      pattern_count += 1

    # Final stage here.
    connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

    j = increment_over_specials(j, 1)
    connect_wire_iib_s1_regular(arch, dirn, sizes[0], i, j, 'B', side)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i, j, 'B', side)

    j = increment_over_specials(j, 1)
    i_offset = wrap_interconnect_index_within_bundle(i, 1)
    # TODO(aryap): There is actually a feedback path from the north-1 wire
    # _originating_ in this tile, not landing here.
    connect_wire_iib_s1_regular(arch, dirn, sizes[0], i_offset, j, 'B', side)

    j = increment_over_specials(j, 1)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i_offset, j, 'A', side)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i_offset, j, 'B', side)

    j = increment_over_specials(j, 1)
    connect_wire_iib_s1_regular(arch, dirn, sizes[1], i_offset, j, 'A', side)

    j = 0
    connect_wire_iib_s1_regular(arch, dirn, sizes[0], i_offset, j, 'A', side)
    i += 2


  IIBS1SpecialConfig = collections.namedtuple('IIBS1SpecialConfig', [
    'interconnect_index',
    'target_mux_index',
    'mux_outputs_tuple',
    'interconnect_source_side',
    'iib_s1_sides_tuple'])
  specials = {
    Direction.NORTH: {
    },
    Direction.EAST: {
      # East size 2 (the second wire size) index 0 connects to muxes 11, 22,
      # ports 'B' and 'A' respectively, on the EAST side of the mux bank. And
      # also on the WEST.
      sizes[1]: [
        IIBS1SpecialConfig(0, 11, ('A',), Direction.EAST, (Direction.EAST,)),
        IIBS1SpecialConfig(0, 22, ('A',), Direction.EAST, (Direction.EAST,)),
        IIBS1SpecialConfig(0, 11, ('A',), Direction.WEST, (Direction.WEST,)),
        IIBS1SpecialConfig(0, 22, ('A',), Direction.WEST, (Direction.WEST,)),
      ]
    },
    Direction.WEST: {
      sizes[0]: [
        IIBS1SpecialConfig(0, 11, ('A',), Direction.WEST, (Direction.EAST,)),
        IIBS1SpecialConfig(0, 22, ('A',), Direction.WEST, (Direction.EAST,)),
      ]
    },
    Direction.SOUTH: {
      sizes[0]: [
        IIBS1SpecialConfig(0, 11, ('A',), Direction.EAST, (Direction.EAST,)),
        IIBS1SpecialConfig(0, 22, ('A',), Direction.EAST, (Direction.EAST,)),
        IIBS1SpecialConfig(0, 11, ('A',), Direction.WEST, (Direction.WEST,)),
        IIBS1SpecialConfig(0, 22, ('A',), Direction.WEST, (Direction.WEST,)),
      ],
      sizes[1]: [
        IIBS1SpecialConfig(0, 11, ('A',), Direction.EAST, (Direction.EAST,)),
        IIBS1SpecialConfig(0, 22, ('A',), Direction.EAST, (Direction.EAST,)),
        IIBS1SpecialConfig(0, 11, ('A',), Direction.WEST, (Direction.WEST,)),
        IIBS1SpecialConfig(0, 22, ('A',), Direction.WEST, (Direction.WEST,)),
      ]
    }
  }

  # Make all special connections.
  print(f'making special interconnect/IIB S1 connections')
  for direction in DIRECTIONS.keys():
    for size, configs in specials[direction].items():
      for config in configs:
        i, j, outs, source_side, mux_sides = config
        for mux_side in mux_sides:
          for out in outs:
            connect_wire_iib_s1_regular(
                arch, direction, size, i, j, out, mux_side, in_side=source_side)

  io_tools.write_adjacency_matrix('iib_s1_fan_in.csv',
                                  arch.interconnect_by_name,
                                  arch.iib_s1_by_name,
                                  sort_by_type=True)

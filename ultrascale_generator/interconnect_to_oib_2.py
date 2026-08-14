from constants import *
import io_tools
from node import connect_nodes, disconnect_nodes
from architecture import *

def connect_wire_oib_s2(
  arch, in_direction, in_size_index, in_index,
  out_index, out_port, out_side, explicit_in_size=None, in_side=None):

  # Indices from the front (>=0) are only useful for the first n-1 sizes. This
  # lets us reduce the number of sizes but keep the largest as special cases
  # (using negative indices). God bless python.
  if in_size_index is None:
      in_size = explicit_in_size
  else:
    if in_size_index >= len(INTERCONNECT_WIRE_SIZES) - 1:
      print(f'warning: ignoring connect_wire_oib_s2 for {in_size_index=}')
      return
    in_size = INTERCONNECT_WIRE_SIZES[in_size_index]
  if in_size is None:
    return
  in_wire = arch.get_interconnect_wire_in(in_direction, in_size, in_index, side=in_side)
  if in_wire is None:
    return
  out_mux = arch.get_oib_s2_mux(out_side, out_index, out_port)
  if out_mux is None:
    return
  print(f'connecting {in_wire.name} {in_size=} i={in_index} to {out_index=} '
        f'{out_port} {out_mux.name}')
  connect_nodes(in_wire, out_mux)

# Interconnect to OIB Layer/Stage 2.
# ----------------------------------
#

# FIXME(aryap): span is a function of the number of muxes!
# 48 / 4 + 1 or + 2? ~ fan out of 4
initial_i_j_span_by_direction = {
  Direction.SOUTH: (1, 2, 13),
  Direction.EAST: (0, 1, 13),
  Direction.WEST: (1, 0, 13),
  Direction.NORTH: (1, 4, 14),
}


def increment_over_specials(j, min_increment):
  j_inc = j + min_increment
  # For every special index that appears in the range we're incrementing over,
  # increase by an additional 1.
  for special in set(OIB_S2_SPECIAL_INDICES):
    if special > j and special <= j_inc:
      min_increment += 1
  return j + min_increment


def connect(arch):

  # South connections.

  # 'i' is an index into interconnect wires of particular size in a particular
  # direction in the range 0, ..., 2 * N_INTERCONNECT_PAIRS - 1. The first half
  # of these is allocated to wires originating from the east side of the tile,
  # and the second half is allocated to wires originating from the west side.
  # This is the opposite of the numbering among the receiving muxes, among which
  # the first half are on the west side of the tile and the second half are on
  # the east.  The swap in parity makes sense because we are connecting
  # east-sourced wires to the west-located muxes and vice-versa.

  # We map i in
  #   [0, N-1] -> East side of tile
  #   [N, 2N - 1] -> West side of tile
  # and then wrap around.
  #
  # The exception is size-1 west-facing wires, which only exist on the west side
  # of the tile, and size-1 east-facing wires, which only exist on the east side
  # of the tile.

  # SOUTH wires -> OIB S2
  # ---------------------
  dirn = Direction.SOUTH
  i, initial_j, span = initial_i_j_span_by_direction[dirn]
  pattern_count = 0
  for side in OIB_S2_MUX_SIDES_INCREASING_INDEX:
    j = initial_j
    while j < HALF_N_OIB_S2 - span:
      print(f'{DIRECTIONS[dirn]} pattern {pattern_count}')
      # Stamp out a single instance of the pattern.
      connect_wire_oib_s2(arch, dirn, 0, i, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 0, i, j, 'B', side)

      connect_wire_oib_s2(arch, dirn, 2, i, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 2, i, j, 'B', side)

      j = increment_over_specials(j, 2)
      connect_wire_oib_s2(arch, dirn, -1, i, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i, j + 1, 'A', side)

      j = increment_over_specials(j, 2)
      connect_wire_oib_s2(arch, dirn, 1, i, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 1, i, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i, j + 1, 'A', side)

      j = increment_over_specials(j, 4)
      j_offset = j #j + 1 if pattern_count > 0 else j
      connect_wire_oib_s2(arch, dirn, 0, i + 1, j_offset, 'A', side)
      connect_wire_oib_s2(arch, dirn, 0, i + 1, j_offset, 'B', side)

      connect_wire_oib_s2(arch, dirn, 1, i + 1, j_offset, 'A', side)
      connect_wire_oib_s2(arch, dirn, 1, i + 1, j_offset, 'B', side)

      j = increment_over_specials(j, 4)
      i += 2
      pattern_count += 1

    print(f'{DIRECTIONS[dirn]} cleanup after pattern {pattern_count - 1}')

    # Last phase.
    connect_wire_oib_s2(arch, dirn, 0, i, j, 'A', side)
    connect_wire_oib_s2(arch, dirn, 0, i, j, 'B', side)

    connect_wire_oib_s2(arch, dirn, 2, i, j, 'A', side)
    connect_wire_oib_s2(arch, dirn, 2, i, j, 'B', side)

    connect_wire_oib_s2(arch, dirn, 1, i, j + 5, 'A', side)
    connect_wire_oib_s2(arch, dirn, 2, i + 1, j + 5, 'A', side)

    i_offset = i if int(i / N_INTERCONNECT_PAIRS) % 2 == 0 else i - 1
    connect_wire_oib_s2(arch, dirn, -1, i_offset, j + 2, 'B', side)
    connect_wire_oib_s2(arch, dirn, -1, i_offset, j + 4, 'A', side)
    connect_wire_oib_s2(arch, dirn, -1, i_offset, j + 6, 'A', side)

    i += 2


  # EAST wires -> OIB S2
  # --------------------
  dirn = Direction.EAST
  i, initial_j, span = initial_i_j_span_by_direction[dirn]
  pattern_count = 0
  for side in OIB_S2_MUX_SIDES_INCREASING_INDEX:
    j = initial_j
    while j < HALF_N_OIB_S2 - span:
      print(f'{DIRECTIONS[dirn]} pattern {pattern_count}')
      if side == Direction.WEST:
        i_offset = i % N_INTERCONNECT_PAIRS
        connect_wire_oib_s2(arch, dirn, 0, i_offset, j, 'A', side)
        connect_wire_oib_s2(arch, dirn, 0, i_offset, j, 'B', side)

      connect_wire_oib_s2(arch, dirn, 1, i + 1, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 1, i + 1, j, 'B', side)

      j = increment_over_specials(j, 4)
      if side == Direction.WEST:
        i_offset = (i + 1) % N_INTERCONNECT_PAIRS
        connect_wire_oib_s2(arch, dirn, 0, i_offset, j, 'A', side)
        connect_wire_oib_s2(arch, dirn, 0, i_offset, j, 'B', side)

      connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i + 1, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i + 1, j + 1, 'A', side)

      j = increment_over_specials(j, 2)
      connect_wire_oib_s2(arch, dirn, -1, i + 1, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i + 1, j + 1, 'A', side)

      j = increment_over_specials(j, 2)
      j_offset = j #j + 1 if pattern_count > 0 else j
      connect_wire_oib_s2(arch, dirn, 1, i + 2, j_offset, 'A', side)
      connect_wire_oib_s2(arch, dirn, 1, i + 2, j_offset, 'B', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 2, j_offset, 'A', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 2, j_offset, 'B', side)

      j = increment_over_specials(j, 4)
      i += 2
      pattern_count += 1

    print(f'{DIRECTIONS[dirn]} cleanup after pattern {pattern_count - 1}')

    # Last phase.
    if side == Direction.WEST:
      i_offset = i % N_INTERCONNECT_PAIRS
      connect_wire_oib_s2(arch, dirn, 0, i, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 0, i, j, 'B', side)

    connect_wire_oib_s2(arch, dirn, 1, i + 1, j, 'A', side)
    connect_wire_oib_s2(arch, dirn, 1, i + 1, j, 'B', side)

    j = HALF_N_OIB_S2 - 3
    if side == Direction.WEST:
      connect_wire_oib_s2(arch, dirn, 0, i + 1, j, 'A', side)
    connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'A', side)

    i_offset = i + 2 if int(i / N_INTERCONNECT_PAIRS) % 2 == 0 else i + 1
    connect_wire_oib_s2(arch, dirn, -1, i_offset, j + 1, 'A', side)

    i += 2

  # WEST wires -> OIB S2
  # --------------------
  dirn = Direction.WEST
  i, initial_j, span = initial_i_j_span_by_direction[dirn]
  pattern_count = 0
  for side in OIB_S2_MUX_SIDES_INCREASING_INDEX:
    j = initial_j
    while j < HALF_N_OIB_S2 - span:
      print(f'{DIRECTIONS[dirn]} pattern {pattern_count}')
      # Stamp out a single instance of the pattern.
      connect_wire_oib_s2(arch, dirn, -1, i, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i, j + 1, 'A', side)

      j = increment_over_specials(j, 2)
      connect_wire_oib_s2(arch, dirn, 1, i, j + 1, 'A', side)
      connect_wire_oib_s2(arch, dirn, 1, i, j + 1, 'B', side)
      connect_wire_oib_s2(arch, dirn, 2, i, j + 1, 'A', side)
      connect_wire_oib_s2(arch, dirn, 2, i, j + 1, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i, j + 1, 'A', side)

      j = increment_over_specials(j, 5)
      if side == Direction.EAST:
        connect_wire_oib_s2(arch, dirn, 0, i, j, 'A', side)
        connect_wire_oib_s2(arch, dirn, 0, i, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, 1, i + 1, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 1, i + 1, j, 'B', side)

      j = increment_over_specials(j, 4)
      if side == Direction.EAST:
        connect_wire_oib_s2(arch, dirn, 0, i + 1, j, 'A', side)
        connect_wire_oib_s2(arch, dirn, 0, i + 1, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'B', side)

      j = increment_over_specials(j, 1)
      i += 2
      pattern_count += 1

    print(f'{DIRECTIONS[dirn]} cleanup after pattern {pattern_count - 1}')

    # Last phase.
    connect_wire_oib_s2(arch, dirn, -1, i, j, 'B', side)
    connect_wire_oib_s2(arch, dirn, -1, i, j + 1, 'A', side)

    j = increment_over_specials(j, 2)
    connect_wire_oib_s2(arch, dirn, 1, i, j + 1, 'A', side)
    connect_wire_oib_s2(arch, dirn, 1, i, j + 1, 'B', side)
    connect_wire_oib_s2(arch, dirn, 2, i, j + 1, 'A', side)
    connect_wire_oib_s2(arch, dirn, 2, i, j + 1, 'B', side)

    i_offset = i if int(i / N_INTERCONNECT_PAIRS) % 2 == 0 else i - 1
    connect_wire_oib_s2(arch, dirn, -1, i_offset, j, 'B', side)
    connect_wire_oib_s2(arch, dirn, -1, i_offset, j + 1, 'A', side)

    j = increment_over_specials(j, 3)
    i_offset = i + 2
    connect_wire_oib_s2(arch, dirn, -1, i_offset, j, 'B', side)

    j = HALF_N_OIB_S2 - 1
    i_offset = i + 1
    connect_wire_oib_s2(arch, dirn, 0, i, j, 'A', side)
    connect_wire_oib_s2(arch, dirn, 1, i_offset, j, 'A', side)

    i += 2

  # NORTH wires -> OIB S2
  # ---------------------
  dirn = Direction.NORTH
  i, initial_j, span = initial_i_j_span_by_direction[dirn]
  pattern_count = 0
  for k, side in enumerate(OIB_S2_MUX_SIDES_INCREASING_INDEX):
    j = initial_j
    while j < HALF_N_OIB_S2 - span:
      print(f'{DIRECTIONS[dirn]} pattern {pattern_count}')
      # Stamp out a single instance of the pattern.
      connect_wire_oib_s2(arch, dirn, 0, i, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 0, i, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, 1, i, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 1, i, j, 'B', side)

      j = increment_over_specials(j, 4)
      connect_wire_oib_s2(arch, dirn, 0, i + 1, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 0, i + 1, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, 2, i, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 2, i, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i + 2, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i + 2, j + 1, 'A', side)

      j = increment_over_specials(j, 2)
      connect_wire_oib_s2(arch, dirn, -1, i + 2, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, -1, i + 2, j + 1, 'A', side)

      j = increment_over_specials(j, 2)
      connect_wire_oib_s2(arch, dirn, 1, i + 1, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 1, i + 1, j, 'B', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'A', side)
      connect_wire_oib_s2(arch, dirn, 2, i + 1, j, 'B', side)

      j = increment_over_specials(j, 4)
      i += 2
      pattern_count += 1

    print(f'{DIRECTIONS[dirn]} cleanup after pattern {pattern_count - 1}')

    # Last phase.
    connect_wire_oib_s2(arch, dirn, 0, i, j, 'A', side)
    connect_wire_oib_s2(arch, dirn, 0, i, j, 'B', side)
    connect_wire_oib_s2(arch, dirn, 1, i, j, 'A', side)
    connect_wire_oib_s2(arch, dirn, 1, i, j, 'B', side)
    
    i_offset = i + 1
    connect_wire_oib_s2(arch, dirn, 1, i_offset, j + 1, 'A', side)
    connect_wire_oib_s2(arch, dirn, 1, i_offset, j + 1, 'B', side)
    connect_wire_oib_s2(arch, dirn, 2, i_offset, j + 1, 'A', side)
    connect_wire_oib_s2(arch, dirn, 2, i_offset, j + 1, 'B', side)

    j = 0
    connect_wire_oib_s2(arch, dirn, 0, i_offset, j, 'B', side)
    connect_wire_oib_s2(arch, dirn, 2, i, j, 'B', side)
    connect_wire_oib_s2(
      arch, dirn, -1, (k + 1) % len(OIB_S2_MUX_SIDES_INCREASING_INDEX),
      j, 'B', side)

    i += 2


  FAN_IN = 4


  # Disconnect all specials.
  disallowed = set()
  for direction in DIRECTIONS.keys():
    for size, configs in INTERCONNECT_TO_OIB_S2_SPECIALS[direction].items():
      for config in configs:
        _, j, outs, _, dest_sides = config
        for side in dest_sides:
          for out in outs:
            target_mux = arch.get_oib_s2_mux(side, j, out)
            if target_mux is not None:
              disallowed.add(target_mux)

  OIB_S2_PHASE_MAX_FAN_IN = 3

  # Redistribute connections avoiding the muxes in the disallowed set. If
  # something used to be connected to an 'A', find the last 'B', and if it used
  # to be connected to a 'B', find the next 'A'.
  for mux in disallowed:
    previous_inputs = set(mux.get_in_nodes())
    for previously_connected in previous_inputs:
      disconnect_nodes(previously_connected, mux)
      output = mux.name[-1]
      side = get_side_of_tile_by_cumulative_index(mux.ordinal)

      j = mux.ordinal
      k = 1
      if output == 'B':
        next_out = 'B'
        while k < HALF_N_OIB_S2:
          next_out = 'A' if next_out == 'B' else 'B'
          p = (k + j) % HALF_N_OIB_S2
          new_mux = arch.get_oib_s2_mux(side, p, next_out)

          if len(new_mux.get_in_nodes()) < OIB_S2_PHASE_MAX_FAN_IN:
            print(f'reconnected {previously_connected.name} '
                f'to {new_mux.name}')
            connect_nodes(previously_connected, new_mux)
            break
          k += 1
      elif output == 'A':
        next_out = 'A'
        while k < HALF_N_OIB_S2:
          next_out = 'A' if next_out == 'B' else 'B'
          p = (j - k) % HALF_N_OIB_S2
          new_mux = arch.get_oib_s2_mux(side, p, next_out)

          if len(new_mux.get_in_nodes()) < OIB_S2_PHASE_MAX_FAN_IN:
            print(f'reconnected {previously_connected.name} '
                f'to {new_mux.name}')
            connect_nodes(previously_connected, new_mux)
            break
          k += 1

  # Make all special connections.
  # FIXME(aryap): The source interconnect sides are not correctly set for
  # sizes < 12. I believe I have transcribed these incorrectly.
  #
  # EE4 #0 from the east connects to 11B in the east,
  # EE4 #0 from the west connects to 11B int he west.
  # EE4 #0 from the east doest not connect to both.
  print(f'making special interconnect/OIB S2 connections')
  for direction in DIRECTIONS.keys():
    for size, configs in INTERCONNECT_TO_OIB_S2_SPECIALS[direction].items():
      for config in configs:
        i, j, outs, source_side, dest_sides = config
        for side in dest_sides:
          for out in outs:
            connect_wire_oib_s2(arch, direction, None, i, j, out, side,
                                explicit_in_size=size, in_side=source_side)


  oib_s2_regular_mux_list = []
  for _, inner in arch.oib_s2_regular.items():
    for _, mux_list in inner.items():
      oib_s2_regular_mux_list.extend(mux_list)

  # The final stage is to do some reallocation:
  #list_by_fanin(oib_s2_regular_mux_list)

  io_tools.write_adjacency_matrix(
      'interconnect_to_oib_s2.csv',
      arch.interconnect_by_name,
      {mux.name: mux for mux in arch.oib_s2_regular_list})

import io_tools
from constants import *
from common import *
from node import connect_nodes, disconnect_nodes
import functools

# OIB Layer 2 to OIB Layer 1 (Interconnect).
# ------------------------------------------
#

print()
print('Allocating OIB S2 -> OIB S1')
print('OIB S2 specials:', OIB_S2_SPECIAL_INDICES)

OIBS2S1PatternConfig = collections.namedtuple('OIBS2S1PatternConfig', [
    'pattern_number', 'initial_i', 'initial_j'])

# FIXME(aryap): These static pattern configurations do not generalise to
# differing interconnect wire sizes as-written. For bfg this isn't a big deal,
# since we just drop the size-4 entries. But they must be expressed in terms of
# nth biggest wire length instead of the wire length explicitly, or something
# like that.
oib_s2_s1_pattern_configs = {
  Direction.EAST: {
    Direction.EAST: {
      1: OIBS2S1PatternConfig(2, -9, -1),
      2: OIBS2S1PatternConfig(2, 0, 0),
      4: OIBS2S1PatternConfig(2, -5, 0),
    },
    Direction.NORTH: {
      1: OIBS2S1PatternConfig(1, -6, 0),
      2: OIBS2S1PatternConfig(1, 3, 1),
      4: OIBS2S1PatternConfig(1, -2, 0),
    },
    Direction.SOUTH: {
      1: OIBS2S1PatternConfig(1, 1, 1),
      2: OIBS2S1PatternConfig(1, -4, 0),
      4: OIBS2S1PatternConfig(1, -8, 0),
    },
    Direction.WEST: {
      #1: OIBS2S1PatternConfig(2, ?, ?),
      2: OIBS2S1PatternConfig(2, -6, 0),
      4: OIBS2S1PatternConfig(2, 2, 1),
    }
  },
  Direction.WEST: {
    Direction.EAST: {
      #1: OIBS2S1PatternConfig(2, -8, -1),
      2: OIBS2S1PatternConfig(2, 0, 0),
      4: OIBS2S1PatternConfig(2, -5, 0),
    },
    Direction.NORTH: {
      1: OIBS2S1PatternConfig(1, -6, 0),
      2: OIBS2S1PatternConfig(1, 3, 1),
      4: OIBS2S1PatternConfig(1, -2, 0),
    },
    Direction.SOUTH: {
      1: OIBS2S1PatternConfig(1, 1, 1),
      2: OIBS2S1PatternConfig(1, -4, 0),
      4: OIBS2S1PatternConfig(1, -8, 0),
    },
    Direction.WEST: {
      1: OIBS2S1PatternConfig(2, -3, 0),
      2: OIBS2S1PatternConfig(2, -6, 0),
      4: OIBS2S1PatternConfig(2, 1, 1),
    }
  }
}

# The connections from 11B, 22B to EE1_0 seem to be mirrored by connections
# from 22B and 33B to WW1_0 on the west side. We make an exception for this
# and do it explicitly.
#
# The specials used tend to be near one side of the array but not at the very
# end.
shortest_wire_connections_per_side = {
  Direction.EAST: [
    (2*1 + 1, Direction.EAST, 1, 0, Direction.EAST),
    (2*2 + 1, Direction.EAST, 1, 0, Direction.EAST),
  ],
  Direction.WEST: [
    (2*(len(INTERCONNECT_TO_OIB_S2_SPECIALS) - 2) + 1,
        Direction.WEST, 1, 0, Direction.WEST),
    (2*(len(INTERCONNECT_TO_OIB_S2_SPECIALS) - 3) + 1,
        Direction.WEST, 1, 0, Direction.WEST),
  ]
}

# Distribute connections to/from OIB_S2 bounce muxes. The pattern is small
# enough that we can just copy it mostly. It's also duplicated between the west
# and east sides of the tile with caveat that 1-unit wires are only connected
# by the nearest side.
oib_s2_nonlocal_bounce_distribution = {
  # Outer key is outgoing wire direction.
  Direction.EAST: {
    # Next key is wire size.
    1: [
      # For each, we list the driving OIB S2 non-local bounce mux, which half
      # of the list its in and which direction neighbour it comes from:
      (LowHigh.LOW, Direction.NORTH),
      (LowHigh.HIGH, None)
    ],
    2: [
      (LowHigh.LOW, None),
      (LowHigh.HIGH, Direction.SOUTH),
      (LowHigh.HIGH, Direction.SOUTH)
    ],
  },
  Direction.NORTH: {
    1: [
      (LowHigh.HIGH, Direction.SOUTH)
    ],
    2: [
      (LowHigh.LOW, Direction.NORTH)
    ],
    4: [
      (LowHigh.LOW, Direction.NORTH),
      (LowHigh.LOW, Direction.NORTH),
      (LowHigh.HIGH, None),
    ],
  },
  Direction.SOUTH: {
    1: [
      (LowHigh.HIGH, Direction.SOUTH),
      (LowHigh.HIGH, Direction.SOUTH),
    ],
    4: [
      (LowHigh.LOW, None),
      (LowHigh.HIGH, Direction.SOUTH),
      (LowHigh.HIGH, Direction.SOUTH),
    ],
  },
  Direction.WEST: {
    1: [
      (LowHigh.LOW, Direction.NORTH)
    ],
    2: [
      (LowHigh.LOW, None),
      (LowHigh.HIGH, Direction.SOUTH),
      (LowHigh.HIGH, Direction.SOUTH),
    ],
    4: [
      (LowHigh.HIGH, Direction.SOUTH)
    ]
  }
}


# The goal of this function is to add some number, the min_increment, to i,
# with the caveat that if any integer in the range (i, i+min_increment] is a
# special index, an additional increment is made. If the min_increment is 0 we
# check that the initial i is special as well.
def increment_over_oib_s2_specials(i, min_increment):
  if min_increment == 0 and i in set(OIB_S2_SPECIAL_INDICES):
    min_increment += 1
  i_inc = i + min_increment
  for special in set(OIB_S2_SPECIAL_INDICES):
    if special > i and special <= i_inc:
      min_increment += 1
  return i + min_increment


def connect_oib_s2_to_s1(arch,
                         side_of_tile,
                         driver_index,
                         driver_port,
                         wire_direction,
                         wire_size,
                         target_index,
                         wraparound=False):
  if not wraparound:
    if target_index < 0 or target_index >= N_INTERCONNECT_PAIRS:
      return
    if driver_index < 0 or driver_index >= HALF_N_OIB_S2:
      return
  if (driver_index, driver_port) in OIB_S2_NO_DRIVE[side_of_tile]:
    return
  driver_mux = arch.get_oib_s2_mux(side_of_tile, driver_index, driver_port)
  if driver_mux is None:
    return
  target_mux = arch.get_interconnect_wire_out(
      wire_direction, wire_size, target_index, side_of_tile)
  if target_mux is None:
    return

  print(f'connecting {driver_mux.name} -> {target_mux.name}')
  connect_nodes(driver_mux, target_mux)

def pattern_1(arch, side_of_tile, i, wire_direction, wire_size, j):
  i = increment_over_oib_s2_specials(i, 0)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'B', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'B', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 3)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'A', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'A', wire_direction, wire_size, j)

  j += 1
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'B', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'B', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 3)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'A', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'A', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 2)
  j += 1
  return i, j


def pattern_2(arch, side_of_tile, i, wire_direction, wire_size, j):
  i = increment_over_oib_s2_specials(i, 0)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'B', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'B', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 2)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'A', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'A', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  j += 1
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'B', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'B', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 3)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'A', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 1)
  connect_oib_s2_to_s1(arch, side_of_tile, i, 'A', wire_direction, wire_size, j)

  i = increment_over_oib_s2_specials(i, 2)
  j += 1
  return i, j


patterns = {
  1: pattern_1,
  2: pattern_2,
}


def apply_oib_s2_s1_pattern(arch, side_of_tile, wire_direction, wire_size, config):
  pattern_number, initial_i, initial_j = config
  print(f'Applying pattern {pattern_number}, {initial_i=} '
        f'to {SIDES_OF_TILE[side_of_tile]} tile {DIRECTIONS[wire_direction]}'
        f'{wire_size} wire {initial_j=}')

  pattern_fn = patterns[pattern_number]

  i = initial_i
  j = initial_j
  while j < N_INTERCONNECT_PAIRS and i < HALF_N_OIB_S2:
    i, j = pattern_fn(arch, side_of_tile, i, wire_direction, wire_size, j)


def get_first_mux_with_free_inputs(bundle, excluding=None):
  k = 0
  while k < len(bundle):
    if bundle[k][1].fan_in() < OIB_S1_MAX_NUM_INPUTS:
      return bundle[k][1]
    else:
      k += 1
  return None


# Expects a list of pairs, first entry is an integer position, second entry is
# a MuxNode object.
def get_lowest_fan_in(bundle):
  bundle.sort(key=lambda x: (x[1].fan_in(), x[0]))
  return bundle[0][1]

def get_highest_available_fan_in(bundle):
  bundle.sort(key=lambda x: (x[1].fan_in(), x[0]), reverse=True)
  k = 0
  while bundle[k][1].fan_in() >= OIB_S1_MAX_NUM_INPUTS:
    k += 1
  if k < len(bundle):
    return bundle[k][1]
  return None


def get_first_mux_below_fan_out(nodes, max_fan_out, excluding):
  k = 0
  while k < len(nodes):
    if nodes[k].fan_out() < max_fan_out and nodes[k] not in excluding:
      return nodes[k]
    k += 1
  return None


def get_lowest_fan_out(nodes):
  if not nodes:
    return None
  nodes.sort(key=lambda x: x.fan_out())
  return nodes[0]

def connect(arch):
  for side_of_tile, inner in oib_s2_s1_pattern_configs.items():
    for wire_direction, inner_inner in inner.items():
      for wire_size, config in inner_inner.items():
        apply_oib_s2_s1_pattern(
            arch, side_of_tile, wire_direction, wire_size, config)

  io_tools.write_adjacency_matrix(
      'oib_2_regular_to_1.csv',
      {mux.name: mux for mux in arch.oib_s2_regular_list},
      arch.oib_s1_by_name)

  # Apply irregular nodes:
  #   Reserved regular muxes (specials) connecting to first 1-unit wires
  #   Bounce muxes
  #   Remaining reserved regular muxes (specials)
  non_participating_bundles_per_side = collections.defaultdict(list)

  oib_s2_specials_both_sides = []

  specials_per_side = {}
  for side_of_tile in SIDES_OF_TILE:
    # This is an extremely obtuse way of putting the 'A' and 'B' outputs of each
    # special mux into a list.
    specials = functools.reduce(
        lambda x, y: x + [
            arch.get_oib_s2_mux(side_of_tile, y, 'A'),
            arch.get_oib_s2_mux(side_of_tile, y, 'B')],
        OIB_S2_SPECIAL_INDICES,
        [])
    oib_s2_specials_both_sides.extend(specials)
    specials_per_side[side_of_tile] = specials

    # Make asymmetric connections to shortest-length wires.
    for connection in shortest_wire_connections_per_side[side_of_tile]:
      i, wire_direction, wire_size, j, wire_source = connection
      if i >= len(specials):
        continue

      driver = specials[i]
      target_mux = arch.get_interconnect_wire_out(
          wire_direction, wire_size, j, side_of_tile)

      print(f'{i+1=} {j-1=} connecting {driver.name} -> {target_mux.name}')
      connect_nodes(driver, target_mux)

      non_participating_bundles_per_side[side_of_tile].append(
          (wire_direction, wire_size))


  for side_of_tile in SIDES_OF_TILE:
    for wire_direction, inner in oib_s2_nonlocal_bounce_distribution.items():
      for wire_size, specs in inner.items():
        for low_or_high, neighbour_direction in specs:
          # Get the lowest fan-in mux of correct size and direction:
          out_wires = arch.get_interconnect_wire_out_bundle(
              side_of_tile, wire_direction, wire_size)
          out_wires = list(filter(lambda x: x[1] is not None, out_wires))

          if not out_wires:
            print('could not make nonlocal bounce connection '
                  f'{wire_direction=} {wire_size=} {low_or_high=} '
                  f'{neighbour_direction=}, there are no out_wires here')
            continue
          target_mux = get_lowest_fan_in(out_wires)

          if target_mux is None:
            print(f'warning: no available muxes on {side_of_tile} side '
                  f'{wire_direction=} {wire_size=}')
            continue

          # Get the lowest fan-out driver from the correct tile:
          part = 0 if low_or_high == LowHigh.LOW else 1
          drivers = arch.get_oib_s2_nonlocal_bounce_muxes(
              neighbour_direction, side_of_tile, part)
          driver = get_first_mux_below_fan_out(
              drivers,
              OIB_S2_FAN_OUT,
              set(target_mux.get_in_nodes()) | arch.oib_s2_nonlocal_bounce_no_drive)

          print(f'connecting nonlocal bounce {driver.name} -> {target_mux.name}')
          connect_nodes(driver, target_mux)

  # TODO(aryap) Non-local bounce mux use from neighbouring tiles determines the
  # input/output pins on each block.

  # FIXME(aryap): I do not yet understand how special OIB S2 nodes 0, 11, 22, 33,
  # 44 fan out to the first/second wires in each interconnect bundle.

  # Excluding biggest wires from the number of bundles:
  num_bundles = (len(INTERCONNECT_WIRE_SIZES) - 1) * len(DIRECTIONS)

  for side_of_tile in SIDES_OF_TILE:
    # The order matters, as it's the order the connections will be distributed
    # over:
    specials = specials_per_side[side_of_tile]
    participating_out_bundles = []
    for wire_direction in [
        Direction.EAST, Direction.NORTH, Direction.SOUTH, Direction.WEST]:
      for wire_size in INTERCONNECT_WIRE_SIZES[:-1]:
        if (wire_direction, wire_size) in non_participating_bundles_per_side[
            side_of_tile]:
          participating_out_bundles.append([])
          continue
        # IMPORTANT! out_wires can include None, indicating that no connection
        # should be made. We remove those but keep their place in their list,
        # since lookup by j should be the same on both sides of the tile.
        out_wires = arch.get_interconnect_wire_out_bundle(
            side_of_tile, wire_direction, wire_size)
        out_wires = list(filter(lambda x: x[1] is not None, out_wires))
        participating_out_bundles.append(out_wires)


    def distribute_oib_s2_specials_over_s1(initial_i, initial_j):
      # i is an index over 'specials' now.
      i = initial_i
      j = initial_j
      while i >= 0 and j < num_bundles:
        if i >= len(specials):
          j += 1
          i -= 1
          continue

        driver = specials[i]
        i -= 1

        out_wires = participating_out_bundles[j]
        j += 1
        if not out_wires:
          continue

        if (driver.ordinal, driver.name[-1]) in OIB_S2_NO_DRIVE[side_of_tile]:
          print(f'skipping {driver.name}')
          continue

        if driver.fan_out() >= OIB_S2_FAN_OUT:
          continue

        # One idea: find the lowest-fan-in thing to drive.
        # Better idea: find the first thing with available fan-in to drive.
        target_mux = get_first_mux_with_free_inputs(out_wires)
        if target_mux is None:
          continue

        print(f'{i+1=} {j-1=} connecting {driver.name} -> {target_mux.name} '
              f'existing fan-in {target_mux.fan_in()}')
        connect_nodes(driver, target_mux)

    # FIXME(aryap): This pattern needs to be generalised.
    # FIXME(aryap): I think what I did is write the numbers as if the specials
    # list contained specials for both sides of the tile, but actually it
    # doesn't.
    for i in (3, 5, 8, 10, 13, 15, 18, 20):
      distribute_oib_s2_specials_over_s1(i, 0)


  # How many specials have less than max fan out?
  for driver in oib_s2_specials_both_sides:
    print(driver.fan_out(), driver.name)

  # Assign driver with available fan-out to output muxes with available fan-in.
  # Find the nearest (by ordinal) ones each time?

  io_tools.write_adjacency_matrix(
      'oib_2_specials_to_1.csv',
       {mux.name: mux for mux in oib_s2_specials_both_sides},
       arch.oib_s1_by_name)


  io_tools.write_adjacency_matrix(
      'oib_2_bounce_to_1.csv',
      {mux.name: mux for mux in arch.oib_s2_nonlocal_bounce_list},
      arch.oib_s1_by_name,
      skip_empties=True)

  io_tools.write_adjacency_matrix(
      'oib_2_bounce_and_specials_to_1.csv',
      {mux.name: mux
       for mux in arch.oib_s2_nonlocal_bounce_list + oib_s2_specials_both_sides},
      arch.oib_s1_by_name,
      skip_empties=True)

  #oib_s2_mux_list = (
  #    oib.s2_regular_list + oib_s2_specials_both_sides +
  #    arch.oib_s2_nonlocal_bounce_list)
  io_tools.write_adjacency_matrix(
      'oib_2_to_1.csv',
      arch.oib_s2_by_name,
      arch.oib_s1_by_name,
      skip_empties=False)

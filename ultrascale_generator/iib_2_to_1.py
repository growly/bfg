from constants import *
import io_tools
from node import connect_nodes

# IIB Bypass/bounce muxes.
# --------------------
#

def connect(arch):
  # First step is to put the bounce (B) and bypass (Y) muxes together in a linear
  # list. We interleave half the bounces with the bypasses at the beginning of
  # the range, and half at the end.

  bb_muxes = []

  for i in range(0, HALF_N_IIB_S2_BOUNCE):
    bb_muxes.append(arch.iib_s2_bounce[i])
    bb_muxes.append(arch.iib_s2_bypass[i])

  # Middle section is the bulk of the bypasses.
  start = HALF_N_IIB_S2_BOUNCE
  remaining_bounce = N_IIB_S2_BOUNCE - HALF_N_IIB_S2_BOUNCE
  end = N_IIB_S2_BYPASS - remaining_bounce

  for i in range(start, end):
    bb_muxes.append(arch.iib_s2_bypass[i])

  # Last section alternates bounce and bypass in the opposite way to the first
  # section:
  for i in range(0, remaining_bounce):
    bb_muxes.append(arch.iib_s2_bypass[end + i])
    bb_muxes.append(arch.iib_s2_bounce[start + i])

  print()
  print('Physical-esque arrangement of bypass/bounce muxes:')
  for i, mux in enumerate(bb_muxes):
    print(i, mux.name)

  num_bb_muxes = len(bb_muxes)
  unallocated_bb_muxes = set((i, mux) for i, mux in enumerate(bb_muxes))

  # Store a triple containing the newly added fan-out, original position, and mux
  # object.
  bb_muxes_by_fan_out = []

  # Do not connect second-side connections for 25, 27, 28, 29?
  # TODO(aryap): Not sure where these come from.
  disconnected_a = {5,
                    HALF_N_IIB_S1_REGULAR - 7,
                    HALF_N_IIB_S1_REGULAR - 5,
                    HALF_N_IIB_S1_REGULAR - 4,
                    HALF_N_IIB_S1_REGULAR - 3}

  disconnected_b = {0, HALF_N_IIB_S1_REGULAR - 1} | set(IIB_S1_SPECIAL_INDICES)

  def connect_source_to_iib_s1_regular_a_b(source, iib_s1_side, iib_s1_index):
    new_fan_out = 0
    if iib_s1_index not in disconnected_a:
      sink = arch.get_iib_s1_regular_mux(iib_s1_side, iib_s1_index, 'A')
      if sink is not None:
        print(f'Assigning {source.name} -> {sink.name}')
        connect_nodes(source, sink)
        new_fan_out += 1

    if iib_s1_index not in disconnected_b:
      sink = arch.get_iib_s1_regular_mux(iib_s1_side, iib_s1_index, 'B')
      if sink is not None:
        print(f'Assigning {source.name} -> {sink.name}')
        connect_nodes(source, sink)
        new_fan_out += 1

    return new_fan_out


  skip_allocation = set(IIB_S1_SPECIAL_INDICES) | {
      HALF_N_IIB_S1_REGULAR - 2,
      HALF_N_IIB_S1_REGULAR - 1}

  print()
  i = num_bb_muxes - 1
  j = 2
  spread = 2
  while j < HALF_N_IIB_S1_REGULAR:
    source = bb_muxes[i]

    unallocated_bb_muxes.remove((i, source))

    new_fan_out = 0
    k = 0
    while k < spread:
      index = j + k
      if index in skip_allocation:
        j += 1
        if j >= HALF_N_IIB_S1_REGULAR:
          break
        continue

      new_fan_out += connect_source_to_iib_s1_regular_a_b(source, lut_side, index)

      k += 1

    bb_muxes_by_fan_out.append((new_fan_out, i, source))

    i = (i - 7) % num_bb_muxes
    j += spread

  for i, source in unallocated_bb_muxes:
    bb_muxes_by_fan_out.append((0, i, source))

  bb_muxes_by_fan_out.sort()

  print()
  print('Need to allocate:')
  for fan_in, original_position, mux in bb_muxes_by_fan_out:
    print(fan_in, original_position, mux.name)

  # The particular IIB S1 muxes that are driven here are 0, 1, N-2 and N-1, as well
  # as the 'IIB_S1_SPECIAL_INDICES' from before. N is HALF_N_IIB_S1_REGULAR, or
  # the number of muxes on the IIB S1 bank of this side of the tile.
  fan_across = collections.deque(sorted(set(
    IIB_S1_SPECIAL_INDICES + [0,
                              1,
                              HALF_N_IIB_S1_REGULAR - 2,
                              HALF_N_IIB_S1_REGULAR - 1])))
  num_fan_across = len(fan_across)
  print('Fanning across', ', '.join(str(x) for x in fan_across))

  # Allocate j = 0, then j + 2, next set j += 1 and allocate j and j - 2, then
  # set j += 1 and allocate j and j + 2, etc.
  allocated = set()
  j = 0
  skip_forward = True

  # -1  0  1  2 | 3  4  5  6
  # ------------+------------
  #     0       |
  #           1 |
  #        2    |
  #  3          |
  #             |    4
  #             |          5
  #             |       6
  #             | 7
  #
  # Contiguous groups of 4 are allocated every two iterations. The next starting
  # point is then +4 on the last starting point.
  #
  # But to match the output we're seeing, we have to consider sparsely populated
  # columns, picking the next available each time. The last group might not have
  # all columns and so at each step selecting indices we must check if they have
  # been allocated and, if not, find one that hasn't been.

  def next_unallocated_or_none(start):
    i = 0
    while start in allocated and i < num_fan_across:
      start += 1
    return start if start not in allocated else None

  while len(allocated) < num_fan_across:
    _, original_index, next_driver = bb_muxes_by_fan_out[0]

    k = next_unallocated_or_none(j % num_fan_across)
    if k is None:
      break
    connect_source_to_iib_s1_regular_a_b(next_driver, lut_side, fan_across[k])
    allocated.add(k)

    k = next_unallocated_or_none(
        (j + 2) % num_fan_across if skip_forward else (j - 2) % num_fan_across)
    if k is None:
      break
    connect_source_to_iib_s1_regular_a_b(next_driver, lut_side, fan_across[k])
    allocated.add(k)

    # update fan out
    bb_muxes_by_fan_out[0] = (next_driver.fan_out(), original_index, next_driver)
    bb_muxes_by_fan_out.sort()

    j += 1
    if skip_forward:
      skip_forward = False
    else:
      skip_forward = True
      # j is now 4 greater than what it was when we started the loop with
      # skip_forward = True.
      j += 2

  # Allocate un-allocated bypass/bounce muxes to special muxes (near edges, other
  # specials).

  io_tools.write_adjacency_matrix(
      'iib_2_to_1_bounce.csv',
      {m.name: m for m in bb_muxes},
      arch.iib_s1_by_name)

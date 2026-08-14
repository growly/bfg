# OIB S2 to IIB S1, including
#   IIB S2 to OIB S2
# ---------------------------
#

# The "INT/INT_INT_SDQ" nodes provide a feedback path from the output block
# back to the input block, and can be driven in a few cases directly from the
# input block. We will call them the OIB S2 "bounce" nodes.

# The OIB S2 bounce nodes drive
#   1) global nodes
#   2) IIB S1 nodes
#   3) other OIB S2 nodes
# with different fan-out patterns each.
#
# They are also driven by IIB S1 nodes, with yet another fan-out pattern.

import math
import sys

import architecture
from constants import *
from node import maybe_connect_nodes, connect_nodes, maybe_disconnect_nodes, disconnect_nodes, MuxNode, NoFreeInputs
import io_tools

# If it's 0, 11, 22 and there are 32 muxes per side, it's 0, 11, 22, 32,
# 32 + 11, 32 + 22, ...

IIB_S1_SPECIALS_LESS_0 = list(filter(lambda x: x != 0, IIB_S1_SPECIAL_INDICES))
IIB_S1_SKIP = IIB_S1_SPECIALS_LESS_0 + [
    x + HALF_N_IIB_S1_REGULAR for x in IIB_S1_SPECIALS_LESS_0]


def increment_over_iib_s1_specials(i, min_increment):
  return architecture.increment_over_exclusions(
      i, min_increment, set(IIB_S1_SKIP))


def get_oib_s2_mux_or_none(arch, side_of_tile, index, out_port):
  try:
    return arch.get_oib_s2_mux(side_of_tile, index, out_port)
  except:
    return None


OIB_S2_SKIP = OIB_S2_SPECIAL_INDICES + [
    HALF_N_OIB_S2 + x for x in OIB_S2_SPECIAL_INDICES]
def increment_over_oib_s2_specials(
    i, min_increment, additional_exclusions=set()):
  return architecture.increment_over_exclusions(
      i, min_increment, set(OIB_S2_SKIP) | additional_exclusions)


# This pattern assumes an even number of bounce nodes, with a special node in
# the middle of each half assigned to drive the special nodes in their half of
# the receiving IIB S1 muxes. The first node in the second half of bounce nodes
# is also assigned to drive the latter nodes in the first half of IIB S1
# receiving muxes.
def iib_s1_bounce_fan_out_pattern_1(
    arch, source_muxes, source_increment_fn, source_index, target_index,
    assigned):
  # Starting values.
  i = source_index
  j = target_index

  driver = source_muxes[i]
  target = arch.get_iib_s1_regular_mux(None, j, 'A')
  connect_nodes(driver, target)
  
  # FIXME(aryap): Here and elsewhere: need to gracefully handle full muxes and
  # find a suitable alternative sometimes. Possible here.
  target = arch.get_iib_s1_regular_mux(None, j, 'B')
  connect_nodes(driver, target)

  j = increment_over_iib_s1_specials(j, 1)
  if j >= N_IIB_S1_REGULAR:
    return i, j
  target = arch.get_iib_s1_regular_mux(None, j, 'A')
  connect_nodes(driver, target)
  target = arch.get_iib_s1_regular_mux(None, j, 'B')
  connect_nodes(driver, target)

  i = source_increment_fn(i, 7)
  j = increment_over_iib_s1_specials(j, 1)
  if j >= N_IIB_S1_REGULAR:
    return i, j

  driver = source_muxes[i]
  target = arch.get_iib_s1_regular_mux(None, j, 'A')
  connect_nodes(driver, target)
  target = arch.get_iib_s1_regular_mux(None, j, 'B')
  connect_nodes(driver, target)

  j = increment_over_iib_s1_specials(j, 1)
  if j >= N_IIB_S1_REGULAR:
    return i, j
  target = arch.get_iib_s1_regular_mux(None, j, 'A')
  connect_nodes(driver, target)
  target = arch.get_iib_s1_regular_mux(None, j, 'B')
  connect_nodes(driver, target)

  i = source_increment_fn(i, 7)
  j = increment_over_iib_s1_specials(j, 5)
  return i, j

# This pattern assumes an odd number of nodes, split into two equal halves with
# one extra in the very middle. One from the latter half is assigned as a
# 'special' for driving the special nodes in the second half of the receiving
# muxes, and the single central node is used to drive the latter muxes at the
# end of the first half of receiving nodes.
def iib_s1_bounce_fan_out_pattern_2(
    arch, source_muxes, source_increment_fn, source_index, target_index,
    assigned):
  # Starting values.
  i = source_index
  j = target_index

  if i in assigned:
    return None, j

  driver = source_muxes[i]
  target = arch.get_iib_s1_regular_mux(None, j, 'A')
  connect_nodes(driver, target)
  target = arch.get_iib_s1_regular_mux(None, j, 'B')
  connect_nodes(driver, target)

  assigned.add(i)

  j = increment_over_iib_s1_specials(j, 1)
  if j >= N_IIB_S1_REGULAR:
    return i, j
  target = arch.get_iib_s1_regular_mux(None, j, 'A')
  connect_nodes(driver, target)
  target = arch.get_iib_s1_regular_mux(None, j, 'B')
  connect_nodes(driver, target)

  i = source_increment_fn(i, 8)
  j = increment_over_iib_s1_specials(j, 1)
  if i in assigned:
    return None, j
  if j >= N_IIB_S1_REGULAR:
    return i, j

  driver = source_muxes[i]
  target = arch.get_iib_s1_regular_mux(None, j, 'A')
  connect_nodes(driver, target)
  target = arch.get_iib_s1_regular_mux(None, j, 'B')
  connect_nodes(driver, target)

  j = increment_over_iib_s1_specials(j, 1)
  if j >= N_IIB_S1_REGULAR:
    return i, j
  target = arch.get_iib_s1_regular_mux(None, j, 'A')
  connect_nodes(driver, target)
  target = arch.get_iib_s1_regular_mux(None, j, 'B')
  connect_nodes(driver, target)

  assigned.add(i)

  i = source_increment_fn(i, 8)
  j = increment_over_iib_s1_specials(j, 5)
  return i, j


def oib_s2_bounce_fan_out_pattern(
    arch, source_muxes, source_increment_fn, source_index, target_index,
    assigned):
  i = source_index
  j = target_index

  if i in assigned:
    return None, j

  driver = source_muxes[i]
  target = arch.get_oib_s2_mux(None, j, 'A')
  maybe_connect_nodes(driver, target)
  target = arch.get_oib_s2_mux(None, j, 'B')
  maybe_connect_nodes(driver, target)

  assigned.add(i)

  i = source_increment_fn(i, 8)
  j = increment_over_oib_s2_specials(j, 4)
  if i in assigned:
    return None, j
  if j >=  N_OIB_S2:
    return i, j

  driver = source_muxes[i]
  target = arch.get_oib_s2_mux(None, j, 'A')
  maybe_connect_nodes(driver, target)
  target = arch.get_oib_s2_mux(None, j, 'B')
  maybe_connect_nodes(driver, target)

  assigned.add(i)

  i = source_increment_fn(i, 8)
  j = increment_over_oib_s2_specials(j, 8)
  return i, j


def asymmetric_specials_scheme(
    arch: architecture.Architecture(),
    ordered_muxes: list[MuxNode],
    oib_s2_bounce_special_indices: list[int]):
  # These are reserved for allocation last:
  flank_sources = [ordered_muxes[k] for k in oib_s2_bounce_special_indices]
  half_range = int(len(flank_sources)/ 2)

  for source in flank_sources[1:]:
    for existing in set(source.get_out_nodes()):
      maybe_disconnect_nodes(source, existing)

    for target_index in IIB_S1_SPECIALS_LESS_0:
      target = arch.get_iib_s1_regular_mux(
          IIB_S1_MUX_SIDES_INCREASING_INDEX[1], target_index, 'A')
      connect_nodes(source, target)


def oib_s2_bounce_fan_in_pattern_side(
    arch, target_muxes, source_index, target_index, used_source_indices):
  i = source_index
  j = target_index

  target = target_muxes[j]

  source = arch.get_oib_s2_mux(None, i, 'B')
  connect_nodes(source, target)
  used_source_indices.add(i)

  i = increment_over_oib_s2_specials(i, 1)
  source = arch.get_oib_s2_mux(None, i, 'B')
  connect_nodes(source, target)
  used_source_indices.add(i)

  i = increment_over_oib_s2_specials(i, 2)
  source = arch.get_oib_s2_mux(None, i, 'A')
  connect_nodes(source, target)
  used_source_indices.add(i)

  i = increment_over_oib_s2_specials(i, 1)
  source = arch.get_oib_s2_mux(None, i, 'A')
  connect_nodes(source, target)
  used_source_indices.add(i)
  return i


# The central group pattern connects nearby, clustered OIB S2 muxes from the
# same side of the tile.
def oib_s2_bounce_fan_in_pattern_central(
    arch, target_muxes, source_index, target_index, used_source_indices):
  # mux i, i + 1, i + 4, i + 5 drives target
  i = source_index
  j = target_index

  target = target_muxes[j]

  source = get_oib_s2_mux_or_none(arch, None, i, 'B')
  connect_nodes(source, target)
  used_source_indices.add(i)

  i = increment_over_oib_s2_specials(i, 1)
  source = get_oib_s2_mux_or_none(arch, None, i, 'B')
  connect_nodes(source, target)
  used_source_indices.add(i)

  i = increment_over_oib_s2_specials(i, 3)
  source = get_oib_s2_mux_or_none(arch, None, i, 'A')
  connect_nodes(source, target)
  used_source_indices.add(i)

  i = increment_over_oib_s2_specials(i, 1)
  source = get_oib_s2_mux_or_none(arch, None, i, 'A')
  connect_nodes(source, target)
  used_source_indices.add(i)
  return i



# Ok this might not be true. It's not clear if the evidence supports it. I
# don't htink it actually does, since it just seems to be asymmetric.
# NOTE(aryap): There is a special driver among the bounce nodes. There are
# probably two, and one is probabaly hidden because it wasn't included in the
# query we made to RapidWright. I should check that. In any case it makes for
# sense for it to be included somewhere. For symmetry let's assume there are
# 16, not 15, bounce nodes, two of which are special, not 1, and each special
# can then connect to the special indices in one half of IIB S1. And it's
# PROBABLY in the middle of the range of bounce muxes on each side because that
# probably has something to do with it being approximately physically located
# in the middle, and therefore a useful point to connect to equidistant special
# indices from.
#
# The middle bounce node is also special, and is is apparently reserved for
# connecting to the middle of the IIB S1 range. IIB S1 muxes toward the end of
# the second side don't seem to be connected to, which is an interesting
# asymmetry.
#
# TL;DR: The assymetric case seems to work, the symmetric one does not yet.

def connect(arch: architecture.Architecture):
  assert N_OIB_S2_BOUNCE % 2 != 0, (
      'assumed odd number of bounce muxes for this to work')

  ordered_muxes = arch.oib_s2_bounce_list
  left_special = arch.oib_s2_bounce_special_indices[0]
  right_special = arch.oib_s2_bounce_special_indices[1]

  # Fan-out across global nodes:

  # TODO(aryap): This.

  # Fan-out across IIB S1 nodes:
  # ----------------------------
  # Familiar shuffling pattern with exception around the end of the IIB S1 range
  # on each side of the tile.

  def increment_over_oib_s2_bounce_special_indices(x, min_increment):
    return architecture.increment_over_exclusions_with_rollover(
        x, min_increment, N_OIB_S2_BOUNCE, set(
          arch.oib_s2_bounce_special_indices))

  def increment_over_source(x, min_increment):
    # This one does not check for exclusions.
    return (x + min_increment) % N_OIB_S2_BOUNCE

  j = 1
  i = 0
  assigned = set()
  while j < N_IIB_S1_REGULAR and i is not None:
    i_out, j = iib_s1_bounce_fan_out_pattern_2(
        arch, ordered_muxes, increment_over_source, i, j, assigned)
    # TODO(aryap): We *may* have to intervene and make sure i increments
    # here, but the bigger problem in that case will probably be that the
    # user has opted for an even N_OIB_S2_BOUNCE. (See assertion above.)
    i = i_out

  receivers = dict(arch.iib_s1_by_name)
  receivers.update(arch.oib_s2_by_name)

  # Fan-out to flanks:
  # ------------------
  # Connect first half of flanks to each of the flank IIB S1 flank nodes
  # in the first half, and the second of flanks to those in the second half.
  asymmetric_specials_scheme(arch, ordered_muxes,
                             arch.oib_s2_bounce_special_indices)

  # Re-assign the central OIB S2 bounce to the end of the first half of IIB S1
  # muxes (it's the start of the second half, not really the 'central' one.
  central = ordered_muxes[int(N_OIB_S2_BOUNCE / 2)]
  other_muxes = list(set(ordered_muxes) - set([central]))
  for node in set(central.get_out_nodes()):
    try:
      disconnect_nodes(central, node)
    except KeyError as err:
      # Ignore KeyErrors since we might have removed the node from a shared
      # input driving this mux partition already.
      pass
  count = 0
  j = HALF_N_IIB_S1_REGULAR - 1
  next_out = 'B'
  # TODO(aryap): The fan out, 3, is a parameter. But it's so specific I'm not
  # sure how it should be exposed yet.
  while count < 3:
    target = arch.get_iib_s1_regular_mux(None, j, next_out)
    # Remove any connections to this mux from other OIB S2 bounce muxes:
    for other in other_muxes:
      try:
        disconnect_nodes(other, target)
      except KeyError:
        pass

    try:
      connect_nodes(central, target)
      count += 1
    except NoFreeInputs as err:
      # Ignore full nodes:
      print(
          f'warning: cannot assign central OIB S2 bounce {central} to {target}')

    if next_out == 'A':
      j -= 1
    next_out = 'A' if next_out == 'B' else 'B'

  # Fan-out across regular OIB S2 nodes:
  # ---------------------------------
  def double_skip_over_right_special(x, min_increment):
    x_inc = (x + min_increment) % N_OIB_S2_BOUNCE
    #if x_inc == right_special:
    #  # In practice it just seems to go up by another 8.
    #  x_inc = (x_inc + min_increment) % N_OIB_S2_BOUNCE
    return x_inc

  assigned.clear()
  i = int(N_OIB_S2_BOUNCE / 2)  # This is the index of the 'central' mux above.
  j = 1
  for side_of_tile in OIB_S2_MUX_SIDES_INCREASING_INDEX:
    start, end = BOUNDS_BY_SIDE_OF_TILE[side_of_tile]
    while i is not None and j >= start and j < end:
      i, j = oib_s2_bounce_fan_out_pattern(
          arch, ordered_muxes, double_skip_over_right_special, i, j, assigned)
    if i is None:
      break
    i = (i + 8) % N_OIB_S2_BOUNCE
    j += 1

  # Assign right_special again:
  target_index = OIB_S2_SKIP[-2]
  source = ordered_muxes[right_special]
  for out_port in ('A', 'B'):
    target = arch.get_oib_s2_mux(None, target_index, out_port)
    connect_nodes(source, target)

  # Fan-in from OIB S2 nodes:
  # -------------------------
  # 1. Allocate specials
  # 2. Allocate left, central, right groups.

  # The special nodes among OIB S2 and among the OIB S2 BOUNCE muxes are known
  # in advance, so we allocate them first. The east-side special bounce mux
  # takes inputs from the right-side special regular muxes, and vice versa.

  # TODO(aryap): Maybe we use something sort-of-automatic like SharedMuxNode's
  # get_lowest_fan_out_partition to pick the driver?

  # Each shared OIB S2 mux is used once, so we only track the ordinal we used:
  used_source_indices = set()

  # The drivers seem to be statically ordinals 1, 2 and the middle two specials
  # in the range on that side of the tile:
  source_ports = ['A', 'A', 'B', 'B']

  targets_by_source = {
      Direction.WEST: [arch.oib_s2_bounce_specials[0]],
      Direction.EAST: [arch.oib_s2_bounce_specials[1]]
  }

  source_indices_by_source = {
    # 1, 2, 22, 33
    Direction.EAST: [
      1,
      2,
      OIB_S2_SPECIAL_INDICES[int(len(OIB_S2_SPECIAL_INDICES) / 2)],
      OIB_S2_SPECIAL_INDICES[int(len(OIB_S2_SPECIAL_INDICES) / 2) + 1]],
    # 1, 2, 11, 22
    Direction.WEST: [
      1,
      2,
      OIB_S2_SPECIAL_INDICES[int(len(OIB_S2_SPECIAL_INDICES) / 2) - 1],
      OIB_S2_SPECIAL_INDICES[int(len(OIB_S2_SPECIAL_INDICES) / 2)]
    ]
  }

  for side_of_tile, source_indices in source_indices_by_source.items():
    for k, i in enumerate(source_indices):
      out_port = source_ports[i % len(source_indices)]
      source = arch.get_oib_s2_mux(side_of_tile, i, out_port)
      ordinal = source.ordinal
      for target in targets_by_source[side_of_tile]:
        connect_nodes(source, target)
      used_source_indices.add(ordinal)


  alt_oib_s2_fan_in(arch, source_indices_by_source)
  #pattern_based_oib_s2_fan_in(arch, used_source_indices)

  io_tools.write_adjacency_matrix(
      'oib_2_bounce.csv',
      arch.oib_s2_by_name,
      receivers,
      skip_empties=False)


def pattern_based_oib_s2_fan_in(arch: architecture.Architecture(),
                                used_source_indices):
  # Entries are the start index, pattern function, minimum span required, and
  # the post increment to i.
  patterns_by_group_index = [
      (HALF_N_OIB_S2, oib_s2_bounce_fan_in_pattern_side, 5, 8),
      (0, oib_s2_bounce_fan_in_pattern_central, 6, 7),
      (0, oib_s2_bounce_fan_in_pattern_side, 5, 8)
  ]

  # An alternative to the dumb linear way is to first chunk up the range into
  # groups of the required span size (automatically extending if they contain a
  # special index), and then remove the ones that have existing source indices
  # in them. But we don't do that because I think that's more work on average.
  def first_index_fitting_span(start, span):
    while True:
      p = start
      count = 0
      while p <= N_OIB_S2 - span:
        if p in OIB_S2_SKIP:
          p += 1
          continue
        if p not in used_source_indices:
          count += 1
          p += 1
        else:
          start = p + 1
          break
        if count == span:
          return start
      # Got to the end without finding anything.
      if p >= N_OIB_S2 - span + 1:
        return None

  # This is arranged so that west (high number) OIB S2 muxes feed east (low
  # number) OIB S2 bounce muxes.

  # The start point is the first entry in the second half of the tile (first
  # mux on the west side).
  for g, target_muxes in enumerate(arch.oib_s2_bounce_groups):
    i, pattern_fn, required_span, post_increment = patterns_by_group_index[g]
    i = first_index_fitting_span(i, required_span)
    if i is None:
      print(f'error: can not finding starting index for span {required_span}')
      sys.exit(1)
    print(f'group {g=} starting at {i=} for span={required_span}')
    j = 0
    while j < len(target_muxes) and i < N_OIB_S2:
      if target_muxes[j] in arch.oib_s2_bounce_specials:
        j += 1
        # This is a gross special case excepion for the middle group to avoid
        # the left special (which is really in the centre of the OIB S2 bounce
        # muxes).
        if g == 1:
          i = increment_over_oib_s2_specials(i, required_span + post_increment)
        continue
      i = pattern_fn(arch, target_muxes, i, j, used_source_indices)
      i = increment_over_oib_s2_specials(i, post_increment)
      j += 1


def alt_oib_s2_fan_in(arch: architecture.Architecture(), 
                      source_special_indices_by_side_of_tile):
  # TODO(aryap): I think a decent way to generalise this algorithm is to
  # break the list of M OIB S2 regular muxes into contiguous groups of I + 1
  # muxes, by ordinal. I is the number of inputs to each of the N bounce muxes.
  # N is odd.  The central bounce mux, and the 3/4-point bounce mux, are
  # special receivers. The special senders/drivers are:
  #   1, 2, 22, 33, (M / 2) + 1, (M / 2) + 2, (M / 2) + 11, (M / 2) + 22 
  #
  # Allocate the special senders to receivers in the static manner above.
  # Put the receiving bounce muxes into groups, as above.
  #
  # Then, remove the special senders from the driver list, and find contiguous
  # groups of I + 1 senders. Alternatively allocate each group as drivers to
  # successive bounce muxes from two of the three sending groups:
  #
  # All the senders should be identified by ordinal, which in fact corresponds
  # to a shared mux with two outputs. Assign to either the lowest-fan-out
  # output or use the static pattern above.
  skip_receivers = set(arch.oib_s2_bounce_special_indices)

  bounce_groups = list(arch.oib_s2_bounce_groups)
  assert 3 == len(bounce_groups), (
      f'not sure what to do with {len(bounce_groups)} != 3 receiver groups')

  # We actually want the groups by the ordinals of the shared muxes. At the
  # same time, remove skipped receiving muxes.
  ordinals = []
  for group in bounce_groups:
    replacement = []
    for mux in group:
      ordinal = mux.ordinal
      if ordinal in skip_receivers:
        continue
      replacement.append(ordinal)
    ordinals.append(replacement)

  # Split the central group in two:
  receiver_groups = [
      ordinals[0],
      [ordinals[1][x] for x in range(int(len(ordinals[1]) / 2))],
      [ordinals[1][x]
       for x in range(int(len(ordinals[1]) / 2), len(ordinals[1]))],
      
      ordinals[2]
  ]

  skip_senders = set()
  for side_of_tile, indices in source_special_indices_by_side_of_tile.items():
    base = HALF_N_OIB_S2 * OIB_S2_MUX_SIDES_INCREASING_INDEX.index(side_of_tile)
    for i in indices:
      skip_senders.add(base + i)
  skip_senders.update(OIB_S2_SKIP)

  for side_of_tile in SIDES_OF_TILE.keys():
    start, end = arch.get_oib_s2_mux_ordinal_bounds(side_of_tile)
    skip_senders.add(start)
    skip_senders.add(end)

  # Now chunk up the ordinals into alternating lengths of 5 and 6. Length 5
  # chunks go to flank OIB S2 groups, length 6 chunks go to the central group.
  def make_groups(start, end, spans):
    groups = []
    i = start
    g = 0
    span_limit = spans[g]
    while i < end:
      span_count = 0
      group = []
      while span_count < span_limit and i < end:
        if i in skip_senders:
          i += 1
          continue
        group.append(i)
        i += 1
        span_count += 1

      groups.append(group)
      g += 1
      span_limit = spans[g % len(spans)]
    return groups

  east_groups = make_groups(
      *arch.get_oib_s2_mux_ordinal_bounds(Direction.EAST),
      [6, 5])
  west_groups = make_groups(
      *arch.get_oib_s2_mux_ordinal_bounds(Direction.WEST),
      [5, 6])

  def allocate_group_to_receiver(senders, receiver_index):
    # Allocate two from the front and two from the back:
    receiver = arch.oib_s2_bounce_list[receiver_index]
    ports = ['A', 'A', 'B', 'B']
    for i, k in enumerate(
        [senders[0], senders[1], senders[-2], senders [-1]]):
      sender = arch.get_oib_s2_mux(None, k, ports[i])
      print('connect sender', sender.name, 'to receiver', receiver.name)
      connect_nodes(sender, receiver)

  def allocate_alternate_groups_to_receiver_groups(sender_groups, first, second):
    i = 0
    g = 0
    while i < len(sender_groups) and i < len(first) and i < len(second):
      allocate_group_to_receiver(sender_groups[g], first[i])
      allocate_group_to_receiver(sender_groups[g + 1], second[i])
      g += 2
      i += 1

  print('East groups:')
  for i, group in enumerate(east_groups):
    print(i, ', '.join(str(i) for i in group))
  print('West groups:')
  for i, group in enumerate(west_groups):
    print(i, ', '.join(str(i) for i in group))

  allocate_alternate_groups_to_receiver_groups(
      east_groups, receiver_groups[1], receiver_groups[3])
  allocate_alternate_groups_to_receiver_groups(
      west_groups, receiver_groups[0], receiver_groups[2])

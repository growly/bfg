import numpy as np
import constants
import math
import collections

from node import connect_nodes, disconnect_nodes

import io_tools

# IIB 1 to 2
# ----------
#

# FIXME(aryap): regular s1 muxes are numbers 0 - 31 on the ?EAST? side, and 32
# - 63 on the ?WEST? side. So far we have only defined one side.

# FIXME(aryap): Use the mux nodes throughtout.
  
def connect(arch):
  # Now create the cliques.
  # Allocate according to this table:
  #      % of muxes in friendship of size
  #       3     1     0
  # A   M[1,1]
  # B   M[2,1]  ...
  # C   M[3,1]
  # D   M[4,1]
  #
  # The number of muxes will be ROUNDED DOWN. Any leftovers should be put into
  # group 2 (1-based, i.e. B).
  friendship_membership_ratios = np.array([
    [   0,   0,  1],
    [ 3/4,   0, 3/12],
    [9/16,  6/16, 1/16],
    [24/32, 8/32,  0]
  ])

  muxes_per_group = np.diag(constants.IIB_S2_GROUP_SIZES)

  # The result is a 4x4 matrix where each row is the distribution of muxes across
  # each of the clique sizes:
  #
  #       3     1     0
  # A    M[1, 1]
  # B    M[2, 1] ...
  # C    M[3, 1]
  # D    M[4, 1]
  mux_distribution = np.floor(
    np.matmul(muxes_per_group, friendship_membership_ratios)).astype(int)
  print('Stage 2 mux distribution per clique size per group:')
  print(mux_distribution)
  print()

  # The next thing is to decide which muxes go in which cliques. We allocate in
  # increasing ordinal number. The size-3 groups are made of adjacent muxes in
  # the middle of the range. So if 9 out of 16 muxes belong in size-3 groups,
  # that's 9/3=3 groups in the centre of a range [0, 15]. If there are size-2
  # sets we put half before and half at the end. If there are also size-1 sets we
  # put them all at the end. If there are no size-2 sets, but there are size-1
  # sets, we split them the same way as we would've split size-2 sets.
  #
  # In some cases cliques contain multiple nodes that repeat the same
  # connection pattern. All nodes are stored in a tuple as big as this multiple
  # to indicate that.

  def create_clique(
      size, multiple, source_groups, destination_mux_deque, cliques):
    if len(destination_mux_deque) < size:
      print(f'warning: cannot make clique of size {size}, only '
          f'{len(destination_mux_deque)} muxes remain')
      #import pdb
      #pdb.set_trace()
      size = len(destination_mux_deque)
    destination_muxes = []
    for i in range(0, size):
      entry = []
      for _ in range(0, multiple):
        if not destination_mux_deque:
          continue
        entry.append(destination_mux_deque.popleft())
      destination_muxes.append(tuple(entry))
    if destination_muxes:
      cliques.append(destination_muxes)


  def create_many_cliques(
      size, multiple, count, source_groups, destination_mux_deque,
      cliques):
    i = 0
    while i < count:
      create_clique(
        size, multiple, source_groups, destination_mux_deque,
        cliques)
      i += 1

  # Allocate
  cliques_by_group = {}

  for g, group_distribution in enumerate(mux_distribution):
    multiple = 2 if constants.IIB_S2_GROUP_NAMES[g] == 'D' else 1

    num_size_3, num_size_2, num_size_1 = [
      group_distribution[0] / (3 * multiple),
      group_distribution[1] / (2 * multiple),
      group_distribution[2] / multiple]

    first_half_size_1 = int(math.ceil(num_size_1 / 2))
    remaining_size_1 = num_size_1 - first_half_size_1

    first_half_size_2 = int(math.ceil(num_size_2 / 2))
    remaining_size_2 = num_size_2 - first_half_size_2

    group_muxes = collections.deque(arch.iib_s2_groups[g])
    source_groups = []
    cliques = []

    if first_half_size_2 > 0:
      create_many_cliques(
        2, multiple, first_half_size_2, source_groups, group_muxes,
        cliques)
    elif first_half_size_1 > 0:
      create_many_cliques(
        1, multiple, first_half_size_1, source_groups, group_muxes,
        cliques)

    create_many_cliques(
        3, multiple, num_size_3, source_groups, group_muxes,
        cliques)

    create_many_cliques(
      2, multiple, remaining_size_2, source_groups, group_muxes,
      cliques)

    # If we put half of the size-1 sets at the start, only put the remainder at
    # the end. Otherwise, put them all.
    create_many_cliques(
      1,
      multiple,
      num_size_1 if first_half_size_2 > 0 else remaining_size_1,
      source_groups,
      group_muxes,
      cliques)

    cliques_by_group[g] = cliques
    print('Group', constants.IIB_S2_GROUP_NAMES[g], 'allocated', cliques)

  # Some input groups needs to be short. This facilitates overlapping groups in
  # the next stage. We could probably find another way to achieve the same thing.

  # This is a dict whose keys are successively:
  #   group name, clique size
  # and whose values are a set of which (by ordinal) of the cliques should have a
  # short input-group list (1 less than normal).
  short_cliques = {
    'C': {
      2: {1, 2}
    },
    'D': {
      2: {0, 1}
    }
  }
  def is_short_clique(group, clique_size, clique_index):
    if constants.IIB_S2_GROUP_NAMES[group] not in short_cliques:
      return False
    if clique_size not in short_cliques[constants.IIB_S2_GROUP_NAMES[group]]:
      return False
    indices = short_cliques[constants.IIB_S2_GROUP_NAMES[group]][clique_size]
    return clique_index in indices


  # Now we can assign adjacencies (connectivity).
  #
  # For muxes with L-many inputs we have input group size L/2, meaning two groups
  # are needed to feed each mux.
  #
  # A clique of size F has F-1 pairs that share inputs groups, so it has
  # 2 + (F - 1) = F + 1 input groups?
  #
  # F*L inputs, each group L/2, so with no sharing that's F*L/(L/2) = 2*F
  # but F - 1 inputs come from the same groups so really it's
  # F*L - (F - 1)*L/2 = (2FL - FL +L)/2
  #           = L(F + 1) / 2
  #   unique inputs, requiring
  # L(F+1)/2 / L/2 = F + 1 groups.
  # oh cool a proof!

  edges = []

  def connect_later(drivers, driven):
    for driver in drivers:
      if driver is None or driven is None:
        import pdb
        pdb.set_trace()
      edges.append((driver, driven))

  def allocate_groups_to_clique(input_groups, clique):
    if not input_groups or not clique:
      print('warning: cannot allocate empty grooups or clique '
          f'{input_groups=} {clique=}')
      return

    # There are two supported patterns. If the number of input groups is one
    # more than the size of the clique:
    #           member  member
    #   input_group     X
    #   input_group     X     X
    #   input_group         X
    #
    # If it's the same length:
    #           member  member
    #   input_group     X     X
    #   input_group         X
    #
    # Otherwise, no cigar.

    assert len(input_groups) == len(clique) + 1 or len(
        input_groups) == len(clique), f'{input_groups} vs {clique}'

    i = len(input_groups) - 1
    for entry in reversed(clique):
      # Every mux in the tuple gets connections from the same input muxes.
      for mux in entry:
        connect_later(input_groups[i], mux)
        if i > 0:
          connect_later(input_groups[i - 1], mux)
      i -= 1

  # TODO(aryap): Make this like, parameterised?
  input_group_offsets = {
    'A': -4,  # Expect 4x2 input groups to be used, want half on end half the other.
    'B': 1,
    'C': 1,
    'D': 1
  }


  input_groups_by_group_and_clique = collections.defaultdict(collections.defaultdict)
  for g, cliques in cliques_by_group.items():
    i = input_group_offsets[constants.IIB_S2_GROUP_NAMES[g]]

    clique_size_counts = collections.defaultdict(int)

    repeat = 0
    for c, clique in enumerate(cliques):
      set_size = len(clique)
    
      num_input_groups = set_size if is_short_clique(
          g, set_size, clique_size_counts[set_size]) else set_size + 1

      clique_size_counts[set_size] += 1

      input_groups = []
      j = 0
      while j < num_input_groups:
        k = i % len(arch.iib_s1_groups)
        input_groups.append(arch.iib_s1_groups[k])
        i += 1
        j += 1

      input_groups_by_group_and_clique[g][c] = input_groups

      # TODO(aryap): well, lots, but how to deal with the duplication of sets in
      # group D?

      # TODO(aryap): Need to split input groups to first group between muxes at edges!

      print(f'{g=} {c=} input groups:', input_groups)
      allocate_groups_to_clique(input_groups, clique)

  # 
  # Last we apply post perturbations to the connectivity established so far.

  # Shuffling between groups C, D.

  pruned_edges = []
  # C, D
  alphas = collections.defaultdict(dict)
  betas = collections.defaultdict(dict)
  for g in (2, 3):
    j = 0
    for clique in cliques_by_group[g]:
      print('clique:', clique)
      input_groups = input_groups_by_group_and_clique[g][j]
      select = input_groups[-2]
      alpha = select[0]
      # Get the last mux in the clique:
      if not clique[-1]:
        del clique[-1]
      beta = clique[-1][-1]

      alphas[g][j] = alpha
      #print(f'{g=} {j=} {alpha=} {beta=}')
      
      betas[g][j] = beta

      # Disconnect.
      print(f'{g=} {j=} remove driver: {alpha.name} drivee: {beta.name}')
      try:
        edges.remove((alpha, beta))
      except:
        print('oops')
      pruned_edges.append((alpha, beta))

      j += 1

  g = 3
  for j in range(len(betas[g])):
    print(g, j)
    if j+1 in alphas[g-1]:
      new_edge = (alphas[g-1][j+1], betas[g][j])
      if new_edge not in edges and new_edge not in pruned_edges:
        print(f'1. driver {new_edge[0].name} now drives {new_edge[1].name}')
        edges.append(new_edge)

    # If due to previous configuration beta_(g-1, j) was already driven by
    # alpha_(g, j), we drive beta_(g-1,j-1) instead.
    if j in betas[g-1]:
      new_edge = (alphas[g][j], betas[g-1][j])
      if new_edge in edges or new_edge in pruned_edges:
        if j - 1 in betas[g-1]:
          print(f'using beta_(g-1,j-1) instead')
          new_edge = (alphas[g][j], betas[g-1][j-1])
        else:
          continue
      print(f'2. driver {new_edge[0].name} now drives {new_edge[1].name}')
      edges.append(new_edge)


  # THE LAST STEP is to correct a couple of things:
  #   - over-fan-in and over-fan-out. We find:
  #     - input muxes that have too high a fan out
  #     - output muxes that have too high a fan in
  #   and redistribute among them to make it even.
  #   - mix in unallocated muxes from special, north, local and south feeder
  #   groups

  # Turn (out, in) index pairs denoting edges into connections:

  # Create a copy of all the nodes leading into to this block to check their
  # fan-out. Remove exceptional nodes (local feeders), and then redistribute
  # over the remainder. Add the feeders back at the end.
  #
  # Create a copy of all the nodes leading out of this block to check the
  # number of assigned inputs. If there are too many, disconnect them.
  nodes_in = {}
  nodes_out = {}
  for in_node, out_node in edges:
    nodes_in[in_node.name] = in_node
    nodes_out[out_node.name] = out_node
    connect_nodes(in_node, out_node)

  i = 0
  for group in arch.iib_s1_groups:
    for mux in group:
      mux.group = i
    i += 1

  exceptions_to_redistribution = {
      mux.name: mux for mux in arch.iib_s1_local_feeders}
  for mux in arch.iib_s1_local_feeders:
    del nodes_in[mux.name]

  for node in sorted(
    nodes_in.values(), key=lambda n: len(n.get_out_nodes()), reverse=True):
    if len(node.get_out_nodes()) <= constants.MAX_FAN_OUT:
      # If the max fan out is less than the max, we have nothing left to do.
      break

    available = sorted(nodes_in.values(), key=lambda n: len(n.get_out_nodes()))
    shiftee = available[0]
    if len(shiftee.get_out_nodes()) > constants.MAX_FAN_OUT:
      # If the min available fan out is too great, there's nothing we can do.
      break

    # Shift connection.
    dest = list(node.get_out_nodes())[0]

    print(f'Shifting connection ({node.name}, {dest.name}) '
        f'to ({shiftee.name}, {dest.name})')

    disconnect_nodes(node, dest)
    connect_nodes(shiftee, dest)

  # Disconnect muxes with too many inputs.
  for name, node in sorted(
    nodes_out.items(), key=lambda n: len(n[1].get_in_nodes()), reverse=True):
    num_over = len(node.get_in_nodes()) - constants.IIB_S2_MAX_NUM_INPUTS
    if num_over <= 0:
      break
    for i in range(num_over):
      source = list(node.get_in_nodes())[0]
      disconnect_nodes(source, node)

  # Put the exceptions back.
  nodes_in.update(exceptions_to_redistribution)

  # FIXME(aryap): TO ADDRESS:
  # - unallocated nodes seem to be closely related to the groups they would
  # otherwise belong in. Maybe they should be added as members of existing groups
  # or their own groups? Or maybe using them to catch slack makes the most sense?

  io_tools.write_adjacency_matrix('iib_1_to_2.csv', nodes_in, nodes_out)


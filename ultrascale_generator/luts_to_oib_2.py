import random
import functools

import constants
import io_tools

from node import NodeType, disconnect_nodes, NoFreeInputs

def print_lut_port_name_pairs(pairs):
  print(', '.join(f'{lut.name}/{port}' for lut, port in pairs))

def connect(arch):
  side_of_tile = constants.lut_side

  # There are 32 LUT outputs. 1/4 of them feed four independent mux inputs, the
  # other 3/4 feed two tied mux input pairs. (Fan out always 4.)
  #
  # We need to group the outputs into each bucket:
  N_UNIQUE_MUXES = constants.HALF_N_OIB_S2

  untied_muxes = [
      0,
      int(N_UNIQUE_MUXES / constants.LUT_FAN_OUT),
      N_UNIQUE_MUXES - 3,
      N_UNIQUE_MUXES - 2,
      N_UNIQUE_MUXES - 1]

  # These are the muxes whose 'A' and 'B' inputs we both connect to.
  oib_stage_2_paired_muxes0 = []
  oib_stage_2_paired_muxes1 = []
  # These are the muxes where we connect only to 'A' or 'B'.
  oib_stage_2_untied_muxes = []
  for i in range(N_UNIQUE_MUXES):
    if i in untied_muxes:
      # Choice of 'A' here is somewhat arbitrary. We might want to connect to the
      # 'B' side.
      node = arch.get_oib_s2_mux(side_of_tile, i, 'A')
      oib_stage_2_untied_muxes.append(node)
      continue

    a_node = arch.get_oib_s2_mux(side_of_tile, i, 'A')
    oib_stage_2_paired_muxes0.append(a_node)

    b_node = arch.get_oib_s2_mux(side_of_tile, i, 'B')
    oib_stage_2_paired_muxes1.append(b_node)

  N_PAIRED_MUXES = len(oib_stage_2_paired_muxes0)

  oib_s2_lut_receivers = oib_stage_2_paired_muxes0 + (
      oib_stage_2_paired_muxes1 + oib_stage_2_untied_muxes)

  lut_outputs = list((lut, port) for lut in arch.luts for port in lut.output_order)

  outputs_strafing_untied_inputs = random.sample(
      lut_outputs, k=int(len(lut_outputs) / 8))

  # Connections to the shared inputs on two muxes:
  outputs_strafing_tied_inputs = [
      p for p in lut_outputs if p not in outputs_strafing_untied_inputs]


  print('Allocating specials...')
  # There are a couple of special nodes.
  #
  # One of the untied-input-strafers actually strafes the span of the muxes.
  #
  # This same LUT will also be the only one with an output that drives TWO OIB S2
  # bounce muxes. The output that does this is the one not selected for the
  # special strafe.
  #
  # NOTE(aryap): It seems that both are registered bypass outputs in our example.
  # We do not model that here because it's not clear how that generalises.
  #
  # These are indices into the span of all OIB S2 muxes on this side of the tile.
  indices = constants.OIB_S2_SPECIAL_INDICES[1:]


  target_bounce_muxes = arch.get_oib_s2_nonlocal_bounce_muxes(
      None,  # Local.
      side_of_tile,
      None)  # All parts.
  unused_bounce_indices = list(range(len(target_bounce_muxes)))
  unbounced_luts = list(arch.luts)
  unbounced_ports = list(arch.luts[0].output_order)
  if outputs_strafing_untied_inputs:
    special_lut = random.sample(outputs_strafing_untied_inputs, k=1).pop()
    outputs_strafing_untied_inputs.remove(special_lut)
    lut, port_name = special_lut
    output = 'B'
    for index in indices:
      mux = arch.get_oib_s2_mux(side_of_tile, index, output)
      lut.connect_output(port_name, mux)
      print(f'{lut.name}/{port_name} -> {mux.name}')
      output = 'A' if output == 'B' else 'B'

    ports_on_special_lut = set(lut.registered_bypass_out_order)
    try:
      ports_on_special_lut.remove(port_name)
    except:
      # Might not have picked a bypass port.
      pass
    bounce_port = list(ports_on_special_lut)[0]

    middle_index = int(len(target_bounce_muxes)  / 2)
    last_index = unused_bounce_indices[-1]
    lut.connect_output(bounce_port, target_bounce_muxes[middle_index])
    lut.connect_output(bounce_port, target_bounce_muxes[last_index])

    unused_bounce_indices.remove(middle_index)
    unused_bounce_indices.remove(last_index)
    unbounced_luts.remove(lut)
    unbounced_ports.remove(bounce_port)

  # Pick a random unused LUT/port for each unused OIB S2 bounce mux.
  # FIXME(aryap): This should probably factor in distance from each output to
  # each mux too?
  for index in unused_bounce_indices:
    if not unbounced_ports:
      print(f'warning: no more unbounced ports')
      break
    target_mux = target_bounce_muxes[index]
    lut = random.sample(unbounced_luts, k=1).pop()
    port_name = random.sample(unbounced_ports, k=1).pop()

    lut.connect_output(port_name, target_bounce_muxes[index])

    unbounced_luts.remove(lut)
    unbounced_ports.remove(port_name)

  ## Another two ports connect to alternating muxes from the special indices list,
  ## but to both muxes this time:
  #special = random.sample(outputs_strafing_tied_inputs, k=1).pop()
  #outputs_strafing_tied_inputs.remove(special)
  #lut, port_name = special
  #i = 0
  #while i < len(indices):
  #  index = indices[i]
  #  mux0 = get_oib_s2_mux(lut_side, index, 'A')
  #  mux1 = get_oib_s2_mux(lut_side, index, 'B')
  #  lut.connect_output(port_name, mux0)
  #  lut.connect_output(port_name, mux1)
  #  i += 2
  #
  #special = random.sample(outputs_strafing_tied_inputs, k=1).pop()
  #outputs_strafing_tied_inputs.remove(special)
  #lut, port_name = special
  #i = 1
  #while i < len(indices):
  #  index = indices[i]
  #  mux0 = get_oib_s2_mux(lut_side, index, 'A')
  #  mux1 = get_oib_s2_mux(lut_side, index, 'B')
  #  lut.connect_output(port_name, mux0)
  #  lut.connect_output(port_name, mux1)
  #  i += 2


  print('Allocating...')
  print_lut_port_name_pairs(outputs_strafing_untied_inputs)

  for lut, port_name in outputs_strafing_untied_inputs:
    i = random.randint(0, N_PAIRED_MUXES)
    # The pattern here is to connect to i/B, i+1/A, i+2/B, i+3/A
    j = i
    side_0 = False
    while j - i < constants.LUT_FAN_OUT:
      index = j % N_PAIRED_MUXES
      mux = oib_stage_2_paired_muxes0[
          index] if side_0 else oib_stage_2_paired_muxes1[index]
      try:
        lut.connect_output(port_name, mux)
      except NoFreeInputs as err:
        # If 'available_receivers' is determined correctly this should not
        # happen.
        print(f'warning: could not connect {lut} to {mux}: {err=} mux '
              f'fan-in: {mux.fan_in()}')
      j += 1
      side_0 = not side_0

  print('Allocating...')
  print_lut_port_name_pairs(outputs_strafing_tied_inputs)

  # Approach 1:
  # Select a random mux and assign the next LUT output to it and its neibhours.
  ## as a slight modification, we do not sample with replacement!
  ## we also take out the other muxes in the pattern:
  #
  #def find_next_mux_with_available_inputs(
  #    mux_list, start_index):
  #  i = 0
  #  while i < len(mux_list):
  #    k = (i + start_index) % len(mux_list)
  #    if mux_list[k].fan_in() < FAN_IN:
  #      return mux_list[k]
  #    i += 1
  #  return None
  #
  #population = list(range(N_PAIRED_MUXES))
  #premptively_removed = []
  #for lut, port_name in outputs_strafing_tied_inputs:
  #  i = random.sample(population, k=1)[0]
  #  j = 0
  #  # The pattern here is to connect to i/A, i/B, i+2/A, i+2/B
  #  indices = []
  #  while j < int(LUT_FAN_OUT / 2):
  #    index = (i + 2*j) % N_PAIRED_MUXES
  #    mux0 = find_next_mux_with_available_inputs(
  #        oib_stage_2_paired_muxes0, index)
  #    mux1 = find_next_mux_with_available_inputs(
  #        oib_stage_2_paired_muxes1, index)
  #    print(f'{mux0.fan_in()=} {mux1.fan_in()=}')
  #    indices.append(index)
  #
  #    # It's possible that the second index is in the pre-emptively-remove
  #    # list, so we cancel it if so.
  #    try:
  #      premptively_removed.remove(index)
  #    except:
  #      pass
  #
  #    lut.connect_output(port_name, mux0)
  #    lut.connect_output(port_name, mux1)
  #    #print(f'Assigned {lut.name}/{port_name}')
  #
  #    j += 1
  #
  #  # To prevent a subsequent choice of i landing on the j=0 index, we also
  #  # remove that for this iteration.
  #  #
  #  # OK! But when we repopulate we have to prioritise these ones since there's
  #  # no guarantee they were picked!
  #  previous = (i - 2) % N_PAIRED_MUXES
  #  if previous in population:
  #    indices.append(previous)
  #    premptively_removed.append(previous)
  #
  #  for k in reversed(sorted(indices)):
  #    try:
  #      population.remove(k)
  #    except:
  #      pass
  #  #print(population)
  #
  #  if not population:
  #    if not premptively_removed:
  #      population = list(range(N_PAIRED_MUXES))
  #    else:
  #      population = list(premptively_removed)
  #      premptively_removed.clear()

  ## After every LUT is assigned to at least one mux, it's possible that some
  ## muxes still do not have LUT outputs as inputs.
  #population = [mux for mux in oib_s2_lut_receivers if len(mux.get_in_nodes()) == 0]
  #while population:
  #  # We could add a connection to the lowest-fan-out LUT:
  #  #lut_outputs_and_fan_out = list(
  #  #  (len(output.get_out_nodes()), output)
  #  #  for output in lut.output_nodes.values()
  #  #  for lut in luts)
  #  #lut_outputs_and_fan_out.sort(key=lambda x: x[0])
  #  #print(lut_outputs_and_fan_out)
  #
  #  # Or we could swap connection with the highest-fan-in mux:
  #  muxes_and_fan_in = sorted(
  #      [(mux.fan_in(), mux) for mux in oib_s2_lut_receivers],
  #      key=lambda x: x[0])
  #
  #  fan_in, donor_mux = muxes_and_fan_in.pop()
  #  while fan_in >= FAN_IN and muxes_and_fan_in:
  #    fan_in, donor_mux = muxes_and_fan_in.pop()
  #  if fan_in >= FAN_IN:
  #    print('warning: no muxes have spare inputs')
  #    break
  #
  #  if not donor_mux.get_in_nodes():
  #    # No nodes have fan-in?!
  #    break
  #  lut_port = donor_mux.get_in_nodes().pop()
  #  lut_port.remove_out_node(donor_mux)
  #
  #  mux = population[0]
  #  del population[0]
  #  print(f'swapping {lut_port.name} from {donor_mux.name} to {mux.name}')
  #
  #  mux.add_in_node(lut_port)
  #  lut_port.add_out_node(mux)

  # Approach 2 
  # Again with the LUT outputs in order, start with a random mux and find the next mux that can accommodate the pattern of connections.
  #
  #population = list(range(N_PAIRED_MUXES))
  #premptively_removed = []
  #for lut, port_name in outputs_strafing_tied_inputs:
  #  i = random.sample(population, k=1)[0]
  #  muxes = find_next_mux_pair_free(i)
  #  if muxes is None:
  #    print('warning: no A/B-free mux pairs found')
  #    break
  #  mux0, mux1, mux2, mux3 = muxes
  #  lut.connect_output(port_name, mux0)
  #  lut.connect_output(port_name, mux1)
  #  lut.connect_output(port_name, mux2)
  #  lut.connect_output(port_name, mux3)
  #
  #  for o in [mux0.ordinal, mux1.ordinal, mux2.ordinal, mux3.ordinal]:
  #    try:
  #      # Will try to remove the same ordinal for an 'A', 'B' pair. It's ok to
  #      # remove a whole ordinal if only one side of the mux is used, since we
  #      # enforce a MIN_GAP of 1 anyway.
  #      population.remove(o)
  #    except:
  #      pass
  #  print(f'Assigned {lut.name}/{port_name}')
  #

  # Finds the next pair of muxes that are separated by at least the given number
  # of muxes, with both 'A' and 'B' inputs free.
  MIN_GAP = 1
  MAX_GAP = 3

  def find_next_mux_pair_free(
      side_of_tile, min_gap, max_gap, start_index, b_a=False):

    def is_free_pair_A_B(start_index, gap):
      k = start_index
      mux0 = arch.get_oib_s2_mux(side_of_tile, k, 'A')
      #import pdb
      #pdb.set_trace()
      if not mux0.ok_for_lut_connection():
        return None
      mux1 = arch.get_oib_s2_mux(side_of_tile, k, 'B')
      if not mux1.ok_for_lut_connection():
        return None
      k = (k + gap + 1) % constants.HALF_N_OIB_S2
      mux2 = arch.get_oib_s2_mux(side_of_tile, k, 'A')
      if not mux2.ok_for_lut_connection():
        return None
      mux3 = arch.get_oib_s2_mux(side_of_tile, k, 'B')
      if not mux3.ok_for_lut_connection():
        return None
      return (mux0, mux1, mux2, mux3)

    def is_free_pair_B_A(start_index, gap):
      k = start_index
      mux0 = arch.get_oib_s2_mux(side_of_tile, k, 'B')
      if not mux0.ok_for_lut_connection():
        return None
      k = (k + 1) % constants.HALF_N_OIB_S2
      mux1 = arch.get_oib_s2_mux(side_of_tile, k, 'A')
      if not mux1.ok_for_lut_connection():
        return None
      k = (k + gap) % constants.HALF_N_OIB_S2
      mux2 = arch.get_oib_s2_mux(side_of_tile, k, 'B')
      if not mux2.ok_for_lut_connection():
        return None
      k = (k + gap + 1) % constants.HALF_N_OIB_S2
      mux3 = arch.get_oib_s2_mux(side_of_tile, k, 'A')
      if not mux3.ok_for_lut_connection():
        return None
      return (mux0, mux1, mux2, mux3)

    i = 0
    while i < constants.HALF_N_OIB_S2:
      k = (start_index + i) % constants.HALF_N_OIB_S2
      for g in range(min_gap, max_gap + 1):
        if not b_a:
          muxes = is_free_pair_A_B(k, g)
        else:
          muxes = is_free_pair_B_A(k, g)
        if muxes is not None:
          return muxes
      i += 1
    return None

  # Approach 3
  # Now select LUT outputs in random order, and find the next mux that can
  # accomodate the pattern of connections for each.
  #
  # Still only looks for A/B, A/B inputs, since we allocate untied inputs (B/A,
  # B/A) above. As a fall back we check for available B/A, B/A pairs.
  #
  # This can successfully be used to allocate more LUTs if the separate 1/4 of
  # the LUT outputs are not explicitly allocated to B/A patterns, and instead are
  # only allocated as a result of the A/B failure. (To do this, simply set
  # 'outputs_strafing_untied_inputs' to the empty list [] above.)
  population = list(outputs_strafing_tied_inputs)
  random.shuffle(population)
  for lut, port_name in population:
    print(f'Assigning {lut.name}/{port_name}')
    muxes = find_next_mux_pair_free(side_of_tile, MIN_GAP, MAX_GAP, 0)
    if muxes is None:
      print('warning: no A/B-free mux pairs found, trying bigger gap')
      muxes = find_next_mux_pair_free(side_of_tile, MIN_GAP, MAX_GAP + 2, 0)
    if muxes is None:
      print('warning: no A/B-free mux pairs found, trying B/A pattern')
      muxes = find_next_mux_pair_free(side_of_tile, MIN_GAP, MAX_GAP + 2, 0, b_a=True)
    if muxes is None:
      print('warning: no B/A-free mux pairs found')
      break
    mux0, mux1, mux2, mux3 = muxes
    lut.connect_output(port_name, mux0)
    lut.connect_output(port_name, mux1)
    lut.connect_output(port_name, mux2)
    lut.connect_output(port_name, mux3)

  # At this point there are going to be LUTs without output connections, and
  # there are going to be muxes without fully-connected inputs.
  # We try to remedy this with some bounds: LUTs should not connect to muxes too
  # far apart. Or maybe they should be able to and we just run the algorithm a
  # few times to find the best solution.
  leftovers = [
      (lut, port_name)
      for lut, port_name in lut_outputs
      if lut.output_nodes[port_name].fan_out() < constants.LUT_FAN_OUT]

  print('need to deal with (', len(leftovers), '):')
  print_lut_port_name_pairs(leftovers)

  # Since connecting to one side of a mux partition affects the other side, we
  # have to re-evaluate the available muxes on every loop, track the other
  # sides of the shared partitions we're connecting to, or just make sure we
  # handle failures to connect correctly (rolling back if necessary) and move
  # on.
  def available_inputs(mux):
    return mux.fan_in() < constants.OIB_S2_MAX_NUM_INPUTS
  available_receivers = list(filter(available_inputs, oib_s2_lut_receivers))


  while leftovers and available_receivers:
    lut, port_name = leftovers.pop()
    mux = available_receivers.pop()
    try:
      lut.connect_output(port_name, mux)
    except NoFreeInputs as err:
      # If 'available_receivers' is determined correctly this should not
      # happen.
      print(f'warning: could not connect {lut} to {mux}: {err=} mux fan-in: {mux.fan_in()}')
      continue
    if mux.fan_in() < constants.OIB_S2_MAX_NUM_INPUTS:
      available_receivers.insert(0, mux)
    if lut.output_nodes[port_name].fan_out() < constants.LUT_FAN_OUT:
      leftovers.insert(0, (lut, port_name))

  # The last step is to enforce max number of inputs from LUT outputs.
  for mux in oib_s2_lut_receivers:
    lut_drivers = mux.inputs_of_type(NodeType.LUT)
    while len(lut_drivers) > constants.OIB_S2_MAX_NUM_LUT_INPUTS:
      lut = lut_drivers.pop()
      print(f'warning: too many LUT inputs to MUX; '
            f'disconnecting {lut.name} from {mux.name}')
      disconnect_nodes(lut, mux)
      lut_drivers = mux.inputs_of_type(NodeType.LUT)

  print()
  #list_by_fanin(oib_s2_regular_mux_list)

  # FIXME(aryap): there should be no empties. All muxes must be driven by at
  # least one LUT. We observed that. Count fan-ins by first and second half of
  # mux list
  half_range = int(len(oib_s2_lut_receivers) / 2)
  num_0 = 0
  count = 0
  for mux in oib_s2_lut_receivers[:half_range]:
    print(mux.name, ":", ', '.join(n.name for n in mux.get_in_nodes()))
    fan_in = len(mux.get_in_nodes())
    if fan_in == 0:
      num_0 += 1
    count += fan_in
  print(count)
  count = 0
  for mux in oib_s2_lut_receivers[half_range + 1:]:
    print(mux.name, ":", ', '.join(n.name for n in mux.get_in_nodes()))
    fan_in = len(mux.get_in_nodes())
    if fan_in == 0:
      num_0 += 1
    count += fan_in
  print(count)
  print('# 0: ', num_0)

  lut_ports_as_nodes = {}
  for lut in arch.luts:
    lut_ports_as_nodes.update(lut.outputs_by_node_name())

  io_tools.write_adjacency_matrix(
      'oib_luts_to_oib_s2.csv',
      lut_ports_as_nodes,
      {n.name: n for n in oib_s2_lut_receivers})

  # The final stage is to do some reallocation:
  #list_by_fanin(oib_s2_regular_mux_list)

  all_oib_s2_drivers = {
      k: v for k, v in
      list(lut_ports_as_nodes.items()) + list(arch.interconnect_by_name.items())}
  io_tools.write_adjacency_matrix(
      'oib_s2_fan_in.csv',
      all_oib_s2_drivers,
      arch.oib_s2_by_name,
      sort_by_type=True)


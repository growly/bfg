import constants
import io_tools

# IIB 2 to LUTs
# ----------

# LUTs are connected:
#   bounce muxes (not in IIB L2) -> bypass input X
#   bypass muxes -> bypass input I, X
#   IIB_2 group C -> 1/3 of LUT inputs
#   IIB_2 group D -> 2/3 of LUT inputs


def connect(arch):
  print('=============')
  print('IIB 2 -> LUTs')
  print('=============')

  #lut_inputs = dict(output_nodes)

  # Group A has the bounce muxes.
  bounce_muxes = arch.iib_s2_bounce
  N_BOUNCE = len(bounce_muxes)

  # Group B has all the bypass muxes.
  bypass_muxes = arch.iib_s2_bypass
  N_BYPASS = len(bypass_muxes)

  half_bounce = int(N_BOUNCE / 2)
  remaining_bounce = N_BOUNCE - half_bounce

  for i in range(half_bounce):
    arch.get_lut(2 * i).connect_first_bypass(bounce_muxes[i])
  for i in range(remaining_bounce):
    arch.get_lut(-1 - 2 * i).connect_first_bypass(bounce_muxes[-1 - i])

  num_luts = arch.num_luts()

  assert N_BYPASS + N_BOUNCE >= num_luts * constants.BYPASS_INPUTS

  # This is how many bypass muxes over the one-per-LUT allocation need to be allocated:
  bypass_overflow = max(N_BYPASS - num_luts, 0)
  half_bypass_shoulder = int(bypass_overflow / 2)
  remaining_bypass_shoulder = bypass_overflow - half_bypass_shoulder

  offset = 0
  for i in range(half_bypass_shoulder):
    arch.get_lut(1 + 2*i).connect_first_bypass(bypass_muxes[i + offset])
  for i in range(min(num_luts, N_BYPASS)):
    while i + offset < constants.NUM_LUTS:
      mux = bypass_muxes[half_bypass_shoulder + i]
      try:
        arch.get_lut(i + offset).connect_first_bypass(mux)
        break
      except:
        # Failing to connect to a free bypass means we should find another LUT
        # with a free bypass!
        print(f'attempting to reconnect {mux} to next lut')
        offset += 1
  for i in range(remaining_bypass_shoulder):
    arch.get_lut(-2 - 2*i).connect_first_bypass(bypass_muxes[-1 -i + offset])

  group_C = list(arch.iib_s2_groups[2])
  group_D = list(arch.iib_s2_groups[3])

  # Group C is half as big as group D, so it feeds half as many LUT inputs. That
  # means 1/3 of the LUT inputs connect here.
  num_group_C = int(constants.K * len(group_C)/(len(group_D) + len(group_C)))
  num_group_D = constants.K - num_group_C

  # For ultrascale this is 2, 4, resp.
  print(f'{num_group_C=} {num_group_D=}')

  TUPLE_SIZE = 2
  tuples = []
  i = 0
  while i < arch.num_luts():
    j = 0
    group = []
    while j < TUPLE_SIZE:
      group.append(arch.get_lut(j + i))
      j += 1
    tuples.append(tuple(group))
    i += 2

  # num_group_C is the number of times each tuple must be allocated to the slots
  # [0, len(group_C))
  # pick a starting point.
  # try to allocate pair i = 0 there. then try to allocate pair i + 1 after it.
  # then repeat this going backwards, applying pair i before the starting point,
  # and pair i + 1 before that. repeat until there are num_group_C repetitions,
  # alternating direction each time.
  allocation = [None] * len(group_C)

  def get_available(group):
    available_muxes = list(filter(
        lambda x: x.fan_out() < constants.IIB_S2_MAX_FAN_OUT, group))
    available_luts = list(filter(
        lambda x: x.num_free_ports() > 0, arch.luts))
    pairs = zip(available_muxes, available_luts)
    return pairs

  def print_allocations():
    print(list(reversed(list(n.name if n is not None else '-' for n in allocation))))

  initial = int(len(allocation) / 2)

  # THIS IS A VERY COMPLICATED WAY OF DOING IT. But I can explain how it
  # generalises, unlike in a more straightforward approach. Though I'm not sure,
  # which approach preserves properties we think matter?
  #
  # What's happening here's is that we're taking pairs of pairs, allocating them
  # in sequence going forward, then going backward, skipping over allocated spots
  # each time. The only tricky bit is that, after we allocate a pair of pairs
  # _twice_, we move our starting cursor before the lowest spot reached so far.
  # If going in the other direction, this might be after the biggest spot
  # reached.
  #
  # This has the property of grouping two identical tuples together, flanking
  # them on either side with another pair of tuples, but the fact that one tuple
  # is allocated to first and last slots, and another to the in-between slots,
  # seems like an accident. But actually, I think this groups and interleaves
  # successive pairs nicely if it continues.
  reverse = True
  j = 0
  NUM_STEPS = 2
  while sum(map(lambda x: 1 if x is None else 0, allocation)) > 0:
    p = 0
    min_k = initial
    max_k = initial
    while p < num_group_C:
      step = 0
      k = initial
      attempts = 0
      while step < NUM_STEPS and attempts < int(len(allocation) / TUPLE_SIZE):
        if j + step >= len(tuples):
          break
        group = tuples[j + step]
        if allocation[k] == None:
          for q in range(TUPLE_SIZE):
            if k + q >= len(allocation):
              break
            allocation[k + q] = group[q]
          step += 1
          print('allocated', tuple(n.name for n in group), 'at', k)
        else:
          attempts += 1
          print(k, ' already taken')

        k_step = TUPLE_SIZE

        if reverse:
          min_k = min(min_k, k - k_step) % len(allocation)
          k = (k - k_step) % len(allocation)
        else:
          max_k = max(min_k, k + k_step) % len(allocation)
          k = (k + k_step) % len(allocation)

      reverse = not reverse
      p += 1

    j += NUM_STEPS
    if j < len(tuples):
      if reverse and min_k is not None:
        initial = min_k
      elif max_k is not None and j - NUM_STEPS >= 0:
        initial = max_k
      reverse = not reverse
    else:
      break

  # TODO(aryap): It's possible due to the geometry of our input groups that a
  # clean, complete allocation is not made with this algorithm. So we should
  # patch up any gaps in allocation.

  print('\nAllocating group C')
  print_allocations()

  # Connect according to allocation:
  for i, lut in enumerate(allocation):
    lut = allocation[i]
    mux = group_C[i]
    if not lut or not mux:
        print(f'warning: skipping allocation {lut} -> {mux}')
        continue
    lut.connect_first_input(mux)

  pairs = list(get_available(group_C))
  while pairs:
    for mux, lut in pairs:
      print(f'cleanup group C: allocating {mux} to {lut}')
      lut.connect_first_input(mux)
    pairs = list(get_available(group_C))


  print('\nAllocating group D')
  allocation = [None] * len(group_D)

  # This is very similar to the group C case, with one extra parameter
  # being that on step #2 the allocations are doubled.
  #
  # Allocate 4 tuples to 16 spots. Each tuple is allocated 4 times.
  def allocate_large_lut_group(allocation_start,
                               num_allocations,
                               tuples_start,
                               num_tuples,
                               allocations_per_tuple,
                               rotation):

    def rotate_partial_range(initial,
                 range_start,
                 range_length):
      rotated = (initial - range_start) % range_length + range_start
      return rotated

    i = allocation_start
    # First allocate 1 to the start of the range, going backwards:
    j = tuples_start + num_tuples - 1
    while j >= tuples_start:
      k = 0
      while k < TUPLE_SIZE:
        allocation[i + k] = tuples[j][k]
        k += 1
      j -= 1
      i += k

    print_allocations()

    num_allocations -= (i - allocation_start)
    count = allocations_per_tuple - 1

    # Second, allocate the remaining tuples, count times each. Rotate the whole
    # thing if requested:
    boundary_start = i
    j = tuples_start
    while i - allocation_start < num_allocations:
      for p in range(count):
        for k in range(TUPLE_SIZE):
          index = i + (k + TUPLE_SIZE * p)
          rotated = rotate_partial_range(
            index + rotation * TUPLE_SIZE, 
            boundary_start,
            num_allocations)
          allocation[rotated] = tuples[j][k]
      # Next tuple.
      j += 1
      # Next available slot.
      i += count * TUPLE_SIZE
      print_allocations()

  # Half the range.
  allocation_step_size = int(len(group_D) / 2)
  # 2.
  num_steps = int(len(group_D) / allocation_step_size)
  tuple_step_size = int(len(tuples) / num_steps)
  allocations_per_tuple = int(len(group_D) / (len(tuples) * TUPLE_SIZE))

  for step in range(num_steps):
    allocate_large_lut_group(
        allocation_step_size * step,
        allocation_step_size,
        tuple_step_size * step,
        tuple_step_size,
        allocations_per_tuple,
        step)

  print_allocations()

  # Connect according to allocation:
  for i, lut in enumerate(allocation):
    lut = allocation[i]
    if lut is None:
      print(f'warning: lut is None in allocation at {i}')
      continue
    mux = group_D[i]
    lut.connect_first_input(mux)

  # Allocate IIB S2 muxes with available fan out to "nearby" (ordinally) LUTs.
  pairs = list(get_available(group_D))
  while pairs:
    for mux, lut in pairs:
      print(f'cleanup group D: allocating {mux} to {lut}')
      lut.connect_first_input(mux)
    pairs = list(get_available(group_D))

  io_tools.write_adjacency_matrix(
    'iib_2_to_luts.csv',
    arch.iib_s2_by_name,
    arch.get_lut_ports_by_name())

  print()
  arch.print_lut_ports()


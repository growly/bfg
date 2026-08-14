from constants import *
import io_tools
from node import connect_nodes, disconnect_nodes

# Long-wire drivers:
# ------------------

# This is tedious. We will define exception and mapping functions.
# i is the index of the source wire. j is the index of the sink wire.
#
# In principal we could make this depend on the source side too, but they
# appear to always be the same between the two.
mapping_functions = {
  (Direction.EAST, MAX_SIZE): {
    (Direction.EAST, MAX_SIZE): {
      'is_exception': lambda j: j in (N_INTERCONNECT_PAIRS - 1,),
      'is_connected': lambda i, j: (
        j != 0 and int((i + 1) / 2) == int ((j + 1) / 2)) or (
        i == 0 and j == 0)
    },
    (Direction.NORTH, MAX_SIZE): {
      'is_exception': lambda j: (j + 1) % 2 == 0,
      'is_connected': lambda i, j: (
        j != 0 and int((i + 1)/ 2) == j / 2) or (i == 0 and j == 0)
    },
    (Direction.SOUTH, MAX_SIZE): {
      'is_exception': lambda j: j == N_INTERCONNECT_PAIRS - 1 or j % 2 == 0,
      'is_connected': lambda i, j: int((i + 1)/ 2) == int((j + 1) / 2)
    }
  },
  (Direction.EAST, NEXT_SIZE): {
    (Direction.EAST, MAX_SIZE): {
      'is_exception': lambda j: False,
      'is_connected': lambda i, j: i == j,
    },
    (Direction.NORTH, MAX_SIZE): {
      'is_exception': lambda j: (j + 1) % 2 == 0,
      'is_connected': lambda i, j: i == j,
    },
    (Direction.SOUTH, MAX_SIZE): {
      'is_exception': lambda j: j % 2 == 0,
      'is_connected': lambda i, j: i == j,
    }
  },
  (Direction.NORTH, MAX_SIZE): {
    (Direction.EAST, MAX_SIZE): {
      'is_exception': lambda j: (j + 1) % 2 == 0,
      'is_connected': lambda i, j: 2 * int(i / 2) == j,
    },
    (Direction.NORTH, MAX_SIZE): {
      'is_exception': lambda j: False,
      'is_connected': lambda i, j: int(i / 2) == int(j / 2),
    },
    (Direction.WEST, MAX_SIZE): {
      'is_exception': lambda j: (j + 1) % 2 == 0,
      'is_connected': lambda i, j: 2 * int(i / 2) == j,
    }
  },
  (Direction.NORTH, NEXT_SIZE): {
    (Direction.EAST, MAX_SIZE): {
      'is_exception': lambda j: (j + 1) % 2 == 0,
      'is_connected': lambda i, j: (i + 1) % N_INTERCONNECT_PAIRS == j
    },
    (Direction.NORTH, MAX_SIZE): {
      'is_exception': lambda j: False,
      'is_connected': lambda i, j: (i + 1) % N_INTERCONNECT_PAIRS == j
    },
    (Direction.WEST, MAX_SIZE): {
      'is_exception': lambda j: (j + 1) % 2 == 0,
      'is_connected': lambda i, j: i == j
    }
  },
  (Direction.SOUTH, MAX_SIZE): {
    (Direction.EAST, MAX_SIZE): {
      'is_exception': lambda j: j % 2 == 0,
      'is_connected': lambda i, j: 2 * int(i / 2) == j - 1,
    },
    (Direction.SOUTH, MAX_SIZE): {
      'is_exception': lambda j: False,
      'is_connected': lambda i, j: int(i / 2) == int(j / 2),
    },
    (Direction.WEST, MAX_SIZE): {
      'is_exception': lambda j: j % 2 == 0,
      'is_connected': lambda i, j: 2 * int(i / 2) == j - 1,
    }
  },
  (Direction.SOUTH, NEXT_SIZE): {
    (Direction.EAST, MAX_SIZE): {
      'is_exception': lambda j: j % 2 == 0,
      'is_connected': lambda i, j: (i - 1) % N_INTERCONNECT_PAIRS == j
    },
    (Direction.SOUTH, MAX_SIZE): {
      'is_exception': lambda j: False,
      'is_connected': lambda i, j: (i + 1) % N_INTERCONNECT_PAIRS == j
    },
    (Direction.WEST, MAX_SIZE): {
      'is_exception': lambda j: j % 2 == 0,
      'is_connected': lambda i, j: i == j
    }
  },
  (Direction.WEST, MAX_SIZE): {
    (Direction.NORTH, MAX_SIZE): {
      'is_exception': lambda j: j % 2 == 0,
      'is_connected': lambda i, j:  2 * int(i / 2) == (j - 1) % N_INTERCONNECT_PAIRS,
    },
    (Direction.SOUTH, MAX_SIZE): {
      'is_exception': lambda j: (j + 1) % 2 == 0,
      'is_connected': lambda i, j: 2 * int(i / 2) == j,
    },
    (Direction.WEST, MAX_SIZE): {
      'is_exception': lambda j: False,
      'is_connected': lambda i, j: int(i / 2) == int(j / 2)
    }
  },
  (Direction.WEST, NEXT_SIZE): {
    (Direction.NORTH, MAX_SIZE): {
      'is_exception': lambda j: j % 2 == 0,
      'is_connected': lambda i, j: (i + 1) % N_INTERCONNECT_PAIRS == j
    },
    (Direction.SOUTH, MAX_SIZE): {
      'is_exception': lambda j: (j + 1) % 2 == 0,
      'is_connected': lambda i, j: (i - 1) % N_INTERCONNECT_PAIRS == j
    },
    (Direction.WEST, MAX_SIZE): {
      'is_exception': lambda j: False,
      'is_connected': lambda i, j: i == j,
    }
  },
}

def connect(arch):
  for in_wire_key, in_wires in arch.interconnect_wires.items():
    io_direction, in_direction, in_size, in_site = in_wire_key
    if io_direction != 'IN':
      continue

    in_function_key = (in_direction, in_size)
    if in_function_key not in mapping_functions:
      continue
    functions_by_in_wire = mapping_functions[in_function_key]

    for i, in_wire in enumerate(in_wires):
      for out_wire_key, out_wires in arch.interconnect_wires.items():
        io_direction, out_direction, out_size, out_site = out_wire_key
        if io_direction != 'OUT':
          continue

        out_function_key = (out_direction, out_size)
        if out_function_key not in functions_by_in_wire:
          continue

        exception_fn = functions_by_in_wire[out_function_key]['is_exception']
        connected_fn = functions_by_in_wire[out_function_key]['is_connected']

        for j, out_wire in enumerate(out_wires):
          if exception_fn(j):
            continue
          if connected_fn(i, j):
            connect_nodes(in_wire, out_wire)

  # We denote the special wire moving diagonally south-east as i = 0, and the one
  # moving diagonally north-east as i = 1.
  ee12_east = WireKey('OUT', Direction.EAST, INTERCONNECT_WIRE_SIZES[-1], None)
  ee12_south = WireKey(
      'OUT', Direction.SOUTH, INTERCONNECT_WIRE_SIZES[-1], None)
  print(ee12_east)
  print(ee12_south)
  print(arch.interconnect_wires)
  connect_nodes(
      arch.interconnect_specials[0], arch.interconnect_wires[ee12_east][-1])
  connect_nodes(
      arch.interconnect_specials[0], arch.interconnect_wires[ee12_south][-1])
  connect_nodes(
      arch.interconnect_specials[1], arch.interconnect_wires[ee12_east][-1])
  connect_nodes(
      arch.interconnect_specials[1], arch.interconnect_wires[ee12_south][-1])

  interconnect_in_wires = {}
  interconnect_out_wires = {}

  for wire_key, wire_list in arch.interconnect_wires.items():
    io_direction, _, size, _ = wire_key
    wires_by_name = {wire.name: wire for wire in wire_list}
    if io_direction == 'IN':
      interconnect_in_wires.update(wires_by_name)
    elif io_direction == 'OUT':
      interconnect_out_wires.update(wires_by_name)

  io_tools.write_adjacency_matrix('interconnect_to_interconnect.csv',
                                  interconnect_in_wires,
                                  interconnect_out_wires)

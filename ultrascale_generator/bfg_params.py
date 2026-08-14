import collections
from common import Direction

# MUX types
# ---------

# IIB S1 regular: 6:1 shared (7:2)
# IIB S1 feeders: ?
# IIB S2 regular: 6:1
# IIB S2 bypass: 6:1 incl. VCC? 7:1?
# IIB S2 bounce:
# OIB S2 regular: 4:1 shared (5:2)
# OIB S1 regular: 6:1
# OIB S1? bounce?

# LUT section preamble
# --------------------
# LUT size
K = 4
BYPASS_INPUTS = 1

# Num luts
NUM_LUTS = 8

LUT_FAN_OUT = 4


N_INTERCONNECT_PAIRS = 4
DIRECTIONS = {
  Direction.NORTH: 'NN',
  Direction.EAST: 'EE',
  Direction.SOUTH: 'SS',
  Direction.WEST: 'WW'
}
SIDES_OF_TILE = {
  Direction.EAST: 'E',
  Direction.WEST: 'W'
}
IO_DIRECTIONS = ['IN', 'OUT']

# The patterns below strafe sizes in this order.
INTERCONNECT_WIRE_SIZES = [1, 2, 6]
INTERCONNECT_IN_MAX_FAN_OUT = 8

# 12 in UltraScale
# 6 for BFG
MAX_SIZE = max(INTERCONNECT_WIRE_SIZES)
# 4 in UltraScale
# 2 for BFG
NEXT_SIZE = max(set(INTERCONNECT_WIRE_SIZES) - {MAX_SIZE})

# IIB S1 preamble
# ---------------

# FIXME(aryap): Parts of this algorithm only consider one side of the
# interconnect tile. For those, we explicitly set this constant:
lut_side = Direction.EAST

N_FEEDERS_NORTH = 4
N_FEEDERS_SOUTH = 4
N_FEEDERS_LOCAL = 3


# Friendship sets are at most size 3.
CLIQUE_SIZE = 3   # Elsewhere this is "F".
MAX_FAN_OUT = 6

# Number of regular PHYSICAL S1 muxes across both sides of the tile.
N_IIB_S1_REGULAR = 32

# Number of regular S1 muxes on each side of the tile.
HALF_N_IIB_S1_REGULAR = int(N_IIB_S1_REGULAR / 2)

IIB_S1_MUX_SIDES_INCREASING_INDEX = (Direction.EAST, Direction.WEST)
IIB_S1_MAX_NUM_INPUTS = 6
IIB_S1_NUM_SHARED_INPUTS = 5
IIB_S1_MAX_FAN_OUT = MAX_FAN_OUT

# FIXME(aryap): These are the EAST-side indices. The other side's is these +
# HALF_N_IIB_S1_REGULAR.

# Where the hell does this come from?
# is it ceil(32 / 3) ~ 11, breaking it into thirds?
# TODO(aryap): Is it a coincidence that 11, 22, etc turn up in the output side too?
IIB_S1_SPECIAL_INDICES = [0, 11, 22]

# IIB S2 preamble
# ---------------

# Inputs are 6-muxes, so L = 6
IIB_S2_MAX_NUM_INPUTS  = 6  # Elsewhere this in "L".

IIB_S2_MAX_FAN_OUT = 1
IIB_S2_BOUNCE_FAN_OUT = 6

N_IIB_S2 = 80
HALF_N_IIB_S2 = int(N_IIB_S2 / 2)

# Split stage 2 muxes into receiving groups
# For bfg, we expect (per side) 40 IIB_S2 muxes split into groups:
#   A: 2
#   B: 6 <- A + B = num luts
#   C: 8
#   D: 24
IIB_S2_GROUP_DIVISOR = 20
IIB_S2_GROUP_SIZES = (
  int(HALF_N_IIB_S2 / IIB_S2_GROUP_DIVISOR),    # group A
  int(3 * HALF_N_IIB_S2 / IIB_S2_GROUP_DIVISOR),  # group B
  int (4 * HALF_N_IIB_S2 / IIB_S2_GROUP_DIVISOR),   # group C
  int(12 * HALF_N_IIB_S2 / IIB_S2_GROUP_DIVISOR))  # group D

# 'A', 'B', 'C', 'D', ...
IIB_S2_GROUP_NAMES = [chr(ord('A') + i) for i in range(len(IIB_S2_GROUP_SIZES))]

# IIB S2 to LUTs
# --------------

# LUT outputs to OIB layer 2
# --------------------------
#

# TODO(aryap): Careful! This is a whole number of these muxes PER side of tile,
# whereas elsewhere we consider the total number for both sides...
# Group A has the bounce muxes.
N_IIB_S2_BOUNCE = IIB_S2_GROUP_SIZES[0]
HALF_N_IIB_S2_BOUNCE = int(N_IIB_S2_BOUNCE / 2)
# Group B has all the bypass muxes.
N_IIB_S2_BYPASS = IIB_S2_GROUP_SIZES[1]
HALF_N_IIB_S2_BYPASS = int(N_IIB_S2_BYPASS / 2)

assert N_IIB_S2_BYPASS >= N_IIB_S2_BOUNCE

# OIB S2 preamble
# ---------------
#
# Again we start by using integer indices in place of Nodes for everything:
# This is a count of PHYSICAL muxes.
HALF_N_OIB_S2 = 24
N_OIB_S2 = 2 * HALF_N_OIB_S2
OIB_S2_MUX_SIDES_INCREASING_INDEX = (Direction.EAST, Direction.WEST)

# Number of non-local bounce muxes per side of tile:
HALF_N_OIB_S2_NONLOCAL_BOUNCE = 5
OIB_S2_NONLOCAL_SPLIT = int(HALF_N_OIB_S2_NONLOCAL_BOUNCE / 2)

# Our data had 15. We tried to reason that there were maybe 16. But actually
# let's stick with what the data shows, at 15.
N_OIB_S2_BOUNCE = 7
OIB_S2_BOUNCE_FAN_OUT = 8

OIB_S2_NONLOCAL_NO_DRIVE_INDICES = [0, 3, 4]
OIB_S2_FAN_OUT = 4

BOUNDS_BY_SIDE_OF_TILE = {
  OIB_S2_MUX_SIDES_INCREASING_INDEX[0]: (0, HALF_N_OIB_S2),
  OIB_S2_MUX_SIDES_INCREASING_INDEX[1]: (HALF_N_OIB_S2, 2 * HALF_N_OIB_S2)
}
SIDE_OF_TILE_BY_BOUNDS = [
  (bounds, side) for side, bounds in BOUNDS_BY_SIDE_OF_TILE.items()
]

OIB_S2_MAX_NUM_LUT_INPUTS = 2
OIB_S2_MAX_NUM_INPUTS = 4
OIB_S2_NUM_SHARED_INPUTS = 3
OIB_S2_MAX_FAN_OUT = MAX_FAN_OUT

# For HALF_N_OIB_S2 = 48 we expect 0, 11, 22, 33, 44. That's contrivedly:
#
# k*(floor(HALF_N_OIB_S2 / LUT_FAN_OUT) - 1) for k in 0, 1, 2, 4
OIB_S2_SPECIAL_INDICES = [
    k * int(HALF_N_OIB_S2 / LUT_FAN_OUT - 1) for k in range(LUT_FAN_OUT + 1)]

# These muxes do other mysterious things.
OIB_S2_NO_DRIVE_PER_SIDE = [
  (OIB_S2_SPECIAL_INDICES[0], 'A'),
  (OIB_S2_SPECIAL_INDICES[1], 'A'),
  (HALF_N_OIB_S2 - 3, 'B'),
  (HALF_N_OIB_S2 - 2, 'B'),
  (HALF_N_OIB_S2 - 1, 'B')
]

# And they are repeated on both sides:
OIB_S2_NO_DRIVE = {
  Direction.EAST: list(OIB_S2_NO_DRIVE_PER_SIDE),
  Direction.WEST: list(OIB_S2_NO_DRIVE_PER_SIDE),
}

OIBS2SpecialConfig = collections.namedtuple('OIBS2SpecialConfig', [
  'interconnect_index',
  'target_mux_index',
  'mux_outputs',
  'interconnect_source_side',
  'oib_s2_side'])
INTERCONNECT_TO_OIB_S2_SPECIALS = {
  Direction.NORTH: {
    INTERCONNECT_WIRE_SIZES[-1]: [
      OIBS2SpecialConfig(1, 22, ('B',), None, (Direction.EAST,)),
      OIBS2SpecialConfig(1, 33, ('A',), None, (Direction.EAST,)),
      OIBS2SpecialConfig(0, 22, ('B',), None, (Direction.WEST,)),
      OIBS2SpecialConfig(0, 33, ('A',), None, (Direction.WEST,)),
    ]
  },
  Direction.EAST: {
    INTERCONNECT_WIRE_SIZES[1]: [
      OIBS2SpecialConfig(0, 11, ('B',), Direction.EAST, (Direction.EAST,)),
      OIBS2SpecialConfig(0, 11, ('B',), Direction.WEST, (Direction.WEST,))
    ],
    #INTERCONNECT_WIRE_SIZES[2]: [
    #  OIBS2SpecialConfig(0, 11, ('B',), Direction.EAST, (Direction.EAST,)),
    #  OIBS2SpecialConfig(0, 11, ('B',), Direction.WEST, (Direction.WEST,))
    #],
  },
  Direction.WEST: {
    INTERCONNECT_WIRE_SIZES[-1]: [
      OIBS2SpecialConfig(0, 44, ('B',), None, (Direction.WEST,)),
      OIBS2SpecialConfig(1, 44, ('B',), None, (Direction.EAST,)),
    ],
    INTERCONNECT_WIRE_SIZES[0]: [
      OIBS2SpecialConfig(0, 33, ('A', 'B'), Direction.WEST, (Direction.EAST,))
    ],
    #INTERCONNECT_WIRE_SIZES[2]: [
    #  OIBS2SpecialConfig(0, 33, ('A', 'B'), Direction.EAST, (Direction.EAST,)),
    #  OIBS2SpecialConfig(0, 33, ('A', 'B'), Direction.WEST, (Direction.WEST,))
    #],
  },
  Direction.SOUTH: {
    INTERCONNECT_WIRE_SIZES[0]: [
      OIBS2SpecialConfig(0, 22, ('A', 'B'), Direction.EAST, (Direction.EAST,)),
      OIBS2SpecialConfig(0, 22, ('A', 'B'), Direction.WEST, (Direction.WEST,))
    ],
    #INTERCONNECT_WIRE_SIZES[1]: [
    #  OIBS2SpecialConfig(0, 22, ('A', 'B'), Direction.EAST, (Direction.EAST,)),
    #  OIBS2SpecialConfig(0, 22, ('A', 'B'), Direction.WEST, (Direction.WEST,))
    #]
  }
}



# OIB S1 preamble
# ---------------
#

# We do not want 8:1 muxes, so should we use 4:1 or 6:1?
OIB_S1_MAX_NUM_INPUTS = 6
# We do not connect these here, they are all pins to other blocks.
OIB_S1_MAX_FAN_OUT = 0

# Extra-tile interconnect generation.
# -----------------------------------

# Wires by io direction, wire direction, then size, then source side :

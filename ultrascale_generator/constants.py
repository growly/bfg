import collections
import random
from common import Direction

# This is the kludge we use to set constants for different fabrics.
#from bfg_params import *
from ultrascale_params import *

#@dataclass(unsafe_hash=True)
#class WireKey:
#  io_direction: Direction
#  wire_direction: Direction
#  size: int
#  sourec_side: Direction
WireKey = collections.namedtuple(
  'WireKey', ['io_direction', 'wire_direction', 'size', 'source_side'])

#seed = random.randrange(sys.maxsize)
#random.seed(seed)
#print('seed:', seed)

# This seed did well when explicitly allocating 1/4 of the LUT outputs to the
# B/A pattern.
#random.seed(4894203567079800142)

# This seed did well when explicitly allocating 1/8 of the LUT outputs to the
# B/A pattern.
random.seed(314082660935717772)

INTERCONNECT_SIDE_INCREASING_INDEX = (Direction.EAST, Direction.WEST)

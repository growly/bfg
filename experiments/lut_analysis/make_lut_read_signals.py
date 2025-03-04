import itertools
import math

LUT_SIZE=4
NUM_PERMUTATIONS=1

# We feed in indices like 1, 6, 15, which are binary numbers 0001, 0110, 1111.
# This function ensure they are all padded equally, to the width of the widest
# value. It then uses zip() to transpose the list of values [0001, 0110, 1111]
# such that each value is read column-wise in the order it is given. That is,
# the example sequence becomes:
#  0 0 1
#  0 1 1
#  0 1 1
#  1 0 1
#
# The least significant bits in the input become the bottom-most (last) entries
# in the output sequence.
def generate_patterns(indices):
    # bin(i) includes a '0b' prefix.
    max_width = len(format(max(indices), 'b'))
    format_specifier = '{:0' + str(max_width) + 'b}'
    binary_forms = [format_specifier.format(i) for i in indices]
    signals = [list(b) for b in binary_forms]

    # zip(*signals) returns tuples of characters, each of which must be turned
    # back into a string.
    return [''.join(t) for t in zip(*signals)]

def generate_generators(patterns):
    generators = []
    num_generators = len(patterns)
    i = 0
    for pattern in reversed(patterns):

        # source_name p_node n_node pat_statement high_voltage low_voltage delay
        # rise_time fall_time sample_period
        generator = f'va{i}_driver a{i} VSS pat (' + (
            '{1.8} {0} {test_start} {50p} {50p} {read_clock / 2} ' +
            f'b0{pattern}' + '0 0)')
        i = i + 1
        generators.insert(0, generator)
    return generators

#generate_patterns([1, 6, 13, 15])
permutations = itertools.permutations([i for i in range(int(math.pow(2, LUT_SIZE)))])

#i = 0
#for permutation in permutations:
#
#    generators = generate_generators(generate_patterns(permutation))
#    for generator in generators:
#        print(generator)
#
#    i = i + 1
#    if i >= NUM_PERMUTATIONS:
#        break

# If the scan chain is loaded with alternating 0 and 1 bits, then we have to
# read them in this order. That is, this is the mux order that yields the
# sequentially increasing scan order:
fixed = [1, 0, 2, 3, 15, 14, 12, 13, 11, 10, 8, 9, 5, 4, 6, 7]
generators = generate_generators(generate_patterns(fixed))
for generator in generators:
    print(generator)

def program_in_order(order, values=None):
    data = []
    if values:
        data = zip(order, values)
    else:
        last_datum = 0
        for index in order:
            data.append((index, last_datum))
            last_datum = (last_datum + 1) % 2
    
    print(data)
    data.sort(key=lambda x: x[0])
    print(data)

    pattern = ''.join(str(x[1]) for x in data)
    return pattern

# On the other hand, it is useful to program the scan chain such that the
# natural mux read order yields the alternating 0 and 1 bits. In that case, we
# have to assign the alternating 0 and 1 bits in mux order and then sort for
# scan order to know what order they should be programmed in:
from collections import namedtuple
LutOrder = namedtuple('LutOrder', 'mux_order name scan_order')
# mux order                  scan order
bfg_luts = [
    LutOrder( 0, "LUT_DFXTP_0_1.Q",  1),
    LutOrder( 1, "LUT_DFXTP_0_0.Q",  0),
    LutOrder( 2, "LUT_DFXTP_0_2.Q",  2),
    LutOrder( 3, "LUT_DFXTP_0_3.Q",  3),
    LutOrder( 4, "LUT_DFXTP_1_5.Q", 13),
    LutOrder( 5, "LUT_DFXTP_1_4.Q", 12),
    LutOrder( 6, "LUT_DFXTP_1_6.Q", 14),
    LutOrder( 7, "LUT_DFXTP_1_7.Q", 15),
    LutOrder( 8, "LUT_DFXTP_1_2.Q", 10),
    LutOrder( 9, "LUT_DFXTP_1_3.Q", 11),
    LutOrder(10, "LUT_DFXTP_1_1.Q",  9),
    LutOrder(11, "LUT_DFXTP_1_0.Q",  8),
    LutOrder(12, "LUT_DFXTP_0_6.Q",  6),
    LutOrder(13, "LUT_DFXTP_0_7.Q",  7),
    LutOrder(14, "LUT_DFXTP_0_5.Q",  5),
    LutOrder(15, "LUT_DFXTP_0_4.Q",  4),
]
scan_order = [x.scan_order for x in bfg_luts]
print(program_in_order(scan_order))

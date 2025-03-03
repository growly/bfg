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
        generator = f'va{i}_driver a{i} VSS pat(' + (
            '{1.8} {0} {test_start} {50p} {50p {read_clock / 2} ' +
            f'b{pattern}' + '0)')
        i = i + 1
        generators.insert(0, generator)
    return generators

#generate_patterns([1, 6, 13, 15])
permutations = itertools.permutations([i for i in range(int(math.pow(2, LUT_SIZE)))])

i = 0
for permutation in permutations:

    generators = generate_generators(generate_patterns(permutation))
    for generator in generators:
        print(generator)

    i = i + 1
    if i >= NUM_PERMUTATIONS:
        break

#!/usr/bin/env python3
import argparse
import tabulate

from architecture import Architecture
import constants

import iib_1_to_2
import iib_2_to_luts
import iib_2_to_1
import interconnect_only
import interconnect_to_iib_1
import interconnect_to_oib_2
import luts_to_oib_2
import oib_2_to_1
import oib_2_bounce

def make_architecture():
  arch = Architecture()
  arch.build_all()

  # THE ORDER MATTERS A LOT!

  iib_1_to_2.connect(arch)

  iib_2_to_luts.connect(arch)

  interconnect_to_iib_1.connect(arch)

  iib_2_to_1.connect(arch)

  oib_2_bounce.connect(arch)

  interconnect_to_oib_2.connect(arch)

  luts_to_oib_2.connect(arch)

  interconnect_only.connect(arch)

  oib_2_to_1.connect(arch)

  return arch

  # After interconnect allocation.
  #fan_in_checkpoint_0 = [(mux, mux.fan_in()) for mux in oib_s2_regular_mux_list]

  ## After LUT output allocation.
  #fan_in_checkpoint_1 = [(mux, mux.fan_in()) for mux in arch.oib_s2_regular_list]
  #rows = [['mux', 'fan in @ 1', 'fan in @ 0']]
  #as_dict = dict(fan_in_checkpoint_0)
  #for mux, fan_in_1 in sorted(fan_in_checkpoint_1, key=lambda x: x[1], reverse=True):
  #  fan_in_0 = as_dict[mux]
  #  rows.append([mux.name, fan_in_1, fan_in_0])
  #print(tabulate.tabulate(rows, headers='firstrow'))

  #print()
  #for mux in oib_s2_nonlocal_bounce_list:
  #  print(mux.fan_in(), mux.name)
  #
  #print()
  #for lut in luts:
  #  for port_name, out_node in lut.output_nodes.items():
  #    number = len(out_node.get_out_nodes())
  #    print(lut.name, port_name, number)
  #    for i, node in enumerate(out_node.get_out_nodes()):
  #      print(i+1, node.name)
  #
  #print_lut_outputs()
  #
  ### So... sort OIB S1 by fan-in, ascending:
  ##oib_s1_by_fan_in = sorted(
  ##    [(mux.fan_in(), mux) for mux in interconnect_out_wires.values()],
  ##    key=lambda x: x[0])
  ##
  ##for fan_in, mux in oib_s1_by_fan_in:
  ##  print(fan_in, mux.name)
  ##
  ##
  ###oib_s2_by_fan_out = sorted(
  ###    [(mux.fan_out(), mux) for mux in oib_s2_regular_mux_list],
  ###    key=lambda x: x[0])
  ###for fan_out, mux in oib_s2_by_fan_out:
  ###  if fan_out >= 4:
  ###    break
  ###  print(fan_out, mux.name)
  ##

def sanity_check_shared_muxes(
    mux_list, max_inputs_per_mux, max_shared_between_muxes):
  rows = [['', 'mux', 'paired', 'left only', 'shared', 'right only']]
  reported_pairs = set()
  for mux in mux_list:
    max_unique_between_pair = (2 * (
        max_inputs_per_mux - max_shared_between_muxes) +
        max_shared_between_muxes)
    #input_sharers = set(mux.parent.input_sharers) - set([mux])

    #if input_sharers is None:
    #  rows.append(['', mux.name] + [''] * (len(rows[0]) - 2))
    #  continue

    paired = list(mux.input_sharers)[0]
    
    if (mux, paired) in reported_pairs or (paired, mux) in reported_pairs:
      continue
    reported_pairs.add((mux, paired))

    left_only = mux.get_in_nodes() - paired.get_in_nodes()
    both = mux.get_in_nodes() & paired.get_in_nodes()
    right_only = paired.get_in_nodes() - mux.get_in_nodes()
    either = mux.get_in_nodes() | paired.get_in_nodes()

    row = ['',
           mux.name,
           paired.name,
           ', '.join(m.name for m in left_only),
           ', '.join(m.name for m in both),
           ', '.join(m.name for m in right_only)]

    if len(either) > max_unique_between_pair:
      row[0] = '*'

    rows.append(row)
  print(tabulate.tabulate(rows, headers='firstrow'))

def human_sanity_checks(arch):
  print()
  sanity_check_shared_muxes(arch.oib_s2_regular_list,
                            constants.OIB_S2_MAX_NUM_INPUTS,
                            constants.OIB_S2_NUM_SHARED_INPUTS)

  print()
  sanity_check_shared_muxes(arch.iib_s1_regular_list,
                            constants.IIB_S1_MAX_NUM_INPUTS,
                            constants.IIB_S1_NUM_SHARED_INPUTS)

  # TODO(aryap): sanity check all inputs/outputs where they are expected to
  # exist!

if __name__ == '__main__':
  parser = argparse.ArgumentParser(
      description='Generate Xilinx RapidWright routing graph architecture')
  parser.add_argument(
      '--inspect',
      action='store_true',
      help='Launch web-based architecture inspector')
  parser.add_argument(
      '--port',
      type=int,
      default=9123,
      help='Port for the web inspector (default: 9123)')

  args = parser.parse_args()

  arch = make_architecture()

  if args.inspect:
    # Launch the architecture inspector
    from architecture_inspector import ArchitectureInspector
    print("\nLaunching Architecture Inspector...")
    inspector = ArchitectureInspector(arch, port=args.port)
    inspector.run()
  else:
    # Normal output
    print()
    arch.print_stats()

    print()
    arch.sectional_summary()
    #arch.report()

    print()
    arch.report_io()

    arch.write_all()

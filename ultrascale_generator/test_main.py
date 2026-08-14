#!/usr/bin/env python3
'''
Tests for the Xilinx RapidWright routing graph generator.

Ok we need to run some tests. There are two categories:

Category 1: Are the invariants of the devices violated?
  - No excessive fan-out
  - No excessive number of inputs (more than available)
    - In particular for shared muxes - Are all outputs that need to be connected, connected?
  - Are there disconnected outputs? Are they ok to be?

Category 2: Are the generated patterns "close enough" to the originals we're
modelling?
  - Do they match the patterns (manual, done by me on first pass)
  - Do they preserve the "soft invariants", things like:
    - Distribution of inputs over sections/groups
    - Proximity of fan-out connections to one another (i.e. wire length)
    - ???
'''

import pytest
from unittest.mock import Mock, patch
import functools

import constants
import main
import node

arch = main.make_architecture()

interconnect_in_list = list(arch.interconnect_by_name.values())
iib_s1_list = list(arch.iib_s1_by_name.values())
iib_s2_list = list(arch.iib_s2_by_name.values())
oib_s2_list = list(arch.oib_s2_by_name.values())
oib_s1_list = list(arch.oib_s1_by_name.values())

class TestMain:
  '''End-to-end tests for architecture generation.'''
  def get_all_muxes(self, arch):
    source_lists = [
        arch.interconnect_by_name,
        arch.iib_s1_by_name,
        arch.iib_s2_by_name,
        arch.oib_s2_by_name,
        arch.oib_s1_by_name
    ]
    all_values = functools.reduce(
        lambda x, y: x + y,
        list(list(v for v in s.values()) for s in source_lists),
        [])
    return all_values

  def test_number_of_muxes(self):
    pass

  def disabledtest_no_outputless_muxes(self):
    # All but the outgoing interconnect should have connected outputs in this
    # block.
    source_lists = [
        arch.interconnect_by_name,
        arch.iib_s1_by_name,
        arch.iib_s2_by_name,
        arch.oib_s2_by_name,
    ]
    muxes = functools.reduce(
        lambda x, y: x + y,
        list(list(v for v in s.values()) for s in source_lists),
        [])

    for i, mux in enumerate(muxes):
      pytest.assume(len(mux.get_out_nodes()) != 0, f'{mux} has no outputs')

class TestInterconnectIn():
  def test_num_outputs(self):
    for mux in interconnect_in_list:
      assert mux.fan_out() <= constants.INTERCONNECT_IN_MAX_FAN_OUT, (
          f'{mux} has too many outputs')


class TestIIBS1:

  def test_num_inputs(self):
    for mux in iib_s1_list:
      assert mux.fan_in() <= constants.IIB_S1_MAX_NUM_INPUTS, (
          f'{mux} has too many inputs')

  def test_num_outputs(self):
    for mux in iib_s1_list:
      assert mux.fan_out() <= constants.IIB_S1_MAX_FAN_OUT, (
          f'{mux} has too many outputs')

  def test_shared_mux_inputs(self):
    shared_muxes = set()
    for mux in iib_s1_list:
      if mux is node.SharedMuxPartitionNode:
        shared_muxes.add(mux.parent)

    for shared in shared_muxes:
      assert len(shared) <= shared.num_shared_inputs, (
          f'{shared} has too many shared inputs')
      for output, inputs in shared.unique_inputs_per_output.items():
        assert len(inputs) <= shared.unique_inputs_per_port, (
            f'{shared}/{output} has too many shared inputs')


class TestIIBS2:

  def test_num_inputs(self):
    for mux in iib_s2_list:
      assert mux.fan_in() <= constants.IIB_S2_MAX_NUM_INPUTS, (
          f'{mux} has too many inputs')

  def test_num_outputs(self):
    for mux in iib_s2_list:
      if 'BOUNCE' in mux.name or 'BYPASS' in mux.name:
        assert mux.fan_out() <= constants.IIB_S2_BOUNCE_FAN_OUT, (
            f'{mux} has too many outputs')
      else:
        assert mux.fan_out() <= constants.IIB_S2_MAX_FAN_OUT, (
            f'{mux} has too many outputs')

  def test_bypass_bounce(self):
    pass


class TestOIBS2:

  def test_num_inputs(self):
    for mux in oib_s2_list:
      assert mux.fan_in() <= constants.OIB_S2_MAX_NUM_INPUTS, (
          f'{mux} has too many inputs')

  def test_num_outputs(self):
    for mux in oib_s2_list:
      if 'BOUNCE' in mux.name:
        assert mux.fan_out() <= constants.OIB_S2_BOUNCE_FAN_OUT, (
            f'{mux} has too many outputs')
      else:
        assert mux.fan_out() <= constants.OIB_S2_MAX_FAN_OUT, (
            f'{mux} has too many outputs')

  def test_bypass_bounce(self):
    pass


class TestOIBS1:

  def test_num_inputs(self):
    for mux in oib_s1_list:
      assert mux.fan_in() <= constants.OIB_S1_MAX_NUM_INPUTS, (
          f'{mux} has too many inputs')

  def test_num_outputs(self):
    for mux in oib_s1_list:
      assert mux.fan_out() <= constants.OIB_S1_MAX_FAN_OUT, (
          f'{mux} has too many outputs')


if __name__ == '__main__':
  pytest.main([__file__, '-v'])

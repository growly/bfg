// TypeScript interface for Sky130TransmissionGate parameter protobuf

import { CompassDirection } from '../common.js';

export interface Sky130TransmissionGate {
  p_width_nm?: number;
  p_length_nm?: number;
  n_width_nm?: number;
  n_length_nm?: number;
  stacks_left?: boolean;
  stacks_right?: boolean;
  vertical_tab_pitch_nm?: number;
  vertical_tab_offset_nm?: number;
  poly_pitch_nm?: number;
  nmos_ll_vertical_offset_nm?: number;
  nmos_ll_vertical_pitch_nm?: number;
  min_p_tab_diff_separation_nm?: number;
  min_n_tab_diff_separation_nm?: number;
  min_poly_boundary_separation_nm?: number;
  tabs_should_avoid_nearest_vias?: boolean;
  draw_nwell?: boolean;
  p_tab_position?: CompassDirection;
  n_tab_position?: CompassDirection;
}

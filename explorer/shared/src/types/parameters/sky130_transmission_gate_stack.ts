// TypeScript interface for Sky130TransmissionGateStack parameter protobuf

export interface Sky130TransmissionGateStackSequence {
  nets: string[];
}

export interface Sky130TransmissionGateStack {
  sequences?: Sky130TransmissionGateStackSequence[];
  p_width_nm?: number;
  p_length_nm?: number;
  n_width_nm?: number;
  n_length_nm?: number;
  li_width_nm?: number;
  min_height_nm?: number;
  poly_contact_vertical_pitch_nm?: number;
  poly_contact_vertical_offset_nm?: number;
  poly_pitch_nm?: number;
  horizontal_pitch_nm?: number;
  min_p_tab_diff_separation_nm?: number;
  min_n_tab_diff_separation_nm?: number;
  min_poly_boundary_separation_nm?: number;
  insert_dummy_poly?: boolean;
  expand_wells_to_vertical_bounds?: boolean;
}

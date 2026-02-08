// TypeScript interface for InterconnectWireBlock parameter protobuf

export type RoutingTrackDirection = 'TRACK_HORIZONTAL' | 'TRACK_VERTICAL';

export interface InterconnectWireBlockBundle {
  num_wires?: number;
}

export interface InterconnectWireBlockChannel {
  name?: string;
  break_out?: number[];
  num_bundles?: number;
  bundle?: InterconnectWireBlockBundle;
}

export interface InterconnectWireBlock {
  direction?: RoutingTrackDirection;
  grow_down?: boolean;
  grow_left?: boolean;

  horizontal_layer?: string;
  via_layer?: string;
  vertical_layer?: string;

  horizontal_wire_width_nm?: number;
  horizontal_wire_pitch_nm?: number;
  horizontal_wire_offset_nm?: number;
  vertical_wire_width_nm?: number;
  vertical_wire_pitch_nm?: number;
  vertical_wire_offset_nm?: number;

  length?: number;

  channels?: InterconnectWireBlockChannel[];
}

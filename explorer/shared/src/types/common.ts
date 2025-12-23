// Common enums and types used across parameter definitions

export enum CompassDirection {
  WEST = 'WEST',
  NORTH_WEST = 'NORTH_WEST',
  NORTH = 'NORTH',
  NORTH_EAST = 'NORTH_EAST',
  EAST = 'EAST',
  SOUTH_EAST = 'SOUTH_EAST',
  SOUTH = 'SOUTH',
  SOUTH_WEST = 'SOUTH_WEST',
}

export type GeneratorName =
  | 'Sky130TransmissionGate'
  | 'Sky130TransmissionGateStack'
  | 'Sky130InterconnectMux1'
  | 'Sky130Decap'
  | 'LutB'
  | 'Interconnect'
  | 'Slice';

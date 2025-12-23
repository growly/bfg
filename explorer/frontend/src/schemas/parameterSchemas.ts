// Parameter schemas for all BFG generators

export type FieldType = 'number' | 'boolean' | 'string' | 'enum' | 'array';

export interface FieldSchema {
  name: string;
  label: string;
  type: FieldType;
  unit?: string;
  defaultValue?: unknown;
  enumValues?: string[];
  description?: string;
  min?: number;
  max?: number;
  step?: number;
  // For array fields
  itemType?: 'string' | 'object';
  itemFields?: FieldSchema[];
}

export interface GeneratorSchema {
  name: string;
  displayName: string;
  description: string;
  fields: FieldSchema[];
  exampleParams: Record<string, unknown>;
}

export const GENERATOR_SCHEMAS: Record<string, GeneratorSchema> = {
  Sky130Decap: {
    name: 'Sky130Decap',
    displayName: 'Sky130 Decap',
    description: 'Decoupling capacitor cell',
    fields: [
      {
        name: 'width_nm',
        label: 'Width',
        type: 'number',
        unit: 'nm',
        defaultValue: 1380,
        min: 0,
        step: 10,
        description: 'Cell width in nanometers',
      },
      {
        name: 'height_nm',
        label: 'Height',
        type: 'number',
        unit: 'nm',
        defaultValue: 2720,
        min: 0,
        step: 10,
        description: 'Cell height in nanometers',
      },
      {
        name: 'nfet_0_width_nm',
        label: 'NFET Width',
        type: 'number',
        unit: 'nm',
        min: 0,
        step: 10,
        description: 'NFET transistor width (optional)',
      },
      {
        name: 'pfet_0_width_nm',
        label: 'PFET Width',
        type: 'number',
        unit: 'nm',
        min: 0,
        step: 10,
        description: 'PFET transistor width (optional)',
      },
      {
        name: 'nfet_0_length_nm',
        label: 'NFET Length',
        type: 'number',
        unit: 'nm',
        min: 0,
        step: 10,
        description: 'NFET transistor length (optional)',
      },
      {
        name: 'pfet_0_length_nm',
        label: 'PFET Length',
        type: 'number',
        unit: 'nm',
        min: 0,
        step: 10,
        description: 'PFET transistor length (optional)',
      },
      {
        name: 'label_pins',
        label: 'Label Pins',
        type: 'boolean',
        defaultValue: true,
        description: 'Add text labels to pins',
      },
      {
        name: 'draw_overflowing_vias_and_pins',
        label: 'Draw Overflowing Vias/Pins',
        type: 'boolean',
        defaultValue: true,
        description: 'Draw vias and pins that extend beyond cell boundary',
      },
    ],
    exampleParams: {
      width_nm: 1380,
      height_nm: 2720,
      label_pins: true,
      draw_overflowing_vias_and_pins: true,
    },
  },

  Sky130TransmissionGate: {
    name: 'Sky130TransmissionGate',
    displayName: 'Sky130 Transmission Gate',
    description: 'CMOS transmission gate with configurable transistor dimensions',
    fields: [
      {
        name: 'p_width_nm',
        label: 'PMOS Width',
        type: 'number',
        unit: 'nm',
        defaultValue: 1000,
        min: 0,
        step: 10,
        description: 'PMOS transistor width',
      },
      {
        name: 'p_length_nm',
        label: 'PMOS Length',
        type: 'number',
        unit: 'nm',
        defaultValue: 150,
        min: 0,
        step: 10,
        description: 'PMOS transistor length',
      },
      {
        name: 'n_width_nm',
        label: 'NMOS Width',
        type: 'number',
        unit: 'nm',
        defaultValue: 650,
        min: 0,
        step: 10,
        description: 'NMOS transistor width',
      },
      {
        name: 'n_length_nm',
        label: 'NMOS Length',
        type: 'number',
        unit: 'nm',
        defaultValue: 150,
        min: 0,
        step: 10,
        description: 'NMOS transistor length',
      },
      {
        name: 'stacks_left',
        label: 'Stacks Left',
        type: 'boolean',
        defaultValue: false,
        description: 'Enable stacking on left side',
      },
      {
        name: 'stacks_right',
        label: 'Stacks Right',
        type: 'boolean',
        defaultValue: false,
        description: 'Enable stacking on right side',
      },
      {
        name: 'vertical_tab_pitch_nm',
        label: 'Vertical Tab Pitch',
        type: 'number',
        unit: 'nm',
        defaultValue: 170,
        min: 0,
        step: 10,
        description: 'Vertical spacing between tabs',
      },
      {
        name: 'vertical_tab_offset_nm',
        label: 'Vertical Tab Offset',
        type: 'number',
        unit: 'nm',
        defaultValue: 170,
        min: 0,
        step: 10,
        description: 'Initial vertical offset for tabs',
      },
      {
        name: 'poly_pitch_nm',
        label: 'Poly Pitch',
        type: 'number',
        unit: 'nm',
        defaultValue: 500,
        min: 0,
        step: 10,
        description: 'Polysilicon gate pitch',
      },
      {
        name: 'p_tab_position',
        label: 'PMOS Tab Position',
        type: 'enum',
        enumValues: ['WEST', 'NORTH_WEST', 'NORTH', 'NORTH_EAST', 'EAST', 'SOUTH_EAST', 'SOUTH', 'SOUTH_WEST'],
        description: 'Position of PMOS substrate tab',
      },
      {
        name: 'n_tab_position',
        label: 'NMOS Tab Position',
        type: 'enum',
        enumValues: ['WEST', 'NORTH_WEST', 'NORTH', 'NORTH_EAST', 'EAST', 'SOUTH_EAST', 'SOUTH', 'SOUTH_WEST'],
        description: 'Position of NMOS substrate tab',
      },
      {
        name: 'draw_nwell',
        label: 'Draw N-Well',
        type: 'boolean',
        defaultValue: true,
        description: 'Draw N-well layer',
      },
    ],
    exampleParams: {
      p_width_nm: 1000,
      p_length_nm: 150,
      n_width_nm: 650,
      n_length_nm: 150,
      poly_pitch_nm: 500,
    },
  },

  Sky130TransmissionGateStack: {
    name: 'Sky130TransmissionGateStack',
    displayName: 'Sky130 Transmission Gate Stack',
    description: 'Stack of transmission gates with net sequences',
    fields: [
      {
        name: 'sequences',
        label: 'Gate Sequences',
        type: 'array',
        itemType: 'object',
        itemFields: [
          {
            name: 'nets',
            label: 'Nets (comma-separated)',
            type: 'string',
            description: 'Net names separated by commas, e.g., "A,B,C"',
          },
        ],
        description: 'Sequences of nets forming transmission gates',
      },
      {
        name: 'p_width_nm',
        label: 'PMOS Width',
        type: 'number',
        unit: 'nm',
        defaultValue: 1000,
        min: 0,
        step: 10,
      },
      {
        name: 'p_length_nm',
        label: 'PMOS Length',
        type: 'number',
        unit: 'nm',
        defaultValue: 150,
        min: 0,
        step: 10,
      },
      {
        name: 'n_width_nm',
        label: 'NMOS Width',
        type: 'number',
        unit: 'nm',
        defaultValue: 650,
        min: 0,
        step: 10,
      },
      {
        name: 'n_length_nm',
        label: 'NMOS Length',
        type: 'number',
        unit: 'nm',
        defaultValue: 150,
        min: 0,
        step: 10,
      },
      {
        name: 'li_width_nm',
        label: 'Local Interconnect Width',
        type: 'number',
        unit: 'nm',
        min: 0,
        step: 10,
        description: 'Width of local interconnect layer',
      },
      {
        name: 'min_height_nm',
        label: 'Minimum Height',
        type: 'number',
        unit: 'nm',
        defaultValue: 2720,
        min: 0,
        step: 10,
        description: 'Minimum cell height',
      },
      {
        name: 'poly_pitch_nm',
        label: 'Poly Pitch',
        type: 'number',
        unit: 'nm',
        defaultValue: 600,
        min: 0,
        step: 10,
      },
      {
        name: 'horizontal_pitch_nm',
        label: 'Horizontal Pitch',
        type: 'number',
        unit: 'nm',
        defaultValue: 460,
        min: 0,
        step: 10,
      },
      {
        name: 'insert_dummy_poly',
        label: 'Insert Dummy Poly',
        type: 'boolean',
        defaultValue: true,
        description: 'Insert dummy polysilicon for density',
      },
      {
        name: 'expand_wells_to_vertical_bounds',
        label: 'Expand Wells Vertically',
        type: 'boolean',
        defaultValue: true,
        description: 'Expand wells to cell vertical bounds',
      },
    ],
    exampleParams: {
      sequences: [{ nets: 'A,B,C' }],
      p_width_nm: 1000,
      p_length_nm: 150,
      n_width_nm: 650,
      n_length_nm: 150,
      min_height_nm: 2720,
    },
  },

  Sky130InterconnectMux1: {
    name: 'Sky130InterconnectMux1',
    displayName: 'Sky130 Interconnect Mux1',
    description: 'Multiplexer for interconnect routing',
    fields: [
      {
        name: 'num_inputs',
        label: 'Number of Inputs',
        type: 'number',
        defaultValue: 6,
        min: 1,
        max: 32,
        step: 1,
        description: 'Number of multiplexer inputs',
      },
      {
        name: 'num_outputs',
        label: 'Number of Outputs',
        type: 'number',
        defaultValue: 1,
        min: 1,
        max: 8,
        step: 1,
        description: 'Number of multiplexer outputs',
      },
      {
        name: 'poly_pitch_nm',
        label: 'Poly Pitch',
        type: 'number',
        unit: 'nm',
        defaultValue: 500,
        min: 0,
        step: 10,
      },
      {
        name: 'vertical_pitch_nm',
        label: 'Vertical Pitch',
        type: 'number',
        unit: 'nm',
        defaultValue: 340,
        min: 0,
        step: 10,
      },
      {
        name: 'vertical_offset_nm',
        label: 'Vertical Offset',
        type: 'number',
        unit: 'nm',
        defaultValue: 170,
        min: 0,
        step: 10,
      },
      {
        name: 'horizontal_pitch_nm',
        label: 'Horizontal Pitch',
        type: 'number',
        unit: 'nm',
        defaultValue: 460,
        min: 0,
        step: 10,
      },
      {
        name: 'vertical_routing_channel_width_nm',
        label: 'Vertical Routing Channel Width',
        type: 'number',
        unit: 'nm',
        min: 0,
        step: 10,
        description: 'Width of vertical routing channels',
      },
      {
        name: 'power_ground_strap_width_nm',
        label: 'Power/Ground Strap Width',
        type: 'number',
        unit: 'nm',
        defaultValue: 300,
        min: 0,
        step: 10,
      },
    ],
    exampleParams: {
      num_inputs: 6,
      num_outputs: 1,
      poly_pitch_nm: 500,
      vertical_pitch_nm: 340,
      horizontal_pitch_nm: 460,
    },
  },

  LutB: {
    name: 'LutB',
    displayName: 'LUT-B',
    description: 'K-input look-up table (memory-based logic)',
    fields: [
      {
        name: 'lut_size',
        label: 'LUT Size (K)',
        type: 'number',
        defaultValue: 4,
        min: 2,
        max: 6,
        step: 1,
        description: 'Number of LUT inputs (K-LUT)',
      },
    ],
    exampleParams: {
      lut_size: 4,
    },
  },

  Interconnect: {
    name: 'Interconnect',
    displayName: 'Interconnect',
    description: '2D grid interconnect tile',
    fields: [
      {
        name: 'num_rows',
        label: 'Number of Rows',
        type: 'number',
        defaultValue: 8,
        min: 1,
        max: 64,
        step: 1,
        description: 'Number of multiplexer rows',
      },
      {
        name: 'num_columns',
        label: 'Number of Columns',
        type: 'number',
        defaultValue: 16,
        min: 1,
        max: 64,
        step: 1,
        description: 'Number of multiplexer columns',
      },
    ],
    exampleParams: {
      num_rows: 8,
      num_columns: 16,
    },
  },

  Slice: {
    name: 'Slice',
    displayName: 'Slice',
    description: 'Circuit slice (no configurable parameters)',
    fields: [],
    exampleParams: {},
  },
};

export function getGeneratorSchema(generatorName: string): GeneratorSchema | undefined {
  return GENERATOR_SCHEMAS[generatorName];
}

// Utility to format JavaScript objects as protocol buffer text format

// Preprocess params to handle special field transformations
function preprocessParams(params: Record<string, unknown>): Record<string, unknown> {
  const result: Record<string, unknown> = {};

  for (const [key, value] of Object.entries(params)) {
    if (value === undefined || value === null) {
      continue;
    }

    // Handle channels array for InterconnectWireBlock
    if (key === 'channels' && Array.isArray(value)) {
      result[key] = value.map((channel: Record<string, unknown>) => {
        const processedChannel: Record<string, unknown> = {};

        for (const [channelKey, channelValue] of Object.entries(channel)) {
          if (channelKey === 'break_out' && typeof channelValue === 'string') {
            // Convert comma-separated string to array of integers
            processedChannel[channelKey] = channelValue
              .split(',')
              .map((s: string) => parseInt(s.trim(), 10))
              .filter((n: number) => !isNaN(n));
          } else if (channelKey === 'num_wires') {
            // Move num_wires into bundle sub-message
            processedChannel['bundle'] = { num_wires: channelValue };
          } else {
            processedChannel[channelKey] = channelValue;
          }
        }

        return processedChannel;
      });
    } else {
      result[key] = value;
    }
  }

  return result;
}

export function formatProtoText(
  params: Record<string, unknown>,
  indent = 0
): string {
  // Apply preprocessing only at top level
  const processedParams = indent === 0 ? preprocessParams(params) : params;

  const lines: string[] = [];
  const prefix = '  '.repeat(indent);

  for (const [key, value] of Object.entries(processedParams)) {
    if (value === undefined || value === null) {
      continue;
    }

    if (typeof value === 'boolean') {
      lines.push(`${prefix}${key}: ${value}`);
    } else if (typeof value === 'number') {
      lines.push(`${prefix}${key}: ${value}`);
    } else if (typeof value === 'string') {
      // Handle enum values (don't quote them)
      if (isEnumValue(key, value)) {
        lines.push(`${prefix}${key}: ${value}`);
      } else {
        lines.push(`${prefix}${key}: "${value}"`);
      }
    } else if (Array.isArray(value)) {
      // Handle repeated fields
      for (const item of value) {
        if (typeof item === 'object' && item !== null) {
          lines.push(`${prefix}${key} {`);
          lines.push(formatProtoText(item as Record<string, unknown>, indent + 1));
          lines.push(`${prefix}}`);
        } else {
          // Repeated primitive
          if (typeof item === 'string') {
            lines.push(`${prefix}${key}: "${item}"`);
          } else {
            lines.push(`${prefix}${key}: ${item}`);
          }
        }
      }
    } else if (typeof value === 'object') {
      // Nested message
      lines.push(`${prefix}${key} {`);
      lines.push(formatProtoText(value as Record<string, unknown>, indent + 1));
      lines.push(`${prefix}}`);
    }
  }

  return lines.join('\n');
}

// Check if a value is an enum (CompassDirection, RoutingTrackDirection, etc.)
function isEnumValue(key: string, value: string): boolean {
  const compassDirections = [
    'WEST',
    'NORTH_WEST',
    'NORTH',
    'NORTH_EAST',
    'EAST',
    'SOUTH_EAST',
    'SOUTH',
    'SOUTH_WEST',
  ];

  const routingTrackDirections = ['TRACK_HORIZONTAL', 'TRACK_VERTICAL'];

  const enumFieldMap: Record<string, string[]> = {
    p_tab_position: compassDirections,
    n_tab_position: compassDirections,
    direction: routingTrackDirections,
  };

  return key in enumFieldMap && enumFieldMap[key].includes(value);
}

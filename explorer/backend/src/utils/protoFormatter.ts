// Utility to format JavaScript objects as protocol buffer text format

export function formatProtoText(
  params: Record<string, unknown>,
  indent = 0
): string {
  const lines: string[] = [];
  const prefix = '  '.repeat(indent);

  for (const [key, value] of Object.entries(params)) {
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

// Check if a value is an enum (CompassDirection, etc.)
function isEnumValue(key: string, value: string): boolean {
  const enumFields = ['p_tab_position', 'n_tab_position'];
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

  return enumFields.includes(key) && compassDirections.includes(value);
}

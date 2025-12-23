// Service for converting GDS to SVG using Python gdspy

import { exec } from 'child_process';
import { promisify } from 'util';
import fs from 'fs/promises';
import path from 'path';

const execAsync = promisify(exec);

export class Gds2SvgError extends Error {
  constructor(
    message: string,
    public stderr?: string,
    public stdout?: string
  ) {
    super(message);
    this.name = 'Gds2SvgError';
  }
}

// Sky130 layer style definitions for SVG rendering
const SKY130_SVG_STYLE = `{
    (81, 4): {'fill': None, 'stroke': '#ff00ff', 'stroke-width': 2},
    (95, 20): {'fill': None, 'stroke': 'orange', 'stroke-width': 2},
    (93, 44): {'fill': None, 'stroke': '#e61f0d', 'stroke-width': 2},
    (94, 20): {'fill': None, 'stroke': '#9900e6', 'stroke-width': 2},
    (64, 20): {'fill': None, 'stroke': '#94bfa7', 'stroke-width': 2},
    (66, 20): {'fill': None, 'stroke': '#0000ff', 'stroke-width': 2},
    (67, 20): {'fill': None, 'stroke': '#00ff00', 'stroke-width': 2},
    (68, 20): {'fill': None, 'stroke': '#ff0000', 'stroke-width': 2},
}`;

function generateGdspyScript(
  gdsPath: string,
  cellName: string,
  svgPath: string
): string {
  return `#!/usr/bin/env python3
import gdspy
import sys

SKY130_SVG_STYLE = ${SKY130_SVG_STYLE}

try:
    library = gdspy.GdsLibrary(infile='${gdsPath}')

    # Find the top-level cell (usually the last one)
    if not library.cells:
        print("ERROR: No cells found in GDS file", file=sys.stderr)
        sys.exit(1)

    # Try to find cell by name, otherwise use the last cell
    cell = None
    if '${cellName}':
        cell = library.cells.get('${cellName}')

    if cell is None:
        # Use the last cell as top-level
        cell_names = list(library.cells.keys())
        cell_name = cell_names[-1]
        cell = library.cells[cell_name]
        print(f"Using cell: {cell_name}", file=sys.stderr)

    cell.write_svg(
        '${svgPath}',
        scaling=500,
        background='#ffffff',
        style=SKY130_SVG_STYLE
    )

    print(f"Successfully generated SVG: ${svgPath}", file=sys.stderr)
except Exception as e:
    print(f"ERROR: {str(e)}", file=sys.stderr)
    sys.exit(1)
`;
}

export async function convertToSvg(
  gdsPath: string,
  cellName: string,
  workDir: string
): Promise<string> {
  const svgPath = path.join(workDir, 'output.svg');
  const scriptPath = path.join(workDir, 'convert.py');

  try {
    // Generate and write Python script
    const pythonScript = generateGdspyScript(gdsPath, cellName, svgPath);
    await fs.writeFile(scriptPath, pythonScript, 'utf-8');
    await fs.chmod(scriptPath, 0o755);

    // Execute Python script
    const command = `python3 ${scriptPath}`;

    console.log(`Executing gds2svg: ${command}`);

    const { stdout, stderr } = await execAsync(command, {
      cwd: workDir,
      maxBuffer: 50 * 1024 * 1024, // 50MB buffer for SVG
    });

    if (stdout) {
      console.log(`gds2svg stdout: ${stdout}`);
    }
    if (stderr) {
      console.log(`gds2svg stderr: ${stderr}`);
    }

    // Verify SVG file was created
    try {
      await fs.access(svgPath);
    } catch {
      throw new Gds2SvgError(
        `Failed to generate SVG file: ${svgPath}`,
        stderr,
        stdout
      );
    }

    // Read and return SVG content
    const svgContent = await fs.readFile(svgPath, 'utf-8');
    return svgContent;
  } catch (error) {
    if (error instanceof Gds2SvgError) {
      throw error;
    }

    if (error instanceof Error && 'code' in error && error.code === 'ENOENT') {
      throw new Gds2SvgError(
        `Python3 not found. Please install Python 3 and gdspy: pip install gdspy`
      );
    }

    throw new Gds2SvgError(
      `GDS to SVG conversion failed: ${error instanceof Error ? error.message : String(error)}`,
      error instanceof Error && 'stderr' in error ? String(error.stderr) : undefined,
      error instanceof Error && 'stdout' in error ? String(error.stdout) : undefined
    );
  }
}

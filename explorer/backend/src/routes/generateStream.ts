// API route for streaming layout generation with real-time output

import express, { Request, Response } from 'express';
import { spawn } from 'child_process';
import fs from 'fs/promises';
import path from 'path';
import { v4 as uuidv4 } from 'uuid';
import { config } from '../config/paths.js';
import { formatProtoText } from '../utils/protoFormatter.js';

const router = express.Router();

interface GenerateRequest {
  generator: string;
  parameters: Record<string, unknown>;
}

router.post('/', async (req: Request, res: Response) => {
  const { generator, parameters } = req.body as GenerateRequest;

  // Validate request
  if (!generator) {
    return res.status(400).json({ status: 'error', message: 'Generator name is required' });
  }

  if (!parameters || typeof parameters !== 'object') {
    return res.status(400).json({ status: 'error', message: 'Parameters object is required' });
  }

  // Set up SSE headers
  res.setHeader('Content-Type', 'text/event-stream');
  res.setHeader('Cache-Control', 'no-cache');
  res.setHeader('Connection', 'keep-alive');
  res.flushHeaders();

  const sendEvent = (event: string, data: any) => {
    res.write(`event: ${event}\n`);
    res.write(`data: ${JSON.stringify(data)}\n\n`);
  };

  const sessionId = uuidv4();
  const tmpDirAbsolute = path.resolve(config.tmpDir);
  const workDir = path.join(tmpDirAbsolute, sessionId);

  try {
    await fs.mkdir(workDir, { recursive: true });

    // Write parameters to .pb.txt file
    const paramsPath = path.join(workDir, 'params.pb.txt');
    const protoText = formatProtoText(parameters);
    await fs.writeFile(paramsPath, protoText, 'utf-8');

    const outputBase = path.join(workDir, 'output');

    // Step 1: Run BFG
    sendEvent('stage', { stage: 'bfg', message: 'Running BFG generator...' });

    const bfgArgs = [
      '--technology', path.resolve(config.technologyPb),
      '--primitives', path.resolve(config.primitivesPb),
      '--external_circuits', path.resolve(config.sky130hdPb),
      '--write_text_format',
      '--run_generator', generator,
      '--params', paramsPath,
      '--output_library', outputBase,
    ];

    const bfgProcess = spawn(config.bfgBinary, bfgArgs, { cwd: workDir });

    let bfgStdout = '';
    let bfgStderr = '';

    bfgProcess.stdout.on('data', (data) => {
      const text = data.toString();
      bfgStdout += text;
      sendEvent('stdout', { stage: 'bfg', data: text });
    });

    bfgProcess.stderr.on('data', (data) => {
      const text = data.toString();
      bfgStderr += text;
      sendEvent('stderr', { stage: 'bfg', data: text });
    });

    const bfgExitCode = await new Promise<number>((resolve) => {
      bfgProcess.on('close', (code) => resolve(code || 0));
    });

    if (bfgExitCode !== 0) {
      sendEvent('error', {
        stage: 'bfg',
        message: 'BFG generation failed',
        stdout: bfgStdout,
        stderr: bfgStderr
      });
      res.end();
      return;
    }

    // Step 2: Convert to GDS
    sendEvent('stage', { stage: 'proto2gds', message: 'Converting to GDS...' });

    const libraryPb = `${outputBase}.library.pb`;
    const gdsPath = path.join(workDir, 'output.gds');

    const proto2gdsArgs = [
      '-i', libraryPb,
      '-t', config.technologyPb,
      '-o', gdsPath,
    ];

    const proto2gdsProcess = spawn(config.proto2gdsBinary, proto2gdsArgs, { cwd: workDir });

    let proto2gdsStdout = '';
    let proto2gdsStderr = '';

    proto2gdsProcess.stdout.on('data', (data) => {
      const text = data.toString();
      proto2gdsStdout += text;
      sendEvent('stdout', { stage: 'proto2gds', data: text });
    });

    proto2gdsProcess.stderr.on('data', (data) => {
      const text = data.toString();
      proto2gdsStderr += text;
      sendEvent('stderr', { stage: 'proto2gds', data: text });
    });

    const proto2gdsExitCode = await new Promise<number>((resolve) => {
      proto2gdsProcess.on('close', (code) => resolve(code || 0));
    });

    if (proto2gdsExitCode !== 0) {
      sendEvent('error', {
        stage: 'proto2gds',
        message: 'GDS conversion failed',
        stdout: proto2gdsStdout,
        stderr: proto2gdsStderr
      });
      res.end();
      return;
    }

    // Step 3: Convert to SVG
    sendEvent('stage', { stage: 'gds2svg', message: 'Converting to SVG...' });

    const svgPath = path.join(workDir, 'output.svg');
    const scriptPath = path.join(workDir, 'convert.py');

    const pythonScript = generateGdspyScript(gdsPath, generator, svgPath);
    await fs.writeFile(scriptPath, pythonScript, 'utf-8');
    await fs.chmod(scriptPath, 0o755);

    const gds2svgProcess = spawn('python3', [scriptPath], { cwd: workDir });

    let gds2svgStdout = '';
    let gds2svgStderr = '';

    gds2svgProcess.stdout.on('data', (data) => {
      const text = data.toString();
      gds2svgStdout += text;
      sendEvent('stdout', { stage: 'gds2svg', data: text });
    });

    gds2svgProcess.stderr.on('data', (data) => {
      const text = data.toString();
      gds2svgStderr += text;
      sendEvent('stderr', { stage: 'gds2svg', data: text });
    });

    const gds2svgExitCode = await new Promise<number>((resolve) => {
      gds2svgProcess.on('close', (code) => resolve(code || 0));
    });

    if (gds2svgExitCode !== 0) {
      sendEvent('error', {
        stage: 'gds2svg',
        message: 'SVG conversion failed',
        stdout: gds2svgStdout,
        stderr: gds2svgStderr
      });
      res.end();
      return;
    }

    // Read output files
    const svgContent = await fs.readFile(svgPath, 'utf-8');
    const libraryPbTxt = await fs.readFile(`${outputBase}.library.pb.txt`, 'utf-8').catch(() => '');
    const packagePbTxt = await fs.readFile(`${outputBase}.package.pb.txt`, 'utf-8').catch(() => '');

    // Send success event with all data
    sendEvent('complete', {
      svg: svgContent,
      libraryProto: libraryPbTxt,
      packageProto: packagePbTxt,
      stdout: [
        '=== BFG Generation ===',
        bfgStdout,
        '\n=== VLSIR to GDS Conversion ===',
        proto2gdsStdout,
        '\n=== GDS to SVG Conversion ===',
        gds2svgStdout,
      ].join('\n'),
      stderr: [
        '=== BFG Generation ===',
        bfgStderr,
        '\n=== VLSIR to GDS Conversion ===',
        proto2gdsStderr,
        '\n=== GDS to SVG Conversion ===',
        gds2svgStderr,
      ].join('\n'),
    });

    res.end();
  } catch (error) {
    sendEvent('error', {
      stage: 'setup',
      message: error instanceof Error ? error.message : 'Unknown error',
    });
    res.end();
  }
});

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

function generateGdspyScript(gdsPath: string, cellName: string, svgPath: string): string {
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

export default router;

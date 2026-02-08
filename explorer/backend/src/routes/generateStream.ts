// API route for streaming layout generation with real-time output

import express, { Request, Response } from 'express';
import { spawn } from 'child_process';
import fs from 'fs/promises';
import { appendFile } from 'fs/promises';
import path from 'path';
import { v4 as uuidv4 } from 'uuid';
import { config } from '../config/paths.js';
import { formatProtoText } from '../utils/protoFormatter.js';

const router = express.Router();

// Helper to append to logfile with timestamp
async function logToFile(logPath: string, message: string): Promise<void> {
  const timestamp = new Date().toISOString();
  await appendFile(logPath, `[${timestamp}] ${message}\n`, 'utf-8');
}

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

  // Create logfile path
  const logPath = path.join(workDir, 'generation.log');

  try {
    await fs.mkdir(workDir, { recursive: true });

    // Initialize logfile
    await logToFile(logPath, `=== Generation Session Started ===`);
    await logToFile(logPath, `Session ID: ${sessionId}`);
    await logToFile(logPath, `Generator: ${generator}`);
    await logToFile(logPath, `Parameters: ${JSON.stringify(parameters, null, 2)}`);

    // Write parameters to .pb.txt file
    const paramsPath = path.join(workDir, 'params.pb.txt');
    const protoText = formatProtoText(parameters);
    await fs.writeFile(paramsPath, protoText, 'utf-8');
    await logToFile(logPath, `Parameters written to: ${paramsPath}`);

    const outputBase = path.join(workDir, 'output');

    // Step 1: Run BFG
    sendEvent('stage', { stage: 'bfg', message: 'Running BFG generator...' });

    const bfgArgs = [
      '--logtostderr',
      '--technology', path.resolve(config.technologyPb),
      '--primitives', path.resolve(config.primitivesPb),
      '--external_circuits', path.resolve(config.sky130hdPb),
      '--write_text_format',
      '--run_generator', generator,
      '--params', paramsPath,
      '--output_library', outputBase,
    ];

    const bfgCommand = `${config.bfgBinary} ${bfgArgs.join(' ')}`;
    await logToFile(logPath, `\n=== Stage 1: BFG Generation ===`);
    await logToFile(logPath, `Command: ${bfgCommand}`);

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

    await logToFile(logPath, `Exit code: ${bfgExitCode}`);
    await logToFile(logPath, `STDOUT:\n${bfgStdout || '(empty)'}`);
    await logToFile(logPath, `STDERR:\n${bfgStderr || '(empty)'}`);

    if (bfgExitCode !== 0) {
      await logToFile(logPath, `ERROR: BFG generation failed`);
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

    const proto2gdsCommand = `${config.proto2gdsBinary} ${proto2gdsArgs.join(' ')}`;
    await logToFile(logPath, `\n=== Stage 2: Proto2GDS Conversion ===`);
    await logToFile(logPath, `Command: ${proto2gdsCommand}`);

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

    await logToFile(logPath, `Exit code: ${proto2gdsExitCode}`);
    await logToFile(logPath, `STDOUT:\n${proto2gdsStdout || '(empty)'}`);
    await logToFile(logPath, `STDERR:\n${proto2gdsStderr || '(empty)'}`);

    if (proto2gdsExitCode !== 0) {
      await logToFile(logPath, `ERROR: GDS conversion failed`);
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

    const gds2svgCommand = `python3 ${scriptPath}`;
    await logToFile(logPath, `\n=== Stage 3: GDS2SVG Conversion ===`);
    await logToFile(logPath, `Command: ${gds2svgCommand}`);

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

    await logToFile(logPath, `Exit code: ${gds2svgExitCode}`);
    await logToFile(logPath, `STDOUT:\n${gds2svgStdout || '(empty)'}`);
    await logToFile(logPath, `STDERR:\n${gds2svgStderr || '(empty)'}`);

    if (gds2svgExitCode !== 0) {
      await logToFile(logPath, `ERROR: SVG conversion failed`);
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

    await logToFile(logPath, `\n=== Generation Complete ===`);
    await logToFile(logPath, `Output files generated in: ${workDir}`);

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
    const errorMessage = error instanceof Error ? error.message : 'Unknown error';
    // Try to log the error, but don't fail if workDir wasn't created
    try {
      await logToFile(logPath, `\n=== ERROR ===`);
      await logToFile(logPath, `Stage: setup`);
      await logToFile(logPath, `Error: ${errorMessage}`);
    } catch {
      // Ignore logging errors if directory doesn't exist
    }
    sendEvent('error', {
      stage: 'setup',
      message: errorMessage,
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

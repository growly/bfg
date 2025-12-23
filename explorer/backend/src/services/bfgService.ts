// Service for invoking the BFG CLI to generate layouts

import { exec } from 'child_process';
import { promisify } from 'util';
import fs from 'fs/promises';
import path from 'path';
import { v4 as uuidv4 } from 'uuid';
import { config } from '../config/paths.js';
import { formatProtoText } from '../utils/protoFormatter.js';

const execAsync = promisify(exec);

export interface BfgOutput {
  workDir: string;
  libraryPb: string;
  libraryPbTxt: string;
  packagePbTxt: string;
}

export class BfgError extends Error {
  constructor(
    message: string,
    public stderr?: string,
    public stdout?: string
  ) {
    super(message);
    this.name = 'BfgError';
  }
}

export async function generateLayout(
  generator: string,
  params: Record<string, unknown>
): Promise<BfgOutput> {
  // Create temp directory with unique session ID
  const sessionId = uuidv4();
  // Ensure tmpDir is absolute
  const tmpDirAbsolute = path.resolve(config.tmpDir);
  const workDir = path.join(tmpDirAbsolute, sessionId);

  try {
    await fs.mkdir(workDir, { recursive: true });

    // Write parameters to .pb.txt file
    const paramsPath = path.join(workDir, 'params.pb.txt');
    const protoText = formatProtoText(params);
    await fs.writeFile(paramsPath, protoText, 'utf-8');

    // Prepare output paths
    const outputBase = path.join(workDir, 'output');

    // Invoke BFG CLI
    const bfgArgs = [
      '--technology', path.resolve(config.technologyPb),
      '--primitives', path.resolve(config.primitivesPb),
      '--external_circuits', path.resolve(config.sky130hdPb),
      '--write_text_format',
      '--run_generator', generator,
      '--params', paramsPath,
      '--output_library', outputBase,
    ];

    const command = `${config.bfgBinary} ${bfgArgs.join(' ')}`;

    console.log(`Executing BFG: ${command}`);

    const { stdout, stderr } = await execAsync(command, {
      cwd: workDir,
      maxBuffer: 10 * 1024 * 1024, // 10MB buffer
    });

    console.log(`BFG stdout: ${stdout}`);
    if (stderr) {
      console.warn(`BFG stderr: ${stderr}`);
    }

    // Read output files
    const libraryPb = `${outputBase}.library.pb`;
    const libraryPbTxt = `${outputBase}.library.pb.txt`;
    const packagePbTxt = `${outputBase}.package.pb.txt`;

    // Verify files exist
    try {
      await fs.access(libraryPb);
    } catch {
      throw new BfgError(
        `BFG did not generate expected output file: ${libraryPb}`,
        stderr,
        stdout
      );
    }

    // Read text format files
    let libraryText = '';
    let packageText = '';

    try {
      libraryText = await fs.readFile(libraryPbTxt, 'utf-8');
    } catch (error) {
      console.warn(`Could not read library text format: ${error}`);
    }

    try {
      packageText = await fs.readFile(packagePbTxt, 'utf-8');
    } catch (error) {
      console.warn(`Could not read package text format: ${error}`);
    }

    return {
      workDir,
      libraryPb,
      libraryPbTxt: libraryText,
      packagePbTxt: packageText,
    };
  } catch (error) {
    // Clean up on error
    try {
      await fs.rm(workDir, { recursive: true, force: true });
    } catch (cleanupError) {
      console.error(`Failed to cleanup work directory: ${cleanupError}`);
    }

    if (error instanceof BfgError) {
      throw error;
    }

    if (error instanceof Error && 'code' in error && error.code === 'ENOENT') {
      throw new BfgError(
        `BFG binary not found at: ${config.bfgBinary}. Please build BFG or update the BFG_BINARY environment variable.`
      );
    }

    throw new BfgError(
      `BFG generation failed: ${error instanceof Error ? error.message : String(error)}`,
      error instanceof Error && 'stderr' in error ? String(error.stderr) : undefined,
      error instanceof Error && 'stdout' in error ? String(error.stdout) : undefined
    );
  }
}

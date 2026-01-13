// Service for converting VLSIR protobuf to GDS format using proto2gds

import { exec } from 'child_process';
import { promisify } from 'util';
import fs from 'fs/promises';
import path from 'path';
import { config } from '../config/paths.js';

const execAsync = promisify(exec);

export class Proto2GdsError extends Error {
  constructor(
    message: string,
    public stderr?: string,
    public stdout?: string
  ) {
    super(message);
    this.name = 'Proto2GdsError';
  }
}

export interface Proto2GdsOutput {
  gdsPath: string;
  stdout: string;
  stderr: string;
}

export async function convertToGds(
  libraryPbPath: string,
  workDir: string
): Promise<Proto2GdsOutput> {
  const gdsPath = path.join(workDir, 'output.gds');

  try {
    const args = [
      '-i', libraryPbPath,
      '-t', config.technologyPb,
      '-o', gdsPath,
    ];

    const command = `${config.proto2gdsBinary} ${args.join(' ')}`;

    console.log(`Executing proto2gds: ${command}`);

    const { stdout, stderr } = await execAsync(command, {
      cwd: workDir,
      maxBuffer: 10 * 1024 * 1024,
    });

    if (stdout) {
      console.log(`proto2gds stdout: ${stdout}`);
    }
    if (stderr) {
      console.warn(`proto2gds stderr: ${stderr}`);
    }

    // Verify GDS file was created
    try {
      await fs.access(gdsPath);
    } catch {
      throw new Proto2GdsError(
        `proto2gds did not generate expected output file: ${gdsPath}`,
        stderr,
        stdout
      );
    }

    return { gdsPath, stdout, stderr };
  } catch (error) {
    if (error instanceof Proto2GdsError) {
      throw error;
    }

    if (error instanceof Error && 'code' in error && error.code === 'ENOENT') {
      throw new Proto2GdsError(
        `proto2gds binary not found at: ${config.proto2gdsBinary}. ` +
        `Please build Layout21 or update the PROTO2GDS_BINARY environment variable.`
      );
    }

    throw new Proto2GdsError(
      `proto2gds conversion failed: ${error instanceof Error ? error.message : String(error)}`,
      error instanceof Error && 'stderr' in error ? String(error.stderr) : undefined,
      error instanceof Error && 'stdout' in error ? String(error.stdout) : undefined
    );
  }
}

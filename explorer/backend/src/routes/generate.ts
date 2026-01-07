// API route for generating layouts

import express, { Request, Response } from 'express';
import { generateLayout, BfgError } from '../services/bfgService.js';
import { convertToGds, Proto2GdsError } from '../services/proto2gdsService.js';
import { convertToSvg, Gds2SvgError } from '../services/gds2svgService.js';

const router = express.Router();

interface GenerateRequest {
  generator: string;
  parameters: Record<string, unknown>;
}

interface GenerateSuccessResponse {
  status: 'success';
  data: {
    svg: string;
    libraryProto: string;
    packageProto: string;
    stdout: string;
    stderr: string;
  };
}

interface GenerateErrorResponse {
  status: 'error';
  message: string;
  details?: string;
}

router.post('/', async (req: Request, res: Response) => {
  try {
    const { generator, parameters } = req.body as GenerateRequest;

    // Validate request
    if (!generator) {
      const errorResponse: GenerateErrorResponse = {
        status: 'error',
        message: 'Generator name is required',
      };
      return res.status(400).json(errorResponse);
    }

    if (!parameters || typeof parameters !== 'object') {
      const errorResponse: GenerateErrorResponse = {
        status: 'error',
        message: 'Parameters object is required',
      };
      return res.status(400).json(errorResponse);
    }

    console.log(`Generating ${generator} with parameters:`, parameters);

    // Step 1: Generate layout with BFG
    const bfgOutput = await generateLayout(generator, parameters);

    // Step 2: Convert VLSIR to GDS
    const gdsOutput = await convertToGds(bfgOutput.libraryPb, bfgOutput.workDir);

    // Step 3: Convert GDS to SVG
    // Use generator name as cell name (BFG typically names the top cell after the generator)
    const svgOutput = await convertToSvg(gdsOutput.gdsPath, generator, bfgOutput.workDir);

    // Combine all stdout and stderr from the three steps
    const combinedStdout = [
      '=== BFG Generation ===',
      bfgOutput.stdout,
      '\n=== VLSIR to GDS Conversion ===',
      gdsOutput.stdout,
      '\n=== GDS to SVG Conversion ===',
      svgOutput.stdout,
    ].join('\n');

    const combinedStderr = [
      '=== BFG Generation ===',
      bfgOutput.stderr,
      '\n=== VLSIR to GDS Conversion ===',
      gdsOutput.stderr,
      '\n=== GDS to SVG Conversion ===',
      svgOutput.stderr,
    ].join('\n');

    // Return success response
    const successResponse: GenerateSuccessResponse = {
      status: 'success',
      data: {
        svg: svgOutput.svgContent,
        libraryProto: bfgOutput.libraryPbTxt,
        packageProto: bfgOutput.packagePbTxt,
        stdout: combinedStdout,
        stderr: combinedStderr,
      },
    };

    res.json(successResponse);
  } catch (error) {
    console.error('Generation error:', error);

    let message = 'Unknown error occurred';
    let details: string | undefined;

    if (error instanceof BfgError) {
      message = `BFG generation failed: ${error.message}`;
      details = error.stderr || error.stdout;
    } else if (error instanceof Proto2GdsError) {
      message = `GDS conversion failed: ${error.message}`;
      details = error.stderr || error.stdout;
    } else if (error instanceof Gds2SvgError) {
      message = `SVG conversion failed: ${error.message}`;
      details = error.stderr || error.stdout;
    } else if (error instanceof Error) {
      message = error.message;
    }

    const errorResponse: GenerateErrorResponse = {
      status: 'error',
      message,
      details,
    };

    res.status(500).json(errorResponse);
  }
});

export default router;

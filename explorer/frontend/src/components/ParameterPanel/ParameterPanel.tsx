import React, { useState, useEffect } from 'react';
import { apiClient } from '../../api/client';
import { GenerationResult } from '../Layout/TwoPaneLayout';
import { getGeneratorSchema } from '../../schemas/parameterSchemas';
import ParameterFormField from './ParameterFormField';
import './ParameterPanel.css';

interface ParameterPanelProps {
  onGenerationComplete: (result: GenerationResult) => void;
  onGenerationError: (error: string, errorDetails?: string) => void;
  onGenerationStart: () => void;
  onGenerationEnd: () => void;
  onStreamOutput: (stdout: string, stderr: string) => void;
}

const generators = [
  'Sky130TransmissionGate',
  'Sky130TransmissionGateStack',
  'Sky130InterconnectMux1',
  'Sky130InterconnectMux2',
  'Sky130Decap',
  'LutB',
  'Interconnect',
  'InterconnectWireBlock',
  'Slice',
] as const;

const ParameterPanel: React.FC<ParameterPanelProps> = ({
  onGenerationComplete,
  onGenerationError,
  onGenerationStart,
  onGenerationEnd,
  onStreamOutput,
}) => {
  const [selectedGenerator, setSelectedGenerator] = useState<string>('Sky130Decap');
  const [parameters, setParameters] = useState<Record<string, unknown>>({});
  const [isInitialLoad, setIsInitialLoad] = useState<boolean>(true);

  // Load default parameters when generator changes
  useEffect(() => {
    const schema = getGeneratorSchema(selectedGenerator);
    if (schema) {
      setParameters(schema.exampleParams);
    }
  }, [selectedGenerator]);

  const handleGenerate = async () => {
    const API_BASE_URL = import.meta.env.VITE_API_URL || '/api';
    let accumulatedStdout = '';
    let accumulatedStderr = '';

    try {
      onGenerationStart();

      const response = await fetch(`${API_BASE_URL}/generate-stream`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          generator: selectedGenerator,
          parameters,
        }),
      });

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const reader = response.body?.getReader();
      const decoder = new TextDecoder();

      if (!reader) {
        throw new Error('No response body');
      }

      let buffer = '';
      let currentEvent = '';

      while (true) {
        const { done, value } = await reader.read();
        if (done) break;

        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split('\n');
        buffer = lines.pop() || '';

        for (const line of lines) {
          if (line.startsWith('event:')) {
            currentEvent = line.substring(6).trim();
          } else if (line.startsWith('data:')) {
            try {
              const data = JSON.parse(line.substring(5).trim());

              if (currentEvent === 'stdout') {
                accumulatedStdout += data.data;
                onStreamOutput(accumulatedStdout, accumulatedStderr);
              } else if (currentEvent === 'stderr') {
                accumulatedStderr += data.data;
                onStreamOutput(accumulatedStdout, accumulatedStderr);
              } else if (currentEvent === 'stage') {
                // Stage change - could display this
                console.log(`Stage: ${data.stage} - ${data.message}`);
              } else if (currentEvent === 'complete') {
                onGenerationComplete({
                  svg: data.svg,
                  libraryProto: data.libraryProto,
                  packageProto: data.packageProto,
                  stdout: data.stdout,
                  stderr: data.stderr,
                });
                return;
              } else if (currentEvent === 'error') {
                onGenerationError(data.message, data.stderr || data.stdout || data.details);
                return;
              }
            } catch (e) {
              console.error('Failed to parse SSE data:', e);
            }
          } else if (line === '') {
            // Empty line resets event
            currentEvent = '';
          }
        }
      }

    } catch (error: any) {
      onGenerationError(
        error instanceof Error ? error.message : 'Failed to generate layout',
        accumulatedStderr || error.toString()
      );
    } finally {
      onGenerationEnd();
    }
  };

  const handleParameterChange = (key: string, value: unknown) => {
    setParameters((prev) => {
      const newParams = { ...prev };
      if (value === undefined || value === '') {
        delete newParams[key];
      } else {
        newParams[key] = value;
      }
      return newParams;
    });
  };

  const handleLoadExample = () => {
    const schema = getGeneratorSchema(selectedGenerator);
    if (schema) {
      setParameters(schema.exampleParams);
      // Generate after loading example
      setTimeout(() => handleGenerate(), 0);
    }
  };

  // Auto-generate when generator changes
  useEffect(() => {
    // Skip auto-generation on initial mount
    if (isInitialLoad) {
      setIsInitialLoad(false);
      return;
    }

    // Generate when generator changes
    handleGenerate();
  }, [selectedGenerator, isInitialLoad]);

  const handleFieldComplete = () => {
    // Generate when a field change is complete
    handleGenerate();
  };

  const schema = getGeneratorSchema(selectedGenerator);

  return (
    <div className="parameter-panel">
      <h2>Generator Parameters</h2>

      <div className="form-group">
        <label htmlFor="generator-select">Generator:</label>
        <select
          id="generator-select"
          value={selectedGenerator}
          onChange={(e) => {
            setSelectedGenerator(e.target.value);
          }}
        >
          {generators.map((gen) => (
            <option key={gen} value={gen}>
              {getGeneratorSchema(gen)?.displayName || gen}
            </option>
          ))}
        </select>
      </div>

      {schema && schema.description && (
        <p className="generator-description">{schema.description}</p>
      )}

      {schema && schema.fields.length > 0 ? (
        <div className="parameter-form">
          {schema.fields.map((field) => (
            <ParameterFormField
              key={field.name}
              field={field}
              value={parameters[field.name]}
              onChange={(value) => handleParameterChange(field.name, value)}
              onComplete={handleFieldComplete}
            />
          ))}
        </div>
      ) : (
        <div className="no-parameters">
          <p>This generator has no configurable parameters.</p>
        </div>
      )}

      <div className="button-group">
        {schema && schema.fields.length > 0 && (
          <button
            onClick={handleLoadExample}
            className="secondary-button"
          >
            Load Example
          </button>
        )}
        <button
          onClick={handleGenerate}
          className="generate-button"
        >
          Generate
        </button>
      </div>
    </div>
  );
};

export default ParameterPanel;

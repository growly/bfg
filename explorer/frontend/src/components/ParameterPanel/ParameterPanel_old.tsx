import React, { useState } from 'react';
import { apiClient } from '../../api/client';
import { GenerationResult } from '../Layout/TwoPaneLayout';
import './ParameterPanel.css';

interface ParameterPanelProps {
  onGenerationComplete: (result: GenerationResult) => void;
  onGenerationError: (error: string) => void;
  onGenerationStart: () => void;
  onGenerationEnd: () => void;
}

const generators = [
  'Sky130TransmissionGate',
  'Sky130TransmissionGateStack',
  'Sky130InterconnectMux1',
  'Sky130Decap',
  'LutB',
  'Interconnect',
  'Slice',
] as const;

const ParameterPanel: React.FC<ParameterPanelProps> = ({
  onGenerationComplete,
  onGenerationError,
  onGenerationStart,
  onGenerationEnd,
}) => {
  const [selectedGenerator, setSelectedGenerator] = useState<string>('Sky130TransmissionGate');
  const [parameters, setParameters] = useState<Record<string, unknown>>({});

  const handleGenerate = async () => {
    try {
      onGenerationStart();

      const response = await apiClient.generate({
        generator: selectedGenerator,
        parameters,
      });

      if (response.status === 'success' && response.data) {
        onGenerationComplete(response.data);
      } else {
        onGenerationError(response.message || 'Unknown error');
      }
    } catch (error) {
      onGenerationError(
        error instanceof Error ? error.message : 'Failed to generate layout'
      );
    } finally {
      onGenerationEnd();
    }
  };

  const handleParameterChange = (key: string, value: unknown) => {
    setParameters((prev) => ({
      ...prev,
      [key]: value,
    }));
  };

  const renderSimpleForm = () => {
    // Simple form for initial testing
    return (
      <div className="simple-form">
        <p>Selected Generator: {selectedGenerator}</p>
        <p style={{ fontSize: '0.9em', color: '#666' }}>
          Parameters can be edited via JSON for now:
        </p>
        <textarea
          value={JSON.stringify(parameters, null, 2)}
          onChange={(e) => {
            try {
              const parsed = JSON.parse(e.target.value);
              setParameters(parsed);
            } catch {
              // Invalid JSON, ignore
            }
          }}
          rows={15}
          style={{ width: '100%', fontFamily: 'monospace', fontSize: '0.9em' }}
        />
      </div>
    );
  };

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
            setParameters({}); // Reset parameters on generator change
          }}
        >
          {generators.map((gen) => (
            <option key={gen} value={gen}>
              {gen}
            </option>
          ))}
        </select>
      </div>

      {renderSimpleForm()}

      <button
        onClick={handleGenerate}
        className="generate-button"
        style={{ marginTop: '1rem' }}
      >
        Generate
      </button>
    </div>
  );
};

export default ParameterPanel;

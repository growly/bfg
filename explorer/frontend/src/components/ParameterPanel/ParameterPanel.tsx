import React, { useState, useEffect } from 'react';
import { apiClient } from '../../api/client';
import { GenerationResult } from '../Layout/TwoPaneLayout';
import { getGeneratorSchema } from '../../schemas/parameterSchemas';
import ParameterFormField from './ParameterFormField';
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

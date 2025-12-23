import React, { useState } from 'react';
import ParameterPanel from '../ParameterPanel/ParameterPanel';
import VisualizationPanel from '../VisualizationPanel/VisualizationPanel';
import './TwoPaneLayout.css';

export interface GenerationResult {
  svg: string;
  libraryProto: string;
  packageProto: string;
}

const TwoPaneLayout: React.FC = () => {
  const [result, setResult] = useState<GenerationResult | null>(null);
  const [isGenerating, setIsGenerating] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const handleGenerationComplete = (newResult: GenerationResult) => {
    setResult(newResult);
    setError(null);
  };

  const handleGenerationError = (errorMessage: string) => {
    setError(errorMessage);
    setResult(null);
  };

  const handleGenerationStart = () => {
    setIsGenerating(true);
    setError(null);
  };

  const handleGenerationEnd = () => {
    setIsGenerating(false);
  };

  return (
    <div className="two-pane-layout">
      <div className="left-pane">
        <ParameterPanel
          onGenerationComplete={handleGenerationComplete}
          onGenerationError={handleGenerationError}
          onGenerationStart={handleGenerationStart}
          onGenerationEnd={handleGenerationEnd}
        />
      </div>
      <div className="right-pane">
        <VisualizationPanel
          result={result}
          isLoading={isGenerating}
          error={error}
        />
      </div>
    </div>
  );
};

export default TwoPaneLayout;

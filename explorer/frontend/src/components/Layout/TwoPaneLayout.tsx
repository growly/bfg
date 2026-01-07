import React, { useState } from 'react';
import ParameterPanel from '../ParameterPanel/ParameterPanel';
import VisualizationPanel from '../VisualizationPanel/VisualizationPanel';
import OutputPanel from '../OutputPanel/OutputPanel';
import './TwoPaneLayout.css';

export interface GenerationResult {
  svg: string;
  libraryProto: string;
  packageProto: string;
  stdout: string;
  stderr: string;
}

const TwoPaneLayout: React.FC = () => {
  const [result, setResult] = useState<GenerationResult | null>(null);
  const [isGenerating, setIsGenerating] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [showOutputPane, setShowOutputPane] = useState(false);
  const [currentOutput, setCurrentOutput] = useState<{ stdout: string; stderr: string }>({ stdout: '', stderr: '' });

  const handleGenerationComplete = (newResult: GenerationResult) => {
    setResult(newResult);
    setCurrentOutput({ stdout: newResult.stdout, stderr: newResult.stderr });
    setError(null);
    // Hide output pane on successful generation
    setShowOutputPane(false);
  };

  const handleGenerationError = (errorMessage: string, errorDetails?: string) => {
    setError(errorMessage);
    // Update output with error details if available
    if (errorDetails) {
      setCurrentOutput({ stdout: '', stderr: errorDetails });
    }
    // Keep output pane visible on error
    setShowOutputPane(true);
  };

  const handleGenerationStart = () => {
    setIsGenerating(true);
    setError(null);
    // Clear current output and show output pane when generation starts
    setCurrentOutput({ stdout: '', stderr: '' });
    setShowOutputPane(true);
  };

  const handleGenerationEnd = () => {
    setIsGenerating(false);
  };

  const toggleOutputPane = () => {
    setShowOutputPane(!showOutputPane);
  };

  return (
    <div className={`two-pane-layout ${showOutputPane ? 'with-output-pane' : ''}`}>
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
        <button
          className={`toggle-output-button ${error ? 'has-error' : ''}`}
          onClick={toggleOutputPane}
          title={showOutputPane ? 'Hide output panel' : 'Show output panel'}
        >
          <span>{showOutputPane ? '▼' : '▲'}</span>
          <span>Tool Output</span>
          {error && <span>⚠</span>}
        </button>
      </div>
      {showOutputPane && (
        <div className="output-pane">
          <OutputPanel
            stdout={currentOutput.stdout}
            stderr={currentOutput.stderr}
            isGenerating={isGenerating}
            onClose={() => setShowOutputPane(false)}
          />
        </div>
      )}
    </div>
  );
};

export default TwoPaneLayout;

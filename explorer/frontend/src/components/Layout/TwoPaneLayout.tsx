import React, { useState, useCallback, useEffect, useRef } from 'react';
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
  const [outputPaneHeight, setOutputPaneHeight] = useState(250);
  const isDragging = useRef(false);
  const layoutRef = useRef<HTMLDivElement>(null);

  const handleMouseMove = useCallback((e: MouseEvent) => {
    if (!isDragging.current || !layoutRef.current) return;
    const layoutRect = layoutRef.current.getBoundingClientRect();
    const newHeight = layoutRect.bottom - e.clientY;
    const maxHeight = layoutRect.height * 0.7;
    setOutputPaneHeight(Math.max(100, Math.min(newHeight, maxHeight)));
  }, []);

  const handleMouseUp = useCallback(() => {
    isDragging.current = false;
    document.body.style.cursor = '';
    document.body.style.userSelect = '';
  }, []);

  useEffect(() => {
    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);
    return () => {
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, [handleMouseMove, handleMouseUp]);

  const handleDragStart = (e: React.MouseEvent) => {
    e.preventDefault();
    isDragging.current = true;
    document.body.style.cursor = 'row-resize';
    document.body.style.userSelect = 'none';
  };

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

  const handleStreamOutput = (stdout: string, stderr: string) => {
    setCurrentOutput({ stdout, stderr });
  };

  const toggleOutputPane = () => {
    setShowOutputPane(!showOutputPane);
  };

  return (
    <div
      ref={layoutRef}
      className={`two-pane-layout ${showOutputPane ? 'with-output-pane' : ''}`}
      style={showOutputPane ? { gridTemplateRows: `1fr ${outputPaneHeight}px` } : undefined}
    >
      <div className="left-pane">
        <ParameterPanel
          onGenerationComplete={handleGenerationComplete}
          onGenerationError={handleGenerationError}
          onGenerationStart={handleGenerationStart}
          onGenerationEnd={handleGenerationEnd}
          onStreamOutput={handleStreamOutput}
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
          <div className="resize-handle" onMouseDown={handleDragStart} />
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

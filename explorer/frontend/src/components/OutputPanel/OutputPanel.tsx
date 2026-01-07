import React, { useEffect, useRef } from 'react';
import './OutputPanel.css';

interface OutputPanelProps {
  stdout: string;
  stderr: string;
  isGenerating: boolean;
  onClose: () => void;
}

const OutputPanel: React.FC<OutputPanelProps> = ({
  stdout,
  stderr,
  isGenerating,
  onClose,
}) => {
  const stderrRef = useRef<HTMLPreElement>(null);
  const stdoutRef = useRef<HTMLPreElement>(null);

  // Auto-scroll to bottom when new output arrives
  useEffect(() => {
    if (stderrRef.current) {
      stderrRef.current.scrollTop = stderrRef.current.scrollHeight;
    }
  }, [stderr]);

  useEffect(() => {
    if (stdoutRef.current) {
      stdoutRef.current.scrollTop = stdoutRef.current.scrollHeight;
    }
  }, [stdout]);
  return (
    <div className="output-panel">
      <div className="output-header">
        <h3>Tool Output</h3>
        {isGenerating && (
          <div className="generating-indicator" style={{ flex: 1, marginLeft: '1rem', padding: '0.25rem 1rem' }}>
            <div className="spinner-small"></div>
            <span>Generation in progress...</span>
          </div>
        )}
        <button onClick={onClose} className="close-button" title="Close">
          ✕
        </button>
      </div>
      <div className="output-content">
        {stderr && (
          <div className="output-section">
            <h4>Standard Error</h4>
            <pre ref={stderrRef} className="output-text stderr">{stderr}</pre>
          </div>
        )}
        {stdout && (
          <div className="output-section">
            <h4>Standard Output</h4>
            <pre ref={stdoutRef} className="output-text stdout">{stdout}</pre>
          </div>
        )}
        {!stdout && !stderr && !isGenerating && (
          <div className="no-output">No output available</div>
        )}
      </div>
    </div>
  );
};

export default OutputPanel;

import React from 'react';
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
  return (
    <div className="output-panel">
      <div className="output-header">
        <h3>Tool Output</h3>
        <button onClick={onClose} className="close-button" title="Close">
          ✕
        </button>
      </div>
      <div className="output-content">
        {isGenerating && (
          <div className="generating-indicator">
            <div className="spinner-small"></div>
            <span>Generation in progress...</span>
          </div>
        )}
        {stderr && (
          <div className="output-section">
            <h4>Standard Error</h4>
            <pre className="output-text stderr">{stderr}</pre>
          </div>
        )}
        {stdout && (
          <div className="output-section">
            <h4>Standard Output</h4>
            <pre className="output-text stdout">{stdout}</pre>
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

import React, { useState } from 'react';
import { GenerationResult } from '../Layout/TwoPaneLayout';
import './VisualizationPanel.css';

interface VisualizationPanelProps {
  result: GenerationResult | null;
  isLoading: boolean;
  error: string | null;
}

type TabType = 'graphic' | 'proto';

const VisualizationPanel: React.FC<VisualizationPanelProps> = ({
  result,
  isLoading,
  error,
}) => {
  const [activeTab, setActiveTab] = useState<TabType>('graphic');

  if (isLoading) {
    return (
      <div className="visualization-panel">
        <div className="loading">
          <div className="spinner"></div>
          <p>Generating layout...</p>
        </div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="visualization-panel">
        <div className="error">
          <h3>Error</h3>
          <p>{error}</p>
        </div>
      </div>
    );
  }

  if (!result) {
    return (
      <div className="visualization-panel">
        <div className="empty-state">
          <p>Select a generator and click "Generate" to see results</p>
        </div>
      </div>
    );
  }

  return (
    <div className="visualization-panel">
      <div className="tabs">
        <button
          className={`tab ${activeTab === 'graphic' ? 'active' : ''}`}
          onClick={() => setActiveTab('graphic')}
        >
          Graphic
        </button>
        <button
          className={`tab ${activeTab === 'proto' ? 'active' : ''}`}
          onClick={() => setActiveTab('proto')}
        >
          Proto Text
        </button>
      </div>

      <div className="tab-content">
        {activeTab === 'graphic' && (
          <div className="graphic-view">
            <div
              className="svg-container"
              dangerouslySetInnerHTML={{ __html: result.svg }}
            />
          </div>
        )}

        {activeTab === 'proto' && (
          <div className="proto-view">
            <h3>Library Proto</h3>
            <pre className="proto-text">{result.libraryProto}</pre>

            <h3>Package Proto</h3>
            <pre className="proto-text">{result.packageProto}</pre>
          </div>
        )}
      </div>
    </div>
  );
};

export default VisualizationPanel;

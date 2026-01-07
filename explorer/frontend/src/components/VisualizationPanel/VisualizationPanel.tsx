import React, { useState, useRef, useEffect } from 'react';
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
  const [zoom, setZoom] = useState<number>(1);
  const [pan, setPan] = useState<{ x: number; y: number }>({ x: 0, y: 0 });
  const [isPanning, setIsPanning] = useState<boolean>(false);
  const [panStart, setPanStart] = useState<{ x: number; y: number }>({ x: 0, y: 0 });
  const svgContainerRef = useRef<HTMLDivElement>(null);

  // Reset view when switching results
  useEffect(() => {
    setZoom(1);
    setPan({ x: 0, y: 0 });
  }, [result]);

  const handleWheel = (e: React.WheelEvent<HTMLDivElement>) => {
    e.preventDefault();
    const delta = e.deltaY > 0 ? 0.9 : 1.1;
    const newZoom = Math.max(0.1, Math.min(10, zoom * delta));

    // Zoom towards cursor position
    if (svgContainerRef.current) {
      const rect = svgContainerRef.current.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const y = e.clientY - rect.top;

      const zoomRatio = newZoom / zoom;
      setPan({
        x: x - (x - pan.x) * zoomRatio,
        y: y - (y - pan.y) * zoomRatio,
      });
    }

    setZoom(newZoom);
  };

  const handleMouseDown = (e: React.MouseEvent<HTMLDivElement>) => {
    if (e.button === 0) { // Left mouse button
      setIsPanning(true);
      setPanStart({ x: e.clientX - pan.x, y: e.clientY - pan.y });
      e.preventDefault();
    }
  };

  const handleMouseMove = (e: React.MouseEvent<HTMLDivElement>) => {
    if (isPanning) {
      setPan({
        x: e.clientX - panStart.x,
        y: e.clientY - panStart.y,
      });
    }
  };

  const handleMouseUp = () => {
    setIsPanning(false);
  };

  const handleMouseLeave = () => {
    setIsPanning(false);
  };

  const handleReset = () => {
    setZoom(1);
    setPan({ x: 0, y: 0 });
  };

  const handleZoomIn = () => {
    setZoom(Math.min(10, zoom * 1.2));
  };

  const handleZoomOut = () => {
    setZoom(Math.max(0.1, zoom / 1.2));
  };

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
            <div className="zoom-controls">
              <button onClick={handleZoomIn} title="Zoom In">+</button>
              <button onClick={handleZoomOut} title="Zoom Out">−</button>
              <button onClick={handleReset} title="Reset View">⟲</button>
              <span className="zoom-level">{Math.round(zoom * 100)}%</span>
            </div>
            <div
              ref={svgContainerRef}
              className={`svg-container ${isPanning ? 'panning' : ''}`}
              onWheel={handleWheel}
              onMouseDown={handleMouseDown}
              onMouseMove={handleMouseMove}
              onMouseUp={handleMouseUp}
              onMouseLeave={handleMouseLeave}
            >
              <div
                className="svg-content"
                style={{
                  transform: `translate(${pan.x}px, ${pan.y}px) scale(${zoom})`,
                  transformOrigin: '0 0',
                }}
                dangerouslySetInnerHTML={{ __html: result.svg }}
              />
            </div>
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

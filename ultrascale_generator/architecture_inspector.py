#!/usr/bin/env python3
"""
Web-based inspector for Architecture objects.
Provides a Flask server to visualize and explore the state of an Architecture instance.
"""

from flask import Flask, jsonify, render_template_string
import threading
import webbrowser
from typing import Optional


class ArchitectureInspector:
    """Web-based inspector for Architecture objects."""

    def __init__(self, architecture, port=5000, debug=False):
        """
        Initialize the inspector.

        Args:
            architecture: The Architecture object to inspect
            port: Port to run the web server on (default: 5000)
            debug: Enable Flask debug mode (default: False)
        """
        self.architecture = architecture
        self.port = port
        self.debug = debug
        self.app = Flask(__name__)
        self._setup_routes()
        self._server_thread = None

    def _setup_routes(self):
        """Set up Flask routes."""

        @self.app.route('/')
        def index():
            """Serve the main inspection page."""
            return render_template_string(HTML_TEMPLATE)

        @self.app.route('/api/stats')
        def stats():
            """Get basic statistics about the architecture."""
            return jsonify({
                'interconnect_in': len(self.architecture.interconnect_by_name),
                'iib_s1_muxes': len(self.architecture.iib_s1_by_name),
                'iib_s2_muxes': len(self.architecture.iib_s2_by_name),
                'oib_s2_muxes': len(self.architecture.oib_s2_by_name),
                'oib_s1_muxes': len(self.architecture.oib_s1_by_name),
                'luts': len(self.architecture.luts) if self.architecture.luts else 0,
                'input_wires': len(self.architecture.input_wires),
                'output_wires': len(self.architecture.output_wires),
            })

        @self.app.route('/api/interconnect')
        def interconnect():
            """Get interconnect nodes."""
            nodes = []
            for name, node in self.architecture.interconnect_by_name.items():
                nodes.append({
                    'name': name,
                    'type': str(node.type) if hasattr(node, 'type') else 'Unknown',
                    'fan_in': node.fan_in() if hasattr(node, 'fan_in') else 0,
                    'fan_out': node.fan_out() if hasattr(node, 'fan_out') else 0,
                })
            return jsonify(nodes)

        @self.app.route('/api/iib_s1')
        def iib_s1():
            """Get IIB S1 muxes."""
            nodes = []
            for name, node in self.architecture.iib_s1_by_name.items():
                nodes.append({
                    'name': name,
                    'type': str(node.type) if hasattr(node, 'type') else 'Unknown',
                    'fan_in': node.fan_in() if hasattr(node, 'fan_in') else 0,
                    'fan_out': node.fan_out() if hasattr(node, 'fan_out') else 0,
                    'ordinal': node.ordinal if hasattr(node, 'ordinal') else None,
                })
            return jsonify(nodes)

        @self.app.route('/api/iib_s2')
        def iib_s2():
            """Get IIB S2 muxes."""
            nodes = []
            for name, node in self.architecture.iib_s2_by_name.items():
                nodes.append({
                    'name': name,
                    'type': str(node.type) if hasattr(node, 'type') else 'Unknown',
                    'fan_in': node.fan_in() if hasattr(node, 'fan_in') else 0,
                    'fan_out': node.fan_out() if hasattr(node, 'fan_out') else 0,
                    'ordinal': node.ordinal if hasattr(node, 'ordinal') else None,
                })
            return jsonify(nodes)

        @self.app.route('/api/oib_s2')
        def oib_s2():
            """Get OIB S2 muxes."""
            nodes = []
            for name, node in self.architecture.oib_s2_by_name.items():
                nodes.append({
                    'name': name,
                    'type': str(node.type) if hasattr(node, 'type') else 'Unknown',
                    'fan_in': node.fan_in() if hasattr(node, 'fan_in') else 0,
                    'fan_out': node.fan_out() if hasattr(node, 'fan_out') else 0,
                    'ordinal': node.ordinal if hasattr(node, 'ordinal') else None,
                })
            return jsonify(nodes)

        @self.app.route('/api/oib_s1')
        def oib_s1():
            """Get OIB S1 muxes."""
            nodes = []
            for name, node in self.architecture.oib_s1_by_name.items():
                nodes.append({
                    'name': name,
                    'type': str(node.type) if hasattr(node, 'type') else 'Unknown',
                    'fan_in': node.fan_in() if hasattr(node, 'fan_in') else 0,
                    'fan_out': node.fan_out() if hasattr(node, 'fan_out') else 0,
                })
            return jsonify(nodes)

        @self.app.route('/api/luts')
        def luts():
            """Get LUT information."""
            if not self.architecture.luts:
                return jsonify([])

            lut_data = []
            for lut in self.architecture.luts:
                lut_info = {
                    'name': lut.name if hasattr(lut, 'name') else f'LUT_{lut.index}',
                    'index': lut.index if hasattr(lut, 'index') else None,
                    'k': lut.k if hasattr(lut, 'k') else None,
                }
                lut_data.append(lut_info)
            return jsonify(lut_data)

        @self.app.route('/api/node/<node_name>')
        def node_detail(node_name):
            """Get detailed information about a specific node."""
            # Search in all node collections
            node = None
            node_dict = None

            collections = [
                ('interconnect', self.architecture.interconnect_by_name),
                ('iib_s1', self.architecture.iib_s1_by_name),
                ('iib_s2', self.architecture.iib_s2_by_name),
                ('oib_s2', self.architecture.oib_s2_by_name),
                ('oib_s1', self.architecture.oib_s1_by_name),
            ]

            for collection_name, collection in collections:
                if node_name in collection:
                    node = collection[node_name]
                    node_dict = collection_name
                    break

            if not node:
                return jsonify({'error': f'Node {node_name} not found'}), 404

            # Get inputs and outputs
            inputs = []
            outputs = []

            if hasattr(node, 'get_in_nodes'):
                for in_node in node.get_in_nodes():
                    inputs.append(in_node.name if hasattr(in_node, 'name') else str(in_node))

            if hasattr(node, 'get_out_nodes'):
                for out_node in node.get_out_nodes():
                    outputs.append(out_node.name if hasattr(out_node, 'name') else str(out_node))

            return jsonify({
                'name': node_name,
                'collection': node_dict,
                'type': str(node.type) if hasattr(node, 'type') else 'Unknown',
                'ordinal': node.ordinal if hasattr(node, 'ordinal') else None,
                'fan_in': node.fan_in() if hasattr(node, 'fan_in') else 0,
                'fan_out': node.fan_out() if hasattr(node, 'fan_out') else 0,
                'inputs': inputs,
                'outputs': outputs,
                'max_num_inputs': node.max_num_inputs if hasattr(node, 'max_num_inputs') else None,
            })

        @self.app.route('/api/io')
        def io_summary():
            """Get input/output wire summary."""
            return jsonify({
                'inputs': [wire.name if hasattr(wire, 'name') else str(wire)
                          for wire in self.architecture.input_wires[:100]],  # Limit for performance
                'outputs': [wire.name if hasattr(wire, 'name') else str(wire)
                           for wire in self.architecture.output_wires[:100]],
                'total_inputs': len(self.architecture.input_wires),
                'total_outputs': len(self.architecture.output_wires),
            })

    def start(self, open_browser=True):
        """
        Start the web server in a background thread.

        Args:
            open_browser: If True, automatically open the browser (default: True)
        """
        if self._server_thread and self._server_thread.is_alive():
            print(f"Inspector already running at http://localhost:{self.port}")
            return

        def run_server():
            self.app.run(host='0.0.0.0', port=self.port, debug=self.debug, use_reloader=False)

        self._server_thread = threading.Thread(target=run_server, daemon=True)
        self._server_thread.start()

        url = f"http://localhost:{self.port}"
        print(f"Architecture Inspector started at {url}")

        if open_browser:
            # Give the server a moment to start
            threading.Timer(1.0, lambda: webbrowser.open(url)).start()

    def run(self, open_browser=True):
        """
        Run the web server (blocking).

        Args:
            open_browser: If True, automatically open the browser (default: True)
        """
        url = f"http://localhost:{self.port}"
        print(f"Architecture Inspector running at {url}")

        if open_browser:
            threading.Timer(1.0, lambda: webbrowser.open(url)).start()

        self.app.run(host='0.0.0.0', port=self.port, debug=self.debug)


# HTML template for the inspection interface
HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <title>Architecture Inspector</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: #1e1e1e;
            color: #d4d4d4;
            padding: 20px;
        }

        h1 {
            color: #4ec9b0;
            margin-bottom: 10px;
        }

        h2 {
            color: #569cd6;
            margin-top: 30px;
            margin-bottom: 15px;
            border-bottom: 1px solid #3e3e42;
            padding-bottom: 5px;
        }

        .container {
            max-width: 1400px;
            margin: 0 auto;
        }

        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }

        .stat-card {
            background: #252526;
            border: 1px solid #3e3e42;
            padding: 20px;
            border-radius: 6px;
            transition: transform 0.2s;
        }

        .stat-card:hover {
            transform: translateY(-2px);
            border-color: #007acc;
        }

        .stat-label {
            color: #858585;
            font-size: 12px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-bottom: 8px;
        }

        .stat-value {
            color: #4ec9b0;
            font-size: 32px;
            font-weight: bold;
        }

        .section {
            background: #252526;
            border: 1px solid #3e3e42;
            border-radius: 6px;
            padding: 20px;
            margin: 20px 0;
        }

        .node-list {
            max-height: 400px;
            overflow-y: auto;
            margin-top: 10px;
        }

        .node-item {
            background: #1e1e1e;
            border: 1px solid #3e3e42;
            padding: 12px;
            margin: 8px 0;
            border-radius: 4px;
            cursor: pointer;
            transition: all 0.2s;
        }

        .node-item:hover {
            background: #2d2d30;
            border-color: #007acc;
        }

        .node-name {
            color: #dcdcaa;
            font-weight: bold;
            margin-bottom: 4px;
        }

        .node-meta {
            color: #858585;
            font-size: 12px;
        }

        .loading {
            text-align: center;
            padding: 40px;
            color: #858585;
        }

        .error {
            background: #3a1d1d;
            border: 1px solid #c72e2e;
            color: #f48771;
            padding: 15px;
            border-radius: 4px;
            margin: 10px 0;
        }

        .tabs {
            display: flex;
            gap: 5px;
            margin-bottom: 20px;
            border-bottom: 1px solid #3e3e42;
        }

        .tab {
            padding: 10px 20px;
            background: transparent;
            border: none;
            color: #858585;
            cursor: pointer;
            border-bottom: 2px solid transparent;
            transition: all 0.2s;
        }

        .tab:hover {
            color: #d4d4d4;
        }

        .tab.active {
            color: #4ec9b0;
            border-bottom-color: #4ec9b0;
        }

        .tab-content {
            display: none;
        }

        .tab-content.active {
            display: block;
        }

        .modal {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0,0,0,0.7);
        }

        .modal-content {
            background-color: #252526;
            margin: 5% auto;
            padding: 30px;
            border: 1px solid #3e3e42;
            border-radius: 6px;
            width: 80%;
            max-width: 800px;
            max-height: 80vh;
            overflow-y: auto;
        }

        .close {
            color: #858585;
            float: right;
            font-size: 28px;
            font-weight: bold;
            cursor: pointer;
        }

        .close:hover {
            color: #d4d4d4;
        }

        .detail-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px;
            margin: 20px 0;
        }

        .detail-item {
            background: #1e1e1e;
            padding: 12px;
            border-radius: 4px;
            border: 1px solid #3e3e42;
        }

        .detail-label {
            color: #858585;
            font-size: 12px;
            margin-bottom: 4px;
        }

        .detail-value {
            color: #4ec9b0;
            font-weight: bold;
        }

        .connection-list {
            list-style: none;
            max-height: 300px;
            overflow-y: auto;
        }

        .connection-list li {
            background: #1e1e1e;
            padding: 8px 12px;
            margin: 4px 0;
            border-radius: 4px;
            border-left: 3px solid #007acc;
            cursor: pointer;
        }

        .connection-list li:hover {
            background: #2d2d30;
        }

        ::-webkit-scrollbar {
            width: 10px;
        }

        ::-webkit-scrollbar-track {
            background: #1e1e1e;
        }

        ::-webkit-scrollbar-thumb {
            background: #3e3e42;
            border-radius: 5px;
        }

        ::-webkit-scrollbar-thumb:hover {
            background: #4e4e52;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Architecture Inspector</h1>
        <p style="color: #858585; margin-bottom: 20px;">Interactive visualization of Xilinx RapidWright routing graph architecture</p>

        <div id="stats" class="stats-grid">
            <div class="loading">Loading statistics...</div>
        </div>

        <div class="tabs">
            <button class="tab active" onclick="showTab('interconnect')">Interconnect</button>
            <button class="tab" onclick="showTab('iib_s1')">IIB S1</button>
            <button class="tab" onclick="showTab('iib_s2')">IIB S2</button>
            <button class="tab" onclick="showTab('oib_s2')">OIB S2</button>
            <button class="tab" onclick="showTab('oib_s1')">OIB S1</button>
            <button class="tab" onclick="showTab('luts')">LUTs</button>
            <button class="tab" onclick="showTab('io')">I/O</button>
        </div>

        <div id="interconnect" class="tab-content active section">
            <h2>Interconnect Nodes</h2>
            <div class="node-list" id="interconnect-list">
                <div class="loading">Loading...</div>
            </div>
        </div>

        <div id="iib_s1" class="tab-content section">
            <h2>IIB S1 Muxes</h2>
            <div class="node-list" id="iib_s1-list">
                <div class="loading">Loading...</div>
            </div>
        </div>

        <div id="iib_s2" class="tab-content section">
            <h2>IIB S2 Muxes</h2>
            <div class="node-list" id="iib_s2-list">
                <div class="loading">Loading...</div>
            </div>
        </div>

        <div id="oib_s2" class="tab-content section">
            <h2>OIB S2 Muxes</h2>
            <div class="node-list" id="oib_s2-list">
                <div class="loading">Loading...</div>
            </div>
        </div>

        <div id="oib_s1" class="tab-content section">
            <h2>OIB S1 Muxes</h2>
            <div class="node-list" id="oib_s1-list">
                <div class="loading">Loading...</div>
            </div>
        </div>

        <div id="luts" class="tab-content section">
            <h2>LUTs</h2>
            <div class="node-list" id="luts-list">
                <div class="loading">Loading...</div>
            </div>
        </div>

        <div id="io" class="tab-content section">
            <h2>Input/Output Wires</h2>
            <div id="io-content">
                <div class="loading">Loading...</div>
            </div>
        </div>
    </div>

    <!-- Modal for node details -->
    <div id="nodeModal" class="modal">
        <div class="modal-content">
            <span class="close" onclick="closeModal()">&times;</span>
            <div id="modal-body">
                <div class="loading">Loading...</div>
            </div>
        </div>
    </div>

    <script>
        // Load statistics
        async function loadStats() {
            try {
                const response = await fetch('/api/stats');
                const stats = await response.json();

                const statsHtml = Object.entries(stats).map(([key, value]) => `
                    <div class="stat-card">
                        <div class="stat-label">${key.replace(/_/g, ' ')}</div>
                        <div class="stat-value">${value.toLocaleString()}</div>
                    </div>
                `).join('');

                document.getElementById('stats').innerHTML = statsHtml;
            } catch (error) {
                document.getElementById('stats').innerHTML = `
                    <div class="error">Failed to load statistics: ${error.message}</div>
                `;
            }
        }

        // Load nodes for a specific category
        async function loadNodes(category, containerId) {
            try {
                const response = await fetch(`/api/${category}`);
                const nodes = await response.json();

                if (nodes.length === 0) {
                    document.getElementById(containerId).innerHTML = '<p style="color: #858585;">No nodes found</p>';
                    return;
                }

                const nodesHtml = nodes.slice(0, 100).map(node => `
                    <div class="node-item" onclick="showNodeDetails('${node.name}')">
                        <div class="node-name">${node.name}</div>
                        <div class="node-meta">
                            Type: ${node.type || 'N/A'} |
                            Fan In: ${node.fan_in || 0} |
                            Fan Out: ${node.fan_out || 0}
                            ${node.ordinal !== null && node.ordinal !== undefined ? ` | Ordinal: ${node.ordinal}` : ''}
                        </div>
                    </div>
                `).join('');

                const limitMessage = nodes.length > 100 ?
                    `<p style="color: #858585; margin-top: 10px;">Showing 100 of ${nodes.length} nodes</p>` : '';

                document.getElementById(containerId).innerHTML = nodesHtml + limitMessage;
            } catch (error) {
                document.getElementById(containerId).innerHTML = `
                    <div class="error">Failed to load nodes: ${error.message}</div>
                `;
            }
        }

        // Load I/O summary
        async function loadIO() {
            try {
                const response = await fetch('/api/io');
                const io = await response.json();

                const html = `
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 20px;">
                        <div>
                            <h3 style="color: #569cd6; margin-bottom: 10px;">Input Wires (${io.total_inputs})</h3>
                            <div class="node-list" style="max-height: 500px;">
                                ${io.inputs.map(name => `
                                    <div class="node-item" onclick="showNodeDetails('${name}')">
                                        <div class="node-name">${name}</div>
                                    </div>
                                `).join('')}
                                ${io.inputs.length < io.total_inputs ?
                                    `<p style="color: #858585; padding: 10px;">Showing first ${io.inputs.length} of ${io.total_inputs}</p>` : ''}
                            </div>
                        </div>
                        <div>
                            <h3 style="color: #569cd6; margin-bottom: 10px;">Output Wires (${io.total_outputs})</h3>
                            <div class="node-list" style="max-height: 500px;">
                                ${io.outputs.map(name => `
                                    <div class="node-item" onclick="showNodeDetails('${name}')">
                                        <div class="node-name">${name}</div>
                                    </div>
                                `).join('')}
                                ${io.outputs.length < io.total_outputs ?
                                    `<p style="color: #858585; padding: 10px;">Showing first ${io.outputs.length} of ${io.total_outputs}</p>` : ''}
                            </div>
                        </div>
                    </div>
                `;

                document.getElementById('io-content').innerHTML = html;
            } catch (error) {
                document.getElementById('io-content').innerHTML = `
                    <div class="error">Failed to load I/O: ${error.message}</div>
                `;
            }
        }

        // Show node details in modal
        async function showNodeDetails(nodeName) {
            const modal = document.getElementById('nodeModal');
            modal.style.display = 'block';
            document.getElementById('modal-body').innerHTML = '<div class="loading">Loading node details...</div>';

            try {
                const response = await fetch(`/api/node/${encodeURIComponent(nodeName)}`);
                const node = await response.json();

                if (node.error) {
                    document.getElementById('modal-body').innerHTML = `
                        <div class="error">${node.error}</div>
                    `;
                    return;
                }

                const html = `
                    <h2>${node.name}</h2>
                    <div class="detail-grid">
                        <div class="detail-item">
                            <div class="detail-label">Collection</div>
                            <div class="detail-value">${node.collection}</div>
                        </div>
                        <div class="detail-item">
                            <div class="detail-label">Type</div>
                            <div class="detail-value">${node.type}</div>
                        </div>
                        <div class="detail-item">
                            <div class="detail-label">Fan In</div>
                            <div class="detail-value">${node.fan_in}</div>
                        </div>
                        <div class="detail-item">
                            <div class="detail-label">Fan Out</div>
                            <div class="detail-value">${node.fan_out}</div>
                        </div>
                        ${node.ordinal !== null && node.ordinal !== undefined ? `
                        <div class="detail-item">
                            <div class="detail-label">Ordinal</div>
                            <div class="detail-value">${node.ordinal}</div>
                        </div>
                        ` : ''}
                        ${node.max_num_inputs !== null && node.max_num_inputs !== undefined ? `
                        <div class="detail-item">
                            <div class="detail-label">Max Inputs</div>
                            <div class="detail-value">${node.max_num_inputs}</div>
                        </div>
                        ` : ''}
                    </div>

                    <div style="margin-top: 30px;">
                        <h3 style="color: #569cd6; margin-bottom: 10px;">Inputs (${node.inputs.length})</h3>
                        ${node.inputs.length > 0 ? `
                            <ul class="connection-list">
                                ${node.inputs.map(input => `
                                    <li onclick="showNodeDetails('${input}')">${input}</li>
                                `).join('')}
                            </ul>
                        ` : '<p style="color: #858585;">No inputs</p>'}
                    </div>

                    <div style="margin-top: 20px;">
                        <h3 style="color: #569cd6; margin-bottom: 10px;">Outputs (${node.outputs.length})</h3>
                        ${node.outputs.length > 0 ? `
                            <ul class="connection-list">
                                ${node.outputs.map(output => `
                                    <li onclick="showNodeDetails('${output}')">${output}</li>
                                `).join('')}
                            </ul>
                        ` : '<p style="color: #858585;">No outputs</p>'}
                    </div>
                `;

                document.getElementById('modal-body').innerHTML = html;
            } catch (error) {
                document.getElementById('modal-body').innerHTML = `
                    <div class="error">Failed to load node details: ${error.message}</div>
                `;
            }
        }

        function closeModal() {
            document.getElementById('nodeModal').style.display = 'none';
        }

        // Close modal when clicking outside
        window.onclick = function(event) {
            const modal = document.getElementById('nodeModal');
            if (event.target == modal) {
                modal.style.display = 'none';
            }
        }

        // Tab switching
        function showTab(tabName) {
            // Hide all tabs
            document.querySelectorAll('.tab-content').forEach(tab => {
                tab.classList.remove('active');
            });
            document.querySelectorAll('.tab').forEach(tab => {
                tab.classList.remove('active');
            });

            // Show selected tab
            document.getElementById(tabName).classList.add('active');
            event.target.classList.add('active');

            // Load data if not already loaded
            const listId = `${tabName}-list`;
            const listElement = document.getElementById(listId);

            if (tabName === 'io') {
                const ioContent = document.getElementById('io-content');
                if (ioContent.querySelector('.loading')) {
                    loadIO();
                }
            } else if (listElement && listElement.querySelector('.loading')) {
                loadNodes(tabName, listId);
            }
        }

        // Initial load
        loadStats();
        loadNodes('interconnect', 'interconnect-list');
    </script>
</body>
</html>
"""


if __name__ == '__main__':
    # Example usage
    from architecture import Architecture

    arch = Architecture()
    arch.build_all()

    inspector = ArchitectureInspector(arch, port=9123)
    inspector.run()

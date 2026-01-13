# BFG Explorer - Quick Start Guide

## Prerequisites Check

Before starting, ensure you have:

1. **Python 3 with gdspy:**
   ```bash
   pip install gdspy
   python3 -c "import gdspy" && echo "✓ gdspy installed" || echo "✗ gdspy missing"
   ```

2. **BFG binary built:**
   ```bash
   ls /home/arya/src/bfg/build/bfg && echo "✓ BFG binary found" || echo "✗ BFG binary missing"
   ```

3. **proto2gds binary (optional but recommended):**
   ```bash
   ls /home/arya/src/Layout21/target/debug/proto2gds && echo "✓ proto2gds found" || echo "✗ proto2gds missing"
   ```

## Starting the Explorer

### Step 1: Start the Backend

Open a terminal and run:

```bash
cd /home/arya/src/bfg/explorer/backend
npm run dev
```

You should see:
```
BFG Explorer backend running on port 3000
Environment: development
BFG Binary: /home/arya/src/bfg/build/bfg
```

Keep this terminal open.

### Step 2: Start the Frontend

Open a **second terminal** and run:

```bash
cd /home/arya/src/bfg/explorer/frontend
npm run dev
```

You should see:
```
  VITE v5.x.x  ready in xxx ms

  ➜  Local:   http://localhost:5173/
  ➜  Network: use --host to expose
```

### Step 3: Open in Browser

Navigate to: **http://localhost:5173**

## Using the Explorer

### Basic Workflow

1. **Select a generator** from the dropdown (default: Sky130TransmissionGate)

2. **Enter parameters** in the JSON textarea. For example:
   ```json
   {
     "p_width_nm": 1000,
     "p_length_nm": 150,
     "n_width_nm": 650,
     "n_length_nm": 150
   }
   ```

3. **Click "Generate"**

4. **View the result:**
   - **Graphic tab:** Shows the SVG rendering of the generated layout
   - **Proto Text tab:** Shows the raw VLSIR protocol buffer output

### Example Parameters for Each Generator

#### Sky130Decap
```json
{
  "width_nm": 1380,
  "height_nm": 2720,
  "label_pins": true,
  "draw_overflowing_vias_and_pins": true
}
```

#### LutB
```json
{
  "lut_size": 4
}
```

#### Interconnect
```json
{
  "num_rows": 8,
  "num_columns": 16
}
```

#### Sky130TransmissionGate
```json
{
  "p_width_nm": 1000,
  "p_length_nm": 150,
  "n_width_nm": 650,
  "n_length_nm": 150,
  "poly_pitch_nm": 500,
  "vertical_tab_pitch_nm": 170,
  "vertical_tab_offset_nm": 170
}
```

#### Sky130TransmissionGateStack
```json
{
  "sequences": [
    {
      "nets": ["A", "B", "C"]
    }
  ],
  "p_width_nm": 1000,
  "p_length_nm": 150,
  "n_width_nm": 650,
  "n_length_nm": 150,
  "min_height_nm": 2720
}
```

#### Sky130InterconnectMux1
```json
{
  "num_inputs": 6,
  "num_outputs": 1,
  "poly_pitch_nm": 500,
  "vertical_pitch_nm": 340,
  "horizontal_pitch_nm": 460
}
```

#### Slice
```json
{}
```
(Slice has no parameters)

## Troubleshooting

### Error: "BFG binary not found"

The backend can't find the BFG executable. Fix:

```bash
# Build BFG if not already built
cd /home/arya/src/bfg
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Or update the path in explorer/backend/.env
nano explorer/backend/.env
# Change BFG_BINARY to correct path
```

### Error: "proto2gds binary not found"

The backend can't find proto2gds. Fix:

```bash
# Build Layout21 if not already built
cd /home/arya/src/Layout21
cargo build

# Or update the path in explorer/backend/.env
nano explorer/backend/.env
# Change PROTO2GDS_BINARY to correct path
```

### Error: "No module named 'gdspy'"

Python gdspy is not installed. Fix:

```bash
pip install gdspy
# Or use pip3 if pip points to Python 2
pip3 install gdspy
```

### Frontend shows connection error

Ensure the backend is running on port 3000:

```bash
# Check if backend is running
curl http://localhost:3000/api/health

# Should return: {"status":"ok","timestamp":"..."}
```

### Port conflicts

If ports 3000 or 5173 are already in use:

**Backend (port 3000):**
```bash
# Edit explorer/backend/.env
nano explorer/backend/.env
# Change: PORT=3001 (or any free port)
```

**Frontend (port 5173):**
```bash
# Edit explorer/frontend/vite.config.ts
nano explorer/frontend/vite.config.ts
# Change: server.port to 5174 (or any free port)
# Also update: server.proxy['/api'].target to match backend port
```

## Next Steps

1. **Try different generators** - Explore all 7 generators with various parameters

2. **Examine the proto text** - Switch to the "Proto Text" tab to see the raw VLSIR output

3. **Modify parameters** - Experiment with different values to see how they affect the layout

4. **Future enhancements:**
   - Visual form builder (coming soon!)
   - Parameter presets and examples
   - SVG pan/zoom controls
   - Download GDS files

## Getting Help

If you encounter issues:

1. Check the backend terminal for error messages
2. Check the frontend browser console (F12) for errors
3. Verify all prerequisites are installed
4. Ensure all file paths in `backend/.env` are correct

Enjoy exploring BFG generators!

# BFG Explorer

Web-based interactive tool for exploring BFG generator parameters and visualizing generated layouts.

## Features

- Interactive parameter editing for all 7 BFG generators
- Real-time visualization of generated layouts (SVG)
- View protocol buffer text output
- Two-pane interface: parameters (left) and visualization (right)

## Prerequisites

- Node.js >= 18.0.0
- Python 3 with `gdspy` installed: `pip install gdspy`
- BFG binary built at `/home/arya/src/bfg/build/bfg`
- proto2gds binary from Layout21 project

## Setup

1. Install dependencies:
```bash
cd explorer
npm install
```

2. Configure backend environment:
```bash
cd backend
cp .env.example .env
# Edit .env to set correct paths for BFG_BINARY, PROTO2GDS_BINARY, etc.
```

## Development

Run both frontend and backend in development mode:

```bash
# Terminal 1 - Backend
cd backend
npm run dev  # Starts on http://localhost:3000

# Terminal 2 - Frontend
cd frontend
npm run dev  # Starts on http://localhost:5173
```

The frontend will proxy API requests to the backend automatically.

## Production Build

```bash
npm run build
cd backend
npm start
```

The backend serves both the API and the frontend static files.

## Architecture

- **Frontend**: React + TypeScript + Vite
- **Backend**: Node.js + Express + TypeScript
- **Shared**: Common TypeScript types for parameters

## Supported Generators

1. Sky130TransmissionGate
2. Sky130TransmissionGateStack
3. Sky130InterconnectMux1
4. Sky130InterconnectMux2
5. Sky130Decap
6. LutB
7. Interconnect
8. Slice

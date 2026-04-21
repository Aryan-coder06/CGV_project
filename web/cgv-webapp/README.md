# CGV Web App

Web conversion of the CGV desktop visualizer with:
- Home page (3 module choices)
- Algorithm Visualizer UI (Theory + Visualize)
- Live DDA stepping controls
- Real-time 3D grid visualization using Three.js/WebGL

## Run

```bash
cd web/cgv-webapp
python3 -m http.server 8090
```

Open: `http://localhost:8090`

## Current status

- DDA is fully interactive in the web UI.
- Paint App and Graph Visualizer are scaffolded as placeholders for next implementation.

import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

const sidebar = document.getElementById('sidebar');
const viewportTop = document.getElementById('viewportTop');
const threeRoot = document.getElementById('threeRoot');

const ALGO_OPTIONS = [
  { id: 'dda', label: 'DDA (Digital Differential Analyzer)', enabled: true },
  { id: 'bresenham', label: "Bresenham's Line", enabled: false },
  { id: 'wu', label: "Xiaolin Wu's Anti-Aliased Line", enabled: false },
  { id: 'circle', label: 'Midpoint Circle', enabled: false },
  { id: 'fill', label: 'Flood Fill', enabled: false }
];

const state = {
  mode: 'home',
  panelTab: 'visualize',
  algo: 'dda',
  inputs: { x1: 0, y1: 0, x2: 10, y2: 8, k: 4 },
  engine: null,
};

class DDAEngine {
  constructor(x1, y1, x2, y2) {
    this.init(x1, y1, x2, y2);
  }

  init(x1, y1, x2, y2) {
    this.x1 = x1; this.y1 = y1; this.x2 = x2; this.y2 = y2;
    this.dx = x2 - x1;
    this.dy = y2 - y1;
    this.steps = Math.max(Math.abs(this.dx), Math.abs(this.dy));
    this.steps = this.steps === 0 ? 1 : this.steps;
    this.xInc = this.dx / this.steps;
    this.yInc = this.dy / this.steps;

    this.i = 0;
    this.totalSteps = this.steps + 1;
    this.finished = false;
    this.points = [];
    this.latestPoint = null;
    this.calc = ['Click Step +1 to see the exact calculation.'];
  }

  step() {
    if (this.finished) return;

    const x = this.x1 + this.i * this.xInc;
    const y = this.y1 + this.i * this.yInc;
    const px = Math.round(x);
    const py = Math.round(y);

    this.latestPoint = { x: px, y: py };
    this.points.push(this.latestPoint);

    this.calc = [
      `Step i = ${this.i}`,
      `x = x1 + i * xInc = ${this.x1} + ${this.i} * ${this.xInc.toFixed(4)} = ${x.toFixed(4)}`,
      `y = y1 + i * yInc = ${this.y1} + ${this.i} * ${this.yInc.toFixed(4)} = ${y.toFixed(4)}`,
      `pixel = (round(${x.toFixed(4)}), round(${y.toFixed(4)})) = (${px}, ${py})`,
      `Plotted pixels = ${this.points.length}`
    ];

    this.i += 1;
    if (this.i >= this.totalSteps) {
      this.finished = true;
    }
  }

  stepK(k) {
    const count = Math.max(1, Number(k) || 1);
    for (let n = 0; n < count && !this.finished; n += 1) {
      this.step();
    }
  }

  runAll() {
    while (!this.finished) this.step();
  }

  snapshot() {
    return {
      currentStep: this.i,
      totalSteps: this.totalSteps,
      finished: this.finished,
      points: this.points,
      latestPoint: this.latestPoint,
      vars: [
        `dx = ${this.dx}`,
        `dy = ${this.dy}`,
        `steps = ${this.steps}`,
        `xInc = ${this.xInc.toFixed(4)}`,
        `yInc = ${this.yInc.toFixed(4)}`
      ],
      calc: this.calc
    };
  }

  static theory() {
    return `WHAT IS IT?
DDA draws a line by moving in tiny equal steps from start to end.
It computes fractional x/y and rounds to the nearest pixel each step.

CORE IDEA
1) dx = x2 - x1, dy = y2 - y1
2) steps = max(|dx|, |dy|)
3) xInc = dx / steps, yInc = dy / steps
4) repeat i = 0..steps:
   x = x1 + i*xInc
   y = y1 + i*yInc
   plot (round(x), round(y))

WHY THIS IS GOOD
+ Very easy to understand
+ Great for learning pixel plotting

LIMITATION
- Uses floating-point arithmetic`; }
}

function createThreeView(rootEl) {
  const renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
  renderer.setClearColor(0x0f1730);
  rootEl.appendChild(renderer.domElement);

  const scene = new THREE.Scene();
  scene.fog = new THREE.Fog(0x0f1730, 26, 62);

  const camera = new THREE.PerspectiveCamera(55, 1, 0.1, 150);
  camera.position.set(0, 21, 24);

  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;
  controls.target.set(0, 0, 0);

  scene.add(new THREE.AmbientLight(0xffffff, 0.5));
  const key = new THREE.DirectionalLight(0xffffff, 0.85);
  key.position.set(12, 24, 18);
  scene.add(key);

  const grid = new THREE.GridHelper(42, 42, 0x274677, 0x243053);
  scene.add(grid);

  const cubes = new Map();
  const defaultColor = new THREE.Color(0x2f3b61);
  const geometry = new THREE.BoxGeometry(0.88, 0.88, 0.52);

  for (let x = -20; x <= 20; x += 1) {
    for (let y = -20; y <= 20; y += 1) {
      const material = new THREE.MeshStandardMaterial({ color: defaultColor.clone(), roughness: 0.35, metalness: 0.08 });
      const mesh = new THREE.Mesh(geometry, material);
      mesh.position.set(x, 0.3, y);
      scene.add(mesh);
      cubes.set(`${x},${y}`, mesh);
    }
  }

  const axisX = new THREE.Mesh(new THREE.BoxGeometry(41.2, 0.1, 0.1), new THREE.MeshBasicMaterial({ color: 0x6dc3ff }));
  axisX.position.set(0, 0.72, 0);
  scene.add(axisX);

  const axisY = new THREE.Mesh(new THREE.BoxGeometry(0.1, 0.1, 41.2), new THREE.MeshBasicMaterial({ color: 0x8c75ff }));
  axisY.position.set(0, 0.72, 0);
  scene.add(axisY);

  function setPixelColor(x, y, colorHex) {
    const mesh = cubes.get(`${x},${y}`);
    if (!mesh) return;
    mesh.material.color.setHex(colorHex);
  }

  function resetPixels() {
    cubes.forEach((mesh) => mesh.material.color.copy(defaultColor));
  }

  function resize() {
    const w = rootEl.clientWidth;
    const h = rootEl.clientHeight;
    if (!w || !h) return;
    renderer.setSize(w, h, false);
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
  }

  function animate() {
    controls.update();
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
  }

  window.addEventListener('resize', resize);
  resize();
  animate();

  return {
    resetCamera() {
      camera.position.set(0, 21, 24);
      controls.target.set(0, 0, 0);
      controls.update();
    },
    paintFromSnapshot(snapshot, x1, y1, x2, y2) {
      resetPixels();
      snapshot.points.forEach((p) => setPixelColor(p.x, p.y, 0xdb5f43));
      if (snapshot.latestPoint) setPixelColor(snapshot.latestPoint.x, snapshot.latestPoint.y, 0xffdc54);
      setPixelColor(x1, y1, 0x3ddc76);
      setPixelColor(x2, y2, 0x63b9ff);
    },
    setIdlePalette() {
      resetPixels();
    }
  };
}

const threeView = createThreeView(threeRoot);

function startDDA() {
  const { x1, y1, x2, y2 } = state.inputs;
  state.engine = new DDAEngine(Number(x1), Number(y1), Number(x2), Number(y2));
}

function renderViewportTop() {
  if (state.mode !== 'visualizer') {
    viewportTop.innerHTML = '<strong>3D Grid Preview</strong><span class="pill">Ready</span>';
    return;
  }

  viewportTop.innerHTML = `
    <strong>3D Pixel Grid — Visualize Mode</strong>
    <div class="action-row">
      <button class="btn" id="homeTopBtn">Back to Home</button>
      <button class="btn" id="camResetBtn">Reset Camera</button>
    </div>
  `;

  document.getElementById('homeTopBtn').onclick = () => {
    state.mode = 'home';
    render();
  };
  document.getElementById('camResetBtn').onclick = () => threeView.resetCamera();
}

function renderHome() {
  sidebar.innerHTML = `
    <div class="block">
      <h2>Welcome to CGV Web Studio</h2>
      <p style="margin-top:8px;color:var(--muted)">Choose one module to continue. This is the web conversion of your desktop CGV workflow.</p>
      <div class="home-cards">
        <button class="mode-btn" id="goPaint">1) Paint App <small style="color:#9cb0e9">(Coming soon)</small></button>
        <button class="mode-btn" id="goViz">2) Algorithm Visualizer</button>
        <button class="mode-btn" id="goGraph">3) Graph Visualizer <small style="color:#9cb0e9">(Coming soon)</small></button>
      </div>
    </div>
  `;

  document.getElementById('goPaint').onclick = () => { state.mode = 'paint'; render(); };
  document.getElementById('goGraph').onclick = () => { state.mode = 'graph'; render(); };
  document.getElementById('goViz').onclick = () => {
    state.mode = 'visualizer';
    state.panelTab = 'visualize';
    startDDA();
    render();
  };

  threeView.setIdlePalette();
}

function renderPlaceholder(modeName) {
  sidebar.innerHTML = `
    <div class="block">
      <h2>${modeName}</h2>
      <p style="margin-top:10px;color:var(--muted)">This module is not implemented yet in web mode.</p>
      <p style="margin-top:8px;color:var(--muted)">You can continue with Algorithm Visualizer for now.</p>
      <div class="action-row" style="margin-top:12px">
        <button class="btn" id="toHome">Back Home</button>
        <button class="btn good" id="toViz">Open Algorithm Visualizer</button>
      </div>
    </div>
  `;
  document.getElementById('toHome').onclick = () => { state.mode = 'home'; render(); };
  document.getElementById('toViz').onclick = () => { state.mode = 'visualizer'; startDDA(); render(); };
}

function renderTheoryTab() {
  return `<div class="block"><h3>DDA Theory</h3><div class="mono" style="margin-top:10px">${DDAEngine.theory()}</div></div>`;
}

function renderVisualizerTab(snapshot) {
  const done = snapshot.finished ? 'Yes' : 'No';
  const latest = snapshot.latestPoint ? `(${snapshot.latestPoint.x}, ${snapshot.latestPoint.y})` : '-';

  return `
    <div class="block">
      <h3>Input Coordinates</h3>
      <div class="row" style="margin-top:10px">
        <div><label>Start X</label><input type="number" id="x1" value="${state.inputs.x1}" /></div>
        <div><label>Start Y</label><input type="number" id="y1" value="${state.inputs.y1}" /></div>
        <div><label>End X</label><input type="number" id="x2" value="${state.inputs.x2}" /></div>
        <div><label>End Y</label><input type="number" id="y2" value="${state.inputs.y2}" /></div>
      </div>
      <div class="action-row" style="margin-top:10px">
        <button class="btn good" id="applyReset">Apply & Reset</button>
      </div>
    </div>

    <div class="block">
      <h3>Execution Controls</h3>
      <div class="action-row" style="margin-top:10px">
        <button class="btn good" id="step1">Step +1</button>
        <input type="number" id="ksteps" value="${state.inputs.k}" style="max-width:90px" />
        <button class="btn" id="stepK">Step K</button>
        <button class="btn warn" id="runAll">Run All</button>
        <button class="btn bad" id="resetOnly">Reset</button>
      </div>
      <div class="kpi">
        <div><div class="v">${snapshot.currentStep}</div><div class="t">Current Step</div></div>
        <div><div class="v">${snapshot.totalSteps}</div><div class="t">Total Steps</div></div>
        <div><div class="v">${done}</div><div class="t">Finished</div></div>
      </div>
      <div class="kpi">
        <div><div class="v">${snapshot.points.length}</div><div class="t">Pixels Plotted</div></div>
        <div><div class="v">${latest}</div><div class="t">Latest Pixel</div></div>
        <div><div class="v">DDA</div><div class="t">Algorithm</div></div>
      </div>
    </div>

    <div class="block">
      <h3>Current Variables</h3>
      <div class="mono" style="margin-top:8px">${snapshot.vars.join('\n')}</div>
    </div>

    <div class="block">
      <h3>Step Calculation</h3>
      <div class="mono" style="margin-top:8px">${snapshot.calc.join('\n')}</div>
    </div>
  `;
}

function mountVisualizerActions() {
  const num = (id) => Number(document.getElementById(id).value || 0);

  const applyInputs = () => {
    state.inputs.x1 = num('x1');
    state.inputs.y1 = num('y1');
    state.inputs.x2 = num('x2');
    state.inputs.y2 = num('y2');
    state.inputs.k = Math.max(1, num('ksteps') || state.inputs.k || 1);
  };

  document.getElementById('applyReset').onclick = () => {
    applyInputs();
    startDDA();
    render();
  };

  document.getElementById('step1').onclick = () => {
    applyInputs();
    state.engine.step();
    render();
  };

  document.getElementById('stepK').onclick = () => {
    applyInputs();
    state.engine.stepK(state.inputs.k);
    render();
  };

  document.getElementById('runAll').onclick = () => {
    state.engine.runAll();
    render();
  };

  document.getElementById('resetOnly').onclick = () => {
    startDDA();
    render();
  };
}

function renderVisualizer() {
  if (!state.engine) startDDA();
  const snapshot = state.engine.snapshot();

  const algoOptions = ALGO_OPTIONS.map((a) =>
    `<option value="${a.id}" ${a.id === state.algo ? 'selected' : ''} ${a.enabled ? '' : 'disabled'}>${a.label}${a.enabled ? '' : ' (Soon)'}</option>`
  ).join('');

  sidebar.innerHTML = `
    <div class="block">
      <h2>Algorithm Visualizer</h2>
      <p style="margin-top:8px;color:var(--muted)">Converted to web UI with a live 3D pixel grid for understanding plotting.</p>
      <div style="margin-top:10px">
        <label>Select Algorithm</label>
        <select id="algoSelect">${algoOptions}</select>
      </div>
      <div class="tabs">
        <button class="tab ${state.panelTab === 'theory' ? 'active' : ''}" id="tabTheory">Theory</button>
        <button class="tab ${state.panelTab === 'visualize' ? 'active' : ''}" id="tabVisualize">Visualize</button>
      </div>
    </div>

    ${state.panelTab === 'theory' ? renderTheoryTab() : renderVisualizerTab(snapshot)}
  `;

  document.getElementById('algoSelect').onchange = (ev) => {
    const chosen = ev.target.value;
    const chosenMeta = ALGO_OPTIONS.find((a) => a.id === chosen);
    if (!chosenMeta?.enabled) return;
    state.algo = chosen;
    if (state.algo === 'dda') startDDA();
    render();
  };

  document.getElementById('tabTheory').onclick = () => { state.panelTab = 'theory'; render(); };
  document.getElementById('tabVisualize').onclick = () => { state.panelTab = 'visualize'; render(); };

  if (state.panelTab === 'visualize') {
    mountVisualizerActions();
  }

  threeView.paintFromSnapshot(snapshot, Number(state.inputs.x1), Number(state.inputs.y1), Number(state.inputs.x2), Number(state.inputs.y2));
}

function render() {
  renderViewportTop();

  if (state.mode === 'home') {
    renderHome();
  } else if (state.mode === 'paint') {
    renderPlaceholder('Paint App');
  } else if (state.mode === 'graph') {
    renderPlaceholder('Graph Visualizer');
  } else {
    renderVisualizer();
  }
}

render();

#pragma once
// src/sim/WebPage.h — grapher-specific browser UI.
// Polls /frame.bin (raw RGB565), same as the typewriter skeleton this
// architecture is based on. Arrow keys pan, 'c' centers, digit keys
// 1-4 graph the corresponding equation slot, and the text box lets you
// type a new equation into a slot and graph it immediately.

namespace simhal {

inline const char* kIndexHtml = R"HTML(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>PrometheusGrapher — live buffer</title>
<style>
  html, body {
    background: #111; color: #7CFC9A; font-family: monospace;
    display: flex; flex-direction: column; align-items: center;
    margin: 0; padding: 24px; min-height: 100vh; box-sizing: border-box;
  }
  h1 { font-size: 14px; font-weight: normal; opacity: 0.7; margin: 0 0 12px; }
  canvas { image-rendering: pixelated; border: 2px solid #333; background: black; }
  .row { margin-top: 14px; display: flex; gap: 8px; align-items: center; }
  input, select, button {
    background: #1a1a1a; color: #7CFC9A; border: 1px solid #444;
    font-family: monospace; font-size: 13px; padding: 4px 8px;
  }
  button { cursor: pointer; }
  p { font-size: 12px; opacity: 0.5; max-width: 480px; text-align: center; }
</style>
</head>
<body>
  <h1>PrometheusGrapher — live framebuffer over HTTP</h1>
  <canvas id="c" tabindex="0"></canvas>
  <div class="row">
    <select id="slot">
      <option value="0">Slot 1</option>
      <option value="1">Slot 2</option>
      <option value="2">Slot 3</option>
      <option value="3">Slot 4</option>
    </select>
    <input id="eq" type="text" placeholder="e.g. sin(x)*20" size="20">
    <button id="graphBtn">Graph</button>
    <button id="centerBtn">Center (c)</button>
  </div>
  <p>Click the canvas, then use arrow keys to pan or 'c' to re-center.
  Type an equation above and hit Graph (or press 1-4 on the canvas to
  graph an existing slot as-is). No shell, no terminal.</p>

<script>
const SCALE = 3;
const canvas = document.getElementById('c');
const ctx = canvas.getContext('2d');
let off = document.createElement('canvas');
let offCtx = off.getContext('2d');
canvas.focus();

async function pollFrame() {
  try {
    const res = await fetch('/frame.bin', {cache: 'no-store'});
    const buf = await res.arrayBuffer();
    const view = new DataView(buf);
    const w = view.getUint16(0, true);
    const h = view.getUint16(2, true);
    if (off.width !== w || off.height !== h) {
      off.width = w; off.height = h;
      canvas.width = w * SCALE; canvas.height = h * SCALE;
    }
    const img = offCtx.createImageData(w, h);
    for (let i = 0; i < w * h; i++) {
      const px = view.getUint16(4 + i * 2, true);
      const r5 = (px >> 11) & 0x1F, g6 = (px >> 5) & 0x3F, b5 = px & 0x1F;
      img.data[i*4+0] = Math.round(r5 * 255 / 31);
      img.data[i*4+1] = Math.round(g6 * 255 / 63);
      img.data[i*4+2] = Math.round(b5 * 255 / 31);
      img.data[i*4+3] = 255;
    }
    offCtx.putImageData(img, 0, 0);
    ctx.imageSmoothingEnabled = false;
    ctx.drawImage(off, 0, 0, w, h, 0, 0, w * SCALE, h * SCALE);
  } catch (e) { /* server mid-restart, try again next tick */ }
}

function sendKey(body) { fetch('/key', {method: 'POST', body}); }

canvas.addEventListener('keydown', (e) => {
  e.preventDefault();
  if (e.key === 'ArrowLeft') return sendKey('left');
  if (e.key === 'ArrowRight') return sendKey('right');
  if (e.key === 'ArrowUp') return sendKey('up');
  if (e.key === 'ArrowDown') return sendKey('down');
  if (e.key === 'c') return sendKey('center');
  if (['1','2','3','4'].includes(e.key)) return sendKey('graph:' + (e.key - 1));
});

document.getElementById('centerBtn').addEventListener('click', () => sendKey('center'));
document.getElementById('graphBtn').addEventListener('click', () => {
  const slot = document.getElementById('slot').value;
  const eq = document.getElementById('eq').value;
  fetch('/equation', {method: 'POST', body: slot + ':' + eq});
  canvas.focus();
});

setInterval(pollFrame, 50);
pollFrame();
</script>
</body>
</html>
)HTML";

}  // namespace simhal

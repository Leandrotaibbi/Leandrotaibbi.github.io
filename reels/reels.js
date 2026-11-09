/* 12 reels · 1080×1920 · 30 s.  v3 — "viewport" format.
   A real 3D scene (perspective projection, painter-sorted shaded faces, wireframe,
   floor grid, fog) framed like a Blender viewport: overlay text, navigation gizmo,
   timeline, status bar. Below: parameter / derived panels, a formula block and one
   sentence per phase. Every frame is a pure function of (reel, t). */

const W = 1080, H = 1920, DUR = 30, FPS = 30;
const BPM = 130, BEAT = 60 / BPM, BAR = 4 * BEAT, S16 = BEAT / 4;
const PHASES = 4, PH = DUR / PHASES;
const F_UI = 'DejaVu Sans, Verdana, sans-serif';      /* Blender's UI face */
const F_MONO = 'ShareTech', F_SERIF = 'STIX', F_NUM = 'VT323';

/* ─── palettes: one per episode, none shared ─── */
const PAL = {
  phosphor: { bg0:'#0c1210', bg1:'#060907', grid:'#1d3a2a', wire:'#39ff14', face:'#0f2a18', emit:'#ff2bd6', text:'#d8ffd0', dim:'#5f9a6a', light:false },
  amber:    { bg0:'#121a3a', bg1:'#070b1e', grid:'#26305c', wire:'#ffb000', face:'#2a2a4a', emit:'#ffe9a0', text:'#fff1cc', dim:'#8a86b0', light:false },
  ice:      { bg0:'#0a1324', bg1:'#03060f', grid:'#17325a', wire:'#9fd3ff', face:'#132a4a', emit:'#ff4040', text:'#e6f3ff', dim:'#6c8fb8', light:false },
  paper:    { bg0:'#f4efe4', bg1:'#e2dbcb', grid:'#c9c0ab', wire:'#2a2622', face:'#d9d0bd', emit:'#b8860b', text:'#1d1a16', dim:'#7b7264', light:true },
  violet:   { bg0:'#1a1030', bg1:'#0a0616', grid:'#34245e', wire:'#b388ff', face:'#2a1d52', emit:'#c6ff00', text:'#efe6ff', dim:'#8f7ab8', light:false },
  blood:    { bg0:'#1e0a0a', bg1:'#0a0303', grid:'#4a1a1a', wire:'#ff3b3b', face:'#3a1010', emit:'#ffffff', text:'#ffe0e0', dim:'#a85656', light:false },
  studio:   { bg0:'#e2e5ea', bg1:'#b9bfc8', grid:'#9aa2ae', wire:'#1a1d22', face:'#c4c9d1', emit:'#1f5bff', text:'#15171b', dim:'#5d6470', light:true },
  hazard:   { bg0:'#111111', bg1:'#000000', grid:'#2c2c1a', wire:'#ffe600', face:'#2a2a00', emit:'#00e5ff', text:'#fffbd0', dim:'#8c8a5a', light:false },
  rose:     { bg0:'#241226', bg1:'#0f0611', grid:'#4a2448', wire:'#ff6ec7', face:'#3a1a3a', emit:'#7dffc4', text:'#ffe4f4', dim:'#a86f9c', light:false },
  blender:  { bg0:'#3d3d3d', bg1:'#232323', grid:'#545454', wire:'#f2f2f2', face:'#7a7a7a', emit:'#e2ff3d', text:'#f0f0f0', dim:'#a0a0a0', light:false },
  lagoon:   { bg0:'#07262a', bg1:'#031214', grid:'#124a4a', wire:'#2ee6d6', face:'#0d3a3a', emit:'#ffd166', text:'#dffcf8', dim:'#5fa39c', light:false },
  lavender: { bg0:'#ece9f4', bg1:'#cfc9e2', grid:'#aaa3c4', wire:'#2b1e66', face:'#d6d0ea', emit:'#c8102e', text:'#1b1440', dim:'#6f668f', light:true },
};
let P = PAL.phosphor;

/* ─── helpers ─── */
const clamp = (x, a = 0, b = 1) => Math.max(a, Math.min(b, x));
const lerp = (a, b, k) => a + (b - a) * k;
const ease = (t, a, b) => { const k = clamp((t - a) / (b - a)); return k * k * (3 - 2 * k); };
const easeOut = (t, a, b) => { const k = clamp((t - a) / (b - a)); return 1 - (1 - k) ** 3; };
const hex2 = h => { const n = parseInt(h.slice(1), 16); return [n >> 16, (n >> 8) & 255, n & 255]; };
const rgba = (h, a) => { const [r, g, b] = hex2(h); return `rgba(${r},${g},${b},${a})`; };
const mix = (h1, h2, k) => { const a = hex2(h1), b = hex2(h2); return `rgb(${a.map((v, i) => Math.round(lerp(v, b[i], k))).join(',')})`; };
const shade = (h, k) => { const [r, g, b] = hex2(h); const f = v => Math.round(clamp(v * k, 0, 255)); return `rgb(${f(r)},${f(g)},${f(b)})`; };
const fmt = (x, d = 2) => x.toFixed(d);
const euclid = (k, n, rot = 0) => Array.from({ length: n }, (_, i) => ((((i + rot) * k) % n) < k) ? 1 : 0);
const beatPulse = t => Math.exp(-((t % BEAT) / BEAT) * 7);
const PHI = (1 + Math.sqrt(5)) / 2;

function font(ctx, size, fam = F_MONO, w = 400, it = false) { ctx.font = `${it ? 'italic ' : ''}${w} ${size}px ${fam}`; }
function text(ctx, s, x, y, { size = 22, fam = F_MONO, w = 400, it = false, color = P.text, align = 'left', alpha = 1, ls = 0 } = {}) {
  if (alpha <= 0) return;
  ctx.save(); ctx.globalAlpha = alpha; font(ctx, size, fam, w, it); ctx.fillStyle = color; ctx.textAlign = align; ctx.textBaseline = 'alphabetic';
  if (ls) ctx.letterSpacing = ls + 'px'; ctx.fillText(s, x, y); ctx.restore();
}
function rect(ctx, x, y, w, h, fill, stroke, lw = 1) { if (fill) { ctx.fillStyle = fill; ctx.fillRect(x, y, w, h); } if (stroke) { ctx.strokeStyle = stroke; ctx.lineWidth = lw; ctx.strokeRect(x, y, w, h); } }
function line(ctx, x1, y1, x2, y2, color, w = 1, alpha = 1) { ctx.save(); ctx.globalAlpha = alpha; ctx.strokeStyle = color; ctx.lineWidth = w; ctx.beginPath(); ctx.moveTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke(); ctx.restore(); }
function circle(ctx, x, y, r, fill, stroke, lw = 1, alpha = 1) { ctx.save(); ctx.globalAlpha = alpha; ctx.beginPath(); ctx.arc(x, y, r, 0, Math.PI * 2); if (fill) { ctx.fillStyle = fill; ctx.fill(); } if (stroke) { ctx.strokeStyle = stroke; ctx.lineWidth = lw; ctx.stroke(); } ctx.restore(); }

/* ─── serif text with *italic* and **bold** markup, wrapped & centred ─── */
function rich(ctx, str, cx, y, maxW, size, color, align = 'center') {
  const toks = [];
  str.split(/(\*\*[^*]+\*\*|\*[^*]+\*)/).forEach(seg => { if (!seg) return; const b = seg.startsWith('**'), it = !b && seg.startsWith('*'); seg.replace(/\*/g, '').split(' ').forEach(wd => { if (!wd) return;
    if (/^[.,;:!?…)]+$/.test(wd) && toks.length) toks.at(-1).tail = (toks.at(-1).tail || '') + wd; else toks.push({ w: wd, b, it }); }); });
  const meas = tk => { font(ctx, size, F_SERIF, tk.b ? 700 : 400, tk.it); let w = ctx.measureText(tk.w).width; if (tk.tail) { font(ctx, size, F_SERIF, 400); w += ctx.measureText(tk.tail).width; } return w; };
  font(ctx, size, F_SERIF); const sp = ctx.measureText(' ').width;
  const lines = [[]]; let cur = 0;
  for (const tk of toks) { const wdt = meas(tk); if (cur + (cur ? sp : 0) + wdt > maxW && lines.at(-1).length) { lines.push([]); cur = 0; } lines.at(-1).push(tk); cur += (cur ? sp : 0) + wdt; }
  ctx.save(); ctx.textBaseline = 'alphabetic'; ctx.textAlign = 'left';
  lines.forEach((ln, li) => { const wsum = ln.reduce((s, tk) => s + meas(tk), 0) + sp * (ln.length - 1); let x = align === 'center' ? cx - wsum / 2 : cx; const yy = y + li * size * 1.28;
    for (const tk of ln) { font(ctx, size, F_SERIF, tk.b ? 700 : 400, tk.it); ctx.fillStyle = tk.b ? P.emit : color; ctx.fillText(tk.w, x, yy); const ww = ctx.measureText(tk.w).width;
      if (tk.tail) { font(ctx, size, F_SERIF); ctx.fillStyle = color; ctx.fillText(tk.tail, x + ww, yy); } x += meas(tk) + sp; } });
  ctx.restore(); return lines.length;
}

/* ═══════════════ 3D ═══════════════ */
const VP = { x: 0, y: 300, w: W, h: 860 };            /* viewport rectangle */
const CAM = { f: 1150, dist: 8.1, yaw: 0, pitch: -0.42, cy: VP.y + VP.h * 0.52, dy: 0.0 };
let cam = { ...CAM };
const L = (() => { const v = [0.45, 0.85, 0.6]; const n = Math.hypot(...v); return v.map(x => x / n); })();
function rotY([x, y, z], a) { const c = Math.cos(a), s = Math.sin(a); return [c * x + s * z, y, -s * x + c * z]; }
function rotX([x, y, z], a) { const c = Math.cos(a), s = Math.sin(a); return [x, c * y - s * z, s * y + c * z]; }
function rotZ([x, y, z], a) { const c = Math.cos(a), s = Math.sin(a); return [c * x - s * y, s * x + c * y, z]; }
function toCam(p) { let q = rotY(p, cam.yaw); q = rotX(q, cam.pitch); return [q[0], q[1] - cam.dy, q[2] + cam.dist]; }
function proj(c) { const s = cam.f / c[2]; return { x: W / 2 + c[0] * s, y: cam.cy - c[1] * s, s, z: c[2] }; }
const fog = z => clamp(1 - (z - cam.dist + 1.6) / 7, .12, 1);

/* scene = list of drawables sorted by depth (painter's algorithm) */
let SCENE = [], STATS = { v: 0, f: 0, e: 0, o: 0 };
function addMesh(mesh, mat, xf = p => p) {
  const vc = mesh.v.map(p => toCam(xf(p))), vs = vc.map(proj); STATS.v += mesh.v.length; STATS.o++;
  for (const f of mesh.f) {
    const a = vc[f[0]], b = vc[f[1]], c = vc[f[2]];
    const u = [b[0] - a[0], b[1] - a[1], b[2] - a[2]], v = [c[0] - a[0], c[1] - a[1], c[2] - a[2]];
    let n = [u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0]]; const nl = Math.hypot(...n) || 1; n = n.map(x => x / nl);
    const cz = f.reduce((s, i) => s + vc[i][2], 0) / f.length; const cx = f.reduce((s, i) => s + vc[i][0], 0) / f.length, cy = f.reduce((s, i) => s + vc[i][1], 0) / f.length;
    const view = [-cx, -cy, -cz]; const vl = Math.hypot(...view) || 1; const nv = n[0] * view[0] + n[1] * view[1] + n[2] * view[2];
    if (mat.cull && nv < 0) continue;
    const nn = nv < 0 ? n.map(x => -x) : n; const diff = Math.max(0, nn[0] * L[0] + nn[1] * L[1] + nn[2] * L[2]);
    const hv = [L[0] + view[0] / vl, L[1] + view[1] / vl, L[2] + view[2] / vl]; const hl = Math.hypot(...hv) || 1; const spec = Math.pow(Math.max(0, (nn[0] * hv[0] + nn[1] * hv[1] + nn[2] * hv[2]) / hl), 28) * (mat.spec ?? .5);
    SCENE.push({ z: cz, kind: 'face', pts: f.map(i => vs[i]), k: .28 + .72 * diff, spec, mat }); STATS.f++;
  }
  if (mesh.e) for (const [i, j] of mesh.e) { SCENE.push({ z: (vc[i][2] + vc[j][2]) / 2, kind: 'edge', a: vs[i], b: vs[j], mat }); STATS.e++; }
}
function addPoint(p, mat) { const c = toCam(p), s = proj(c); SCENE.push({ z: c[2], kind: 'pt', p: s, mat }); STATS.v++; }
function addLine(p, q, mat) { const a = toCam(p), b = toCam(q); SCENE.push({ z: (a[2] + b[2]) / 2, kind: 'line', a: proj(a), b: proj(b), mat }); STATS.e++; }
function addLabel(p, s, mat) { const c = toCam(p); SCENE.push({ z: c[2] - .01, kind: 'label', p: proj(c), s, mat }); }
function drawScene(ctx) {
  SCENE.sort((a, b) => b.z - a.z);
  ctx.save(); ctx.beginPath(); ctx.rect(VP.x, VP.y, VP.w, VP.h); ctx.clip();
  for (const d of SCENE) {
    const fa = fog(d.z);
    if (d.kind === 'face') {
      const m = d.mat; ctx.beginPath(); d.pts.forEach((p, i) => i ? ctx.lineTo(p.x, p.y) : ctx.moveTo(p.x, p.y)); ctx.closePath();
      ctx.globalAlpha = (m.alpha ?? 1) * fa; ctx.fillStyle = m.emissive ? m.color : shade(m.color, d.k * 1.25); ctx.fill();
      if (d.spec > .02 && !m.emissive) { ctx.globalAlpha = d.spec * fa * .9; ctx.fillStyle = m.light ? '#ffffff' : P.wire; ctx.fill(); }
      if (m.wire) { ctx.globalAlpha = (m.wireAlpha ?? .55) * fa; ctx.strokeStyle = m.wire; ctx.lineWidth = m.lw ?? 1; ctx.stroke(); }
    } else if (d.kind === 'edge' || d.kind === 'line') {
      ctx.globalAlpha = (d.mat.alpha ?? .9) * fa; ctx.strokeStyle = d.mat.color; ctx.lineWidth = (d.mat.lw ?? 1.5) * (d.a.s / (cam.f / cam.dist)); ctx.beginPath(); ctx.moveTo(d.a.x, d.a.y); ctx.lineTo(d.b.x, d.b.y); ctx.stroke();
    } else if (d.kind === 'pt') {
      const r = (d.mat.r ?? .05) * d.p.s; ctx.globalAlpha = (d.mat.alpha ?? 1) * fa;
      if (d.mat.glow) { const g = ctx.createRadialGradient(d.p.x, d.p.y, 0, d.p.x, d.p.y, r * 3.2); g.addColorStop(0, rgba(d.mat.color, .55)); g.addColorStop(1, rgba(d.mat.color, 0)); ctx.fillStyle = g; ctx.beginPath(); ctx.arc(d.p.x, d.p.y, r * 3.2, 0, Math.PI * 2); ctx.fill(); }
      const g2 = ctx.createRadialGradient(d.p.x - r * .35, d.p.y - r * .35, r * .1, d.p.x, d.p.y, r); g2.addColorStop(0, d.mat.flat ? d.mat.color : '#ffffff'); g2.addColorStop(.25, d.mat.color); g2.addColorStop(1, shade(d.mat.color, .35));
      ctx.fillStyle = g2; ctx.beginPath(); ctx.arc(d.p.x, d.p.y, r, 0, Math.PI * 2); ctx.fill();
    } else if (d.kind === 'label') {
      ctx.globalAlpha = fa; font(ctx, (d.mat.size ?? 16) * Math.min(1.3, d.p.s / (cam.f / cam.dist)), F_UI); ctx.fillStyle = d.mat.color; ctx.textAlign = 'center'; ctx.fillText(d.s, d.p.x, d.p.y + (d.mat.dy ?? -14));
    }
  }
  ctx.restore(); ctx.globalAlpha = 1;
}
/* floor grid in the XZ plane */
function floor(ctx, y = -1.2, n = 8, step = 0.5) {
  for (let i = -n; i <= n; i++) { const major = i % 2 === 0; addLine([-n * step, y, i * step], [n * step, y, i * step], { color: P.grid, lw: major ? 1.4 : .7, alpha: major ? .9 : .5 }); addLine([i * step, y, -n * step], [i * step, y, n * step], { color: P.grid, lw: major ? 1.4 : .7, alpha: major ? .9 : .5 }); }
  addLine([-n * step, y, 0], [n * step, y, 0], { color: '#c84848', lw: 1.6, alpha: .8 }); addLine([0, y, -n * step], [0, y, n * step], { color: '#5a9a3a', lw: 1.6, alpha: .8 });
}
/* primitives (unit-ish, centred) */
function torus(R, r, nu, nv) { const v = [], f = []; for (let i = 0; i < nu; i++) { const a = i / nu * Math.PI * 2; for (let j = 0; j < nv; j++) { const b = j / nv * Math.PI * 2; v.push([(R + r * Math.cos(b)) * Math.cos(a), r * Math.sin(b), (R + r * Math.cos(b)) * Math.sin(a)]); } }
  for (let i = 0; i < nu; i++) for (let j = 0; j < nv; j++) { const i2 = (i + 1) % nu, j2 = (j + 1) % nv; f.push([i * nv + j, i2 * nv + j, i2 * nv + j2, i * nv + j2]); } return { v, f }; }
function sphere(r, nu = 12, nv = 8) { const v = [], f = []; for (let j = 0; j <= nv; j++) { const b = j / nv * Math.PI; for (let i = 0; i < nu; i++) { const a = i / nu * Math.PI * 2; v.push([r * Math.sin(b) * Math.cos(a), r * Math.cos(b), r * Math.sin(b) * Math.sin(a)]); } }
  for (let j = 0; j < nv; j++) for (let i = 0; i < nu; i++) { const i2 = (i + 1) % nu; f.push([j * nu + i, j * nu + i2, (j + 1) * nu + i2, (j + 1) * nu + i]); } return { v, f }; }
function box(w, h, d) { const x = w / 2, y = h / 2, z = d / 2; const v = [[-x, -y, -z], [x, -y, -z], [x, y, -z], [-x, y, -z], [-x, -y, z], [x, -y, z], [x, y, z], [-x, y, z]];
  const f = [[0, 1, 2, 3], [5, 4, 7, 6], [4, 0, 3, 7], [1, 5, 6, 2], [3, 2, 6, 7], [4, 5, 1, 0]]; const e = [[0, 1], [1, 2], [2, 3], [3, 0], [4, 5], [5, 6], [6, 7], [7, 4], [0, 4], [1, 5], [2, 6], [3, 7]]; return { v, f, e }; }
/* tube swept along a curve c(u), u∈[0,1] */
function tube(cfn, n, r, m = 8, closed = false) { const v = [], f = []; let prevN = null;
  for (let i = 0; i <= n; i++) { const u = i / n, p = cfn(u), p2 = cfn(u + 1e-3); let tdir = p2.map((x, k) => x - p[k]); const tl = Math.hypot(...tdir) || 1; tdir = tdir.map(x => x / tl);
    let nrm = prevN ? prevN : (Math.abs(tdir[1]) < .9 ? [0, 1, 0] : [1, 0, 0]); let bn = [tdir[1] * nrm[2] - tdir[2] * nrm[1], tdir[2] * nrm[0] - tdir[0] * nrm[2], tdir[0] * nrm[1] - tdir[1] * nrm[0]]; const bl = Math.hypot(...bn) || 1; bn = bn.map(x => x / bl);
    nrm = [bn[1] * tdir[2] - bn[2] * tdir[1], bn[2] * tdir[0] - bn[0] * tdir[2], bn[0] * tdir[1] - bn[1] * tdir[0]]; prevN = nrm;
    for (let j = 0; j < m; j++) { const a = j / m * Math.PI * 2; v.push(p.map((x, k) => x + r * (Math.cos(a) * nrm[k] + Math.sin(a) * bn[k]))); } }
  for (let i = 0; i < n; i++) for (let j = 0; j < m; j++) { const j2 = (j + 1) % m; f.push([i * m + j, (i + 1) * m + j, (i + 1) * m + j2, i * m + j2]); } return { v, f }; }
const T = { tr: (dx, dy, dz) => p => [p[0] + dx, p[1] + dy, p[2] + dz], ry: a => p => rotY(p, a), rx: a => p => rotX(p, a), rz: a => p => rotZ(p, a), sc: s => p => p.map(x => x * s), chain: (...fs) => p => fs.reduce((q, f) => f(q), p) };

/* ═══════════════ viewport chrome (Blender-like) ═══════════════ */
function viewportChrome(ctx, reel, t, n, frame) {
  /* overlay text, top-left */
  const ox = 28, oy = VP.y + 40;
  text(ctx, 'User Perspective', ox, oy, { size: 24, fam: F_UI, color: P.text });
  text(ctx, `(${frame}) Collection | ${reel.obj}`, ox, oy + 32, { size: 24, fam: F_UI, color: P.text });
  text(ctx, `${BPM} BPM · bar ${String(Math.floor(t / BAR) + 1).padStart(2, '0')} · step ${String(Math.floor(t / S16) % 16).padStart(2, '0')}`, ox, oy + 60, { size: 19, fam: F_UI, color: P.dim });
  text(ctx, `${P.light ? 'EEVEE Next' : 'Cycles'} · 128 spp · denoise ON · ${frame}/900`, ox, oy + 88, { size: 17, fam: F_UI, color: P.dim });
  /* navigation gizmo, top-right */
  const gx = W - 92, gy = VP.y + 92, gr = 46;
  circle(ctx, gx, gy, gr + 14, rgba(P.light ? '#000000' : '#ffffff', .06));
  const axes = [[[1, 0, 0], '#ff3352', 'X'], [[0, 1, 0], '#8bdc00', 'Y'], [[0, 0, 1], '#2890ff', 'Z']];
  const projA = v => { let q = rotY(v, cam.yaw); q = rotX(q, cam.pitch); return { x: gx + q[0] * gr, y: gy - q[1] * gr, z: q[2] }; };
  const items = []; axes.forEach(([v, col, lbl]) => { const p = projA(v), m = projA(v.map(x => -x)); items.push({ z: p.z, col, lbl, p, pos: true }); items.push({ z: m.z, col, p: m, pos: false }); });
  items.sort((a, b) => b.z - a.z).forEach(it => { if (it.pos) { line(ctx, gx, gy, it.p.x, it.p.y, it.col, 2.5); circle(ctx, it.p.x, it.p.y, 10, it.col); text(ctx, it.lbl, it.p.x, it.p.y + 4.5, { size: 12, fam: F_UI, w: 700, color: '#000', align: 'center' }); } else circle(ctx, it.p.x, it.p.y, 8, rgba(it.col, .35), it.col, 1.2); });
  /* right-side tool strip */
  const tx = W - 36; ['⊕', '◎', '✥', '▣', '⌗'].forEach((s, i) => { rect(ctx, tx - 14, VP.y + 190 + i * 40, 28, 28, rgba(P.light ? '#000' : '#fff', .08)); text(ctx, s, tx, VP.y + 210 + i * 40, { size: 15, fam: F_UI, color: P.dim, align: 'center' }); });
  /* timeline strip */
  const ty = VP.y + VP.h + 4, tx0 = 28, tx1 = W - 28;
  rect(ctx, 0, ty, W, 52, rgba(P.light ? '#ffffff' : '#000000', .25));
  for (let fr = 0; fr <= 900; fr += 50) { const x = lerp(tx0, tx1, fr / 900); line(ctx, x, ty + 36, x, ty + 46, P.dim, 1, fr % 100 ? .4 : .9); if (fr % 100 === 0) text(ctx, String(fr), x, ty + 30, { size: 13, fam: F_UI, color: P.dim, align: 'center' }); }
  for (let b = 0; b * BAR <= DUR; b++) { const x = lerp(tx0, tx1, b * BAR / DUR); line(ctx, x, ty + 40, x, ty + 50, P.wire, 1, .35); }
  const px = lerp(tx0, tx1, t / DUR); rect(ctx, px - 1, ty, 2, 52, P.emit); rect(ctx, px - 20, ty + 2, 40, 18, P.emit); text(ctx, String(frame), px, ty + 16, { size: 13, fam: F_UI, w: 700, color: P.light ? '#fff' : '#000', align: 'center' });
}
function statusBar(ctx, reel, t, n) {
  const y = 1766; rect(ctx, 0, y - 26, W, 38, rgba(P.light ? '#ffffff' : '#000000', .28));
  const s = `${reel.obj}.blend | Verts ${STATS.v.toLocaleString('en')} | Edges ${STATS.e.toLocaleString('en')} | Faces ${STATS.f.toLocaleString('en')} | Obj 1/${STATS.o} | ${(48 + STATS.f / 90).toFixed(1)} MiB | 4.2.3`;
  text(ctx, s, 28, y, { size: 17, fam: F_UI, color: P.dim });
  text(ctx, '@leandro_taibbi', W - 28, y, { size: 17, fam: F_UI, color: P.text, align: 'right' });
}
/* data panels + formula + sentence */
function panels(ctx, reel, t, p) {
  const y0 = 1232; const col = (title, items, x, w) => {
    rect(ctx, x, y0 - 26, w, 190, rgba(P.light ? '#000000' : '#ffffff', .045), rgba(P.dim, .35));
    text(ctx, title, x + 14, y0 - 6, { size: 13, fam: F_UI, w: 700, color: P.dim, ls: 2 });
    items.slice(0, 5).forEach(([k, v, hi], i) => { text(ctx, k, x + 14, y0 + 26 + i * 30, { size: 23, color: P.dim }); text(ctx, v, x + w - 14, y0 + 26 + i * 30, { size: 23, color: hi ? P.emit : P.text, align: 'right' }); }); };
  const tp = t - p * PH; col('PARAMETERS', reel.params(t, p, tp), 28, 500); col('DERIVED', reel.derived(t, p, tp), 552, 500);
  /* formula block */
  const fy = 1436; rect(ctx, 28, fy - 2, 1024, 1, rgba(P.dim, .4));
  let fyy = fy + 42; reel.formulas(t, p).slice(0, 2).forEach(s => { fyy += rich(ctx, s, W / 2, fyy, 1000, 36, P.text) * 48; });
  rect(ctx, 28, fy + 118, 1024, 1, rgba(P.dim, .4));
  /* sentence */
  const t0 = p * PH; const a = ease(t, t0, t0 + .3) * (p < PHASES - 1 ? 1 - ease(t, t0 + PH - .25, t0 + PH) : 1);
  ctx.save(); ctx.globalAlpha = a; rich(ctx, reel.phases[p], W / 2, 1598, 1010, 33, P.text); ctx.restore();
  /* phase pips */
  for (let i = 0; i < PHASES; i++) rect(ctx, W / 2 - 46 + i * 24, 1712, 16, 4, i === p ? P.emit : rgba(P.dim, .5));
}

function renderFrame(ctx, reel, t) {
  const n = REELS.indexOf(reel) + 1; P = PAL[reel.pal]; const p = Math.min(PHASES - 1, Math.floor(t / PH)); const frame = Math.round(t * FPS);
  ctx.setTransform(1, 0, 0, 1, 0, 0); ctx.globalAlpha = 1; ctx.shadowBlur = 0;
  const g = ctx.createLinearGradient(0, 0, 0, H); g.addColorStop(0, P.bg0); g.addColorStop(.6, P.bg1); g.addColorStop(1, P.bg1); ctx.fillStyle = g; ctx.fillRect(0, 0, W, H);
  /* viewport background: radial vignette like a studio HDRI */
  const vg = ctx.createRadialGradient(W / 2, VP.y + VP.h * .45, 60, W / 2, VP.y + VP.h * .45, 900); vg.addColorStop(0, rgba(P.light ? '#ffffff' : '#ffffff', P.light ? .35 : .05)); vg.addColorStop(1, 'rgba(0,0,0,0)'); ctx.fillStyle = vg; ctx.fillRect(VP.x, VP.y, VP.w, VP.h);
  line(ctx, 0, VP.y, W, VP.y, rgba(P.dim, .5)); line(ctx, 0, VP.y + VP.h, W, VP.y + VP.h, rgba(P.dim, .5));
  /* title band */
  text(ctx, reel.title, 28, 246, { size: 58, fam: F_NUM, color: P.text });
  text(ctx, `EP.${String(n).padStart(2, '0')}/12`, W - 28, 246, { size: 58, fam: F_NUM, color: P.emit, align: 'right' });
  text(ctx, reel.tags, 28, 282, { size: 21, color: P.dim });
  /* 3D */
  SCENE = []; STATS = { v: 0, f: 0, e: 0, o: 0 }; cam = { ...CAM, yaw: 0.42 + 0.22 * Math.sin(t * 0.13), pitch: -0.40 + 0.05 * Math.sin(t * .21 + 1) };
  reel.scene(t, p, t - p * PH);
  drawScene(ctx);
  viewportChrome(ctx, reel, t, n, frame);
  panels(ctx, reel, t, p);
  statusBar(ctx, reel, t, n);
}

/* ═══════════════ EPISODES ═══════════════ */
const REELS = [

/* 01 ─ EUCLID / CHRISTOFFEL */
{ short: 'euclid', title: 'EUCLIDEAN RHYTHMS', tags: 'Bjorklund · Christoffel words · Sturmian sequences · rotations in ℤ/16ℤ', pal: 'phosphor', obj: 'EP01_euclid_ring',
  phases: [
    '*E(k,n)* is the Christoffel word of slope *k/n*: the lattice path that stays closest to the line *y = kx/n*. Bjorklund (2003) rediscovered Bresenham (1965).',
    '5/16 = [0; 3, 5]. The inter-onset intervals are 3 and 4, appearing 3 and 1 times: a Sturmian word has **two** gap lengths, never more.',
    '7/16 = [0; 2, 3, 2]. Layered on E(5,16) they coincide on **3** of 16 steps — gcd(5,7) = 1 guarantees the two polygons never align.',
    'Rotation is adding a unit in ℤ/16ℤ: same necklace, a new bracelet. **Zero new notes**, a different groove every bar.' ],
  params(t, p) { return [['k (hits)', p >= 2 ? '5 · 7' : '5'], ['n (steps)', '16'], ['slope k/n', p >= 2 ? '0.3125 · 0.4375' : '0.3125'], ['cont. fraction', p >= 2 ? '[0;3,5] · [0;2,3,2]' : '[0; 3, 5]'], ['rotation r', p === 3 ? String(Math.floor((t - 3 * PH) / BAR) % 16) : '0'], ['16th', fmt(S16 * 1000, 2) + ' ms']]; },
  derived(t, p) { const e5 = euclid(5, 16, p === 3 ? Math.floor((t - 3 * PH) / BAR) % 16 : 0), e7 = euclid(7, 16); return [['E(5,16)', e5.map(v => v ? 'x' : '.').join(''), true], ['E(7,16)', p >= 2 ? e7.map(v => v ? 'x' : '.').join('') : '—'], ['IOI set', p >= 2 ? '{3,4} · {2,3}' : '{3, 3, 3, 3, 4}'], ['shared steps', p >= 2 ? String(e5.reduce((s, v, i) => s + (v & e7[i]), 0)) : '—'], ['gcd(5,7)', '1'], ['necklaces', 'C(16,5)/16 ≈ 273']]; },
  formulas() { return ['*E*(*k*,*n*)*ᵢ* = ⌊(*i*+1)*k*/*n*⌋ − ⌊*ik*/*n*⌋', '5/16 = 1/(3 + 1/5)   ·   7/16 = 1/(2 + 1/(3 + 1/2))']; },
  scene(t, p, tp) {
    floor(this);
    const n = 16, R = 1.9; const rot = p === 3 ? Math.floor(tp / BAR) % n : 0; const e5 = euclid(5, n, rot), e7 = euclid(7, n);
    addMesh(torus(R, .035, 64, 6), { color: P.face, wire: null, spec: .6 });
    const pos = i => { const a = -Math.PI / 2 + i / n * Math.PI * 2; return [R * Math.cos(a), 0, R * Math.sin(a)]; };
    const step = Math.floor((t % BAR) / S16);
    for (let i = 0; i < n; i++) { const a = e5[i], b = p >= 2 && e7[i]; const q = pos(i); q[1] = a ? .22 : b ? -.22 : 0;
      if (a || b) addMesh(sphere(.13, 12, 8), { color: a ? P.wire : P.emit, spec: .9, cull: true }, T.tr(...q)); else addPoint(q, { color: P.dim, r: .04, flat: true });
      if (i === step) addMesh(torus(.26, .02, 24, 6), { color: P.text, emissive: true }, T.tr(q[0], q[1], q[2]));
      addLabel([q[0] * 1.14, q[1] + .02, q[2] * 1.14], String(i), { color: P.dim, size: 13 }); }
    const poly = (arr, y, col) => { const idx = arr.map((v, i) => v ? i : -1).filter(i => i >= 0); idx.forEach((i, k) => { const a = pos(i), b = pos(idx[(k + 1) % idx.length]); a[1] = b[1] = y; addLine(a, b, { color: col, lw: 3, alpha: .95 }); }); };
    if (p >= 1) poly(e5, .22, P.wire); if (p >= 2) poly(e7, -.22, P.emit);
    /* playhead */
    const a = -Math.PI / 2 + (t % BAR) / BAR * Math.PI * 2; addLine([0, 0, 0], [R * Math.cos(a), 0, R * Math.sin(a)], { color: P.text, lw: 1.2, alpha: .7 });
    /* the lattice line y = kx/n as a staircase on the floor */
    if (p <= 1) for (let i = 0; i < n; i++) { const y = Math.floor(i * 5 / n) * .12 - 1.19, y2 = Math.floor((i + 1) * 5 / n) * .12 - 1.19; addLine([-2.4 + i * .3, y, 2.9], [-2.4 + (i + 1) * .3, y, 2.9], { color: P.wire, lw: 1.5, alpha: .8 }); if (y2 !== y) addLine([-2.4 + (i + 1) * .3, y, 2.9], [-2.4 + (i + 1) * .3, y2, 2.9], { color: P.emit, lw: 1.5, alpha: .8 }); }
    if (p <= 1) addLine([-2.4, -1.19, 2.9], [2.4, -1.19 + 5 * .12, 2.9], { color: P.dim, lw: 1, alpha: .8 });
  } },

/* 02 ─ CRT TORUS */
{ short: 'crt torus', title: 'CHINESE REMAINDER THEOREM', tags: 'ℤ/112ℤ ≅ ℤ/16ℤ × ℤ/7ℤ · torus orbits · coprime loop lengths', pal: 'amber', obj: 'EP02_Z16xZ7_torus',
  phases: [
    'Every step is a point on a torus: (*i* mod 16, *i* mod 7). Because gcd(16,7) = 1 the orbit visits all **112** cells exactly once before closing.',
    'Chinese remainder theorem: ℤ/112ℤ ≅ ℤ/16ℤ × ℤ/7ℤ. From the pair (*a*, *b*) back to the step: *i* = 49*a* + 64*b* (mod 112).',
    '16 ≡ 2 (mod 7): each bar the 7-step bleep slides **two** steps against the kick. The drift is the music; seven bars close the loop.',
    'Replace 7 by 8: gcd = 8 and the torus collapses to **16** cells — the pattern repeats every bar. Coprimality is the whole trick.' ],
  params(t, p) { return [['m (kick loop)', '16 steps'], ['n (bleep loop)', p === 3 ? '8 steps' : '7 steps'], ['gcd(m,n)', p === 3 ? '8' : '1'], ['lcm(m,n)', p === 3 ? '16' : '112', true], ['cycle length', p === 3 ? fmt(16 * S16, 2) + ' s' : fmt(112 * S16, 2) + ' s'], ['step i', String(Math.floor(t / S16) % 112).padStart(3, '0')]]; },
  derived(t, p) { const i = Math.floor(t / S16) % 112; return [['i mod 16', String(i % 16)], ['i mod 7', String(i % 7)], ['CRT check', `49·${i % 16}+64·${i % 7} ≡ ${(49 * (i % 16) + 64 * (i % 7)) % 112}`, true], ['16 mod 7', '2  (slide/bar)'], ['bar', String(Math.floor(i / 16) + 1) + ' / 7'], ['cells visited', p === 3 ? '16 / 112' : Math.min(112, i + 1) + ' / 112']]; },
  formulas() { return ['ℤ/112ℤ ≅ ℤ/16ℤ × ℤ/7ℤ   (gcd = 1)', '*i* ≡ *a* (mod 16), *i* ≡ *b* (mod 7)  ⇒  *i* = 49*a* + 64*b* (mod 112)']; },
  scene(t, p) {
    floor(this, -1.5);
    const m = 16, nn = p === 3 ? 8 : 7, R = 1.55, r = .62;
    const tor = torus(R, r, m * 2, nn * 2); addMesh(tor, { color: P.face, wire: rgba(P.wire, .9), wireAlpha: .25, spec: .35, cull: true, alpha: .96 });
    const cell = (a, b, lift = 0) => { const u = (a + .5) / m * Math.PI * 2, v = (b + .5) / nn * Math.PI * 2; return [(R + (r + lift) * Math.cos(v)) * Math.cos(u), (r + lift) * Math.sin(v), (R + (r + lift) * Math.cos(v)) * Math.sin(u)]; };
    const cur = Math.floor(t / S16); const cyc = p === 3 ? 16 : 112;
    const visited = Math.min(cyc, cur + 1);
    for (let k = Math.max(0, cur - 111); k <= cur; k++) { const age = cur - k; if (age > 111) continue; const q = cell(k % m, k % nn, .02); const kick = k % 4 === 0; addPoint(q, { color: kick ? P.wire : P.emit, r: kick ? .06 : .045, alpha: .25 + .75 * Math.exp(-age / 60), flat: true }); }
    const q = cell(cur % m, cur % nn, .08); addMesh(sphere(.16, 12, 8), { color: P.emit, spec: 1, cull: true }, T.tr(...q)); addPoint(q, { color: P.emit, r: .1, glow: true, alpha: .9 });
    for (let k = 1; k < 8; k++) { const a = cell(((cur - k) % m + m) % m, ((cur - k) % nn + nn) % nn, .05), b = cell(((cur - k + 1) % m + m) % m, ((cur - k + 1) % nn + nn) % nn, .05); addLine(a, b, { color: P.emit, lw: 2.2, alpha: 1 - k / 9 }); }
    for (let a = 0; a < m; a += 4) { const q2 = cell(a, 0, .3); addLabel(q2, 'a=' + a, { color: P.dim, size: 12 }); }
    addLabel([0, r + 1.2, 0], `(${cur % m}, ${cur % nn})  →  i = ${cur % cyc}`, { color: P.text, size: 18 });
  } },

/* 03 ─ BEATING */
{ short: 'beating', title: 'BEATING & DETUNE', tags: 'sum-to-product identity · envelope at Δf · cents = 1200·log₂ · grid-locked AM', pal: 'ice', obj: 'EP03_phasor_pair',
  phases: [
    'sin *a* + sin *b* = 2 sin((*a*+*b*)/2)·cos((*a*−*b*)/2): a carrier at the **mean** frequency with an envelope at half the difference.',
    '|cos| peaks twice per period, so the audible beat is at Δ*f*, not Δ*f*/2. 55.000 vs 55.300 Hz → 0.300 Hz, every 3.333 s. Unrelated to 130 BPM.',
    'Choose Δ*f* = 1/(2 bars) = 130/(60·8) = **0.2708 Hz**. In pitch: 1200·log₂(55.2708/55) = 8.5 cents — below the JND for pitch, far above it for rhythm.',
    'Two free oscillators, zero modulators: amplitude modulation phase-locked to the grid by arithmetic. **No LFO**, no sidechain, one number.' ],
  params(t, p) { const f2 = p >= 2 ? 55 + 1 / (2 * BAR) : 55.3; return [['f₁', '55.0000 Hz'], ['f₂', fmt(f2, 4) + ' Hz', p >= 2], ['Δf = f₂ − f₁', fmt(f2 - 55, 4) + ' Hz'], ['carrier (f₁+f₂)/2', fmt((55 + f2) / 2, 4) + ' Hz'], ['2 bars', fmt(2 * BAR, 4) + ' s'], ['1/(2 bars)', fmt(1 / (2 * BAR), 4) + ' Hz']]; },
  derived(t, p) { const f2 = p >= 2 ? 55 + 1 / (2 * BAR) : 55.3, d = f2 - 55; return [['beat period 1/Δf', fmt(1 / d, 3) + ' s', p >= 2], ['in bars', fmt(1 / d / BAR, 3)], ['cents 1200·log₂', fmt(1200 * Math.log2(f2 / 55), 2) + ' ¢'], ['phase now', fmt(((t * d) % 1) * 360, 1) + '°'], ['envelope |cos|', fmt(Math.abs(Math.cos(Math.PI * d * t)), 3)], ['locked to grid', p >= 2 ? 'yes' : 'no']]; },
  formulas() { return ['sin *a* + sin *b* = 2·sin((*a* + *b*)/2)·cos((*a* − *b*)/2)', 'Δ*f* = BPM / (60 · beats per cycle)   ·   ¢ = 1200·log₂(*f*₂/*f*₁)']; },
  scene(t, p) {
    floor(this, -1.4);
    const f1 = 55, f2 = p >= 2 ? 55 + 1 / (2 * BAR) : 55.3, d = f2 - f1;
    /* two phasors on a disc in the XY plane, behind the ribbon (visually slowed) */
    const ph1 = t * .9, ph2 = t * .9 + 2 * Math.PI * d * t; const cx = -1.5, cy = .55, cz = -1.6, R = .85;
    addMesh(torus(R, .018, 48, 6), { color: P.dim, emissive: true, alpha: .7 }, T.chain(T.rx(Math.PI / 2), T.tr(cx, cy, cz)));
    addLine([cx, cy, cz], [cx + Math.cos(ph1) * R, cy + Math.sin(ph1) * R, cz], { color: P.wire, lw: 3 }); addLine([cx, cy, cz], [cx + Math.cos(ph2) * R, cy + Math.sin(ph2) * R, cz], { color: P.emit, lw: 3 });
    const sx = cx + (Math.cos(ph1) + Math.cos(ph2)) * R / 2, sy = cy + (Math.sin(ph1) + Math.sin(ph2)) * R / 2; addLine([cx, cy, cz], [sx, sy, cz], { color: P.text, lw: 2, alpha: .9 }); addPoint([sx, sy, cz], { color: P.text, r: .06, glow: true });
    addLabel([cx, cy + R + .25, cz], `A, B phasors · Δφ = ${fmt(((d * t) % 1) * 360, 0)}°`, { color: P.text, size: 14 });
    addLabel([cx, cy - R - .15, cz], `|A + B| = 2·|cos(Δφ/2)| = ${fmt(2 * Math.abs(Math.cos(Math.PI * d * t)), 3)}`, { color: P.dim, size: 12, dy: 12 });
    /* envelope ribbon: 8 s window along x, depth z = 1.0 */
    const win = 8, Nn = 180, z = 1.0; const env = u => Math.abs(Math.cos(Math.PI * d * (u * win + t - win)));
    for (let i = 0; i < Nn; i++) { const u = i / Nn, u2 = (i + 1) / Nn; const x = lerp(-2.6, 2.6, u), x2 = lerp(-2.6, 2.6, u2);
      addLine([x, -0.2 + env(u) * 1.0, z], [x2, -0.2 + env(u2) * 1.0, z], { color: P.wire, lw: 2.5 }); addLine([x, -0.2 - env(u) * 1.0, z], [x2, -0.2 - env(u2) * 1.0, z], { color: P.wire, lw: 2.5 });
      if (i % 2 === 0) { const c = Math.sin(2 * Math.PI * 3.1 * (u * win)) * env(u); addLine([x, -0.2 - c, z], [x, -0.2 + c, z], { color: P.dim, lw: .8, alpha: .55 }); } }
    if (p >= 2) for (let b = 0; b < 6; b++) { const tb = (t % (2 * BAR)) + b * 2 * BAR; if (tb < win) { const x = lerp(2.6, -2.6, tb / win); addLine([x, -1.35, z], [x, 1.0, z], { color: P.emit, lw: 1.5, alpha: .9 }); addLabel([x, 1.1, z], '2 bars', { color: P.emit, size: 12 }); } }
    addLabel([2.6, -1.4, z], 'now', { color: P.dim, size: 12, dy: 14 }); addLabel([-2.6, -1.4, z], '−8 s', { color: P.dim, size: 12, dy: 14 });
    addLabel([0, 1.3, z], `envelope |cos(π·Δf·t)| · Δf = ${fmt(d, 4)} Hz · period ${fmt(1 / d, 3)} s`, { color: P.text, size: 14 });
  } },

/* 04 ─ φ (paper) */
{ short: 'phi delay', title: 'THE GOLDEN DELAY', tags: 'continued fractions · Hurwitz bound · three-gap theorem · Weyl equidistribution', pal: 'paper', obj: 'EP04_golden_cone',
  phases: [
    'φ = [1; 1, 1, 1, …] has the slowest-converging continued fraction of any real. Hurwitz: |φ − *p/q*| > 1/(√5·*q*²) for every fraction — the **most irrational** number.',
    'A 3/4-beat delay is *q* = 4: after four repeats the tap is back on the grid. The tail is a comb of coincidences, not a cloud.',
    'Taps at *n*/φ (mod 1) on the beat-circle: by the Steinhaus three-gap theorem *n* points make at most **three** distinct gaps, and never coincide.',
    '285.3 ms at 130 BPM. Repeats at 0.618, 1.236, 1.854, 2.472… beats — equidistributed (Weyl), never on a 16th. Feedback 60 %.' ],
  params(t, p) { const r = p <= 1 && p > 0 ? .75 : 1 / PHI; return [['φ', '1.6180339887'], ['1/φ = φ − 1', '0.6180339887'], ['beat @130', fmt(BEAT * 1000, 2) + ' ms'], ['ratio', p === 1 ? '3/4 = 0.75' : p === 0 ? '—' : '1/φ', p >= 2], ['delay time', p === 0 ? '—' : fmt(BEAT * 1000 * r, 1) + ' ms'], ['feedback', '0.60']]; },
  derived(t, p) { const gaps = n => { const pts = Array.from({ length: n }, (_, k) => ((k + 1) / PHI) % 1).sort((a, b) => a - b); const g = pts.map((v, i) => i ? v - pts[i - 1] : v + 1 - pts.at(-1)); return new Set(g.map(x => x.toFixed(4))).size; }; const n = Math.min(9, 2 + Math.floor(t / 2)); return [['convergents', '1, 2, 3/2, 5/3, 8/5, 13/8'], ['|φ − 13/8|', fmt(Math.abs(PHI - 13 / 8), 5)], ['1/(√5·8²)', fmt(1 / (Math.sqrt(5) * 64), 5)], ['taps n', String(n)], ['distinct gaps', p >= 2 ? String(gaps(n)) : (p === 1 ? '1 (on grid)' : '—'), true], ['on a 16th?', p === 1 ? 'every 4th' : 'never']]; },
  formulas() { return ['φ = 1 + 1/(1 + 1/(1 + 1/(1 + ⋯)))   ·   |φ − *p*/*q*| > 1/(√5 *q*²)', '*t*ₙ = *n*·*T*beat/φ   ⇒   {*n*/φ} equidistributed on [0,1)']; },
  scene(t, p, tp) {
    floor(this, -1.5, 8, .5);
    /* golden spiral tube on a cone */
    const spiral = u => { const th = u * Math.PI * 6; const r = 2.2 * Math.pow(PHI, -th / (Math.PI / 2)) ; return [r * Math.cos(th), -1.2 + u * 2.2, r * Math.sin(th)]; };
    addMesh(tube(spiral, 180, .045, 8), { color: P.face, wire: null, spec: .7, cull: true });
    /* beat circle with taps */
    const R = 1.4, ratio = p === 1 ? .75 : 1 / PHI; addMesh(torus(R, .02, 64, 6), { color: P.wire, emissive: true, alpha: .8 }, T.tr(0, -1.49, 0));
    for (let i = 0; i < 16; i++) { const a = i / 16 * Math.PI * 2; addPoint([R * Math.cos(a), -1.47, R * Math.sin(a)], { color: P.dim, r: i % 4 ? .025 : .045, flat: true }); }
    if (p >= 1) { const n = Math.min(12, Math.floor(tp / .55) + 1); for (let k = 1; k <= n; k++) { const pos = (k * ratio) % 1; const a = pos * Math.PI * 2; const h = Math.pow(.62, k) * 1.6; const on = Math.abs(pos * 16 - Math.round(pos * 16)) < 1e-6; const x = R * Math.cos(a), z = R * Math.sin(a);
      addLine([x, -1.47, z], [x, -1.47 + h, z], { color: on ? P.dim : P.emit, lw: 4 }); addMesh(sphere(.07, 10, 6), { color: on ? P.dim : P.emit, spec: .8, cull: true }, T.tr(x, -1.47 + h, z)); addLabel([x * 1.12, -1.4, z * 1.12], fmt(k * ratio, 3), { color: on ? P.dim : P.text, size: 12 }); } }
    addLabel([0, 1.25, 0], p === 0 ? 'φ = (1 + √5)/2' : `ratio ${p === 1 ? '3/4' : '1/φ'} → ${fmt(BEAT * 1000 * ratio, 1)} ms`, { color: P.text, size: 18 });
  } },

/* 05 ─ PRIMES / CONJUNCTIONS */
{ short: 'primes', title: 'PRIME LOOP LENGTHS', tags: 'lcm · conjunctions · P(coprime) = 1/ζ(2) = 6/π² · state space', pal: 'violet', obj: 'EP05_orrery_3_5_7',
  phases: [
    'Three loops of 3, 5 and 7 bars are three planets: a conjunction of all three every lcm = **105** bars = 3:14 at 130 BPM. Kepler for the dancefloor.',
    'Powers of two are the worst case: lcm(4, 8, 16) = 16. The structure repeats every 29.5 s and everyone in the room counts it without knowing.',
    'Two random lengths are coprime with probability 1/ζ(2) = 6/π² ≈ **60.8 %**. Three primes make it a certainty — and lcm = the product.',
    '5·7·11 = 385 bars = 11:50, longer than the track itself. Minimum material, **maximum state space**: 385 distinct bars from three clips.' ],
  params(t, p) { const L = p === 1 ? [4, 8, 16] : p === 3 ? [5, 7, 11] : [3, 5, 7]; const l = L.reduce((a, b) => a * b / gcd(a, b)); return [['loop A', L[0] + ' bars'], ['loop B', L[1] + ' bars'], ['loop C', L[2] + ' bars'], ['lcm(A,B,C)', l + ' bars', true], ['= seconds', fmt(l * BAR, 1) + ' s'], ['= mm:ss', `${Math.floor(l * BAR / 60)}:${String(Math.round(l * BAR % 60)).padStart(2, '0')}`]]; },
  derived(t, p) { const L = p === 1 ? [4, 8, 16] : p === 3 ? [5, 7, 11] : [3, 5, 7]; const bar = Math.floor(t / BAR * 2.5); const l = L.reduce((a, b) => a * b / gcd(a, b)); return [['bar counter', String(bar % l).padStart(3, '0') + ' / ' + l], ['state (A,B,C)', `(${L.map(x => bar % x).join(',')})`, true], ['pairwise gcd', L.map((x, i) => gcd(x, L[(i + 1) % 3])).join(', ')], ['P(coprime)', '6/π² = 0.6079'], ['ζ(2)', 'π²/6 = 1.6449'], ['next conjunction', (l - bar % l) + ' bars']]; },
  formulas() { return ['lcm(*a*,*b*,*c*) = *abc*  ⇔  pairwise coprime', 'P(gcd(*a*,*b*) = 1) = 1/ζ(2) = 6/π² ≈ 0.6079']; },
  scene(t, p) {
    floor(this, -1.5);
    const Ls = p === 1 ? [4, 8, 16] : p === 3 ? [5, 7, 11] : [3, 5, 7]; const cols = [P.wire, P.emit, P.text]; const bar = t / BAR * 2.5;
    addMesh(sphere(.22, 14, 10), { color: P.emit, spec: 1, cull: true }); addPoint([0, 0, 0], { color: P.emit, r: .2, glow: true, alpha: .7 });
    Ls.forEach((len, i) => { const R = .7 + i * .62; addMesh(torus(R, .012, 72, 5), { color: P.dim, emissive: true, alpha: .7 });
      for (let s = 0; s < len; s++) { const a = -Math.PI / 2 + s / len * Math.PI * 2; addPoint([R * Math.cos(a), 0, R * Math.sin(a)], { color: cols[i], r: .03, flat: true, alpha: .6 }); }
      const a = -Math.PI / 2 + (bar % len) / len * Math.PI * 2; const q = [R * Math.cos(a), 0, R * Math.sin(a)];
      addMesh(sphere(.1 + i * .02, 12, 8), { color: cols[i], spec: .9, cull: true }, T.tr(...q)); addLine([0, 0, 0], q, { color: cols[i], lw: 1, alpha: .5 }); addLabel([q[0] * 1.15, .1, q[2] * 1.15], len + ' bars', { color: cols[i], size: 12 }); });
    const l = Ls.reduce((a, b) => a * b / gcd(a, b)); const conj = Math.floor(bar) % l === 0;
    if (conj) addMesh(torus(2.4, .03, 64, 6), { color: P.text, emissive: true });
    addLabel([0, 1.3, 0], `bar ${String(Math.floor(bar) % l).padStart(3, '0')} / ${l}`, { color: P.text, size: 18 });
  } },

/* 06 ─ KICK TUNING (waterfall) */
{ short: 'kick hz', title: 'KICK TUNING', tags: '12-TET · beating in the sub · 3:2 = 701.955 ¢ · FFT bin resolution', pal: 'blood', obj: 'EP06_kick_waterfall',
  phases: [
    '12-TET: *f*(*n*) = 440·2^(*n*/12). The kick\'s pitch sweep decays onto a steady body — a frequency, therefore a **note**. Here: 49.0 Hz = G1.',
    'G1 = 48.999 Hz under a bass at A1 = 55.000 Hz: Δ = 6.0 Hz. In the sub range a whole tone is not an interval, it is a **beating frequency**.',
    'Tune the body to the root, or a fifth below: D1 = 36.708 Hz. 3:2 = 701.955 ¢, only 1.955 ¢ off 12-TET: harmonics 3 and 2 meet at 110.12 vs 110.00 Hz.',
    'One semitone at 49 Hz is **2.9 Hz** — below one bin of a 4096-point FFT at 48 kHz (11.72 Hz/bin). The analyser cannot resolve it. The sub can.' ],
  params(t, p) { const body = p >= 2 ? 55 : 49; return [['A4 reference', '440.000 Hz'], ['kick body', fmt(p >= 2 ? 55 : 48.999, 3) + ' Hz', true], ['nearest note', p >= 2 ? 'A1  (n = −36)' : 'G1  (n = −38)'], ['bass', '55.000 Hz  A1'], ['sweep start', fmt(body * 3.2, 1) + ' Hz'], ['decay τ', '0.45 s']]; },
  derived(t, p) { const body = p >= 2 ? 55 : 48.999; return [['Δf kick−bass', fmt(Math.abs(55 - body), 3) + ' Hz', p < 2], ['beat period', body === 55 ? '∞' : fmt(1 / Math.abs(55 - body), 3) + ' s'], ['interval', body === 55 ? 'unison' : fmt(1200 * Math.log2(55 / body), 1) + ' ¢'], ['semitone @49 Hz', fmt(49 * (Math.pow(2, 1 / 12) - 1), 3) + ' Hz'], ['FFT bin 4096/48k', fmt(48000 / 4096, 2) + ' Hz'], ['fifth below A1', 'D1 = 36.708 Hz']]; },
  formulas() { return ['*f*(*n*) = 440 · 2^(*n*/12)   ·   *n* = 12·log₂(*f*/440)', '3:2 = 1200·log₂(1.5) = 701.955 ¢   ·   Δ*f*bin = *f*s / *N*']; },
  scene(t, p) {
    floor(this, -1.5, 8, .5);
    const body = p >= 2 ? 55 : 49, fmin = 30, fmax = 120; const fx = f => lerp(-2.6, 2.6, Math.log(f / fmin) / Math.log(fmax / fmin));
    /* waterfall: 24 slices back in time, each a spectrum of the kick at that age */
    const slices = 26; for (let s = 0; s < slices; s++) { const age = s * .02 + ((t * 1.3) % BEAT) * 0; const hit = ((t * 1.3) % BEAT) + s * .018; const f0 = body * (1 + 2.2 * Math.exp(-hit * 28)), amp = Math.exp(-hit * 2.2); const z = lerp(1.9, -1.9, s / slices);
      const N = 48; let prev = null; for (let i = 0; i <= N; i++) { const f = fmin * Math.pow(fmax / fmin, i / N); const g = (c, w, a) => a * Math.exp(-((f - c) ** 2) / (2 * w * w)); const y = -1.3 + 2.2 * Math.max(g(f0, 2.5 + 7 * hit, amp), g(body, 1.6, .85), p >= 1 ? g(55, 1.2, .5) : 0, g(2 * body, 4, .18)); const q = [fx(f), y, z]; if (prev) addLine(prev, q, { color: s === 0 ? P.emit : P.wire, lw: s === 0 ? 2.6 : 1.2, alpha: s === 0 ? 1 : .85 - s / slices * .7 }); prev = q; } }
    /* note axis */
    const notes = [['E1', 41.203], ['G1', 48.999], ['A1', 55], ['C2', 65.406], ['E2', 82.407], ['A2', 110]]; notes.forEach(([nm, f]) => { addLine([fx(f), -1.5, 2.0], [fx(f), -1.5, -2.0], { color: P.dim, lw: 1, alpha: .5 }); addLabel([fx(f), -1.55, 2.25], `${nm} ${fmt(f, 1)}`, { color: nm === 'G1' || nm === 'A1' ? P.text : P.dim, size: 11, dy: 10 }); });
    if (p >= 1) { addLine([fx(55), -1.5, 2.0], [fx(55), 1.0, 2.0], { color: P.emit, lw: 2, alpha: .9 }); addLabel([fx(55), 1.1, 2], 'bass A1', { color: P.emit, size: 13 }); }
    addLine([fx(body), -1.5, 2.0], [fx(body), .6, 2.0], { color: P.wire, lw: 2, alpha: .9 }); addLabel([fx(body), .75, 2], `kick ${fmt(body, 0)} Hz`, { color: P.wire, size: 13 });
    if (p === 3) { addLine([fx(49), 1.2, 2], [fx(51.913), 1.2, 2], { color: P.text, lw: 3 }); addLabel([fx(50.4), 1.3, 2], '1 semitone = 2.9 Hz', { color: P.text, size: 12 }); }
  } },

/* 07 ─ TORUS KNOT LFO (studio light) */
{ short: 'lfo knot', title: 'TEMPO-LOCKED LFOs', tags: 'f = BPM/(60·β) · (p,q) torus knots · Kronecker–Weyl density', pal: 'studio', obj: 'EP07_trefoil_LFO',
  phases: [
    '*f* = BPM/(60·β). At 130: 1 bar → 0.5417 Hz, 4 bars → 0.1354, 8 bars → 0.0677, 16 bars → **0.0339 Hz**. A free LFO at 0.1 Hz is none of these.',
    'Two LFOs with rational ratio *p*:*q* draw a (*p*,*q*) torus knot. 3 cycles against 2 per 8 bars: a **trefoil**, closing every 14.77 s.',
    'An irrational ratio never closes: the curve is dense on the torus — Kronecker–Weyl. That is the mathematically correct meaning of "random".',
    'Hardware without a sync input: type the hertz. 0.1354 on the Buchla **is** 4 bars at 130, to the sample. Tempo sync is a division.' ],
  params(t, p) { return [['BPM', '130'], ['1 bar', fmt(BAR, 4) + ' s'], ['LFO A', p >= 1 ? '3 / 8 bars = ' + fmt(3 / (8 * BAR), 4) + ' Hz' : fmt(1 / BAR, 4) + ' Hz', true], ['LFO B', p >= 1 ? '2 / 8 bars = ' + fmt(2 / (8 * BAR), 4) + ' Hz' : '—'], ['ratio p:q', p === 2 ? '√2 : 1' : p >= 1 ? '3 : 2' : '—'], ['knot', p === 2 ? 'dense (never closes)' : p >= 1 ? '(3,2) trefoil' : '—']]; },
  derived(t, p) { const rows = [1, 2, 4, 8, 16].map(b => [b + ' bar' + (b > 1 ? 's' : ''), fmt(BPM / 60 / (4 * b), 4) + ' Hz', b === 16 && p === 0]); rows.push(['re-phase', p === 2 ? 'never' : fmt(8 * BAR, 2) + ' s']); return rows; },
  formulas() { return ['*f*LFO = BPM / (60 · β)   ·   (*p*,*q*) knot: *θ* = *p*·2π*t*/*T*, *φ* = *q*·2π*t*/*T*', 'gcd(*p*,*q*) = 1 ⇒ one closed curve · *p*/*q* ∉ ℚ ⇒ dense on *T*²']; },
  scene(t, p) {
    floor(this, -1.5);
    const R = 1.5, r = .6; addMesh(torus(R, r, 40, 16), { color: P.face, wire: rgba(P.wire, .9), wireAlpha: .12, spec: .5, cull: true, alpha: .55 });
    const pq = p === 2 ? [Math.SQRT2, 1] : [3, 2]; const T8 = 8 * BAR;
    const knot = u => { const a = pq[1] * 2 * Math.PI * u, b = pq[0] * 2 * Math.PI * u; return [(R + (r + .04) * Math.cos(b)) * Math.cos(a), (r + .04) * Math.sin(b), (R + (r + .04) * Math.cos(b)) * Math.sin(a)]; };
    if (p >= 1) { const span = p === 2 ? 2.6 : 1; addMesh(tube(u => knot(u * span), p === 2 ? 420 : 200, .035, 8), { color: P.emit, emissive: false, spec: .8, cull: true }); }
    const u = p === 2 ? (t / T8) : (t / T8) % 1; const q = knot(u); addMesh(sphere(.12, 12, 8), { color: P.wire, spec: 1, cull: true }, T.tr(...q)); addPoint(q, { color: P.emit, r: .09, glow: true, alpha: .8 });
    if (p === 0) { /* five LFO sine ribbons instead */ [1, 2, 4, 8, 16].forEach((b, i) => { const f = BPM / 60 / (4 * b); let prev = null; for (let k = 0; k <= 80; k++) { const x = lerp(-2.6, 2.6, k / 80); const y = .9 - i * .45 + .18 * Math.sin(2 * Math.PI * f * (k / 80 * 29.54 + t * .3)); const q2 = [x, y, 1.6 - i * .2]; if (prev) addLine(prev, q2, { color: i === 4 ? P.emit : P.wire, lw: 1.6 }); prev = q2; } addLabel([-2.75, .9 - i * .45, 1.6 - i * .2], fmt(f, 4), { color: P.text, size: 12 }); }); }
    addLabel([0, r + 1.0, 0], p === 0 ? '5 tempo-locked rates over 16 bars' : p === 2 ? 'ratio √2 : 1 — dense, never closes' : `(3,2) trefoil · t/T = ${fmt(u, 3)}`, { color: P.text, size: 17 });
  } },

/* 08 ─ LFSR (hazard) */
{ short: 'lfsr', title: 'LFSR SEQUENCER', tags: 'GF(2) · primitive polynomials · period 2ᵐ − 1 · Turing Machine module', pal: 'hazard', obj: 'EP08_LFSR_x4_x3_1',
  phases: [
    'A 4-bit shift register with feedback *s*[*n*+4] = *s*[*n*+3] ⊕ *s*[*n*]: the "Turing Machine" module, written in **GF(2)**.',
    '*x*⁴ + *x*³ + 1 is primitive over GF(2): the sequence has maximal period 2⁴ − 1 = **15**, visiting every non-zero state exactly once.',
    '15 against a 16-step bar: the pattern slides one step per bar and closes after lcm(15,16) = 240 steps = **15 bars**. Polyrhythm from a polynomial.',
    'Flip one tap to *x*⁴ + *x*² + 1 = (*x*² + *x* + 1)²: not primitive, the period collapses to **6**. The polynomial is the groove.' ],
  params(t, p) { return [['register bits m', '4'], ['polynomial', p === 3 ? 'x⁴ + x² + 1' : 'x⁴ + x³ + 1', true], ['primitive?', p === 3 ? 'no  (x²+x+1)²' : 'yes'], ['seed', '1 0 0 1'], ['period', p === 3 ? '6' : '2⁴ − 1 = 15'], ['vs bar of 16', p === 3 ? 'lcm = 48 → 3 bars' : 'lcm = 240 → 15 bars']]; },
  derived(t, p) { const seq = lfsr(p === 3 ? [3, 1] : [4, 1], 240); const i = Math.floor(t / S16) % 240; const w = seq.slice(0, p === 3 ? 6 : 15); return [['output bit', String(seq[i]), true], ['state', seq.slice(i, i + 4).join(' ')], ['one period', w.join('')], ['Hamming weight', w.reduce((a, b) => a + b, 0) + ' / ' + w.length], ['step in bar', String(i % 16)], ['slide vs bar', String((i % 16) - (i % (p === 3 ? 6 : 15)) ) + ' steps']]; },
  formulas() { return ['*s*ₙ₊₄ = *s*ₙ₊₃ ⊕ *s*ₙ   ⇔   *x*⁴ + *x*³ + 1 over GF(2)', 'primitive ⇒ period 2^*m* − 1  ·  (*x*²+*x*+1)² ⇒ period 6']; },
  scene(t, p) {
    floor(this, -1.5, 8, .5);
    const taps = p === 3 ? [3, 1] : [4, 1]; const seq = lfsr(taps, 240); const i = Math.floor(t / S16) % 240; const frac = ((t / S16) % 1);
    /* 4 register cubes */
    for (let k = 0; k < 4; k++) { const bit = seq[i + k]; const x = -1.8 + k * 1.2; addMesh(box(.7, .7, .7), { color: bit ? P.wire : P.face, wire: P.dim, wireAlpha: .7, spec: bit ? 1 : .3, cull: true, emissive: !!bit, alpha: bit ? .95 : 1 }, T.tr(x, 0, 0)); addLabel([x, .6, 0], `s[n+${k}] = ${bit}`, { color: P.text, size: 13 }); }
    /* xor node + feedback arc */
    const tapX = k => -1.8 + k * 1.2; const xorP = [1.2, 1.5, 0];
    addMesh(sphere(.22, 12, 8), { color: P.emit, spec: 1, cull: true }, T.tr(...xorP)); addLabel(xorP, '⊕', { color: '#000', size: 18, dy: 6 });
    const tapsIdx = p === 3 ? [2, 0] : [3, 0]; tapsIdx.forEach(k => addLine([tapX(k), .35, 0], xorP, { color: P.emit, lw: 2 }));
    addLine(xorP, [3.0, 1.5, 0], { color: P.emit, lw: 2 }); addLine([3.0, 1.5, 0], [3.0, 0, 0], { color: P.emit, lw: 2 }); addLabel([3.0, -.3, 0], `in: ${seq[i + 4]}`, { color: P.text, size: 13, dy: 10 });
    /* output history as a row of bars on the floor, 30 steps */
    for (let k = 0; k < 30; k++) { const idx = i - k; if (idx < 0) continue; const z = 2.2 - k * .15 * 0 + 1.0; const x = 2.6 - k * .18; const b = seq[idx]; addLine([x, -1.49, 1.9], [x, -1.49 + (b ? .5 : .05), 1.9], { color: k === 0 ? P.emit : b ? P.wire : P.dim, lw: 5, alpha: 1 - k / 34 }); }
    for (let k = 0; k < 16; k++) addPoint([2.6 - k * .18, -1.49, 2.3], { color: k % 4 ? P.dim : P.text, r: k % 4 ? .02 : .035, flat: true });
    addLabel([-2.3, -1.3, 1.9], 'out', { color: P.dim, size: 12 }); addLabel([-2.3, -1.3, 2.3], 'bar', { color: P.dim, size: 12 });
  } },

/* 09 ─ SWING / WEBER (rose) */
{ short: 'swing ms', title: 'SWING IN MILLISECONDS', tags: 'Δt = (s − ½)·T₈ · Weber–Fechner · JND · lane asynchrony', pal: 'rose', obj: 'EP09_groove_lanes',
  phases: [
    'Δ*t* = (*s* − ½)·*T*₈. At 130 BPM *T*₈ = 230.77 ms: 54 % → **+9.23 ms**, 58 % → +18.46 ms, 62 % → +27.69 ms. MPC swing, decoded.',
    'Weber–Fechner: the just-noticeable timing difference is ≈ 2.5 % of the interval, ≈ **5.8 ms** on a 16th. 54 % is above threshold and below awareness.',
    'Swing in % is tempo-relative; in ms it is not. 54 % at 140 BPM is 8.57 ms — the same setting is **7 % tighter**. Think in milliseconds.',
    'Kick at 50 %, hats at 54–56 %: the **asynchrony between lanes** is the groove, not the swing value. One lane swung alone is just late.' ],
  params(t, p, tp) { const s = swingAt(t); return [['BPM', '130'], ['T₈ (8th)', fmt(BEAT * 500, 2) + ' ms'], ['T₁₆ straight', fmt(S16 * 1000, 2) + ' ms'], ['swing s', Math.round(s * 100) + ' %', true], ['Δt = (s−½)·T₈', '+' + fmt((s - .5) * BEAT * 500, 2) + ' ms'], ['off-16th at', fmt(s * BEAT * 500, 2) + ' ms']]; },
  derived(t, p) { const s = swingAt(t); return [['Weber fraction', '≈ 2.5 %'], ['JND on 16th', fmt(.025 * S16 * 1000, 2) + ' ms'], ['Δt / JND', fmt((s - .5) * BEAT * 500 / (.025 * S16 * 1000), 2) + ' ×', p === 1], ['@140 BPM, same %', '+' + fmt((s - .5) * 60 / 140 * 500, 2) + ' ms'], ['kick lane', '50 %  (0 ms)'], ['lane asynchrony', '+' + fmt((s - .5) * BEAT * 500, 2) + ' ms', p === 3]]; },
  formulas() { return ['Δ*t* = (*s* − ½)·*T*₈   ·   *T*₈ = 30/BPM s', 'Weber: Δ*T*/*T* ≈ const ≈ 0.025   ⇒   JND₁₆ ≈ 5.8 ms @130']; },
  scene(t, p) {
    floor(this, -1.5, 8, .5);
    const s = swingAt(t); const T8 = BEAT / 2; const x0 = -2.6, x1 = 2.6; const k = (x1 - x0) / (2 * BEAT); /* 2 beats across */
    const lanes = [['kick', .5, 0.9, P.wire], ['hat', s, -0.9, P.emit]];
    lanes.forEach(([nm, sw, z, col]) => { addLine([x0, -1.49, z], [x1, -1.49, z], { color: P.dim, lw: 1.2 }); addLabel([x0 - .25, -1.4, z], nm, { color: col, size: 13 });
      for (let i = 0; i < 9; i++) { const base = i * BEAT / 4; const off = i % 2 === 1; const tt = base + (off ? (sw - .5) * T8 : 0); const x = x0 + tt * k; const h = nm === 'kick' ? (i % 4 === 0 ? 1.5 : 0) : (off ? .7 : 1.0); if (h === 0) continue;
        const ph = ((t % (2 * BEAT)) - tt); const hot = ph > 0 && ph < .09; addMesh(box(.16, h, .16), { color: hot ? P.text : col, wire: P.dim, wireAlpha: .35, spec: .7, cull: true }, T.tr(x, -1.49 + h / 2, z));
        if (off && nm === 'hat' && sw > .5) { const xs = x0 + base * k; addLine([xs, -1.49 + h + .15, z], [x, -1.49 + h + .15, z], { color: P.text, lw: 2 }); addLabel([(x + xs) / 2, -1.49 + h + .25, z], '+' + fmt((sw - .5) * T8 * 1000, 1) + ' ms', { color: P.text, size: 11 }); } } });
    /* playhead plane */
    const px = x0 + (t % (2 * BEAT)) * k; addLine([px, -1.5, 1.4], [px, -1.5, -1.4], { color: P.text, lw: 1.2, alpha: .6 }); addLine([px, -1.5, -1.4], [px, .6, -1.4], { color: P.text, lw: 1.2, alpha: .4 });
    addLabel([0, 1.2, 0], `swing ${Math.round(s * 100)} %  →  +${fmt((s - .5) * T8 * 1000, 2)} ms`, { color: P.text, size: 18 });
  } },

/* 10 ─ FOURIER (blender grey) */
{ short: 'fourier', title: 'FOURIER & THE CLICK', tags: 'partial sums · Gibbs 8.949 % · δ(t) · Gabor uncertainty', pal: 'blender', obj: 'EP10_partial_sums',
  phases: [
    'Square wave = 4/π·Σ sin((2*k*+1)*x*)/(2*k*+1): odd harmonics with amplitude 1/*n*. Each ribbon behind adds **one** more term.',
    'Near the jump every partial sum overshoots by (2/π)·Si(π) − 1 = **8.949 %**, for any finite *N*. Gibbs (1899): more harmonics narrow it, never remove it.',
    'Limit case δ(*t*): flat spectrum, zero phase. Gabor: Δ*t*·Δ*f* ≥ 1/(4π). A click of σ = 1 ms spans σ*f* ≥ **79.6 Hz** — the only sound that hits every band.',
    'Punch is bandwidth, not level: halve the transient length and the spectrum **doubles** in width. Don\'t EQ the click — shorten it.' ],
  params(t, p, tp) { const N = p === 0 ? Math.max(1, Math.floor(1 + easeOut(tp, 0, 6.5) * 6)) : p === 1 ? 15 : 64; return [['fundamental', '110.00 Hz  A2'], ['harmonics N', p >= 2 ? '∞ (impulse)' : String(N), true], ['highest partial', p >= 2 ? '→ ∞' : fmt(110 * (2 * N - 1), 0) + ' Hz'], ['amplitude law', p >= 2 ? 'flat' : '1 / n, odd n'], ['Si(π)', '1.851937'], ['overshoot', '(2/π)·Si(π) − 1']]; },
  derived(t, p) { return [['Gibbs overshoot', '8.949 %', p === 1], ['square RMS', '1.000  (crest 1.0)'], ['sine RMS', '0.707  (crest 1.414)'], ['Δt·Δf bound', '1/(4π) = 0.0796'], ['σt = 1 ms → σf', '≥ 79.6 Hz', p === 2], ['σt = 0.5 ms → σf', '≥ 159.2 Hz', p === 3]]; },
  formulas() { return ['sq(*x*) = (4/π)·Σₖ sin((2*k*+1)*x*)/(2*k*+1)', 'overshoot = (2/π)·Si(π) − 1 = 0.08949   ·   σ*t*·σ*f* ≥ 1/(4π)']; },
  scene(t, p, tp) {
    floor(this, -1.5, 8, .5);
    const N = p === 0 ? Math.max(1, Math.floor(1 + easeOut(tp, 0, 6.5) * 6)) : p === 1 ? 15 : 0; const imp = p >= 2;
    const layers = imp ? 1 : N; const x0 = -2.6, x1 = 2.6;
    for (let L = 1; L <= Math.max(1, layers); L++) { const z = lerp(1.9, -1.9, layers > 1 ? (L - 1) / (layers - 1) : .5); const K = L; let prev = null; const col = L === layers ? P.emit : P.wire;
      for (let i = 0; i <= 220; i++) { const u = i / 220; let y; if (imp) { const d = (u - .5) * 44; y = Math.abs(d) < .04 ? 1 : Math.sin(Math.PI * d) / (Math.PI * d); } else { y = 0; for (let k = 0; k < K; k++) { const n = 2 * k + 1; y += Math.sin(n * (u * Math.PI * 4 + t * 2)) / n; } y *= 4 / Math.PI; }
        const q = [lerp(x0, x1, u), y * .8, z]; if (prev) addLine(prev, q, { color: col, lw: L === layers ? 2.4 : 1.1, alpha: L === layers ? 1 : .35 + .5 * L / layers }); prev = q; }
      if (!imp) addLabel([x1 + .2, 0, z], 'N=' + K, { color: L === layers ? P.emit : P.dim, size: 11 }); }
    if (!imp) { let prev = null; for (let i = 0; i <= 220; i++) { const u = i / 220; const q = [lerp(x0, x1, u), (Math.sin(u * Math.PI * 4 + t * 2) >= 0 ? 1 : -1) * .8, 2.3]; if (prev && Math.sign(prev[1]) === Math.sign(q[1])) addLine(prev, q, { color: P.dim, lw: 1, alpha: .6 }); prev = q; } addLabel([x1 + .2, 0, 2.3], 'target', { color: P.dim, size: 11 }); }
    /* spectrum bars on the floor, right side */
    for (let n = 1; n <= 31; n++) { const a = imp ? .8 : (n % 2 && n <= 2 * N - 1) ? 1 / n : 0; if (!a) continue; const x = lerp(-2.4, 2.4, (n - 1) / 30); addMesh(box(.1, a * 1.2, .1), { color: imp ? P.emit : n === 1 ? P.text : P.wire, spec: .6, cull: true }, T.tr(x, -1.49 + a * .6, -2.6)); }
    addLabel([0, 1.3, 0], imp ? 'δ(t): all harmonics, equal amplitude' : `N = ${N} odd harmonic${N > 1 ? 's' : ''} · overshoot 8.949 %`, { color: P.text, size: 17 });
  } },

/* 11 ─ LOGISTIC MAP 3D (lagoon) */
{ short: 'logistic', title: 'THE LOGISTIC MAP', tags: 'period doubling · Feigenbaum δ, α · Lyapunov exponent · gen~', pal: 'lagoon', obj: 'EP11_logistic_cloud',
  phases: [
    '*x*ₙ₊₁ = *r*·*x*ₙ(1 − *x*ₙ). Below *r* = 3 one fixed point. Then period 2, 4, 8 … at *r* = 3, 3.4495, 3.5441, 3.5644 — doubling **faster and faster**.',
    'The bifurcation intervals shrink by δ = **4.6692…** — Feigenbaum\'s constant, universal for every unimodal map. Accumulation at *r*∞ = 3.56995: chaos.',
    'Lyapunov exponent λ = ⟨ln|*r*(1 − 2*x*)|⟩ > 0 is the definition of chaos. At *r* = 4, λ = ln 2: exactly **one bit** of unpredictability per step.',
    'Quantise *x* to 16 notes and map *r* to a fader (2.9 → 3.99): one parameter sweeps from a loop to a melody to **noise**. gen~: History x; x = r*x*(1-x).' ],
  params(t, p, tp) { const r = rAt(t, p, tp); return [['map', 'x ↦ r·x(1−x)'], ['r', fmt(r, 4), true], ['r∞ (onset of chaos)', '3.569946'], ['δ (Feigenbaum)', '4.669202'], ['α (Feigenbaum)', '2.502908'], ['quantisation', '16 notes']]; },
  derived(t, p, tp) { const r = rAt(t, p, tp); let x = .3, lam = 0; for (let i = 0; i < 400; i++) { x = r * x * (1 - x); if (i >= 100) lam += Math.log(Math.abs(r * (1 - 2 * x)) + 1e-12); } lam /= 300; const per = r < 3 ? '1' : r < 3.4495 ? '2' : r < 3.5441 ? '4' : r < 3.5644 ? '8' : r < 3.5699 ? '16+' : (r > 3.8284 && r < 3.8415 ? '3 (window)' : 'chaotic'); return [['period', per], ['λ (Lyapunov)', fmt(lam, 4) + (lam > 0 ? '  > 0' : '  < 0'), lam > 0], ['bits / step', lam > 0 ? fmt(lam / Math.LN2, 3) : '0'], ['fixed point 1−1/r', fmt(1 - 1 / r, 4)], ['x now', fmt(x, 4)], ['note', String(Math.floor(x * 16))]]; },
  formulas() { return ['*x*ₙ₊₁ = *r* *x*ₙ(1 − *x*ₙ)   ·   λ = lim (1/*N*)·Σ ln|*r*(1 − 2*x*ₙ)|', 'δ = lim (*r*ₙ − *r*ₙ₋₁)/(*r*ₙ₊₁ − *r*ₙ) = 4.6692…   ·   λ(*r*=4) = ln 2']; },
  scene(t, p, tp) {
    floor(this, -1.5, 8, .5);
    const rmin = 2.8, rmax = 4; const prog = easeOut(t, 0, 10); const cols = Math.floor(prog * 150);
    /* point cloud (r, x_n, x_{n+1}) */
    for (let c = 0; c < cols; c++) { const r = lerp(rmin, rmax, c / 150); let x = .31 + .17 * ((c * 7919) % 13) / 13; for (let i = 0; i < 100; i++) x = r * x * (1 - x); for (let i = 0; i < 28; i++) { const x2 = r * x * (1 - x); addPoint([lerp(-2.6, 2.6, c / 150), lerp(-1.3, 1.3, x), lerp(1.6, -1.6, x2)], { color: P.wire, r: .018, flat: true, alpha: .8 }); x = x2; } }
    const r = rAt(t, p, tp); const rx = lerp(-2.6, 2.6, (r - rmin) / (rmax - rmin));
    addLine([rx, -1.5, 1.8], [rx, -1.5, -1.8], { color: P.emit, lw: 1.5 }); addLine([rx, -1.5, 1.8], [rx, 1.5, 1.8], { color: P.emit, lw: 1.5 });
    /* current orbit as bright spheres */
    let x = .3; for (let i = 0; i < 60; i++) x = r * x * (1 - x); for (let i = 0; i < 16; i++) { const x2 = r * x * (1 - x); addMesh(sphere(.05, 8, 6), { color: P.emit, spec: 1, cull: true, emissive: i === 15 }, T.tr(rx, lerp(-1.3, 1.3, x), lerp(1.6, -1.6, x2))); x = x2; }
    [[3, 'r=3'], [3.4495, '3.4495'], [3.5699, 'r∞'], [3.8284, 'period 3']].forEach(([rv, l]) => { const xx = lerp(-2.6, 2.6, (rv - rmin) / (rmax - rmin)); if (rv <= lerp(rmin, rmax, prog)) { addLine([xx, -1.5, 1.8], [xx, -1.5, -1.8], { color: P.dim, lw: 1, alpha: .6 }); addLabel([xx, -1.55, 2.05], l, { color: P.dim, size: 11, dy: 10 }); } });
    addLabel([-2.6, 1.45, 1.8], 'xₙ ↑', { color: P.dim, size: 12 }); addLabel([2.7, -1.5, -1.8], 'xₙ₊₁ →', { color: P.dim, size: 12 }); addLabel([2.8, -1.45, 1.9], 'r →', { color: P.dim, size: 12 });
    addLabel([rx, 1.65, 1.8], `r = ${fmt(r, 3)}`, { color: P.emit, size: 15 });
  } },

/* 12 ─ RISSET HELIX (lavender) */
{ short: 'risset', title: 'RISSET RHYTHM', tags: 'τ₀·2^(t/T) · Shepard helix · Hann gains · constant mean log-tempo', pal: 'lavender', obj: 'EP12_shepard_helix',
  phases: [
    'τ(*t*) = τ₀·2^(*t*/*T*): tempo rising exponentially, three layers an **octave** apart in tempo, gains on a raised cosine over log τ.',
    'After *T* the tempo has doubled, but layer *k*+1 now carries exactly the amplitude layer *k* had — at τ₀. The state is **periodic**, the percept is not.',
    'Shepard (1964) for pitch, Risset (1971) for rhythm: a circular dimension built from a log scale and a bell-shaped envelope. The helix is the geometry of the illusion.',
    '*T* = 8 bars = 14.77 s. Three clips at ×1, ×2, ×4 with Hann gains offset by *T*/3. The mean log-tempo is constant: **the floor never lands**.' ],
  params(t, p) { const Tt = 7.5; return [['τ₀', '130 BPM'], ['T (doubling)', fmt(Tt, 2) + ' s  (8 bars in clip)'], ['layers', '3  (×1, ×2, ×4)'], ['gain window', 'Hann over log₂ τ'], ['t / T', fmt((t / Tt) % 1, 3), true], ['perceived τ', fmt(130 * Math.pow(2, (t / Tt) % 1), 1) + ' BPM']]; },
  derived(t, p) { const Tt = 7.5, ph = (t / Tt) % 1; const g = l => { const u = l / 2; return (u <= 0 || u >= 1) ? 0 : .5 - .5 * Math.cos(2 * Math.PI * u); }; return [['layer 0 (×1)', fmt(130 * Math.pow(2, ph), 0) + ' BPM · g=' + fmt(g(ph), 2)], ['layer 1 (×2)', fmt(130 * Math.pow(2, ph + 1), 0) + ' BPM · g=' + fmt(g(ph + 1), 2), true], ['layer 2 (×4)', fmt(130 * Math.pow(2, ph + 2), 0) + ' BPM · g=' + fmt(g(ph + 2), 2)], ['Σ gains', fmt(g(ph) + g(ph + 1) + g(ph + 2), 3)], ['mean log₂ τ', 'const = log₂ 260'], ['loop point', 'seamless at t = T']]; },
  formulas() { return ['τₖ(*t*) = τ₀·2^(*t*/*T* + *k*)   ·   *g*ₖ(*t*) = ½[1 − cos(2π(*t*/*T* + *k*)/3)]', 'Σₖ *g*ₖ = const   ·   state(*t* + *T*) = state(*t*) · shift(*k*)']; },
  scene(t, p) {
    floor(this, -1.5, 8, .5);
    const Tt = 7.5, ph = (t / Tt) % 1; const g = l => { const u = l / 2; return (u <= 0 || u >= 1) ? 0 : .5 - .5 * Math.cos(2 * Math.PI * u); };
    /* helix: angle = phase within the cycle, height = log2 tempo */
    const helix = u => { const l = lerp(-1, 3, u); const a = l * 2 * Math.PI; return [1.6 * Math.cos(a), lerp(-1.3, 1.3, (l + 1) / 4), 1.6 * Math.sin(a)]; };
    addMesh(tube(helix, 260, .03, 6), { color: P.face, spec: .7, cull: true });
    for (let l = -1; l <= 3; l++) { const y = lerp(-1.3, 1.3, (l + 1) / 4); addMesh(torus(1.6, .008, 48, 4), { color: P.dim, emissive: true, alpha: .5 }, T.tr(0, y, 0)); addLabel([1.9, y, 0], `×${Math.pow(2, l)} · ${130 * Math.pow(2, l)}`, { color: P.dim, size: 11 }); }
    for (let k = -1; k <= 2; k++) { const l = k + ph; if (l < -1 || l > 3) continue; const gg = g(l); const q = helix((l + 1) / 4); const col = (k + 10) % 2 ? P.emit : P.wire; addMesh(sphere(.08 + gg * .22, 12, 8), { color: col, spec: 1, cull: true, alpha: .35 + .65 * gg }, T.tr(...q)); if (gg > .05) addPoint(q, { color: col, r: .12 * gg, glow: true, alpha: gg }); addLabel([q[0] * 1.25, q[1] + .1, q[2] * 1.25], fmt(130 * Math.pow(2, l), 0), { color: col, size: 12 }); }
    /* gain window as a curve on the right */
    let prev = null; for (let i = 0; i <= 80; i++) { const l = lerp(-1, 3, i / 80); const q = [2.9, lerp(-1.3, 1.3, (l + 1) / 4), -1.2 + g(l) * 1.0]; if (prev) addLine(prev, q, { color: P.text, lw: 1.5, alpha: .7 }); prev = q; }
    addLabel([2.9, 1.5, -1.2], 'g(log₂ τ)', { color: P.dim, size: 12 });
    addLabel([0, 1.65, 0], `perceived ${fmt(130 * Math.pow(2, ph), 1)} BPM ↑   mean log₂τ = const`, { color: P.text, size: 16 });
  } },
];

/* shared small helpers used by episodes */
function gcd(a, b) { return b ? gcd(b, a % b) : a; }
function lfsr(taps, n) { /* taps as polynomial exponents (excluding 1); 4-bit, seed 1001 */ let s = [1, 0, 0, 1]; const out = []; for (let i = 0; i < n + 4; i++) { out.push(s[0]); const fb = taps.reduce((a, e) => a ^ s[4 - e], 0); s = [s[1], s[2], s[3], fb]; } return out; }
function swingAt(t) { const p = Math.floor(t / PH), tp = t - p * PH; return p === 0 ? (tp < 3 ? .5 : .54) : p === 1 ? (tp < 2.5 ? .54 : tp < 5 ? .58 : .62) : p === 2 ? .54 : .54; }
function rAt(t, p, tp) { return p === 0 ? (tp < 2.5 ? 2.9 : tp < 5 ? 3.2 : 3.5) : p === 1 ? lerp(3.5, 3.5699, clamp(tp / 6)) : p === 2 ? (tp < 4 ? 3.7 : 4.0) : lerp(2.9, 3.99, (Math.sin(tp * .8 - 1.5) + 1) / 2); }

if (typeof module !== 'undefined') module.exports = { REELS, renderFrame, DUR };

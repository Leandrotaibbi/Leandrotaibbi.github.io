/* MATH × TECHNO — 12 reels, 1080×1920, 30 s each.  v2 "lesson" format.
   One big visual, one sentence with one bold word, a small sub-line, a formula.
   Every frame is a pure function of (reel, t): same code for preview and MP4. */

const W = 1080, H = 1920, DUR = 30;
const BPM = 130, BEAT = 60 / BPM, BAR = 4 * BEAT, S16 = BEAT / 4;
const PHASES = 4, PH = DUR / PHASES;            /* 4 sentences × 7.5 s */
const MONO = 'Plex', SANS = 'Archivo', DISP = 'Syne';

const DARK = { bg:'#070809', panel:'#14181b', line:'#262d33', ink:'#ecead9', dim:'#8b938d', dim2:'#525c66', cv:'#e8763c', sig:'#62b2c0', ok:'#2ee08a' };
const LIGHT = { bg:'#ecead9', panel:'#e2dfcb', line:'#cfcbb6', ink:'#111417', dim:'#5a6570', dim2:'#8b938d', cv:'#d4561c', sig:'#1f7482', ok:'#1a9a5c' };
let C = DARK;

/* ───────────── helpers ───────────── */
const clamp = (x, a = 0, b = 1) => Math.max(a, Math.min(b, x));
const lerp = (a, b, k) => a + (b - a) * k;
const ease = (t, a, b) => { const k = clamp((t - a) / (b - a)); return k * k * (3 - 2 * k); };
const easeOut = (t, a, b) => { const k = clamp((t - a) / (b - a)); return 1 - (1 - k) ** 3; };
const rgba = (hex, a) => { const n = parseInt(hex.slice(1), 16); return `rgba(${n >> 16},${(n >> 8) & 255},${n & 255},${a})`; };
const euclid = (k, n, rot = 0) => Array.from({ length: n }, (_, i) => ((((i + rot) * k) % n) < k) ? 1 : 0);
const fmt = (x, d = 2) => x.toFixed(d);
const beatPulse = t => Math.exp(-((t % BEAT) / BEAT) * 7);   /* 1 on the kick, decays */

function font(ctx, size, fam = MONO, w = 400) { ctx.font = `${w} ${size}px ${fam}`; }
function text(ctx, s, x, y, { size = 30, fam = MONO, w = 400, color = C.ink, align = 'left', alpha = 1, ls = 0 } = {}) {
  if (alpha <= 0) return;
  ctx.save(); ctx.globalAlpha = alpha; font(ctx, size, fam, w); ctx.fillStyle = color; ctx.textAlign = align; ctx.textBaseline = 'alphabetic';
  if (ls) ctx.letterSpacing = ls + 'px';
  ctx.fillText(s, x, y); ctx.restore();
}
function rect(ctx, x, y, w, h, fill, stroke, lw = 2) { if (fill) { ctx.fillStyle = fill; ctx.fillRect(x, y, w, h); } if (stroke) { ctx.strokeStyle = stroke; ctx.lineWidth = lw; ctx.strokeRect(x, y, w, h); } }
function line(ctx, x1, y1, x2, y2, color, w = 2, alpha = 1) { ctx.save(); ctx.globalAlpha = alpha; ctx.strokeStyle = color; ctx.lineWidth = w; ctx.beginPath(); ctx.moveTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke(); ctx.restore(); }
function circle(ctx, x, y, r, fill, stroke, lw = 2, alpha = 1) { ctx.save(); ctx.globalAlpha = alpha; ctx.beginPath(); ctx.arc(x, y, r, 0, Math.PI * 2); if (fill) { ctx.fillStyle = fill; ctx.fill(); } if (stroke) { ctx.strokeStyle = stroke; ctx.lineWidth = lw; ctx.stroke(); } ctx.restore(); }
function plot(ctx, fn, x0, x1, y0, yScale, n, color, lw = 3, alpha = 1) {
  ctx.save(); ctx.globalAlpha = alpha; ctx.strokeStyle = color; ctx.lineWidth = lw; ctx.lineJoin = 'round'; ctx.lineCap = 'round'; ctx.beginPath();
  for (let i = 0; i <= n; i++) { const u = i / n, x = lerp(x0, x1, u), y = y0 - fn(u) * yScale; i ? ctx.lineTo(x, y) : ctx.moveTo(x, y); }
  ctx.stroke(); ctx.restore();
}
function glow(ctx, color, blur) { ctx.shadowColor = color; ctx.shadowBlur = blur; }

/* rich sentence: **bold** segments, centered, wrapped. Light weight + bold word. */
function sentence(ctx, str, cx, y, maxW, size, alpha = 1) {
  if (alpha <= 0) return 0;
  const toks = []; str.split(/(\*\*[^*]+\*\*)/).forEach(seg => { if (!seg) return; const b = seg.startsWith('**'); seg.replace(/\*\*/g, '').split(' ').forEach(wd => { if (!wd) return;
    if (/^[.,;:!?…]+$/.test(wd) && toks.length) toks.at(-1).tail = wd; else if (/^[.,;:!?…]/.test(wd) && toks.length) { toks.at(-1).tail = wd[0]; toks.push({ w: wd.slice(1), b }); } else toks.push({ w: wd, b }); }); });
  const meas = tk => { font(ctx, size, SANS, tk.b ? 700 : 300); let w = ctx.measureText(tk.w).width; if (tk.tail) { font(ctx, size, SANS, 300); w += ctx.measureText(tk.tail).width; } return w; };
  font(ctx, size, SANS, 300); const sp = ctx.measureText(' ').width;
  const lines = [[]]; let cur = 0;
  for (const tk of toks) { const wdt = meas(tk); if (cur + (cur ? sp : 0) + wdt > maxW && lines.at(-1).length) { lines.push([]); cur = 0; } lines.at(-1).push(tk); cur += (cur ? sp : 0) + wdt; }
  ctx.save(); ctx.globalAlpha = alpha; ctx.textBaseline = 'alphabetic'; ctx.textAlign = 'left';
  lines.forEach((ln, li) => { const wsum = ln.reduce((s, tk) => s + meas(tk), 0) + sp * (ln.length - 1); let x = cx - wsum / 2; const yy = y + li * size * 1.22;
    for (const tk of ln) { font(ctx, size, SANS, tk.b ? 700 : 300); ctx.fillStyle = tk.b ? C.ink : C.dim; ctx.fillText(tk.w, x, yy);
      if (tk.tail) { const wx = ctx.measureText(tk.w).width; font(ctx, size, SANS, 300); ctx.fillStyle = C.dim; ctx.fillText(tk.tail, x + wx, yy); } x += meas(tk) + sp; } });
  ctx.restore(); return lines.length;
}

/* ───────────── frame chrome ───────────── */
function chrome(ctx, reel, t, n, p) {
  /* header: lesson metadata */
  text(ctx, 'LESSON: ' + reel.lesson.toUpperCase(), 60, 262, { size: 26, color: C.ink, ls: 5 });
  text(ctx, 'TRACK: ', 60, 306, { size: 22, color: C.dim, ls: 4 });
  text(ctx, (reel.track || 'LEANDRO TAIBBI – ID').toUpperCase(), 60 + 7 * 17.2, 306, { size: 22, color: C.ink, ls: 4, w: 600 });
  text(ctx, (reel.label || 'UNRELEASED').toUpperCase(), 60, 342, { size: 19, color: C.dim2, ls: 4 });
  text(ctx, BPM + ' BPM', W - 60, 306, { size: 22, color: C.dim, ls: 4, align: 'right' });
  /* 4 phase pills */
  for (let i = 0; i < PHASES; i++) { const x = W - 60 - (PHASES - i) * 34 + 6; rect(ctx, x, 330, 26, 9, i === p ? C.ink : null, i === p ? null : C.dim2, 1.5); }
  line(ctx, 60, 372, W - 60, 372, C.line, 1.5);
  /* series title */
  text(ctx, 'MATH  ×  TECHNO', W / 2, 470, { size: 30, color: C.ink, ls: 9, align: 'center', w: 500 });
  text(ctx, `EP.${String(n).padStart(2, '0')}`, W / 2, 506, { size: 18, color: C.dim2, ls: 5, align: 'center' });
  /* footer */
  text(ctx, reel.formula, W / 2, 1672, { size: 25, color: C.dim, align: 'center' });
  line(ctx, 60, 1706, W - 60, 1706, C.line, 1.5);
  text(ctx, '@leandro_taibbi', 60, 1742, { size: 19, color: C.dim2, ls: 3 });
  text(ctx, 'hypnotic techno · max for live', W - 60, 1742, { size: 19, color: C.dim2, ls: 3, align: 'right' });
}
function copy(ctx, reel, t, p) {
  const t0 = p * PH; const a = ease(t, t0, t0 + .35) * (p < PHASES - 1 ? 1 - ease(t, t0 + PH - .3, t0 + PH) : 1);
  const s = reel.phases[p];
  line(ctx, 260, 1392, W - 260, 1392, C.line, 1.5);
  const n = sentence(ctx, s.text, W / 2, 1468, W - 120, 54, a);
  text(ctx, s.sub, W / 2, 1468 + n * 54 * 1.22 + 10, { size: 23, color: C.dim, align: 'center', alpha: a, ls: 2 });
}
function renderFrame(ctx, reel, t) {
  const n = REELS.indexOf(reel) + 1; C = reel.light ? LIGHT : DARK;
  const p = Math.min(PHASES - 1, Math.floor(t / PH));
  ctx.setTransform(1, 0, 0, 1, 0, 0); ctx.globalAlpha = 1; ctx.shadowBlur = 0;
  rect(ctx, 0, 0, W, H, C.bg);
  if (reel.bg) reel.bg(ctx, t);
  chrome(ctx, reel, t, n, p);
  ctx.save();
  /* subtle pulse of the whole visual on each kick */
  const k = 1 + 0.012 * beatPulse(t); ctx.translate(W / 2, 960); ctx.scale(k, k); ctx.translate(-W / 2, -960);
  reel.draw(ctx, t, p, t - p * PH);
  ctx.restore();
  copy(ctx, reel, t, p);
}

/* visual area */
const V = { x: 90, w: W - 180, top: 580, bot: 1340, cx: W / 2, cy: 960 };

/* ═══════════════════════════ REELS ═══════════════════════════ */
const REELS = [

/* 01 ─ EUCLID */
{ short: 'euclid', lesson: 'Euclidean hats', formula: 'hit(i) = ((i · k) mod n) < k',
  phases: [
    { text: 'Your hi-hats are a **theorem**.', sub: 'E(5,16) · five hits over sixteen steps' },
    { text: 'Spread 5 hits as **evenly** as integers allow.', sub: 'steps 0 · 4 · 7 · 10 · 13' },
    { text: 'Add E(7,16). They **share** three steps.', sub: 'tension without a fill' },
    { text: 'Rotate one step. **New groove.** Zero new notes.', sub: 'Max for Live: one expr object per voice' },
  ],
  draw(ctx, t, p, tp) {
    const n = 16, cx = V.cx, cy = V.cy, R = 330;
    const rot = p === 3 ? Math.floor(tp / BAR) % n : 0;
    const e5 = euclid(5, n, rot), e7 = euclid(7, n);
    const pos = (t % BAR) / BAR, step = Math.floor(pos * n);
    circle(ctx, cx, cy, R, null, C.line, 1.5);
    const poly = (arr, col, a) => { ctx.save(); ctx.globalAlpha = a; ctx.strokeStyle = col; ctx.lineWidth = 3; glow(ctx, col, 18); ctx.beginPath(); let f = true; for (let i = 0; i < n; i++) if (arr[i]) { const g = -Math.PI / 2 + i / n * Math.PI * 2; const x = cx + Math.cos(g) * R, y = cy + Math.sin(g) * R; f ? ctx.moveTo(x, y) : ctx.lineTo(x, y); f = false; } ctx.closePath(); ctx.stroke(); ctx.restore(); };
    if (p >= 1) poly(e5, C.cv, ease(t, PH, PH + 1.2));
    if (p >= 2) poly(e7, C.sig, ease(t, 2 * PH, 2 * PH + 1.2));
    for (let i = 0; i < n; i++) {
      const g = -Math.PI / 2 + i / n * Math.PI * 2, x = cx + Math.cos(g) * R, y = cy + Math.sin(g) * R;
      const a = e5[i], b = p >= 2 && e7[i];
      circle(ctx, x, y, a || b ? 15 : 7, a ? C.cv : b ? C.sig : C.panel, a || b ? null : C.line, 1.5);
      if (a && b) circle(ctx, x, y, 25, null, C.ink, 2);
      if (i === step) circle(ctx, x, y, 32, null, C.ink, 2, .9);
    }
    const g = -Math.PI / 2 + pos * Math.PI * 2; line(ctx, cx, cy, cx + Math.cos(g) * (R - 48), cy + Math.sin(g) * (R - 48), C.ink, 2, .6);
    circle(ctx, cx, cy, 6, C.ink);
    const pat = e5.map((v, i) => v ? 'x' : (p >= 2 && e7[i] ? 'o' : '·')).join(' ');
    text(ctx, pat, cx, V.bot + 20, { size: 26, color: C.dim, align: 'center', ls: 4 });
    text(ctx, p >= 2 ? 'E(5,16) ∪ E(7,16)' : 'E(5,16)', cx, V.top + 10, { size: 22, color: C.dim, align: 'center', ls: 4 });
  } },

/* 02 ─ LCM */
{ short: 'lcm', lesson: 'Two loops, one drift', formula: 'lcm(16, 7) = 112 steps = 7 bars',
  phases: [
    { text: 'Two loops. Sixteen steps against **seven**.', sub: 'kick every 4 · bleep every 7' },
    { text: 'They meet again after **112** steps.', sub: 'lcm(16,7) · seven bars' },
    { text: 'Every bar the bleep lands **somewhere else**.', sub: 'step 0 · 7 · 14 · 5 · 12 · 3 · 10' },
    { text: 'That drift is the **hypnosis**. No automation.', sub: 'Ableton: one clip in 7/16, the rest in 4/4' },
  ],
  draw(ctx, t, p) {
    const cols = 16, rows = 7, cw = V.w / cols, rh = 92, y0 = V.top + 40;
    const cur = Math.floor(t / S16) % 112;
    for (let r = 0; r < rows; r++) for (let i = 0; i < cols; i++) {
      const s = r * cols + i, x = V.x + i * cw, y = y0 + r * rh;
      const kick = i % 4 === 0, bleep = s % 7 === 0, alive = s <= cur || p >= 1;
      ctx.save(); if (bleep && alive) glow(ctx, C.sig, 14);
      rect(ctx, x + 4, y + 4, cw - 8, rh - 8, alive ? (bleep ? C.sig : kick ? rgba(C.cv, .7) : C.panel) : rgba(C.panel, .4)); ctx.restore();
      if (s === cur) rect(ctx, x + 4, y + 4, cw - 8, rh - 8, null, C.ink, 2.5);
      if (bleep && alive && p >= 2) text(ctx, String(i), x + cw / 2, y + rh / 2 + 8, { size: 22, color: C.bg, align: 'center', w: 600 });
    }
    for (let r = 0; r < rows; r++) text(ctx, String(r + 1), V.x - 22, y0 + r * rh + rh / 2 + 7, { size: 18, color: C.dim2, align: 'right' });
    text(ctx, `step ${String(cur).padStart(3, '0')} / 112`, V.x, V.bot + 20, { size: 22, color: C.dim, ls: 3 });
    text(ctx, `${fmt(112 * S16, 1)} s per cycle`, W - V.x, V.bot + 20, { size: 22, color: C.dim, ls: 3, align: 'right' });
  } },

/* 03 ─ BEATING */
{ short: 'beating', lesson: 'Detune is a tempo', formula: 'f_beat = | f₁ − f₂ |      detune = BPM / (60 · beats)',
  phases: [
    { text: 'Detune is a **tempo**.', sub: '55.000 Hz against 55.300 Hz' },
    { text: 'Two sines become one that **breathes**.', sub: 'beat every 3.33 s · random · unmusical' },
    { text: 'Make it breathe every **two bars**.', sub: 'detune = 0.2708 Hz = +8.5 cents' },
    { text: 'A sidechain you never **programmed**.', sub: 'detune(Hz) = BPM / 60 / beats per cycle' },
  ],
  draw(ctx, t, p) {
    const sync = p >= 2; const f1 = 55, f2 = sync ? 55 + 1 / (2 * BAR) : 55.3, fb = f2 - f1;
    const yA = V.top + 110, yB = V.top + 290, yS = V.top + 580, span = .09, sc = t * .02;
    const wave = (f, y, col) => { ctx.save(); glow(ctx, col, 12); plot(ctx, u => Math.sin(2 * Math.PI * f * (u * span + sc)), V.x, V.x + V.w, y, 60, 500, col, 2.5, .9); ctx.restore(); };
    text(ctx, `A  ${fmt(f1, 3)} Hz`, V.x, yA - 86, { size: 20, color: C.dim, ls: 3 }); wave(f1, yA, C.cv);
    text(ctx, `B  ${fmt(f2, 3)} Hz`, V.x, yB - 86, { size: 20, color: C.dim, ls: 3 }); wave(f2, yB, C.sig);
    text(ctx, 'A + B', V.x, yS - 190, { size: 20, color: C.dim, ls: 3 });
    const win = 8;
    plot(ctx, u => Math.cos(Math.PI * fb * (u * win + t - win)) * Math.sin(2 * Math.PI * 6 * u * win), V.x, V.x + V.w, yS, 150, 1600, C.ink, 1.2, .55);
    ctx.save(); glow(ctx, C.cv, 16);
    plot(ctx, u => Math.abs(Math.cos(Math.PI * fb * (u * win + t - win))), V.x, V.x + V.w, yS, 150, 400, C.cv, 3);
    plot(ctx, u => -Math.abs(Math.cos(Math.PI * fb * (u * win + t - win))), V.x, V.x + V.w, yS, 150, 400, C.cv, 3); ctx.restore();
    if (sync) for (let b = 0; b < 6; b++) { const xb = V.x + V.w - ((t % (2 * BAR)) + b * 2 * BAR) / win * V.w; if (xb > V.x) line(ctx, xb, yS - 165, xb, yS + 165, C.sig, 1.5, .7); }
    text(ctx, `f_beat = ${fmt(fb, 4)} Hz   ·   period ${fmt(1 / fb, 3)} s${sync ? '  =  2 bars' : ''}`, V.cx, V.bot + 20, { size: 22, color: sync ? C.ok : C.dim, align: 'center', ls: 2 });
  } },

/* 04 ─ PHI  (light) */
{ short: 'phi', light: true, lesson: 'The irrational delay', formula: 'delay = beat_ms × 1/φ = 461.5 × 0.618 = 285.3 ms',
  phases: [
    { text: 'The most **irrational** delay time.', sub: 'φ = 1.6180339…  ·  1/φ = 0.618…' },
    { text: 'A 3/4-beat delay lands back on the **grid**.', sub: 'four repeats, then nothing new' },
    { text: 'A 1/φ-beat delay **never** lands on a 16th.', sub: '0.618 · 1.236 · 1.854 · 2.472 beats' },
    { text: 'A cloud, not an **echo**.', sub: '285 ms · feedback 60 % · the dub-techno trick' },
  ],
  bg(ctx, t) { /* golden spiral, faint */
    const phi = (1 + Math.sqrt(5)) / 2; ctx.save(); ctx.strokeStyle = rgba(C.sig, .18); ctx.lineWidth = 2; ctx.beginPath();
    for (let i = 0; i <= 700; i++) { const th = i / 700 * Math.PI * 7; const r = 36 * Math.pow(phi, th / (Math.PI / 2)); if (r > 2600) break; const x = 700 + Math.cos(th + t * .05) * r, y = 1000 - Math.sin(th + t * .05) * r; i ? ctx.lineTo(x, y) : ctx.moveTo(x, y); }
    ctx.stroke(); ctx.restore();
  },
  draw(ctx, t, p, tp) {
    const phi = (1 + Math.sqrt(5)) / 2; const golden = p >= 2; const ratio = p === 0 ? null : golden ? 1 / phi : .75;
    const x0 = V.x, x1 = V.x + V.w, beats = 4, bx = b => lerp(x0, x1, b / beats), y = V.cy + 40;
    for (let i = 0; i <= beats * 4; i++) { const big = i % 4 === 0; line(ctx, bx(i / 4), y - (big ? 90 : 36), bx(i / 4), y + (big ? 90 : 36), big ? C.dim : C.line, big ? 2.5 : 1.5); }
    rect(ctx, x0 - 5, y - 300, 10, 300, C.ink);
    if (p === 0) {
      text(ctx, 'φ', V.cx, V.top + 230, { size: 260, fam: DISP, w: 800, color: C.cv, align: 'center' });
      text(ctx, '= (1 + √5) / 2 = [1; 1, 1, 1, 1, …]', V.cx, V.top + 300, { size: 26, color: C.dim, align: 'center' });
    } else {
      const nTaps = Math.min(9, Math.floor(tp / .45) + 1);
      for (let k = 1; k <= nTaps; k++) { const pos = k * ratio; if (pos > beats) break; const x = bx(pos), onGrid = Math.abs(pos * 4 - Math.round(pos * 4)) < 1e-6, amp = Math.pow(.62, k) * 300;
        ctx.save(); if (!onGrid) glow(ctx, C.cv, 14); rect(ctx, x - 5, y - amp, 10, amp, onGrid ? C.dim : C.cv); ctx.restore();
        text(ctx, fmt(pos, 3), x, y + 130, { size: 17, color: onGrid ? C.dim : C.cv, align: 'center' }); }
      text(ctx, golden ? 'ratio = 1/φ = 0.618…' : 'ratio = 3/4 = 0.750', V.cx, V.top + 40, { size: 24, color: C.dim, align: 'center', ls: 3 });
      text(ctx, `${fmt(BEAT * 1000 * ratio, 1)} ms`, V.cx, V.top + 150, { size: 96, fam: DISP, w: 800, color: golden ? C.cv : C.ink, align: 'center' });
    }
    text(ctx, 'four beats · 16th grid', V.cx, V.bot + 20, { size: 20, color: C.dim2, align: 'center', ls: 3 });
  } },

/* 05 ─ PRIMES */
{ short: 'primes', lesson: 'Prime loop lengths', formula: 'lcm(3, 5, 7) = 105 bars = 193.8 s = 3:14',
  phases: [
    { text: "Primes don't **repeat**.", sub: 'three loops: 3 · 5 · 7 bars' },
    { text: 'Nothing lines up for **105** bars.', sub: '3 minutes 14 seconds at 130 BPM' },
    { text: '4 · 8 · 16 repeats every **29 seconds**.', sub: 'you hear it. they hear it.' },
    { text: 'The hypnosis is **arithmetic**.', sub: 'pad 3 bars · hats 5 · acid line 7 · follow actions off' },
  ],
  draw(ctx, t, p) {
    const L = p === 2 ? [4, 8, 16] : [3, 5, 7], cols = [C.cv, C.sig, C.ink], cx = V.cx, cy = V.cy, bar = t / BAR * 2.5;
    const lcm3 = p === 2 ? 16 : 105;
    L.forEach((len, i) => { const R = 150 + i * 100; circle(ctx, cx, cy, R, null, C.line, 1.5);
      for (let s = 0; s < len; s++) { const g = -Math.PI / 2 + s / len * Math.PI * 2; const on = Math.floor(bar) % len === s; ctx.save(); if (on) glow(ctx, cols[i], 16); circle(ctx, cx + Math.cos(g) * R, cy + Math.sin(g) * R, on ? 11 : 6, on ? cols[i] : C.panel, cols[i], 1.5); ctx.restore(); }
      const g = -Math.PI / 2 + (bar % len) / len * Math.PI * 2; line(ctx, cx, cy, cx + Math.cos(g) * R, cy + Math.sin(g) * R, cols[i], 2, .55); });
    text(ctx, String(Math.floor(bar) % lcm3).padStart(3, '0'), cx, cy + 22, { size: 60, fam: DISP, w: 800, color: C.ink, align: 'center' });
    text(ctx, '/ ' + lcm3 + ' bars', cx, cy + 56, { size: 20, color: C.dim, align: 'center', ls: 3 });
    text(ctx, L.join('  ·  '), cx, V.top + 10, { size: 24, color: C.dim, align: 'center', ls: 6 });
    text(ctx, `state (${L.map(l => Math.floor(bar) % l).join(', ')})`, cx, V.bot + 20, { size: 22, color: C.dim, align: 'center', ls: 2 });
  } },

/* 06 ─ KICK TUNING */
{ short: 'kick hz', lesson: 'Your kick has a note', formula: 'f(n) = 440 · 2^(n/12)      G1 = 49.0 Hz · A1 = 55.0 Hz',
  phases: [
    { text: 'Your kick has a **note**.', sub: 'the body settles at 49.0 Hz → G1' },
    { text: 'Bass at A1. Kick at G1. **6 Hz** of mud.', sub: '55.0 − 49.0 = 6.0 Hz beating in the low end' },
    { text: 'Tune the body to the **root**.', sub: 'A1 55.0 Hz · or a fifth below: D1 36.7 Hz' },
    { text: 'One semitone down here is **2.9 Hz**.', sub: "the analyser can't see it. the sub can." },
  ],
  draw(ctx, t, p) {
    const notes = [['E1', -41], ['F1', -40], ['F#1', -39], ['G1', -38], ['G#1', -37], ['A1', -36], ['A#1', -35], ['B1', -34], ['C2', -33], ['C#2', -32], ['D2', -31]];
    const f = n => 440 * Math.pow(2, n / 12), fmin = 38, fmax = 80, x0 = V.x, x1 = V.x + V.w, fx = fr => lerp(x0, x1, Math.log(fr / fmin) / Math.log(fmax / fmin));
    const y = V.cy + 180, tuned = p >= 2, body = tuned ? 55 : 49;
    const hit = (t * 1.3) % BEAT, f0 = body * (1 + 2.2 * Math.exp(-hit * 28)), amp = Math.exp(-hit * 2.2);
    ctx.save(); glow(ctx, C.cv, 18);
    plot(ctx, u => { const fr = fmin * Math.pow(fmax / fmin, u); const g = (c, w, a) => a * Math.exp(-((fr - c) ** 2) / (2 * w * w)); return Math.max(g(f0, 2.2 + 6 * hit, amp), g(body, 1.3, .8)); }, x0, x1, y, 420, 500, C.cv, 3); ctx.restore();
    line(ctx, x0, y, x1, y, C.dim, 1.5);
    notes.forEach(([nm, n]) => { const x = fx(f(n)); line(ctx, x, y, x, y + 14, C.dim, 1.5); text(ctx, nm, x, y + 44, { size: 18, color: nm === 'G1' || nm === 'A1' ? C.ink : C.dim, align: 'center' }); text(ctx, fmt(f(n), 1), x, y + 68, { size: 14, color: C.dim2, align: 'center' }); });
    if (p >= 1) { line(ctx, fx(55), y - 470, fx(55), y, C.sig, 2.5, .9); text(ctx, 'bass A1 55.0', fx(55) + 12, y - 450, { size: 20, color: C.sig }); }
    text(ctx, `kick ${fmt(body, 1)} Hz`, fx(body) + (tuned ? 12 : -12), y - 400, { size: 20, color: C.cv, align: tuned ? 'left' : 'right' });
    if (p === 1) text(ctx, 'Δ 6.0 Hz', (fx(49) + fx(55)) / 2, y - 500, { size: 26, color: C.ink, align: 'center', w: 500 });
    if (p === 3) { line(ctx, fx(49), y - 140, fx(51.9), y - 140, C.ink, 2); text(ctx, '2.9 Hz', (fx(49) + fx(51.9)) / 2, y - 156, { size: 20, color: C.ink, align: 'center' }); }
    text(ctx, `sweep ${fmt(f0, 1).padStart(5)} Hz → ${fmt(body, 1)} Hz`, V.cx, V.bot + 20, { size: 22, color: C.dim, align: 'center', ls: 2 });
  } },

/* 07 ─ LFO HZ */
{ short: 'lfo hz', lesson: 'LFO rates in hertz', formula: 'f = BPM / (60 · beats per cycle)',
  phases: [
    { text: 'Free-running LFOs are **not free**.', sub: 'every rate at 130 BPM has a number' },
    { text: 'One bar is **0.5417 Hz**.', sub: '2 bars 0.2708 · 4 bars 0.1354 · 8 bars 0.0677' },
    { text: 'Sixteen bars is **0.0339 Hz**.', sub: 'one filter sweep every 29.5 s' },
    { text: 'Type it into the **hardware**.', sub: 'tempo sync without a sync cable' },
  ],
  draw(ctx, t, p) {
    const rows = [1, 2, 4, 8, 16], xA = V.x + 250, xB = V.x + V.w;
    rows.forEach((b, i) => { const y = V.top + 70 + i * 145, fr = BPM / 60 / (4 * b), hi = (p === 1 && i === 0) || (p === 2 && i === 4);
      text(ctx, `${b} bar${b > 1 ? 's' : ''}`, V.x, y - 10, { size: 20, color: C.dim, ls: 3 }); text(ctx, fmt(fr, 4), V.x, y + 30, { size: 34, fam: DISP, w: 800, color: hi ? C.cv : p === 0 ? C.dim : C.ink });
      line(ctx, xA, y + 10, xB, y + 10, C.line, 1);
      ctx.save(); if (hi) glow(ctx, C.cv, 14); plot(ctx, u => Math.sin(2 * Math.PI * fr * (u * 29.54 + t * .4)), xA, xB, y + 10, 52, 400, hi ? C.cv : i % 2 ? C.sig : C.ink, hi ? 3.5 : 2, hi ? 1 : .55); ctx.restore(); });
    text(ctx, '←  16 bars  =  29.54 s  →', (xA + xB) / 2, V.top + 10, { size: 18, color: C.dim2, align: 'center', ls: 3 });
    text(ctx, 'Hz', V.x + 2, V.top + 10, { size: 18, color: C.dim2, ls: 3 });
  } },

/* 08 ─ XOR CLOCK (light) */
{ short: 'xor clock', light: true, lesson: 'Clock dividers and XOR', formula: 'period = lcm(2, 3, 5) = 30 steps',
  phases: [
    { text: 'Three dividers. One **XOR**.', sub: 'a pulse every 2 · every 3 · every 5 clocks' },
    { text: 'The pattern repeats every **30** steps.', sub: 'lcm(2, 3, 5) · just under two bars' },
    { text: 'OR gives 22 hits. XOR gives **16**.', sub: 'odd · lopsided · alive' },
    { text: 'Three counters and a **truth table**.', sub: 'twelve lines of gen~ · send it to a 909 rim' },
  ],
  draw(ctx, t, p, tp) {
    const N = 30, cw = V.w / N, cur = Math.floor(t / S16 / 1.4) % N;
    const rows = [2, 3, 5].map(d => Array.from({ length: N }, (_, i) => i % d === 0 ? 1 : 0));
    const op = p === 2 && tp < PH / 2 ? 'or' : 'xor';
    const out = Array.from({ length: N }, (_, i) => op === 'or' ? (rows[0][i] | rows[1][i] | rows[2][i]) : (rows[0][i] ^ rows[1][i] ^ rows[2][i]));
    const draw = (arr, y, col, lbl, h) => { text(ctx, lbl, V.x, y - 18, { size: 18, color: C.dim, ls: 3 }); for (let i = 0; i < N; i++) { const on = arr[i]; ctx.save(); if (on && i === cur) glow(ctx, col, 16); rect(ctx, V.x + i * cw + 3, y + (on ? 0 : h - 6), cw - 6, on ? h : 6, i === cur ? C.ink : on ? col : C.line); ctx.restore(); } };
    draw(rows[0], V.top + 60, C.sig, '/ 2', 70); draw(rows[1], V.top + 190, C.sig, '/ 3', 70); draw(rows[2], V.top + 320, C.sig, '/ 5', 70);
    draw(out, V.top + 500, C.cv, op.toUpperCase() + ' (/2, /3, /5)', 150);
    text(ctx, `${out.reduce((a, b) => a + b, 0)} / 30 hits`, W - V.x, V.top + 482, { size: 20, color: C.cv, align: 'right', ls: 3 });
    text(ctx, out.join(''), V.cx, V.bot + 20, { size: 24, color: C.dim, align: 'center', ls: 6 });
    if (p >= 1) for (let i = 0; i <= N; i += 15) line(ctx, V.x + i * cw, V.top + 20, V.x + i * cw, V.bot - 40, C.dim2, 1, .6);
  } },

/* 09 ─ SWING MS */
{ short: 'swing ms', lesson: 'Swing in milliseconds', formula: 'delay = (swing − 0.5) × 230.77 ms',
  phases: [
    { text: '54 % swing is **9.2 ms**.', sub: '8th @130 = 230.77 ms · straight 16th = 115.38 ms' },
    { text: '58 % is **18.5 ms**. 62 % is 27.7.', sub: 'the off-16th slides inside the 8th' },
    { text: 'You stop hearing it below **10 ms**.', sub: 'felt, not heard' },
    { text: 'Hats at 54. Kick at **50**.', sub: 'the gap is the groove' },
  ],
  draw(ctx, t, p, tp) {
    const sw = p === 0 ? (tp < 2.5 ? .5 : .54) : p === 1 ? (tp < 3.75 ? .58 : .62) : .54;
    const eighth = 230.77, x0 = V.x, x1 = V.x + V.w, k = (x1 - x0) / (2 * eighth), y = V.cy + 110;
    line(ctx, x0, y, x1, y, C.line, 2);
    for (let i = 0; i < 5; i++) { const x = x0 + i * k * eighth / 2, off = i % 2 === 1, dx = off ? (sw - .5) * eighth * k : 0, xx = x + dx, h = off ? 170 : 250;
      if (off) line(ctx, x, y - h, x, y, C.line, 1.5);
      ctx.save(); glow(ctx, off ? C.cv : C.ink, 14); rect(ctx, xx - 7, y - h, 14, h, off ? C.cv : C.ink); ctx.restore();
      text(ctx, fmt(i * eighth / 2 + (off ? (sw - .5) * eighth : 0), 1), xx, y + 44, { size: 18, color: off ? C.cv : C.dim, align: 'center' });
      if (off && sw > .5) { line(ctx, x, y - h - 30, xx, y - h - 30, C.sig, 3); text(ctx, '+' + fmt((sw - .5) * eighth, 1) + ' ms', (x + xx) / 2, y - h - 46, { size: 20, color: C.sig, align: 'center' }); } }
    text(ctx, `${Math.round(sw * 100)} %`, V.cx, V.top + 120, { size: 120, fam: DISP, w: 800, color: C.ink, align: 'center' });
    const ph = (t % BEAT) / BEAT, kOn = ph < .1, hOn = Math.abs(ph - (.5 + (sw - .5))) < .1;
    circle(ctx, V.cx - 40, V.bot + 12, 12, kOn ? C.ink : C.panel, C.line); circle(ctx, V.cx + 40, V.bot + 12, 12, hOn ? C.cv : C.panel, C.line);
    text(ctx, 'kick', V.cx - 40, V.bot + 50, { size: 15, color: C.dim2, align: 'center' }); text(ctx, 'hat', V.cx + 40, V.bot + 50, { size: 15, color: C.dim2, align: 'center' });
  } },

/* 10 ─ FOURIER */
{ short: 'fourier', lesson: 'The click is the spectrum', formula: 'square(x) = 4/π · Σ sin((2k+1)x) / (2k+1)',
  phases: [
    { text: 'A square wave is an infinite **sum**.', sub: 'odd harmonics only · amplitude 1/n' },
    { text: 'Fifteen harmonics. The corners **ring**.', sub: 'Gibbs overshoot 8.9 % · it never goes away' },
    { text: 'A click is **every** frequency at once.', sub: 'impulse → flat spectrum' },
    { text: 'Punch is **bandwidth**, not level.', sub: "shorten the transient. don't EQ it." },
  ],
  draw(ctx, t, p, tp) {
    const K = p === 0 ? Math.max(1, Math.floor(1 + easeOut(tp, 0, 6.5) * 6)) : p === 1 ? 15 : 60, imp = p >= 2;
    const x0 = V.x, x1 = V.x + V.w, y = V.top + 240;
    line(ctx, x0, y, x1, y, C.line, 1);
    if (!imp) { plot(ctx, u => Math.sin(u * Math.PI * 4 + t * 2) >= 0 ? 1 : -1, x0, x1, y, 170, 800, C.dim2, 1.5, .5);
      ctx.save(); glow(ctx, C.cv, 14); plot(ctx, u => { let s = 0; for (let k = 0; k < K; k++) { const n = 2 * k + 1; s += Math.sin(n * (u * Math.PI * 4 + t * 2)) / n; } return s * 4 / Math.PI; }, x0, x1, y, 170, 1200, C.cv, 3); ctx.restore(); }
    else { ctx.save(); glow(ctx, C.cv, 14); plot(ctx, u => { const d = (u - .5) * 44; return Math.abs(d) < .04 ? 1 : Math.sin(Math.PI * d) / (Math.PI * d); }, x0, x1, y, 190, 1400, C.cv, 3); ctx.restore(); }
    const sy = V.bot - 30, bw = V.w / 31;
    for (let n = 1; n <= 31; n++) { const a = imp ? .85 : (n % 2 && n <= 2 * K - 1) ? 1 / n : 0, h = a * 300; ctx.save(); if (a) glow(ctx, imp ? C.sig : C.cv, 10); rect(ctx, x0 + (n - 1) * bw + 3, sy - h, bw - 6, Math.max(h, 2), a ? (imp ? C.sig : n === 1 ? C.ink : C.cv) : C.line); ctx.restore(); }
    text(ctx, imp ? 'δ(t)  ·  all harmonics  ·  equal amplitude' : `${K} odd harmonic${K > 1 ? 's' : ''}  ·  amplitude 1/n`, V.cx, V.top + 10, { size: 20, color: C.dim, align: 'center', ls: 3 });
    text(ctx, 'h1 … h31', V.cx, V.bot + 20, { size: 18, color: C.dim2, align: 'center', ls: 3 });
  } },

/* 11 ─ CHAOS */
{ short: 'chaos', lesson: 'The logistic map', formula: 'x[n+1] = r · x[n] · (1 − x[n])',
  phases: [
    { text: 'One line of code. Infinite **sequencer**.', sub: 'the logistic map · r between 2.8 and 4' },
    { text: 'r = 3.2 → two notes. r = 3.5 → **four**.', sub: 'period doubling · faster and faster' },
    { text: 'r = 3.5699 → **chaos**.', sub: 'deterministic · repeatable · never periodic' },
    { text: 'Map r to a **fader**. That fader is the track.', sub: 'gen~: History x; x = r*x*(1-x); out = floor(x*16)' },
  ],
  draw(ctx, t, p, tp) {
    const x0 = V.x, x1 = V.x + V.w, yb = V.cy + 150, yt = V.top + 20, rmin = 2.8, rmax = 4;
    const prog = easeOut(t, 0, 9), cols = Math.floor(prog * 460);
    ctx.save(); ctx.fillStyle = rgba(C.cv, .6);
    for (let c = 0; c < cols; c++) { const r = lerp(rmin, rmax, c / 460); let x = .31 + .17 * ((c * 7919) % 13) / 13; for (let i = 0; i < 120; i++) x = r * x * (1 - x); for (let i = 0; i < 90; i++) { x = r * x * (1 - x); ctx.fillRect(lerp(x0, x1, c / 460), lerp(yb, yt, x), 1.8, 1.8); } }
    ctx.restore();
    line(ctx, x0, yb, x1, yb, C.dim, 1.5);
    const rSeq = p === 0 ? 2.9 : p === 1 ? (tp < 3.75 ? 3.2 : 3.5) : p === 2 ? 3.7 : lerp(2.9, 3.99, (Math.sin(tp * .9) + 1) / 2);
    const rx = lerp(x0, x1, (rSeq - rmin) / (rmax - rmin));
    ctx.save(); glow(ctx, C.sig, 14); line(ctx, rx, yt - 10, rx, yb, C.sig, 2.5, .9); ctx.restore();
    text(ctx, 'r = ' + fmt(rSeq, 2), rx, yt - 24, { size: 22, color: C.sig, align: 'center', w: 500 });
    let x = .3; const seq = []; for (let i = 0; i < 32 + Math.floor(t * 8); i++) { x = rSeq * x * (1 - x); if (i >= 32) seq.push(x); }
    const last = seq.slice(-16), bw = V.w / 16;
    last.forEach((v, i) => { const h = 20 + v * 130; ctx.save(); if (i === 15) glow(ctx, C.ink, 12); rect(ctx, x0 + i * bw + 3, V.bot - 10 - h, bw - 6, h, i === 15 ? C.ink : rgba(C.sig, .85)); ctx.restore(); });
    text(ctx, 'note ' + String(Math.floor(last[15] * 16)).padStart(2, '0') + ' / 16', W - V.x, yb + 40, { size: 20, color: C.dim, align: 'right', ls: 3 });
    text(ctx, '2.8', x0, yb + 40, { size: 16, color: C.dim2 }); text(ctx, '4.0', x1, yb + 40, { size: 16, color: C.dim2, align: 'right' });
  } },

/* 12 ─ RISSET  (light) */
{ short: 'risset', light: true, lesson: 'The endless acceleration', formula: 'tempo(t) = T₀ · 2^(t / T)      T = 8 bars',
  phases: [
    { text: 'A beat that accelerates **forever**.', sub: 'the same loop at ×1, ×2, ×4' },
    { text: 'Every eight bars the tempo **doubles**.', sub: 'and the next layer takes over, at the tempo you started from' },
    { text: 'You hear acceleration. The mean tempo **never moves**.', sub: 'a Shepard tone, for rhythm' },
    { text: 'The crowd never **lands**.', sub: '3 clips · ×1 ×2 ×4 · Hann gain windows offset by 1/3' },
  ],
  draw(ctx, t, p) {
    const T = 7.5, ph = (t / T) % 1, x0 = V.x, x1 = V.x + V.w, lmin = -1, lmax = 3, lx = l => lerp(x0, x1, (l - lmin) / (lmax - lmin)), y = V.cy + 120;
    for (let l = -1; l <= 3; l++) { line(ctx, lx(l), y - 300, lx(l), y + 20, C.line, 1.5); text(ctx, '×' + Math.pow(2, l), lx(l), y + 56, { size: 20, color: C.dim, align: 'center' }); text(ctx, Math.round(BPM * Math.pow(2, l)) + '', lx(l), y + 82, { size: 14, color: C.dim2, align: 'center' }); }
    const gain = l => { const u = l / 2; return (u <= 0 || u >= 1) ? 0 : .5 - .5 * Math.cos(2 * Math.PI * u); };
    plot(ctx, u => gain(lerp(lmin, lmax, u)), x0, x1, y, 280, 400, C.dim2, 1.5, .6);
    for (let i = -1; i <= 2; i++) { const l = i + ph; if (l < lmin || l > lmax) continue; const g = gain(l), x = lx(l), col = (i + 10) % 2 ? C.cv : C.sig;
      ctx.save(); glow(ctx, col, 16); rect(ctx, x - 12, y - g * 280, 24, g * 280, col); ctx.restore();
      if (g > .08) text(ctx, Math.round(Math.pow(2, l) * BPM) + '', x, y - g * 280 - 16, { size: 18, color: C.ink, align: 'center', w: 500 }); }
    text(ctx, fmt(BPM * Math.pow(2, ph), 1), V.cx, V.top + 130, { size: 110, fam: DISP, w: 800, color: C.cv, align: 'center' });
    text(ctx, 'perceived BPM  ↑', V.cx, V.top + 170, { size: 20, color: C.dim, align: 'center', ls: 4 });
    const yy = V.bot + 12; for (let i = 0; i < 3; i++) { const l = i + ph, f = Math.pow(2, l) * BPM / 60, on = (t * f) % 1 < .18, g = gain(l); circle(ctx, V.cx - 60 + i * 60, yy, 13, on ? rgba((i + 10) % 2 ? C.cv : C.sig, .25 + .75 * g) : C.panel, C.line); }
  } },
];

if (typeof module !== 'undefined') module.exports = { REELS, renderFrame, DUR };

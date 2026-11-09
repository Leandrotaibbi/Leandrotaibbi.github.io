/* MATH × TECHNO — 12 reels, 1080×1920, 30 s each.
   Every frame is a pure function of (reel, t): the same code drives the
   live preview (index.html) and the offline MP4 render (render.js). */

const W = 1080, H = 1920, DUR = 30;
const BPM = 130, BEAT = 60 / BPM, BAR = 4 * BEAT, S16 = BEAT / 4;
const C = { bg:'#070809', panel:'#111417', line:'#232a30', ink:'#ecead9', dim:'#8b938d', dim2:'#5a6570',
            cv:'#e8763c', sig:'#62b2c0', ok:'#2ee08a' };
const MONO = 'Plex', SANS = 'Syne';

/* timeline inside every reel */
const T_HOOK = 2.8, T_TAKE = 24.2, T_CTA = 27.6;

/* ───────────── helpers ───────────── */
const clamp = (x, a = 0, b = 1) => Math.max(a, Math.min(b, x));
const lerp = (a, b, k) => a + (b - a) * k;
const ease = (t, a, b) => { const k = clamp((t - a) / (b - a)); return k * k * (3 - 2 * k); };
const easeOut = (t, a, b) => { const k = clamp((t - a) / (b - a)); return 1 - (1 - k) ** 3; };
const rgba = (hex, a) => { const n = parseInt(hex.slice(1), 16); return `rgba(${n >> 16},${(n >> 8) & 255},${n & 255},${a})`; };
function rng(seed) { let s = seed >>> 0; return () => { s += 0x6D2B79F5; let t = s; t = Math.imul(t ^ (t >>> 15), t | 1); t ^= t + Math.imul(t ^ (t >>> 7), t | 61); return ((t ^ (t >>> 14)) >>> 0) / 4294967296; }; }
const gcd = (a, b) => b ? gcd(b, a % b) : a;
const lcm = (a, b) => a * b / gcd(a, b);
const euclid = (k, n, rot = 0) => Array.from({ length: n }, (_, i) => ((((i + rot) * k) % n) < k) ? 1 : 0);
const fmt = (x, d = 2) => x.toFixed(d);

function font(ctx, size, fam = MONO, w = 400) { ctx.font = `${w} ${size}px ${fam}`; }
function text(ctx, s, x, y, { size = 30, fam = MONO, w = 400, color = C.ink, align = 'left', alpha = 1, ls = 0 } = {}) {
  if (alpha <= 0) return;
  ctx.save(); ctx.globalAlpha = alpha; font(ctx, size, fam, w); ctx.fillStyle = color; ctx.textAlign = align; ctx.textBaseline = 'alphabetic';
  if (ls) ctx.letterSpacing = ls + 'px';
  ctx.fillText(s, x, y); ctx.restore();
}
function fit(ctx, s, maxW, maxSize, fam = SANS, w = 800) { let size = maxSize; font(ctx, size, fam, w); while (ctx.measureText(s).width > maxW && size > 20) { size -= 2; font(ctx, size, fam, w); } return size; }
function wrap(ctx, s, maxW, size, fam, w) {
  font(ctx, size, fam, w); const words = s.split(' '), lines = []; let cur = '';
  for (const wd of words) { const tr = cur ? cur + ' ' + wd : wd; if (ctx.measureText(tr).width > maxW && cur) { lines.push(cur); cur = wd; } else cur = tr; }
  if (cur) lines.push(cur); return lines;
}
function para(ctx, s, x, y, maxW, o = {}) {
  const { size = 30, fam = MONO, w = 400, lh = 1.35, align = 'left' } = o;
  const lines = wrap(ctx, s, maxW, size, fam, w);
  lines.forEach((l, i) => text(ctx, l, x, y + i * size * lh, { ...o, align }));
  return lines.length * size * lh;
}
/* typewriter: returns visible slice of s given seconds since start */
const typed = (s, dt, cps = 42) => dt <= 0 ? '' : s.slice(0, Math.floor(dt * cps));
function rect(ctx, x, y, w, h, fill, stroke) { if (fill) { ctx.fillStyle = fill; ctx.fillRect(x, y, w, h); } if (stroke) { ctx.strokeStyle = stroke; ctx.lineWidth = 2; ctx.strokeRect(x, y, w, h); } }
function line(ctx, x1, y1, x2, y2, color, w = 2, alpha = 1) { ctx.save(); ctx.globalAlpha = alpha; ctx.strokeStyle = color; ctx.lineWidth = w; ctx.beginPath(); ctx.moveTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke(); ctx.restore(); }
function circle(ctx, x, y, r, fill, stroke, lw = 2) { ctx.beginPath(); ctx.arc(x, y, r, 0, Math.PI * 2); if (fill) { ctx.fillStyle = fill; ctx.fill(); } if (stroke) { ctx.strokeStyle = stroke; ctx.lineWidth = lw; ctx.stroke(); } }
function plot(ctx, fn, x0, x1, y0, yScale, n, color, lw = 3, alpha = 1) {
  ctx.save(); ctx.globalAlpha = alpha; ctx.strokeStyle = color; ctx.lineWidth = lw; ctx.lineJoin = 'round'; ctx.beginPath();
  for (let i = 0; i <= n; i++) { const u = i / n, x = lerp(x0, x1, u), y = y0 - fn(u) * yScale; i ? ctx.lineTo(x, y) : ctx.moveTo(x, y); }
  ctx.stroke(); ctx.restore();
}

/* ───────────── backgrounds (one per reel for variety) ───────────── */
const BG = {
  grid(ctx) { ctx.save(); ctx.strokeStyle = rgba(C.ink, .045); ctx.lineWidth = 1; for (let x = 60; x < W; x += 60) line(ctx, x, 0, x, H, rgba(C.ink, .045), 1); for (let y = 0; y < H; y += 60) line(ctx, 0, y, W, y, rgba(C.ink, .045), 1); ctx.restore(); },
  scan(ctx) { ctx.fillStyle = rgba(C.ink, .022); for (let y = 0; y < H; y += 4) ctx.fillRect(0, y, W, 1); },
  dots(ctx) { ctx.fillStyle = rgba(C.ink, .10); for (let x = 30; x < W; x += 40) for (let y = 30; y < H; y += 40) ctx.fillRect(x, y, 2, 2); },
  blue(ctx) { for (let x = 0; x < W; x += 120) line(ctx, x, 0, x, H, rgba(C.sig, .10), 1); for (let y = 0; y < H; y += 120) line(ctx, 0, y, W, y, rgba(C.sig, .10), 1); for (let x = 0; x < W; x += 24) line(ctx, x, 0, x, H, rgba(C.sig, .035), 1); for (let y = 0; y < H; y += 24) line(ctx, 0, y, W, y, rgba(C.sig, .035), 1); },
  plain() {},
  noise(ctx, seed) { const r = rng(seed); ctx.fillStyle = rgba(C.ink, .07); for (let i = 0; i < 2600; i++) ctx.fillRect(r() * W, r() * H, 2, 2); },
  warm(ctx) { const g = ctx.createRadialGradient(W / 2, -200, 50, W / 2, -200, 1500); g.addColorStop(0, rgba(C.cv, .16)); g.addColorStop(1, 'transparent'); ctx.fillStyle = g; ctx.fillRect(0, 0, W, H); },
  cold(ctx) { const g = ctx.createRadialGradient(W / 2, H + 300, 50, W / 2, H + 300, 1600); g.addColorStop(0, rgba(C.sig, .16)); g.addColorStop(1, 'transparent'); ctx.fillStyle = g; ctx.fillRect(0, 0, W, H); },
};

/* ───────────── frame chrome ───────────── */
function chrome(ctx, reel, t, n) {
  const id = String(n).padStart(2, '0');
  text(ctx, '┌ LT // MATH × TECHNO', 60, 182, { size: 22, color: C.cv, ls: 2 });
  text(ctx, `EP.${id} · ${reel.short.toUpperCase()}`, W - 60, 182, { size: 22, color: C.dim, align: 'right', ls: 2 });
  rect(ctx, 60, 204, W - 120, 3, C.line);
  rect(ctx, 60, 204, (W - 120) * (t / DUR), 3, C.cv);
  /* timecode */
  text(ctx, `t=${t.toFixed(2).padStart(5, '0')}s  ${BPM}BPM  bar=${fmt(BAR, 3)}s`, 60, 240, { size: 18, color: C.dim2 });
  text(ctx, `@leandro_taibbi`, W - 60, 240, { size: 18, color: C.dim2, align: 'right' });
  /* footer */
  rect(ctx, 60, 1690, W - 120, 1, C.line);
  text(ctx, 'hypnotic techno · 6× Xone:K2 · Max for Live', 60, 1726, { size: 19, color: C.dim2 });
  text(ctx, '└ save · argue in comments', W - 60, 1726, { size: 19, color: C.dim2, align: 'right' });
}

/* hook: 0 → T_HOOK. Big statement typed in, with cursor. */
function hook(ctx, reel, t) {
  const a = 1 - ease(t, T_HOOK - .35, T_HOOK);
  if (a <= 0) return;
  ctx.save(); ctx.globalAlpha = a;
  text(ctx, '> ' + typed(reel.prompt, t - .1, 46) + (Math.floor(t * 3) % 2 ? '█' : ' '), 60, 560, { size: 28, color: C.sig });
  const lines = reel.hook.split('\n'); const y0 = 720;
  const sz = Math.min(...lines.map(l => fit(ctx, l, W - 120, 108))); const lh = sz * 1.1;
  lines.forEach((l, i) => { const la = easeOut(t, .55 + i * .25, 1.1 + i * .25); text(ctx, l, 60, y0 + i * lh, { size: sz, fam: SANS, w: 800, color: i === lines.length - 1 ? C.cv : C.ink, alpha: la }); });
  para(ctx, reel.sub, 60, y0 + lines.length * lh + 40, W - 120, { size: 30, color: C.dim, alpha: easeOut(t, 1.6, 2.2) });
  ctx.restore();
}
/* captions: console log of steps, each typed in at its time */
function captions(ctx, reel, t) {
  const tb = t - T_HOOK; let y = 1265;
  for (const s of reel.steps) {
    if (tb < s.t) break;
    const col = s.k === 'eq' ? C.sig : s.k === 'hi' ? C.cv : C.ink;
    const str = typed(s.text, tb - s.t, 55);
    const lines = wrap(ctx, str, W - 160, 29, MONO, s.k ? 500 : 400);
    text(ctx, s.k === 'eq' ? '=' : s.k === 'hi' ? '!' : '>', 60, y, { size: 29, color: C.dim2 });
    lines.forEach((l, i) => text(ctx, l, 100, y + i * 40, { size: 29, color: col, w: s.k ? 500 : 400 }));
    y += lines.length * 40 + 14;
  }
}
function take(ctx, reel, t) {
  const a = ease(t, T_TAKE, T_TAKE + .4) * (1 - ease(t, T_CTA - .3, T_CTA));
  if (a <= 0) return;
  ctx.save(); ctx.globalAlpha = a;
  rect(ctx, 0, 1210, W, 420, rgba(C.bg, .92));
  rect(ctx, 60, 1232, 6, 320, C.cv);
  text(ctx, 'APPLY IT', 100, 1272, { size: 22, color: C.cv, ls: 5, w: 600 });
  para(ctx, reel.take, 100, 1340, W - 180, { size: 44, fam: SANS, w: 700, color: C.ink, lh: 1.18 });
  ctx.restore();
}
function cta(ctx, reel, t, n) {
  const a = ease(t, T_CTA, T_CTA + .4);
  if (a <= 0) return;
  ctx.save(); ctx.globalAlpha = a;
  rect(ctx, 0, 0, W, H, rgba(C.bg, .88));
  text(ctx, `EP.${String(n).padStart(2, '0')} / 12`, 60, 700, { size: 26, color: C.dim, ls: 4 });
  const cs = fit(ctx, '× TECHNO', W - 120, 150);
  text(ctx, 'MATH', 60, 840, { size: cs, fam: SANS, w: 800, color: C.ink });
  text(ctx, '× TECHNO', 60, 840 + cs * .95, { size: cs, fam: SANS, w: 800, color: C.cv });
  para(ctx, reel.cta, 60, 1080, W - 120, { size: 32, color: C.ink });
  text(ctx, '@leandro_taibbi', 60, 1220, { size: 40, color: C.sig, w: 600 });
  text(ctx, 'save it · next episode: ' + (REELS[n % REELS.length].short), 60, 1280, { size: 26, color: C.dim });
  ctx.restore();
}

function renderFrame(ctx, reel, t) {
  const n = REELS.indexOf(reel) + 1;
  ctx.setTransform(1, 0, 0, 1, 0, 0); ctx.globalAlpha = 1;
  rect(ctx, 0, 0, W, H, C.bg);
  reel.bg.forEach(b => BG[b](ctx, n * 977));
  chrome(ctx, reel, t, n);
  if (t < T_HOOK) hook(ctx, reel, t);
  const tb = t - T_HOOK;
  if (tb > -0.4) {
    const a = ease(t, T_HOOK - .4, T_HOOK + .3) * (t > T_CTA ? 1 : 1);
    ctx.save(); ctx.globalAlpha = a * (t > T_TAKE ? lerp(1, .55, ease(t, T_TAKE, T_TAKE + .4)) : 1);
    reel.draw(ctx, Math.max(0, tb), t);
    ctx.restore();
    if (t < T_TAKE) captions(ctx, reel, t);
  }
  take(ctx, reel, t);
  cta(ctx, reel, t, n);
}

/* visual area shared by all reels */
const V = { x: 60, y: 380, w: W - 120, h: 820, cx: W / 2, cy: 380 + 410, bottom: 1200 };

/* ═══════════════════════════ REELS ═══════════════════════════ */
const REELS = [

/* 01 ─ EUCLIDEAN RHYTHMS */
{ short: 'euclid', bg: ['grid', 'warm'],
  prompt: 'bjorklund --steps 16 --hits 5',
  hook: 'Your hi-hats\nare a theorem.', sub: 'Euclid, 300 BC. Bjorklund, 2003. Your 909, tonight.',
  steps: [
    { t: 0.3, text: 'spread k hits over n steps as evenly as integers allow' },
    { t: 2.6, k: 'eq', text: 'hit(i) = ((i · k) mod n) < k' },
    { t: 5.2, text: 'E(5,16) → the tresillo family. E(7,16) → your clap pattern.' },
    { t: 9.0, text: 'same algorithm: spacing of leap years, cuban cinquillo, neutron accelerator timing' },
    { t: 13.4, k: 'hi', text: 'layer E(5,16) on E(7,16): they share only 3 steps (0, 7, 10) → tension without a fill' },
    { t: 18.0, text: 'rotate the pattern by one step = a new groove, zero new notes' },
  ],
  take: 'Max for Live: one [expr ($i1*$i2)%$i3<$i2] per voice. Rotate, don’t re-draw.',
  cta: 'E(k,n) · every rhythm you love is a ratio.',
  draw(ctx, t) {
    const n = 16, cx = V.cx, cy = 760, R = 300;
    const k2 = t > 5.2 ? 7 : 5, show2 = t > 5.2, both = t > 13.4;
    const e5 = euclid(5, n), e7 = euclid(7, n, 0);
    const pos = (t % BAR) / BAR; const step = Math.floor(pos * n);
    for (let i = 0; i < n; i++) {
      const a = -Math.PI / 2 + i / n * Math.PI * 2, x = cx + Math.cos(a) * R, y = cy + Math.sin(a) * R;
      const hitA = e5[i], hitB = e7[i];
      circle(ctx, x, y, 14, hitA && (!show2 || both) ? C.cv : hitB && show2 ? C.sig : C.panel, C.line);
      if (both && hitA && hitB) circle(ctx, x, y, 22, null, C.ink, 2);
      text(ctx, String(i).padStart(2, '0'), cx + Math.cos(a) * (R + 50), cy + Math.sin(a) * (R + 50) + 8, { size: 20, color: i === step ? C.ink : C.dim2, align: 'center' });
    }
    /* polygon */
    const poly = (arr, col) => { ctx.save(); ctx.strokeStyle = col; ctx.lineWidth = 3; ctx.globalAlpha = .8; ctx.beginPath(); let f = true; for (let i = 0; i < n; i++) if (arr[i]) { const a = -Math.PI / 2 + i / n * Math.PI * 2; const x = cx + Math.cos(a) * R, y = cy + Math.sin(a) * R; f ? ctx.moveTo(x, y) : ctx.lineTo(x, y); f = false; } ctx.closePath(); ctx.stroke(); ctx.restore(); };
    if (!show2 || both) poly(e5, C.cv); if (show2) poly(e7, C.sig);
    /* playhead */
    const a = -Math.PI / 2 + pos * Math.PI * 2; line(ctx, cx, cy, cx + Math.cos(a) * (R - 30), cy + Math.sin(a) * (R - 30), C.ink, 2, .7);
    circle(ctx, cx, cy, 6, C.ink);
    /* readout */
    text(ctx, both ? 'E(5,16) ∪ E(7,16)' : show2 ? 'E(7,16)' : 'E(5,16)', cx, 1150, { size: 44, fam: SANS, w: 800, color: C.ink, align: 'center' });
    const pat = (arr) => arr.map(v => v ? 'x' : '·').join('');
    text(ctx, (both || !show2 ? pat(e5) : pat(e7)), cx, 1195, { size: 26, color: C.dim, align: 'center', ls: 6 });
    if (both) text(ctx, 'shared steps: ' + e5.reduce((s, v, i) => s + (v && e7[i]), 0), cx + 420, 430, { size: 22, color: C.ink, align: 'right' });
    text(ctx, `step ${String(step).padStart(2, '0')}/16`, 60, 430, { size: 22, color: C.dim });
  } },

/* 02 ─ POLYMETER / LCM */
{ short: 'lcm', bg: ['dots'],
  prompt: 'lcm(7,16)',
  hook: 'Two loops.\nOne meets\nthe other\nevery 7 bars.', sub: 'Least common multiple — the only drop you need.',
  steps: [
    { t: 0.3, text: 'a 16-step kick loop against a 7-step bleep loop' },
    { t: 2.8, k: 'eq', text: 'lcm(16, 7) = 112 steps = 7 bars before they realign' },
    { t: 6.4, text: 'every bar the bleep lands on a different 16th: 0, 7, 14, 5, 12, 3, 10…' },
    { t: 10.6, text: 'that drift is the hypnosis. no automation. no new sounds.' },
    { t: 14.6, k: 'hi', text: 'coprime lengths maximise the cycle: 7 vs 16 → 112. 8 vs 16 → 16 (boring)' },
    { t: 18.8, text: 'at 130 BPM the full cycle takes 12.9 s. the ear never catches it.' },
  ],
  take: 'Set one clip to 7/16 in Ableton, leave the rest in 4/4. Launch. Walk away.',
  cta: 'lcm(a,b) is the length of your idea.',
  draw(ctx, t) {
    const x0 = 80, cw = (W - 160) / 16, rows = 7, rh = 70, y0 = 430;
    const stepsDone = (t / S16); const cur = Math.floor(stepsDone) % 112;
    for (let r = 0; r < rows; r++) for (let i = 0; i < 16; i++) {
      const s = r * 16 + i; const x = x0 + i * cw, y = y0 + r * rh;
      const kick = i % 4 === 0, bleep = s % 7 === 0;
      const alive = s <= cur || t > 16;
      rect(ctx, x + 3, y + 3, cw - 6, rh - 6, alive ? (bleep ? rgba(C.sig, .85) : kick ? rgba(C.cv, .55) : C.panel) : rgba(C.panel, .35), s === cur ? C.ink : null);
      if (bleep && alive) text(ctx, String(i), x + cw / 2, y + rh / 2 + 7, { size: 20, color: C.bg, align: 'center', w: 600 });
    }
    for (let r = 0; r < rows; r++) text(ctx, 'bar ' + (r + 1), 60, y0 + r * rh + 42, { size: 18, color: C.dim2, align: 'right' });
    text(ctx, `step ${String(cur).padStart(3, '0')} / 112`, 60, 1000, { size: 26, color: C.dim });
    text(ctx, `cycle = 112 × ${(S16 * 1000).toFixed(1)} ms = ${fmt(112 * S16, 1)} s`, 60, 1050, { size: 26, color: C.ink });
    text(ctx, '■ kick every 4   ■ bleep every 7', 60, 1100, { size: 24, color: C.dim });
    const w = ease(t, 2.8, 4.5); text(ctx, 'lcm(16,7) = ' + Math.round(112 * w), W - 60, 1000, { size: 44, fam: SANS, w: 800, color: C.cv, align: 'right' });
    text(ctx, 'gcd(16,7) = 1 → coprime', W - 60, 1050, { size: 24, color: C.sig, align: 'right', alpha: ease(t, 14.6, 15.4) });
  } },

/* 03 ─ BEATING / DETUNE */
{ short: 'beating', bg: ['scan', 'cold'],
  prompt: 'osc 55.000 Hz ∥ osc 55.271 Hz',
  hook: 'Detune is\na tempo.', sub: 'Two oscillators, one beat frequency. Make it land on the grid.',
  steps: [
    { t: 0.3, text: 'two sines close in pitch add up to one sine with a slow amplitude wobble' },
    { t: 3.0, k: 'eq', text: 'f_beat = |f₁ − f₂|      period = 1 / f_beat' },
    { t: 6.2, text: '55 Hz vs 55.3 Hz → beat every 3.33 s. random. not musical.' },
    { t: 10.0, k: 'hi', text: 'want the wobble every 2 bars at 130 BPM? 2 bars = 3.692 s → detune = 0.2708 Hz' },
    { t: 15.2, k: 'eq', text: 'cents = 1200 · log₂(55.2708 / 55) = 8.5 ¢' },
    { t: 19.2, text: 'so: +8.5 cents on an A1 sub = a 2-bar sidechain you didn’t program' },
  ],
  take: 'Tune the detune: detune(Hz) = BPM / (60 · beats per cycle). Stop guessing at the knob.',
  cta: 'f_beat = BPM / 60 / beats — detune in the key of time.',
  draw(ctx, t) {
    const sync = t > 10.0; const f1 = 55, f2 = sync ? 55 + 1 / (2 * BAR) : 55.3;
    const fb = Math.abs(f2 - f1);
    const y1 = 540, y2 = 720, y3 = 980; const span = 0.09; /* seconds on screen */
    const scroll = t * 0.02;
    const wave = (f, y, col, amp) => plot(ctx, u => Math.sin(2 * Math.PI * f * (u * span + scroll)), V.x, V.x + V.w, y, amp, 600, col, 2.5, .8);
    text(ctx, `osc A  ${fmt(f1, 3)} Hz`, 60, 470, { size: 22, color: C.dim }); wave(f1, y1, C.cv, 55);
    text(ctx, `osc B  ${fmt(f2, 3)} Hz`, 60, 650, { size: 22, color: C.dim }); wave(f2, y2, C.sig, 55);
    text(ctx, 'A + B', 60, 850, { size: 22, color: C.dim });
    /* sum, long time scale: show envelope over 8 s window */
    const win = 8, tt = t;
    plot(ctx, u => { const x = u * win + tt - win; return Math.cos(Math.PI * fb * x) * Math.sin(2 * Math.PI * 7 * u * win); }, V.x, V.x + V.w, y3, 110, 1400, C.ink, 1.5, .9);
    plot(ctx, u => Math.abs(Math.cos(Math.PI * fb * (u * win + tt - win))), V.x, V.x + V.w, y3, 110, 400, C.cv, 3);
    plot(ctx, u => -Math.abs(Math.cos(Math.PI * fb * (u * win + tt - win))), V.x, V.x + V.w, y3, 110, 400, C.cv, 3);
    /* bar lines */
    for (let b = 0; b < 6; b++) { const xb = V.x + V.w - ((tt % (2 * BAR)) + b * 2 * BAR) / win * V.w; if (xb > V.x) { line(ctx, xb, y3 - 130, xb, y3 + 130, C.line, 2); text(ctx, '2 bars', xb + 8, y3 - 112, { size: 16, color: C.dim2 }); } }
    text(ctx, `f_beat = ${fmt(fb, 4)} Hz   period = ${fmt(1 / fb, 3)} s`, 60, 1150, { size: 28, color: sync ? C.ok : C.ink, w: 500 });
    text(ctx, sync ? '= exactly 2 bars @130' : '≠ anything at 130 BPM', 60, 1192, { size: 24, color: sync ? C.ok : C.cv });
  } },

/* 04 ─ GOLDEN RATIO DELAY */
{ short: 'phi', bg: ['plain', 'warm', 'noise'],
  prompt: 'delay --ratio 1.618033988',
  hook: 'The most\nirrational\ndelay time.', sub: 'φ: the number hardest to approximate with a fraction. Perfect for shimmer.',
  steps: [
    { t: 0.3, k: 'eq', text: 'φ = (1 + √5) / 2 = 1.6180339…   1/φ = 0.618…' },
    { t: 3.4, text: 'continued fraction [1; 1, 1, 1, 1…] → converges slower than any other number' },
    { t: 7.4, text: 'a delay at 3/4 of a beat lands back on the grid in 4 repeats. tidy. dead.' },
    { t: 11.2, k: 'hi', text: 'a delay at 1/φ of a beat (285.3 ms @130) never lands on a 16th — ever' },
    { t: 15.6, text: 'repeats fall at 0.618, 1.236, 1.854, 2.472 beats… all off-grid, none the same distance' },
    { t: 19.6, text: 'result: a cloud, not an echo. the dub-techno trick nobody names.' },
  ],
  take: 'Delay time = beat_ms × 0.618 (285 ms @130). Feedback 60 %. The grid stays clean; the tail floats.',
  cta: 'φ delays: structured, never repeating.',
  draw(ctx, t) {
    const phi = (1 + Math.sqrt(5)) / 2; const golden = t > 11.2;
    const ratio = golden ? 1 / phi : 0.75;
    const x0 = 80, x1 = W - 80, beats = 4; const bx = b => lerp(x0, x1, b / beats);
    const y = 700;
    for (let i = 0; i <= beats * 4; i++) { const h = i % 4 === 0 ? 70 : 30; line(ctx, bx(i / 4), y - h, bx(i / 4), y + h, i % 4 === 0 ? C.dim : C.line, i % 4 === 0 ? 3 : 1.5); }
    text(ctx, '16th grid · 4 beats', 60, 590, { size: 22, color: C.dim });
    /* taps */
    const nTaps = Math.min(8, Math.floor((t - (golden ? 11.2 : 0.3)) / .55) + 1);
    for (let k = 1; k <= nTaps; k++) { const pos = k * ratio; if (pos > beats) break; const x = bx(pos), onGrid = Math.abs(pos * 4 - Math.round(pos * 4)) < 1e-6; const amp = Math.pow(.6, k) * 220;
      rect(ctx, x - 4, y - amp, 8, amp, onGrid ? C.dim : C.cv); text(ctx, fmt(pos, 3), x, y + 110, { size: 18, color: onGrid ? C.dim : C.cv, align: 'center' }); if (onGrid) text(ctx, 'on grid', x, y + 135, { size: 16, color: C.dim2, align: 'center' }); }
    rect(ctx, x0 - 4, y - 260, 8, 260, C.ink); text(ctx, 'dry', x0, y + 110, { size: 18, color: C.ink, align: 'center' });
    text(ctx, golden ? 'ratio = 1/φ = 0.618…' : 'ratio = 3/4 = 0.75', 60, 920, { size: 44, fam: SANS, w: 800, color: golden ? C.cv : C.ink });
    text(ctx, `delay = ${fmt(BEAT * 1000 * ratio, 1)} ms @ ${BPM} BPM`, 60, 975, { size: 28, color: C.ink });
    /* continued fraction + spiral */
    text(ctx, 'φ = 1 + 1/(1 + 1/(1 + 1/(1 + …)))', 60, 1040, { size: 26, color: C.sig, alpha: ease(t, 3.4, 4) });
    const sx = W - 230, sy = 1080, s0 = 110; ctx.save(); ctx.strokeStyle = rgba(C.sig, .7); ctx.lineWidth = 2; ctx.beginPath();
    let a = 0, r = s0, px = sx, py = sy; const segs = Math.floor(ease(t, 3.4, 9) * 160);
    for (let i = 0; i < segs; i++) { const th = i / 160 * Math.PI * 4; const rr = s0 * Math.pow(phi, -th / (Math.PI / 2)); const x = sx + Math.cos(th) * rr, y2 = sy - Math.sin(th) * rr; i ? ctx.lineTo(x, y2) : ctx.moveTo(x, y2); }
    ctx.stroke(); ctx.restore();
  } },

/* 05 ─ PRIME LOOP LENGTHS */
{ short: 'primes', bg: ['grid', 'cold'],
  prompt: 'loops: 3 bars · 5 bars · 7 bars',
  hook: 'Primes don’t\nrepeat.\nNeither\nshould you.', sub: 'Three loops, three prime lengths, one 3-minute non-repeating track.',
  steps: [
    { t: 0.3, text: 'three layers looping at 3, 5 and 7 bars' },
    { t: 2.8, k: 'eq', text: 'lcm(3,5,7) = 105 bars → 105 × 1.846 s = 193.8 s = 3:14 @ 130 BPM' },
    { t: 7.0, text: 'a 4 + 8 + 16 bar stack repeats every 16 bars = 29.5 s. you hear it. they hear it.' },
    { t: 11.4, k: 'hi', text: 'with primes every bar is a combination the track has not played yet' },
    { t: 15.4, text: 'push it: 5 · 7 · 11 = 385 bars = 11:50. longer than the track itself.' },
    { t: 19.4, text: 'the hypnotic part is not the sounds. it is the arithmetic.' },
  ],
  take: 'Clip lengths: pad 3 bars, hats 5 bars, acid line 7 bars. Follow actions off. Record 8 minutes.',
  cta: 'prime loops — minimum material, maximum time.',
  draw(ctx, t) {
    const L = [3, 5, 7], cols = [C.cv, C.sig, C.ink]; const bar = t / BAR * 2.2; /* sped-up bars */
    const cx = V.cx, cy = 720;
    L.forEach((len, i) => { const R = 130 + i * 95; circle(ctx, cx, cy, R, null, C.line, 2);
      for (let s = 0; s < len; s++) { const a = -Math.PI / 2 + s / len * Math.PI * 2; circle(ctx, cx + Math.cos(a) * R, cy + Math.sin(a) * R, 7, (Math.floor(bar) % len) === s ? cols[i] : C.panel, cols[i], 2); }
      const a = -Math.PI / 2 + (bar % len) / len * Math.PI * 2; line(ctx, cx, cy, cx + Math.cos(a) * R, cy + Math.sin(a) * R, cols[i], 3, .8);
      text(ctx, `${len} bars`, cx + R + 14, cy - 8 + 0, { size: 20, color: cols[i], alpha: 0 }); });
    text(ctx, 'bar ' + String(Math.floor(bar) % 105).padStart(3, '0') + ' / 105', cx, cy + 12, { size: 30, color: C.ink, align: 'center', w: 600 });
    text(ctx, 'state (3,5,7) = (' + [3, 5, 7].map(l => Math.floor(bar) % l).join(', ') + ')', cx, 1090, { size: 30, color: C.ink, align: 'center' });
    text(ctx, 'lcm = 105 bars = 193.8 s = 3:14', cx, 1145, { size: 44, fam: SANS, w: 800, color: C.cv, align: 'center', alpha: ease(t, 2.8, 3.6) });
    text(ctx, 'vs 4·8·16 → lcm 16 bars = 29.5 s', cx, 1192, { size: 26, color: C.dim, align: 'center', alpha: ease(t, 7, 7.6) });
    L.forEach((len, i) => text(ctx, `■ ${len}`, 60 + i * 110, 430, { size: 24, color: cols[i] }));
  } },

/* 06 ─ KICK TUNING */
{ short: 'kick hz', bg: ['scan', 'warm'],
  prompt: 'f(n) = 440 · 2^(n/12)',
  hook: 'Your kick\nhas a note.\nFind it.', sub: 'Equal temperament is one formula. Your sub either agrees with it or fights it.',
  steps: [
    { t: 0.3, k: 'eq', text: 'f(n) = 440 · 2^(n/12)   n = semitones from A4' },
    { t: 3.4, text: 'E1 = 41.2 Hz · F1 = 43.7 · G1 = 49.0 · A1 = 55.0 · C2 = 65.4' },
    { t: 7.4, text: 'a 909 kick body sits around 50–60 Hz after the pitch sweep decays. that is a note.' },
    { t: 11.4, k: 'hi', text: 'sub at G1 (49.0) with a bass at A1 (55.0): 6 Hz beating in the low end = mud' },
    { t: 15.6, text: 'tune the kick body to the root (A1 55.0) or a fifth below (D1 36.7, 3:2): the harmonics align, the beating stops' },
    { t: 19.4, text: 'one semitone at 49 Hz is only 2.9 Hz. the spectrum analyser cannot see it. the sub can.' },
  ],
  take: 'Spectrum → read the body peak → nearest f(n). Operator pitch = that note. Sidechain less, tune more.',
  cta: 'f = 440·2^(n/12) — tune the thing that moves the air.',
  draw(ctx, t) {
    const notes = [['E1', -41], ['F1', -40], ['F#1', -39], ['G1', -38], ['G#1', -37], ['A1', -36], ['A#1', -35], ['B1', -34], ['C2', -33], ['C#2', -32], ['D2', -31]]; /* semitones from A4 (MIDI 69) */
    const f = n => 440 * Math.pow(2, n / 12);
    const x0 = 80, x1 = W - 80, fmin = 38, fmax = 80; const fx = fr => lerp(x0, x1, Math.log(fr / fmin) / Math.log(fmax / fmin));
    const y = 900;
    /* spectrum of a decaying kick: fundamental sweeping down to 49 Hz */
    const kickTuned = t > 15.6;
    const body = kickTuned ? 55.0 : 49.0;
    const hit = (t * 1.3) % BEAT; const f0 = body * (1 + 2.2 * Math.exp(-hit * 28)); const amp = Math.exp(-hit * 2.2);
    plot(ctx, u => { const fr = fmin * Math.pow(fmax / fmin, u); let s = 0; const g = (c, w, a) => a * Math.exp(-((fr - c) ** 2) / (2 * w * w)); s += g(f0, 2.2 + 6 * hit, amp); s += g(body, 1.4, .75); return clamp(s, 0, 1.2); }, x0, x1, y, 290, 500, C.cv, 3);
    line(ctx, x0, y, x1, y, C.dim, 2);
    notes.forEach(([nm, n]) => { const fr = f(n), x = fx(fr); line(ctx, x, y, x, y + 16, C.dim, 2); text(ctx, nm, x, y + 44, { size: 18, color: C.dim, align: 'center' }); text(ctx, fmt(fr, 1), x, y + 68, { size: 15, color: C.dim2, align: 'center' }); });
    /* bass marker */
    line(ctx, fx(55), y - 440, fx(55), y, C.sig, 3, .9); text(ctx, 'bass A1 55.0', fx(55) + 10, y - 420, { size: 20, color: C.sig });
    line(ctx, fx(body), y - 380, fx(body), y, C.cv, 3, .9); text(ctx, `kick body ${fmt(body, 1)} Hz`, fx(body) - 10, y - 360, { size: 20, color: C.cv, align: 'right' });
    text(ctx, kickTuned ? 'kick A1 55.0 = bass A1 55.0 → Δ 0 Hz. (or D1 36.7: a fifth below, harmonics align)' : 'kick G1 49.0 vs bass A1 55.0 → Δ = 6.0 Hz beating = mud', 60, 1110, { size: 24, color: kickTuned ? C.ok : C.ink, w: 500 });
    text(ctx, `sweep: ${fmt(f0, 1)} Hz → ${fmt(body, 1)} Hz`, 60, 1160, { size: 24, color: C.dim });
    text(ctx, 'semitone @49 Hz = 2.9 Hz', W - 60, 1160, { size: 24, color: C.sig, align: 'right', alpha: ease(t, 19.4, 20) });
    text(ctx, 'f = 440·2^(n/12)   A4 = 440 Hz', 60, 430, { size: 22, color: C.dim });
  } },

/* 07 ─ LFO IN HERTZ */
{ short: 'lfo hz', bg: ['blue'],
  prompt: 'lfo.rate = BPM / 60 / beats',
  hook: 'Free-running\nLFOs are\nnot free.', sub: 'Every “random” modulation at 130 BPM has a number. Here they are.',
  steps: [
    { t: 0.3, k: 'eq', text: 'f = BPM / (60 · beats per cycle)' },
    { t: 3.0, text: '1 bar → 0.5417 Hz · 2 bars → 0.2708 · 4 bars → 0.1354 · 8 bars → 0.0677 Hz' },
    { t: 7.4, text: '16 bars → 0.0339 Hz. that is one filter sweep per 29.5 s. the “breathing” of a long techno track.' },
    { t: 11.8, k: 'hi', text: 'two LFOs at 0.1354 and 0.0677 Hz re-phase every 14.8 s — an 8-bar structure without a clip' },
    { t: 16.2, text: 'an LFO at 0.1 Hz “sounds random” because 0.1 ≠ k · 0.0339 for any small k' },
    { t: 20.0, text: 'hardware (no sync input): dial the Hz. you now have tempo sync on a Buchla.' },
  ],
  take: 'Table on your phone: 130 BPM → 0.5417 Hz per bar. Divide by bars. Type it into the hardware LFO.',
  cta: 'f = BPM/60/beats — sync without a sync cable.',
  draw(ctx, t) {
    const rows = [[1, 'bar'], [2, 'bars'], [4, 'bars'], [8, 'bars'], [16, 'bars']];
    const x0 = 420, x1 = W - 80;
    rows.forEach(([b, lbl], i) => { const y = 500 + i * 140; const fr = BPM / 60 / (4 * b); const vis = ease(t, 2.6 + i * .9, 3.2 + i * .9);
      text(ctx, `${b} ${lbl}`, 60, y + 10, { size: 26, color: C.dim, alpha: vis }); text(ctx, fmt(fr, 4) + ' Hz', 60, y + 50, { size: 34, color: i === 4 && t > 7.4 ? C.cv : C.ink, w: 600, alpha: vis });
      plot(ctx, u => Math.sin(2 * Math.PI * fr * (u * 29.54 + t * .35) * 1), x0, x1, y + 20, 48, 500, i % 2 ? C.sig : C.cv, 2.5, .9 * vis);
      line(ctx, x0, y + 20, x1, y + 20, C.line, 1); });
    text(ctx, '← 16 bars (29.54 s) →', (x0 + x1) / 2, 450, { size: 20, color: C.dim2, align: 'center' });
    text(ctx, 'f = BPM / 60 / beats', 60, 430, { size: 22, color: C.dim });
    text(ctx, '0.1354 ∧ 0.0677 → re-phase 14.77 s', 60, 1185, { size: 26, color: C.ok, alpha: ease(t, 11.8, 12.5) });
    /* a free-running 0.1 Hz line drifting against the grid */
    if (t > 16.2) { plot(ctx, u => Math.sin(2 * Math.PI * 0.1 * (u * 29.54 + t * .35)), x0, x1, 1150, 30, 500, C.dim, 2, ease(t, 16.2, 17)); text(ctx, '0.1000 Hz "random"', 60, 1150, { size: 24, color: C.dim, alpha: ease(t, 16.2, 17) }); }
  } },

/* 08 ─ CLOCK DIVIDERS + XOR */
{ short: 'xor clock', bg: ['dots', 'warm'],
  prompt: 'clk/2 ⊕ clk/3 ⊕ clk/5',
  hook: 'Three dividers.\nOne XOR.\n30-step\npattern.', sub: 'Boolean logic is a sequencer. Modular people knew. Ableton people, here you go.',
  steps: [
    { t: 0.3, text: 'a divider outputs a pulse every n clocks: /2 /3 /5' },
    { t: 2.8, k: 'eq', text: 'period of the combination = lcm(2,3,5) = 30 steps' },
    { t: 6.2, text: 'AND → 1 hit in 30. OR → 22 hits in 30. XOR → 16 hits, odd, lopsided, alive.' },
    { t: 10.6, k: 'hi', text: 'XOR of /3 and /5 alone: 14 hits in 15 steps with a single hole. a rolling hat that limps once' },
    { t: 15.2, text: 'swap /5 for /7 → period 42 (2.6 bars). the fill writes itself.' },
    { t: 19.2, text: 'this is 3 counters and a truth table. 12 lines of gen~.' },
  ],
  take: 'Max: [counter] ×3 → [% n] → [== 0] → [expr $i1 ^ $i2 ^ $i3]. Send to a 909 rim. Done.',
  cta: 'lcm(2,3,5)=30 — logic gates groove.',
  draw(ctx, t) {
    const N = 30, x0 = 80, cw = (W - 160) / N; const cur = Math.floor(t / S16 / 1.4) % N;
    const divs = [2, 3, 5]; const rows = divs.map(d => Array.from({ length: N }, (_, i) => i % d === 0 ? 1 : 0));
    const op = t > 6.2 ? (t > 10.6 && t < 15.2 ? 'xor35' : 'xor') : 'or';
    const out = Array.from({ length: N }, (_, i) => op === 'xor35' ? (rows[1][i] ^ rows[2][i]) : op === 'xor' ? (rows[0][i] ^ rows[1][i] ^ rows[2][i]) : (rows[0][i] | rows[1][i] | rows[2][i]));
    const draw = (arr, y, col, lbl, big) => { text(ctx, lbl, 60, y - 22, { size: 20, color: C.dim }); for (let i = 0; i < N; i++) { const h = big ? 110 : 60; rect(ctx, x0 + i * cw + 2, y + (arr[i] ? 0 : h - 8), cw - 4, arr[i] ? h : 8, i === cur ? C.ink : arr[i] ? col : C.line); } };
    draw(rows[0], 500, C.sig, 'clk / 2'); draw(rows[1], 620, C.sig, 'clk / 3'); draw(rows[2], 740, C.sig, 'clk / 5');
    draw(out, 900, C.cv, op === 'or' ? 'OR' : op === 'xor35' ? 'XOR (/3,/5)' : 'XOR (/2,/3,/5)', true);
    text(ctx, `hits: ${out.reduce((a, b) => a + b, 0)} / 30`, W - 60, 878, { size: 22, color: C.cv, align: 'right' });
    text(ctx, out.map(v => v ? '1' : '0').join(''), x0, 1080, { size: 26, color: C.dim, ls: 7.6 });
    text(ctx, 'period = lcm(2,3,5) = 30', 60, 1150, { size: 44, fam: SANS, w: 800, color: C.ink, alpha: ease(t, 2.8, 3.6) });
    text(ctx, 'step ' + String(cur).padStart(2, '0'), 60, 430, { size: 22, color: C.dim });
  } },

/* 09 ─ SWING IN MILLISECONDS */
{ short: 'swing ms', bg: ['grid'],
  prompt: 'swing 54% @ 130 BPM → ?ms',
  hook: '54% swing\nis 9.2 ms.', sub: 'MPC swing, decoded. Then stolen for your techno.',
  steps: [
    { t: 0.3, k: 'eq', text: '8th @130 = 230.77 ms · straight 16th = 115.38 ms' },
    { t: 3.4, text: 'MPC swing % = where the off-16th sits inside the 8th. 50 % = dead centre.' },
    { t: 7.0, k: 'eq', text: 'delay = (swing − 0.5) × 230.77 ms → 54 % = 9.2 ms · 58 % = 18.5 ms · 62 % = 27.7 ms' },
    { t: 11.6, k: 'hi', text: 'hearing threshold for timing ≈ 10 ms. 54 % is right at the edge: felt, not heard' },
    { t: 15.6, text: 'at 140 BPM the same 54 % is only 8.6 ms. swing in % is tempo-relative. in ms it is not.' },
    { t: 19.4, text: 'techno rule: hats at 54–56 %, kick at 50 %. the gap IS the groove.' },
  ],
  take: 'Ableton groove pool: timing 100 %, then nudge only the off-16ths by +9 ms. Keep the kick on the grid.',
  cta: 'delay = (swing−0.5)·8th — groove is a number.',
  draw(ctx, t) {
    const x0 = 100, x1 = W - 100, y = 700; const sw = t < 7 ? .5 : t < 11.6 ? .54 : t < 15.6 ? .58 : .54;
    const eighth = 230.77; const k = (x1 - x0) / (2 * eighth);
    rect(ctx, x0, y - 2, x1 - x0, 4, C.line);
    for (let i = 0; i < 5; i++) { const x = x0 + i * k * eighth / 2; const off = i % 2 === 1; const dx = off ? (sw - .5) * eighth * k : 0; const xx = x + dx; const h = off ? 150 : 220;
      rect(ctx, xx - 6, y - h, 12, h, off ? C.cv : C.ink); text(ctx, off ? fmt((i * eighth / 2) + (sw - .5) * eighth, 1) + ' ms' : fmt(i * eighth / 2, 1) + ' ms', xx, y + 46, { size: 18, color: off ? C.cv : C.dim, align: 'center' });
      if (off && sw > .5) { line(ctx, x, y - 170, xx, y - 170, C.sig, 3); text(ctx, '+' + fmt((sw - .5) * eighth, 1) + ' ms', (x + xx) / 2, y - 185, { size: 20, color: C.sig, align: 'center' }); }
      if (i % 2 === 0 && i < 4) { text(ctx, '8th', x + k * eighth / 2, y + 90, { size: 18, color: C.dim2, align: 'center' }); } }
    /* pulse LED */
    const ph = (t % BEAT) / BEAT; const kickOn = ph < .08, hatOn = Math.abs(ph - (.5 + (sw - .5))) < .08;
    circle(ctx, W - 160, 470, 16, kickOn ? C.ink : C.panel, C.line); text(ctx, 'kick', W - 130, 478, { size: 20, color: C.dim });
    circle(ctx, W - 160, 520, 16, hatOn ? C.cv : C.panel, C.line); text(ctx, 'hat', W - 130, 528, { size: 20, color: C.dim });
    text(ctx, `swing ${Math.round(sw * 100)} %`, 60, 920, { size: 72, fam: SANS, w: 800, color: C.ink });
    text(ctx, `= off-16th at ${fmt(sw * eighth, 1)} ms  (+${fmt((sw - .5) * eighth, 1)} ms)`, 60, 975, { size: 28, color: C.cv });
    text(ctx, 'JND for timing ≈ 10 ms', 60, 1060, { size: 26, color: C.sig, alpha: ease(t, 11.6, 12.3) });
    text(ctx, '@140 BPM: 8th = 214.3 ms → 54 % = +8.6 ms', 60, 1110, { size: 26, color: C.dim, alpha: ease(t, 15.6, 16.3) });
    text(ctx, `8th = ${fmt(eighth, 2)} ms`, 60, 430, { size: 22, color: C.dim });
  } },

/* 10 ─ FOURIER / THE CLICK */
{ short: 'fourier', bg: ['scan', 'cold'],
  prompt: 'Σ sin((2k+1)x) / (2k+1)',
  hook: 'A square\nwave is\nan infinite\nsum.', sub: 'And a click is every frequency at once. That is why your kick’s transient cuts.',
  steps: [
    { t: 0.3, k: 'eq', text: 'square(x) = 4/π · Σ sin((2k+1)x) / (2k+1)' },
    { t: 3.4, text: 'odd harmonics only, amplitude 1/n. 1, 1/3, 1/5, 1/7…' },
    { t: 7.0, text: 'add 15 of them and the corners ring: Gibbs overshoot, 8.9 %, never goes away' },
    { t: 11.0, k: 'hi', text: 'the limit case: an impulse (a click) = every frequency, equal amplitude, zero phase' },
    { t: 15.4, text: 'so a 1-sample transient on your kick is the only thing in the mix that hits all bands' },
    { t: 19.2, text: 'the “punch” is bandwidth, not level. do not EQ it. shorten it.' },
  ],
  take: 'Layer a 1–3 ms broadband click on the kick, −12 dB. Then take 2 dB off the body. More punch, less level.',
  cta: 'square = Σ odd/n — the click is the whole spectrum.',
  draw(ctx, t) {
    const K = t < 11 ? Math.max(1, Math.floor(1 + ease(t, 0.3, 9) * 14)) : 60;
    const x0 = 80, x1 = W - 80, y = 640; const impulse = t > 11;
    const sq = u => { let s = 0; for (let k = 0; k < K; k++) { const n = 2 * k + 1; s += Math.sin(n * (u * Math.PI * 4 + t * 2)) / n; } return s * 4 / Math.PI; };
    if (!impulse) { line(ctx, x0, y, x1, y, C.line, 1); plot(ctx, u => (Math.sin(u * Math.PI * 4 + t * 2) >= 0 ? 1 : -1), x0, x1, y, 150, 800, C.dim2, 2, .6); plot(ctx, sq, x0, x1, y, 150, 1200, C.cv, 3);
      text(ctx, `harmonics: ${K}  (odd only)`, 60, 430, { size: 22, color: C.dim }); }
    else { line(ctx, x0, y, x1, y, C.line, 1); const sig = u => { const d = (u - .5) * 40; return Math.abs(d) < .05 ? 1 : Math.sin(Math.PI * d) / (Math.PI * d); }; plot(ctx, u => sig(u), x0, x1, y, 170, 1200, C.cv, 3); text(ctx, 'impulse δ(t)  →  spectrum: flat', 60, 430, { size: 22, color: C.dim }); }
    /* spectrum bars */
    const sy = 1120, sx0 = 80, bw = (W - 160) / 32;
    for (let n = 1; n <= 31; n++) { const amp = impulse ? .8 : (n % 2 === 1 && n <= 2 * K - 1) ? 1 / n : 0; const h = amp * 260; rect(ctx, sx0 + (n - 1) * bw + 4, sy - h, bw - 8, h, impulse ? C.sig : n === 1 ? C.ink : C.cv); if (n % 5 === 0 || n === 1) text(ctx, 'h' + n, sx0 + (n - 1) * bw + bw / 2, sy + 30, { size: 16, color: C.dim2, align: 'center' }); }
    text(ctx, impulse ? 'all harmonics · equal amplitude' : 'odd harmonics · amplitude 1/n', 60, 870, { size: 26, color: C.ink });
    text(ctx, 'Gibbs overshoot ≈ 8.9 %', W - 60, 870, { size: 22, color: C.cv, align: 'right', alpha: ease(t, 7, 7.6) * (impulse ? 0 : 1) });
  } },

/* 11 ─ LOGISTIC MAP */
{ short: 'chaos', bg: ['plain', 'noise', 'warm'],
  prompt: 'x[n+1] = r · x[n] · (1 − x[n])',
  hook: 'One line\nof code.\nInfinite\nsequencer.', sub: 'The logistic map: controlled chaos between “boring” and “noise”, with a knob.',
  steps: [
    { t: 0.3, k: 'eq', text: 'x[n+1] = r · x[n] · (1 − x[n])     0 < x < 1' },
    { t: 3.2, text: 'r < 3 → settles on one value. a sequencer playing one note.' },
    { t: 6.4, text: 'r = 3.2 → period 2. r = 3.5 → period 4. r = 3.56 → period 8. doubling, faster and faster.' },
    { t: 11.0, k: 'hi', text: 'r = 3.5699… chaos. deterministic, repeatable with the same seed, never periodic' },
    { t: 15.2, text: 'r = 3.83 → a window of period 3 inside the chaos. the “triplet” zone.' },
    { t: 19.0, text: 'map x to a 16-note scale, r to a knob: 3.2 is a loop, 3.7 is alive, 3.99 is a kick drum of noise.' },
  ],
  take: 'gen~: History x; x = r*x*(1-x); out = floor(x*16). Map r to a K2 fader, 2.9 → 3.99. That fader is the whole track.',
  cta: 'x = r·x·(1−x) — a melody with one parameter.',
  draw(ctx, t) {
    const x0 = 80, x1 = W - 80, y0 = 1120, y1 = 460; const rmin = 2.8, rmax = 4;
    const prog = ease(t, 0.3, 14); const rNow = lerp(rmin, rmax, prog);
    /* bifurcation diagram, drawn to rNow */
    ctx.save(); ctx.fillStyle = rgba(C.cv, .55);
    const cols = Math.floor(prog * 420);
    for (let c = 0; c < cols; c++) { const r = lerp(rmin, rmax, c / 420); let x = .31 + .17 * ((c * 7919) % 13) / 13; for (let i = 0; i < 120; i++) x = r * x * (1 - x); for (let i = 0; i < 90; i++) { x = r * x * (1 - x); ctx.fillRect(lerp(x0, x1, c / 420), lerp(y0, y1, x), 1.6, 1.6); } }
    ctx.restore();
    line(ctx, x0, y0, x1, y0, C.dim, 2);
    [[3, 'r=3'], [3.449, '3.449'], [3.5699, '3.5699'], [3.83, '3.83']].forEach(([r, l]) => { if (r <= rNow) { const x = lerp(x0, x1, (r - rmin) / (rmax - rmin)); line(ctx, x, y1 - 10, x, y0, C.sig, 1, .6); text(ctx, l, x, y1 - 20, { size: 17, color: C.sig, align: 'center' }); } });
    text(ctx, 'r →', x1, y0 + 40, { size: 20, color: C.dim, align: 'right' }); text(ctx, 'x ↑', 60, y1 - 50, { size: 20, color: C.dim });
    /* live sequence at a chosen r */
    const rSeq = t < 6.4 ? 2.9 : t < 11 ? 3.5 : t < 15.2 ? 3.7 : t < 19 ? 3.83 : 3.7;
    let x = .3; const seq = []; for (let i = 0; i < 48 + Math.floor(t * 8); i++) { x = rSeq * x * (1 - x); if (i >= 32) seq.push(x); }
    const last = seq.slice(-16); const bw = (W - 160) / 16;
    last.forEach((v, i) => { const h = 70 + v * 90; rect(ctx, x0 + i * bw + 3, 1190 - h, bw - 6, h, i === 15 ? C.ink : rgba(C.sig, .85)); });
    text(ctx, `r = ${fmt(rSeq, 2)} → note ${Math.floor(last[15] * 16)}`, W - 60, 1188 - 175, { size: 28, color: C.ink, align: 'right', w: 500 });
    text(ctx, 'Feigenbaum δ = 4.669…', 60, 430, { size: 22, color: C.dim, alpha: ease(t, 6.4, 7) });
  } },

/* 12 ─ RISSET RHYTHM */
{ short: 'risset', bg: ['blue', 'cold'],
  prompt: 'tempo(t) = T₀ · 2^(t/T)',
  hook: 'A beat that\naccelerates\nforever.', sub: 'Risset rhythm: the Shepard tone of tempo. Built from one loop and a crossfade.',
  steps: [
    { t: 0.3, text: 'stack the same loop at tempo ×1, ×2, ×4. each layer fades in from silence and out to silence.' },
    { t: 4.4, k: 'eq', text: 'tempo(t) = T₀ · 2^(t/T)   — every T seconds the tempo doubles…' },
    { t: 8.4, k: 'hi', text: '…but the ×2 layer that was quiet is now the loud one, at exactly the tempo you started from' },
    { t: 12.6, text: 'the brain tracks the loudest layer. it hears acceleration. the average tempo never changes.' },
    { t: 16.6, text: 'gain curve: a raised cosine over log-tempo — the layer peaks at ×2 and dies at ×4' },
    { t: 20.2, text: 'T = 8 bars = 14.8 s. one cycle, then it loops seamlessly. the crowd never lands.' },
  ],
  take: 'Ableton: 3 copies of a clip, warp ×1, ×2, ×4, pitch-shifted. Automate gains with Hann windows offset by 1/3. 8-bar cycle.',
  cta: 'T₀·2^(t/T) — an escalator with no top floor.',
  draw(ctx, t) {
    const T = 8; const ph = (t / T) % 1; /* 0..1 within cycle */
    const x0 = 80, x1 = W - 80;
    /* log-tempo axis: ×0.5 .. ×8 */
    const lmin = -1, lmax = 3; const lx = l => lerp(x0, x1, (l - lmin) / (lmax - lmin));
    const y = 760;
    for (let l = -1; l <= 3; l++) { line(ctx, lx(l), y - 220, lx(l), y + 30, C.line, 2); text(ctx, '×' + Math.pow(2, l), lx(l), y + 60, { size: 20, color: C.dim, align: 'center' }); }
    const gain = l => { const u = (l - 0) / 2; return (u <= 0 || u >= 1) ? 0 : 0.5 - 0.5 * Math.cos(2 * Math.PI * u); }; /* peaks at ×2, dies at ×1 and ×4 */
    /* gain window drawn */
    plot(ctx, u => gain(lerp(lmin, lmax, u)), x0, x1, y, 200, 400, C.dim2, 2, .7);
    /* three layers moving right */
    for (let i = -1; i <= 2; i++) { const l = i + ph; if (l < lmin || l > lmax) continue; const g = gain(l); const x = lx(l); rect(ctx, x - 10, y - g * 200, 20, g * 200, i % 2 ? C.cv : C.sig); text(ctx, fmt(Math.pow(2, l) * BPM, 0) + ' bpm', x, y - g * 200 - 16, { size: 18, color: C.ink, align: 'center', alpha: g > .05 ? 1 : 0 }); }
    /* pulses: each layer flashes at its own tempo */
    text(ctx, 'perceived: ' + fmt(BPM * Math.pow(2, ph), 1) + ' BPM  ↑', 60, 950, { size: 44, fam: SANS, w: 800, color: C.cv });
    text(ctx, 'mean log-tempo: constant (×2 = 260 BPM)', 60, 1000, { size: 26, color: C.dim });
    const yy = 1120; for (let i = 0; i < 3; i++) { const l = i - 1 + ph + 1; const f = Math.pow(2, l) * BPM / 60; const on = (t * f) % 1 < .18; const g = gain(l); circle(ctx, 110 + i * 90, yy, 22, on ? rgba(i % 2 ? C.cv : C.sig, .25 + .75 * g) : C.panel, C.line); }
    text(ctx, '3 layers · Hann window · T = 8 s', 400, yy + 10, { size: 22, color: C.dim });
    text(ctx, `t/T = ${fmt(ph, 2)}`, 60, 430, { size: 22, color: C.dim });
  } },
];

if (typeof module !== 'undefined') module.exports = { REELS, renderFrame, DUR };

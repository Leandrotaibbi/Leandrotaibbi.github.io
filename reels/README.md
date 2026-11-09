# reels/ — MATH × TECHNO

12 Reel da 30 s (1080 × 1920, 30 fps, con audio) su matematica applicata alla techno, in formato
"viewport": una scena 3D vera disegnata su canvas (proiezione prospettica, facce ombreggiate ordinate
per profondità, wireframe, griglia, nebbia) incorniciata come una finestra di Blender — overlay
"User Perspective", gizmo degli assi, timeline, status bar. Sotto: pannelli PARAMETERS / DERIVED che si
aggiornano in tempo reale, blocco formule in serif, una frase per fase (4 × 7.5 s). Ogni frame è una
funzione pura di `(reel, t)`: lo stesso codice fa l'anteprima nel browser e il render MP4.

```
index.html     player: apre il reel, frecce ←/→ cambiano episodio, spazio = pausa,
               click sulla barra = seek. ?reel=7 apre il settimo.
reels.js       motore 3D + chrome Blender + i 12 episodi (title, tags, phases, params, derived, formulas, scene)
synth.py       audio: un loop a 130 BPM per episodio che dimostra la sua matematica → out/audio/EPnn.wav
render.js      Playwright + ffmpeg → out/EPnn-slug.mp4
CAPTIONS.md    didascalie, script voce (it), hashtag, note di pubblicazione
fonts/         STIX Two (regular/italic) · Share Tech Mono · VT323 — in locale, nessuna CDN. L'overlay usa DejaVu Sans di sistema (fallback Verdana).
out/           i 12 MP4 con audio · out/audio/ i WAV
```

## Render

```bash
cd reels
python3 synth.py            # audio, tutti e 12 (~6 min) — oppure: python3 synth.py 3 7
node render.js              # video muti, tutti e 12 (~70 s ciascuno) — oppure: node render.js 3 7
node render.js --mux        # inserisce out/audio/EPnn.wav nei video (AAC 192k)
node render.js --stills     # 4 fotogrammi chiave per reel (uno per frase) in out/stills/
```

Richiede `playwright` (Chromium), `ffmpeg`, `numpy` e `scipy`.

## Aggiungere un episodio

Un oggetto in `REELS` in `reels.js`:

```js
{ short: 'nome', title: 'TITOLO IN ALTO', tags: 'parole chiave · sotto il titolo', pal: 'phosphor', obj: 'EP13_nome_oggetto',
  phases: ['Frase 1 con *corsivo* e **parola chiave**.', '…', '…', '…'],        // 4 × 7.5 s
  params(t, p, tp) { return [['chiave', 'valore', evidenzia?], …]; },         // pannello sinistro, 5 righe
  derived(t, p, tp) { return [['chiave', 'valore'], …]; },                     // pannello destro, 5 righe
  formulas(t, p) { return ['riga 1', 'riga 2']; },                            // serif, *corsivo* per le variabili
  scene(t, p, tp) { floor(this); addMesh(torus(1.5, .5, 48, 16), { color: P.face, wire: P.wire, cull: true }); … } }
```

Primitive 3D: `torus`, `sphere`, `box`, `tube(curva, n, r)`; `addMesh / addPoint / addLine / addLabel`;
trasformazioni `T.tr / T.ry / T.rx / T.rz / T.sc / T.chain`. Palette in `PAL` (una per episodio, `light: true`
per il registro chiaro). Il tempo a 130 BPM è in `BPM / BEAT / BAR / S16`.

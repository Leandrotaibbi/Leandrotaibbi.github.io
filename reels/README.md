# reels/ — MATH × TECHNO

12 Reel da 30 s (1080 × 1920, 30 fps) su matematica applicata alla techno, in formato
"lesson": header con metadati, un visual grande animato a 130 BPM, una frase con una
parola in bold che cambia ogni 7.5 s, la formula in fondo. Ogni frame è una funzione
pura di `(reel, t)`: lo stesso codice fa l'anteprima nel browser e il render MP4.

```
index.html     player: apre il reel, frecce ←/→ cambiano episodio, spazio = pausa,
               click sulla barra = seek. ?reel=7 apre il settimo.
reels.js       motore + i 12 episodi (lesson, 4 frasi, formula, funzione draw)
render.js      Playwright + ffmpeg → out/EPnn-slug.mp4
CAPTIONS.md    didascalie, script voce (it), hashtag, note di pubblicazione
fonts/         Syne + Archivo + IBM Plex Mono in locale (render offline, nessuna CDN)
out/           i 12 MP4 (muti) — l'audio va aggiunto in fase di post
```

## Render

```bash
cd reels
node render.js              # tutti e 12 (~45 s ciascuno)
node render.js 3 7          # solo EP03 ed EP07
node render.js --stills     # 4 fotogrammi chiave per reel (uno per frase) in out/stills/
```

Richiede `playwright` (Chromium) e `ffmpeg` nel PATH.

## Aggiungere un episodio

Un oggetto in `REELS` in `reels.js`:

```js
{ short: 'nome breve', lesson: 'Titolo della lesson', light: false,   // light: true → registro chiaro
  track: 'Leandro Taibbi – Stelle', label: 'Ithica Records',          // opzionali: default ID / UNRELEASED
  formula: 'la formula che sta in fondo',
  phases: [                                                            // 4 frasi × 7.5 s, **bold** = la parola chiave
    { text: 'Frase uno con una parola in **bold**.', sub: 'riga piccola con il numero' },
    { text: '…', sub: '…' }, { text: '…', sub: '…' }, { text: '…', sub: '…' },
  ],
  draw(ctx, t, p, tp) { /* visual: t = secondi, p = fase 0..3, tp = secondi dentro la fase; area V (y 580–1340) */ } }
```

Il tempo a 130 BPM è in `BPM / BEAT / BAR / S16`: cambiando `BPM` tutti i numeri a
schermo si ricalcolano (le didascalie in `CAPTIONS.md` vanno aggiornate a mano).

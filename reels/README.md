# reels/ — MATH × TECHNO

12 Reel da 30 s (1080 × 1920, 30 fps) su matematica applicata alla techno.
Ogni frame è una funzione pura di `(reel, t)`: lo stesso codice fa l'anteprima nel
browser e il render MP4.

```
index.html     player: apre il reel, frecce ←/→ cambiano episodio, spazio = pausa,
               click sulla barra = seek. ?reel=7 apre il settimo.
reels.js       motore + i 12 episodi (hook, step, take, CTA, funzione draw)
render.js      Playwright + ffmpeg → out/EPnn-slug.mp4
CAPTIONS.md    didascalie, script voce (it), hashtag, note di pubblicazione
fonts/         Syne + IBM Plex Mono in locale (render offline, nessuna CDN)
out/           i 12 MP4 (muti) — l'audio va aggiunto in fase di post
```

## Render

```bash
cd reels
node render.js              # tutti e 12 (~45 s ciascuno)
node render.js 3 7          # solo EP03 ed EP07
node render.js --stills     # 6 fotogrammi chiave per reel in out/stills/ (revisione rapida)
```

Richiede `playwright` (Chromium) e `ffmpeg` nel PATH.

## Aggiungere un episodio

Un oggetto in `REELS` in `reels.js`:

```js
{ short: 'nome breve', bg: ['grid', 'warm'],           // sfondi: grid scan dots blue plain noise warm cold
  prompt: 'riga da terminale nell\'hook',
  hook: 'Frase grande\nsu più righe', sub: 'sottotitolo',
  steps: [{ t: 0.3, text: '…' }, { t: 3, k: 'eq', text: 'formula' }, { t: 9, k: 'hi', text: 'punto chiave' }],
  take: 'la mossa pratica (Ableton / M4L / hardware)',
  cta: 'una riga per la schermata finale',
  draw(ctx, tb, t) { /* visual: tb = secondi dall\'inizio del body, area V */ } }
```

I tempi degli step sono relativi al body (parte a 2.8 s). Il visual va disegnato fra
y = 380 e y = 1200; le didascalie partono da y = 1265. Il tempo a 130 BPM è in
`BPM / BEAT / BAR / S16`: cambiando `BPM` tutti i numeri a schermo si ricalcolano.

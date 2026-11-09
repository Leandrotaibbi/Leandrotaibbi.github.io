/* Render reels to MP4 (1080×1920, 30 fps, 30 s) with Playwright + ffmpeg.
   usage:  node render.js                 → all 12 → out/EP01-euclid.mp4 …
           node render.js 3 7             → only reels 3 and 7
           node render.js --stills        → 5 JPEG key-frames per reel in out/stills (fast review) */
const { chromium } = require('playwright');
const { spawn } = require('child_process');
const fs = require('fs'), path = require('path');

const FPS = 30, DUR = 30, W = 1080, H = 1920;
const args = process.argv.slice(2);
const STILLS = args.includes('--stills');
const wanted = args.filter(a => /^\d+$/.test(a)).map(Number);
const OUT = path.join(__dirname, 'out'); fs.mkdirSync(OUT, { recursive: true });

(async () => {
  const browser = await chromium.launch();
  const page = await browser.newPage({ viewport: { width: W, height: H }, deviceScaleFactor: 1 });
  await page.goto('file://' + path.join(__dirname, 'index.html') + '?render=1', { waitUntil: 'load' });
  await page.waitForFunction(() => window.__ready === true);
  const reels = await page.evaluate(() => REELS.map(r => r.short));
  const list = wanted.length ? wanted : reels.map((_, i) => i + 1);

  for (const n of list) {
    const i = n - 1; const slug = reels[i].replace(/\s+/g, '-');
    const name = `EP${String(n).padStart(2, '0')}-${slug}`;
    if (STILLS) {
      const dir = path.join(OUT, 'stills'); fs.mkdirSync(dir, { recursive: true });
      for (const t of [3, 11, 18, 26]) {
        await page.evaluate(([i, t]) => window.__seek(i, t), [i, t]);
        await page.screenshot({ path: path.join(dir, `${name}_t${String(t).padStart(4, '0')}.jpg`), type: 'jpeg', quality: 80 });
      }
      console.log('stills', name); continue;
    }
    const mp4 = path.join(OUT, name + '.mp4');
    const ff = spawn('ffmpeg', ['-y', '-loglevel', 'error', '-f', 'image2pipe', '-framerate', String(FPS), '-i', '-',
      '-c:v', 'libx264', '-pix_fmt', 'yuv420p', '-crf', '20', '-preset', 'medium', '-movflags', '+faststart', mp4], { stdio: ['pipe', 'inherit', 'inherit'] });
    const frames = FPS * DUR; const t0 = Date.now();
    for (let f = 0; f < frames; f++) {
      await page.evaluate(([i, t]) => window.__seek(i, t), [i, f / FPS]);
      const buf = await page.screenshot({ type: 'jpeg', quality: 92 });
      if (!ff.stdin.write(buf)) await new Promise(r => ff.stdin.once('drain', r));
    }
    ff.stdin.end();
    await new Promise(r => ff.on('close', r));
    console.log(`${name}.mp4  ${(fs.statSync(mp4).size / 1e6).toFixed(1)} MB  ${((Date.now() - t0) / 1000).toFixed(0)} s`);
  }
  await browser.close();
})();

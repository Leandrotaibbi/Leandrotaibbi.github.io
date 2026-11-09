#!/usr/bin/env python3
"""Audio for the 12 reels — 30 s each, 48 kHz, 130 BPM, mono→stereo.
Every episode's loop demonstrates the episode's maths (Euclidean hats, CRT bleep,
beating sub, φ delay, prime loop lengths, kick tuning, torus-knot LFOs, LFSR rim,
swing in ms, partial sums, logistic sequencer, Risset rhythm).

usage: python3 synth.py            → out/audio/EPnn.wav (all)
       python3 synth.py 3 7        → only those episodes
"""
import sys, os, math
import numpy as np
from scipy import signal
from scipy.io import wavfile

SR, DUR, BPM = 48000, 30.0, 130.0
BEAT, BAR, S16 = 60 / BPM, 240 / BPM, 15 / BPM
N = int(SR * DUR)
T = np.arange(N) / SR
PH = DUR / 4
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'out', 'audio')
PHI = (1 + 5 ** .5) / 2
rng = np.random.default_rng(7)

# ───────── building blocks ─────────
def env_exp(n, tau):
    return np.exp(-np.arange(n) / SR / tau)

def place(buf, sig, t0, gain=1.0):
    i = int(round(t0 * SR))
    if i >= N or i + len(sig) <= 0:
        return
    s = sig[: N - i] if i >= 0 else sig[-i:]
    j = max(i, 0)
    buf[j:j + len(s)] += gain * s

def kick(f_end=49.0, f_start=None, dur=0.6, click=0.6, tau=0.22):
    f_start = f_start or f_end * 3.2
    n = int(dur * SR); t = np.arange(n) / SR
    f = f_end + (f_start - f_end) * np.exp(-t * 28)
    ph = 2 * np.pi * np.cumsum(f) / SR
    body = np.sin(ph) * np.exp(-t / tau)
    cl = rng.standard_normal(n) * np.exp(-t * 900) * click
    return np.tanh(1.6 * (body + cl))

def noise_burst(dur, hp=6000, lp=None, tau=0.02, order=4):
    n = int(dur * SR)
    x = rng.standard_normal(n) * np.exp(-np.arange(n) / SR / tau)
    sos = signal.butter(order, hp, 'hp', fs=SR, output='sos'); x = signal.sosfilt(sos, x)
    if lp:
        sos = signal.butter(2, lp, 'lp', fs=SR, output='sos'); x = signal.sosfilt(sos, x)
    return x

def hat(open_=False):
    return noise_burst(0.25 if open_ else 0.06, hp=7000, tau=0.09 if open_ else 0.018) * 0.5

def clap():
    x = np.zeros(int(0.3 * SR))
    for k, g in [(0, 1), (0.011, .8), (0.023, .7)]:
        place(x, noise_burst(0.2, hp=900, lp=6000, tau=0.03), k, g)
    return x * 0.6

def rim(f=1800.0):
    n = int(0.08 * SR); t = np.arange(n) / SR
    return (np.sin(2 * np.pi * f * t) * np.exp(-t * 90) + noise_burst(0.08, hp=3000, tau=.006) * .5) * 0.6

def blip(f, dur=0.09, wave='sine'):
    n = int(dur * SR); t = np.arange(n) / SR
    w = np.sin(2 * np.pi * f * t) if wave == 'sine' else signal.square(2 * np.pi * f * t) * .4
    e = np.minimum(1, t / 0.002) * np.exp(-t * 35)
    return w * e

def tone(f, t0, t1, wave='sine', harmonics=None):
    """steady tone between t0,t1 with short fades."""
    i0, i1 = int(t0 * SR), min(N, int(t1 * SR)); n = i1 - i0; t = np.arange(n) / SR
    if harmonics:
        w = sum(np.sin(2 * np.pi * f * k * t) / k for k in harmonics)
    elif wave == 'saw':
        w = signal.sawtooth(2 * np.pi * f * t)
    else:
        w = np.sin(2 * np.pi * f * t)
    fade = int(0.01 * SR); e = np.ones(n); e[:fade] = np.linspace(0, 1, fade); e[-fade:] = np.linspace(1, 0, fade)
    out = np.zeros(N); out[i0:i1] = w * e
    return out

def lowpass_lfo(x, cutoff_fn, q=0.7, block=256):
    """time-varying 2-pole lowpass, coefficients updated per block (cutoff_fn(t) in Hz)."""
    y = np.zeros_like(x); zi = np.zeros(2)
    for i in range(0, N, block):
        fc = float(np.clip(cutoff_fn(i / SR), 60, 18000))
        b, a = signal.iirfilter(2, fc, btype='low', ftype='butter', fs=SR)
        y[i:i + block], zi = signal.lfilter(b, a, x[i:i + block], zi=zi)
    return y

def delay(x, time_s, fb=0.6, mixv=0.5, tone_lp=5000):
    d = int(time_s * SR); y = np.copy(x); buf = np.zeros_like(x)
    sos = signal.butter(1, tone_lp, 'lp', fs=SR, output='sos')
    # feedback loop in chunks of the delay length (exact for fb taps)
    acc = np.copy(x)
    for k in range(1, int(DUR / time_s) + 2):
        acc = np.roll(acc, d); acc[:d] = 0; acc = signal.sosfilt(sos, acc) * fb
        buf += acc
        if np.max(np.abs(acc)) < 1e-4:
            break
    return y + mixv * buf

def reverb(x, amt=0.18):
    out = np.copy(x)
    for d, g in [(0.0297, .75), (0.0371, .72), (0.0411, .69), (0.0437, .66)]:
        n = int(d * SR); y = np.zeros_like(x)
        # comb via lfilter: y[n] = x[n] + g*y[n-D]
        b = np.zeros(n + 1); b[0] = 1; a = np.zeros(n + 1); a[0] = 1; a[-1] = -g
        y = signal.lfilter(b, a, x); out += amt * y * .25
    return out

def sidechain(x, depth=0.6, tau=0.12):
    env = np.ones(N)
    for b in range(int(DUR / BEAT) + 1):
        i = int(b * BEAT * SR); n = int(0.35 * SR); seg = 1 - depth * np.exp(-np.arange(min(n, N - i)) / SR / tau)
        env[i:i + len(seg)] = np.minimum(env[i:i + len(seg)], seg)
    return x * env

def euclid(k, n, rot=0):
    return [1 if ((i + rot) * k) % n < k else 0 for i in range(n)]

def steps(fn, every=S16, t_from=0.0, t_to=DUR):
    """call fn(step_index, time) for every 16th in range"""
    i0 = int(math.ceil(t_from / every)); i1 = int(t_to / every)
    for i in range(i0, i1 + 1):
        fn(i, i * every)

def master(buf, gain_db=-1.0):
    x = np.tanh(buf * 0.65)
    x *= 10 ** (gain_db / 20) / (np.max(np.abs(x)) + 1e-9)
    fade = int(0.01 * SR); x[:fade] *= np.linspace(0, 1, fade); x[-fade:] *= np.linspace(1, 0, fade)
    st = np.stack([x, x], 1)
    return (st * 32767).astype(np.int16)

# ───────── episodes ─────────
def ep01():
    b = np.zeros(N); e5, e7 = euclid(5, 16), euclid(7, 16)
    def f(i, t):
        if i % 4 == 0: place(b, kick(49), t, .9)
        rot = int((t - 3 * PH) // BAR) % 16 if t >= 3 * PH else 0
        if euclid(5, 16, rot)[i % 16]: place(b, hat(), t, .55)
        if t >= 2 * PH and e7[i % 16]: place(b, clap(), t, .5)
    steps(f); return reverb(b, .12)

def ep02():
    b = np.zeros(N)
    def f(i, t):
        if i % 4 == 0: place(b, kick(49), t, .9)
        if i % 7 == 0: place(b, blip(880 if (i // 7) % 2 else 1174.7, .12), t, .45)
        if i % 16 in (4, 12): place(b, hat(), t, .3)
    steps(f); return reverb(b, .15)

def ep03():
    b = np.zeros(N)
    f2a, f2b = 55.3, 55 + 1 / (2 * BAR)
    sub = tone(55, 0, DUR) * .5 + tone(f2a, 0, 2 * PH) * .5 + tone(f2b, 2 * PH, DUR) * .5
    b += sub
    def f(i, t):
        if i % 4 == 0: place(b, kick(110, dur=.25, tau=.08, click=.3), t, .5)
        if i % 16 == 8: place(b, hat(True), t, .15)
    steps(f); return b

def ep04():
    b = np.zeros(N); r = np.zeros(N)
    def f(i, t):
        if i % 4 == 0: place(b, kick(49), t, .9)
        if i % 16 == 0: place(r, rim(1400), t, .9)
    steps(f)
    dry = r * (T < PH)
    d34 = delay(r * ((T >= PH) & (T < 2 * PH)), .75 * BEAT, .6, .8)
    dphi = delay(r * (T >= 2 * PH), BEAT / PHI, .6, .8)
    return reverb(b + dry + d34 + dphi, .2)

def ep05():
    b = np.zeros(N)
    pad_notes = [[110, 164.8, 196], [98, 146.8, 174.6], [87.3, 130.8, 164.8]]       # 3-bar cycle
    hat_pats = [euclid(k, 16) for k in (3, 5, 7, 9, 11)]                               # 5-bar cycle
    acid = [220, 0, 261.6, 0, 0, 196, 0, 0, 220, 0, 0, 0, 293.7, 0, 0, 0]              # per bar, 7 variations via transposition
    for bar in range(int(DUR / BAR) + 1):
        chord = pad_notes[bar % 3]
        for n_ in chord: b += tone(n_, bar * BAR, (bar + 1) * BAR, harmonics=[1, 2, 3]) * .07
    def f(i, t):
        bar, s = i // 16, i % 16
        if s % 4 == 0: place(b, kick(49), t, .85)
        if hat_pats[bar % 5][s]: place(b, hat(), t, .35)
        a = acid[s]
        if a: place(b, blip(a * 2 ** ((bar % 7) * 2 / 12), .1, 'square'), t, .35)
    steps(f); return reverb(sidechain(b, .35), .18)

def ep06():
    b = np.zeros(N)
    bass = lowpass_lfo(tone(55, PH, DUR, 'saw'), lambda t: 180) * .45
    def f(i, t):
        if i % 4 == 0: place(b, kick(48.999 if t < 2 * PH else 55.0, dur=.8, tau=.35), t, .9)
        if i % 16 in (4, 12): place(b, hat(), t, .25)
    steps(f); return b + sidechain(bass, .5)

def ep07():
    b = np.zeros(N)
    drone = tone(55, 0, DUR, 'saw') * .3 + tone(82.4, 0, DUR, 'saw') * .25 + tone(110, 0, DUR, 'saw') * .2
    fA = 3 / (8 * BAR); fB = 2 / (8 * BAR)
    def cutoff(t):
        if t < PH: lfo = np.sin(2 * np.pi * (1 / BAR) * t)
        elif t < 2 * PH: lfo = .5 * np.sin(2 * np.pi * fA * t) + .5 * np.sin(2 * np.pi * fB * t)
        elif t < 3 * PH: lfo = .5 * np.sin(2 * np.pi * fA * t) + .5 * np.sin(2 * np.pi * fB * 2 ** .5 * t)
        else: lfo = np.sin(2 * np.pi * (1 / (4 * BAR)) * t)
        return 300 * 2 ** (2.2 * lfo + 1.2)
    b += lowpass_lfo(drone, cutoff, block=128) * .6
    def f(i, t):
        if i % 4 == 0: place(b, kick(49), t, .8)
    steps(f); return sidechain(b, .4)

def lfsr(taps, n):
    s = [1, 0, 0, 1]; out = []
    for _ in range(n + 4):
        out.append(s[0]); fb = 0
        for e in taps: fb ^= s[4 - e]
        s = [s[1], s[2], s[3], fb]
    return out

def ep08():
    b = np.zeros(N); seqA, seqB = lfsr([4, 1], 480), lfsr([3, 1], 480)
    def f(i, t):
        if i % 4 == 0: place(b, kick(49), t, .9)
        seq = seqB if t >= 3 * PH else seqA
        if seq[i]: place(b, rim(2100 if i % 2 else 1700), t, .6)
        if i % 16 in (2, 6, 10, 14): place(b, hat(), t, .2)
    steps(f); return reverb(b, .12)

def swing_at(t):
    p = int(t // PH); tp = t - p * PH
    if p == 0: return .5 if tp < 3 else .54
    if p == 1: return .54 if tp < 2.5 else (.58 if tp < 5 else .62)
    return .54

def ep09():
    b = np.zeros(N)
    def f(i, t):
        if i % 4 == 0: place(b, kick(49), t, .9)
        s = swing_at(t); off = (s - .5) * BEAT / 2 if i % 2 else 0
        place(b, hat(open_=(i % 4 == 2)), t + off, .5 if i % 2 == 0 else .35)
    steps(f); return reverb(b, .1)

def ep10():
    b = np.zeros(N)
    # growing partial sums on A2, phase 0-1; impulse clicks + kick in phase 2-3
    for k in range(7):
        t0, t1 = k * PH / 7, (k + 1) * PH / 7
        b += tone(110, t0, t1, harmonics=[2 * j + 1 for j in range(k + 1)]) * .25
    b += tone(110, PH, 2 * PH, harmonics=[2 * j + 1 for j in range(15)]) * .22
    def f(i, t):
        if t >= 2 * PH and i % 4 == 0: place(b, kick(49), t, .9)
        if t >= 2 * PH and i % 2 == 0:
            n = int(0.004 * SR); place(b, rng.standard_normal(n) * np.exp(-np.arange(n) / SR / (0.001 if t < 3 * PH else 0.0005)), t, .9)
    steps(f); return b

def r_at(t):
    p = int(t // PH); tp = t - p * PH
    if p == 0: return 2.9 if tp < 2.5 else (3.2 if tp < 5 else 3.5)
    if p == 1: return 3.5 + (3.5699 - 3.5) * min(1, tp / 6)
    if p == 2: return 3.7 if tp < 4 else 4.0
    return 2.9 + (3.99 - 2.9) * (math.sin(tp * .8 - 1.5) + 1) / 2

def ep11():
    b = np.zeros(N); scale = [0, 2, 3, 5, 7, 9, 10, 12, 14, 15, 17, 19, 21, 22, 24, 26]  # dorian, 16 degrees
    x = 0.3
    def f(i, t):
        nonlocal x
        if i % 4 == 0: place(b, kick(49), t, .85)
        r = r_at(t); x = r * x * (1 - x)
        deg = scale[min(15, int(x * 16))]; place(b, blip(110 * 2 ** (deg / 12), .11, 'square'), t, .4)
        if i % 16 in (4, 12): place(b, hat(), t, .25)
    steps(f); return reverb(sidechain(b, .3), .2)

def ep12():
    b = np.zeros(N); Tt = 7.5
    def g(l):
        u = l / 2
        return 0 if u <= 0 or u >= 1 else .5 - .5 * math.cos(2 * math.pi * u)
    for k in range(-1, 3):
        # layer k: tempo 130·2^(t/T + k); schedule hits by integrating the instantaneous rate
        t = 0.0
        while t < DUR:
            l = (t / Tt) % 1 + k
            rate = 130 / 60 * 2 ** l                       # beats per second
            gain = g(l)
            if gain > .02:
                place(b, kick(49 * 2 ** (k * 0.0), dur=.4, tau=.12), t, .8 * gain)
                if gain > .3: place(b, hat(), t + 0.5 / rate, .25 * gain)
            t += 1 / rate
    return reverb(b, .15)

EPISODES = [ep01, ep02, ep03, ep04, ep05, ep06, ep07, ep08, ep09, ep10, ep11, ep12]

if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    wanted = [int(a) for a in sys.argv[1:]] or list(range(1, 13))
    for n in wanted:
        buf = EPISODES[n - 1]()
        path = os.path.join(OUT, f'EP{n:02d}.wav')
        wavfile.write(path, SR, master(buf))
        print(path, f'{np.max(np.abs(buf)):.2f} peak pre-master')

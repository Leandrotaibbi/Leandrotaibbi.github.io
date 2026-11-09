# MATH × TECHNO — 12 Reels · 30 s · 1080×1920 · con audio

Formato "viewport": scena 3D vera (proiezione prospettica, facce ombreggiate ordinate per profondità,
wireframe, griglia a terra, nebbia) incorniciata come una finestra di Blender — overlay "User Perspective",
gizmo degli assi, timeline a 900 frame, status bar con conteggio di vertici/facce. Sotto: due pannelli
(PARAMETERS / DERIVED) con i numeri che si aggiornano in tempo reale, il blocco formule in serif, e una
frase per fase (4 × 7.5 s) con la parola chiave nel colore d'accento.

Tipografia: STIX Two (serif da paper, per formule e frasi) · Share Tech Mono (pannelli dati) · VT323 (titoli)
· DejaVu Sans (overlay del viewport, lo stesso di Blender). Ogni episodio ha la sua palette; nessuna è
condivisa. Tre episodi (04, 07, 12) sono in registro chiaro.

Audio: sintetizzato in `synth.py` (numpy/scipy) a 130 BPM, 48 kHz. Ogni loop dimostra la matematica
dell'episodio (vedi riga "Audio" sotto). È un bozzetto: sostituiscilo con il tuo materiale quando vuoi,
il video è muto fino al mux (`ffmpeg -i video -i out/audio/EPnn.wav`).

Tutti i numeri a schermo sono calcolati a 130 BPM: beat 461.54 ms · bar 1.846 s · 16th 115.38 ms · 8th 230.77 ms.

---

## EP01 · EUCLIDEAN RHYTHMS — `EP01-euclid.mp4`

**Tag a schermo:** Bjorklund · Christoffel words · Sturmian sequences · rotations in ℤ/16ℤ  
**Oggetto Blender:** `EP01_euclid_ring`  
**Audio:** kick 4/4 · hat E(5,16) · clap E(7,16) dalla terza frase · rotazione di uno step per battuta nell'ultima

**Formule a schermo:**
- E(k,n)ᵢ = ⌊(i+1)k/n⌋ − ⌊ik/n⌋
- 5/16 = 1/(3 + 1/5)   ·   7/16 = 1/(2 + 1/(3 + 1/2))

**Le 4 frasi (7.5 s ciascuna):**
1. E(k,n) is the Christoffel word of slope k/n: the lattice path that stays closest to the line y = kx/n. Bjorklund (2003) rediscovered Bresenham (1965).
2. 5/16 = [0; 3, 5]. The inter-onset intervals are 3 and 4, appearing 3 and 1 times: a Sturmian word has two gap lengths, never more.
3. 7/16 = [0; 2, 3, 2]. Layered on E(5,16) they coincide on 3 of 16 steps — gcd(5,7) = 1 guarantees the two polygons never align.
4. Rotation is adding a unit in ℤ/16ℤ: same necklace, a new bracelet. Zero new notes, a different groove every bar.

**VO (it, ~25 s):** Le tue hi-hat sono una parola di Christoffel: il cammino sul reticolo che sta più vicino alla retta y uguale k x su n. Bjorklund nel 2003 ha riscoperto Bresenham del 1965. Cinque sedicesimi è zero, tre, cinque in frazione continua: gli intervalli sono solo 3 e 4, una parola sturmiana ha due distanze e basta. Sovrapponi E(7,16): coincidono su tre step, perché il massimo comun divisore di 5 e 7 è uno. Ruota di uno step: stessa collana, braccialetto nuovo, zero note nuove.

**Caption:**
Your hi-hats are a Christoffel word. E(k,n) is the lattice path closest to y = kx/n — Bjorklund (2003) rediscovered Bresenham (1965). 5/16 = [0;3,5]: inter-onset intervals 3 and 4 only (Sturmian: two gaps, never more). Layer E(7,16): they coincide on 3 of 16 steps because gcd(5,7) = 1. Rotation = adding a unit in Z/16Z — same necklace, new bracelet, zero new notes.
Max for Live: E(k,n)[i] = floor((i+1)k/n) − floor(ik/n).
EP.01 / 12
#euclideanrhythm #bjorklund #christoffelword #maxforlive #hypnotictechno #musicmath #sturmian #modularsynth #technoproducer #blender3d

---

## EP02 · CHINESE REMAINDER THEOREM — `EP02-crt-torus.mp4`

**Tag a schermo:** ℤ/112ℤ ≅ ℤ/16ℤ × ℤ/7ℤ · torus orbits · coprime loop lengths  
**Oggetto Blender:** `EP02_Z16xZ7_torus`  
**Audio:** kick 4/4 · bleep a ciclo di 7 sedicesimi (CRT) · hat sul 2 e 4

**Formule a schermo:**
- ℤ/112ℤ ≅ ℤ/16ℤ × ℤ/7ℤ   (gcd = 1)
- i ≡ a (mod 16), i ≡ b (mod 7)  ⇒  i = 49a + 64b (mod 112)

**Le 4 frasi (7.5 s ciascuna):**
1. Every step is a point on a torus: (i mod 16, i mod 7). Because gcd(16,7) = 1 the orbit visits all 112 cells exactly once before closing.
2. Chinese remainder theorem: ℤ/112ℤ ≅ ℤ/16ℤ × ℤ/7ℤ. From the pair (a, b) back to the step: i = 49a + 64b (mod 112).
3. 16 ≡ 2 (mod 7): each bar the 7-step bleep slides two steps against the kick. The drift is the music; seven bars close the loop.
4. Replace 7 by 8: gcd = 8 and the torus collapses to 16 cells — the pattern repeats every bar. Coprimality is the whole trick.

**VO (it, ~25 s):** Ogni step è un punto su un toro: i modulo sedici, i modulo sette. Siccome sedici e sette sono coprimi l'orbita visita tutte le centododici celle una volta sola. Teorema cinese del resto: da una coppia di coordinate si torna allo step con i uguale quarantanove a più sessantaquattro b modulo centododici. Sedici è congruo a due modulo sette: ogni battuta il bleep scivola di due step contro la cassa. Metti otto al posto di sette e il toro collassa a sedici celle. La coprimalità è tutto il trucco.

**Caption:**
Every step of a 16-against-7 polymeter is a point on a torus (i mod 16, i mod 7). gcd = 1 → the orbit visits all 112 cells once. Chinese remainder theorem: Z/112Z ≅ Z/16Z × Z/7Z, i = 49a + 64b (mod 112). 16 ≡ 2 (mod 7): the bleep slides two steps per bar. Replace 7 with 8 and the torus collapses to 16 cells.
Ableton: one clip in 7/16, everything else 4/4.
EP.02 / 12
#polymeter #chineseremaindertheorem #numbertheory #hypnotictechno #ableton #musicmath #torus #technoproducer #generativemusic #blender3d

---

## EP03 · BEATING & DETUNE — `EP03-beating.mp4`

**Tag a schermo:** sum-to-product identity · envelope at Δf · cents = 1200·log₂ · grid-locked AM  
**Oggetto Blender:** `EP03_phasor_pair`  
**Audio:** sub 55 Hz + 55.3 Hz, poi 55.2708 Hz dalla terza frase (battimento ogni 2 battute) · kick corto sul beat

**Formule a schermo:**
- sin a + sin b = 2·sin((a + b)/2)·cos((a − b)/2)
- Δf = BPM / (60 · beats per cycle)   ·   ¢ = 1200·log₂(f₂/f₁)

**Le 4 frasi (7.5 s ciascuna):**
1. sin a + sin b = 2 sin((a+b)/2)·cos((a−b)/2): a carrier at the mean frequency with an envelope at half the difference.
2. |cos| peaks twice per period, so the audible beat is at Δf, not Δf/2. 55.000 vs 55.300 Hz → 0.300 Hz, every 3.333 s. Unrelated to 130 BPM.
3. Choose Δf = 1/(2 bars) = 130/(60·8) = 0.2708 Hz. In pitch: 1200·log₂(55.2708/55) = 8.5 cents — below the JND for pitch, far above it for rhythm.
4. Two free oscillators, zero modulators: amplitude modulation phase-locked to the grid by arithmetic. No LFO, no sidechain, one number.

**VO (it, ~25 s):** Seno a più seno b uguale due seno della semisomma per coseno della semidifferenza: una portante alla frequenza media e un inviluppo a metà della differenza. Il valore assoluto del coseno ha due picchi per periodo, quindi il battimento che senti è a delta f, non delta f mezzi. Cinquantacinque contro cinquantacinque virgola tre: uno ogni tre secondi e trentatré, scollegato dai centotrenta. Scegli delta f uguale uno su due battute: zero virgola duemilasettecentootto hertz, otto cents e mezzo. Due oscillatori liberi, nessun LFO, nessun sidechain, un numero.

**Caption:**
sin a + sin b = 2·sin((a+b)/2)·cos((a−b)/2): a carrier at the mean frequency, an envelope at half the difference — |cos| peaks twice per period so the beat you hear is at Δf. 55 vs 55.3 Hz → every 3.33 s, unrelated to the grid. Choose Δf = 1/(2 bars) = 130/(60·8) = 0.2708 Hz = +8.5 cents on A1: amplitude modulation phase-locked to the bar from two free oscillators. No LFO, no sidechain.
EP.03 / 12
#detune #beating #psychoacoustics #subbass #hypnotictechno #sounddesign #musicmath #synthesis #technoproducer #blender3d

---

## EP04 · THE GOLDEN DELAY — `EP04-phi-delay.mp4`

**Tag a schermo:** continued fractions · Hurwitz bound · three-gap theorem · Weyl equidistribution  
**Oggetto Blender:** `EP04_golden_cone`  
**Audio:** kick 4/4 · rimshot sull'1 con delay: dry → 3/4 di beat (346 ms, feedback 0.6) → 1/φ (285.3 ms)

**Formule a schermo:**
- φ = 1 + 1/(1 + 1/(1 + 1/(1 + ⋯)))   ·   |φ − p/q| > 1/(√5 q²)
- tₙ = n·Tbeat/φ   ⇒   {n/φ} equidistributed on [0,1)

**Le 4 frasi (7.5 s ciascuna):**
1. φ = [1; 1, 1, 1, …] has the slowest-converging continued fraction of any real. Hurwitz: |φ − p/q| > 1/(√5·q²) for every fraction — the most irrational number.
2. A 3/4-beat delay is q = 4: after four repeats the tap is back on the grid. The tail is a comb of coincidences, not a cloud.
3. Taps at n/φ (mod 1) on the beat-circle: by the Steinhaus three-gap theorem n points make at most three distinct gaps, and never coincide.
4. 285.3 ms at 130 BPM. Repeats at 0.618, 1.236, 1.854, 2.472… beats — equidistributed (Weyl), never on a 16th. Feedback 60 %.

**VO (it, ~25 s):** Phi è uno, uno, uno, uno in frazione continua: converge più lentamente di qualsiasi altro numero reale. Hurwitz: la distanza da ogni frazione p su q è maggiore di uno su radice di cinque q quadro. Un delay a tre quarti di beat ha q uguale quattro: dopo quattro ripetizioni sei di nuovo sulla griglia, un pettine di coincidenze. Un delay a uno su phi: per il teorema dei tre gap di Steinhaus n colpi fanno al massimo tre distanze diverse, e non coincidono mai. Duecentottantacinque millisecondi a centotrenta. Feedback sessanta per cento.

**Caption:**
The most irrational delay time. φ = [1;1,1,1,…] converges slower than any other real; Hurwitz: |φ − p/q| > 1/(√5·q²). A 3/4-beat delay (q = 4) is back on the grid after 4 repeats — a comb of coincidences. Taps at n/φ on the beat-circle obey the Steinhaus three-gap theorem: at most 3 distinct gaps, never a coincidence, equidistributed (Weyl). 285.3 ms @130, feedback 60 %.
EP.04 / 12
#goldenratio #continuedfractions #threegaptheorem #delay #dubtechno #hypnotictechno #musicmath #sounddesign #technoproducer #blender3d

---

## EP05 · PRIME LOOP LENGTHS — `EP05-primes.mp4`

**Tag a schermo:** lcm · conjunctions · P(coprime) = 1/ζ(2) = 6/π² · state space  
**Oggetto Blender:** `EP05_orrery_3_5_7`  
**Audio:** pad a ciclo di 3 battute · hat a ciclo di 5 · acid a ciclo di 7 · kick 4/4

**Formule a schermo:**
- lcm(a,b,c) = abc  ⇔  pairwise coprime
- P(gcd(a,b) = 1) = 1/ζ(2) = 6/π² ≈ 0.6079

**Le 4 frasi (7.5 s ciascuna):**
1. Three loops of 3, 5 and 7 bars are three planets: a conjunction of all three every lcm = 105 bars = 3:14 at 130 BPM. Kepler for the dancefloor.
2. Powers of two are the worst case: lcm(4, 8, 16) = 16. The structure repeats every 29.5 s and everyone in the room counts it without knowing.
3. Two random lengths are coprime with probability 1/ζ(2) = 6/π² ≈ 60.8 %. Three primes make it a certainty — and lcm = the product.
4. 5·7·11 = 385 bars = 11:50, longer than the track itself. Minimum material, maximum state space: 385 distinct bars from three clips.

**VO (it, ~25 s):** Tre loop da tre, cinque e sette battute sono tre pianeti: congiunzione di tutti e tre ogni centocinque battute, tre minuti e quattordici. Le potenze di due sono il caso peggiore: minimo comune multiplo di quattro, otto, sedici è sedici, la struttura si ripete ogni ventinove secondi e mezzo e tutta la sala la conta senza saperlo. Due lunghezze a caso sono coprime con probabilità sei su pi greco quadro, sessanta virgola otto per cento. Tre primi lo rendono certo. Cinque per sette per undici: trecentottantacinque battute, più lunghe della traccia.

**Caption:**
Three loops of 3, 5, 7 bars = three planets: a full conjunction every lcm = 105 bars = 3:14 @130. Powers of two are the worst case: lcm(4,8,16) = 16, the structure repeats every 29.5 s. Two random lengths are coprime with probability 1/ζ(2) = 6/π² ≈ 60.8 %; three primes make it certain and lcm = the product. 5·7·11 = 385 bars = 11:50 — longer than the track.
EP.05 / 12
#primenumbers #lcm #zeta #hypnotictechno #ableton #generativemusic #musicmath #minimaltechno #technoproducer #blender3d

---

## EP06 · KICK TUNING — `EP06-kick-hz.mp4`

**Tag a schermo:** 12-TET · beating in the sub · 3:2 = 701.955 ¢ · FFT bin resolution  
**Oggetto Blender:** `EP06_kick_waterfall`  
**Audio:** kick a 48.999 Hz (G1) per 15 s, poi a 55 Hz (A1) · basso saw A1 dalla seconda frase · battimento a 6 Hz udibile nella prima metà

**Formule a schermo:**
- f(n) = 440 · 2^(n/12)   ·   n = 12·log₂(f/440)
- 3:2 = 1200·log₂(1.5) = 701.955 ¢   ·   Δfbin = fs / N

**Le 4 frasi (7.5 s ciascuna):**
1. 12-TET: f(n) = 440·2^(n/12). The kick's pitch sweep decays onto a steady body — a frequency, therefore a note. Here: 49.0 Hz = G1.
2. G1 = 48.999 Hz under a bass at A1 = 55.000 Hz: Δ = 6.0 Hz. In the sub range a whole tone is not an interval, it is a beating frequency.
3. Tune the body to the root, or a fifth below: D1 = 36.708 Hz. 3:2 = 701.955 ¢, only 1.955 ¢ off 12-TET: harmonics 3 and 2 meet at 110.12 vs 110.00 Hz.
4. One semitone at 49 Hz is 2.9 Hz — below one bin of a 4096-point FFT at 48 kHz (11.72 Hz/bin). The analyser cannot resolve it. The sub can.

**VO (it, ~25 s):** Temperamento equabile: f di n uguale quattrocentoquaranta per due alla n dodicesimi. Lo sweep della cassa decade su un corpo stabile: una frequenza, quindi una nota. Qui quarantanove hertz, Sol uno. Sotto un basso in La uno a cinquantacinque hertz sono sei hertz di differenza: nel sub un tono intero non è un intervallo, è un battimento. Accorda alla tonica o una quinta sotto, Re uno a trentasei virgola sette: tre a due sono settecentouno cents, le armoniche tre e due si incontrano a centodieci. Un semitono a quarantanove hertz sono due virgola nove hertz, meno di un bin di una FFT a quattromilanovantasei punti. L'analizzatore non lo vede. Il sub sì.

**Caption:**
12-TET: f(n) = 440·2^(n/12). The kick body decays onto a frequency, therefore a note: 49.0 Hz = G1. Under a bass at A1 = 55 Hz that is Δ = 6.0 Hz — in the sub a whole tone is a beating frequency, not an interval. Tune to the root or a fifth below (D1 = 36.708 Hz; 3:2 = 701.955 ¢). One semitone at 49 Hz = 2.9 Hz, below one bin of a 4096-point FFT at 48 kHz (11.72 Hz). The analyser cannot resolve it. The sub can.
EP.06 / 12
#kickdrum #subbass #equaltemperament #fft #mixingtips #hypnotictechno #sounddesign #musicmath #technoproducer #blender3d

---

## EP07 · TEMPO-LOCKED LFOs — `EP07-lfo-knot.mp4`

**Tag a schermo:** f = BPM/(60·β) · (p,q) torus knots · Kronecker–Weyl density  
**Oggetto Blender:** `EP07_trefoil_LFO`  
**Audio:** drone saw con filtro: LFO a 1 battuta → due LFO 3:2 su 8 battute (trefoil) → rapporto √2:1 → 4 battute · kick 4/4

**Formule a schermo:**
- fLFO = BPM / (60 · β)   ·   (p,q) knot: θ = p·2πt/T, φ = q·2πt/T
- gcd(p,q) = 1 ⇒ one closed curve · p/q ∉ ℚ ⇒ dense on T²

**Le 4 frasi (7.5 s ciascuna):**
1. f = BPM/(60·β). At 130: 1 bar → 0.5417 Hz, 4 bars → 0.1354, 8 bars → 0.0677, 16 bars → 0.0339 Hz. A free LFO at 0.1 Hz is none of these.
2. Two LFOs with rational ratio p:q draw a (p,q) torus knot. 3 cycles against 2 per 8 bars: a trefoil, closing every 14.77 s.
3. An irrational ratio never closes: the curve is dense on the torus — Kronecker–Weyl. That is the mathematically correct meaning of "random".
4. Hardware without a sync input: type the hertz. 0.1354 on the Buchla is 4 bars at 130, to the sample. Tempo sync is a division.

**VO (it, ~25 s):** f uguale BPM diviso sessanta beta. A centotrenta: una battuta zero virgola cinquequattroununsette hertz, quattro battute zero virgola unotrecinquequattro, sedici battute zero virgola zerotretrenove. Due LFO con rapporto razionale p su q disegnano un nodo torico: tre cicli contro due in otto battute è un trifoglio, che si richiude ogni quattordici secondi e settantasette. Un rapporto irrazionale non si chiude mai: la curva è densa sul toro, Kronecker-Weyl, il vero significato di random. Hardware senza sync: digita gli hertz. Zero virgola unotrecinquequattro sul Buchla sono quattro battute a centotrenta, al campione.

**Caption:**
f = BPM/(60·β): at 130, 1 bar = 0.5417 Hz, 4 bars = 0.1354, 8 bars = 0.0677, 16 bars = 0.0339 Hz. Two LFOs with a rational ratio p:q trace a (p,q) torus knot — 3 against 2 per 8 bars is a trefoil closing every 14.77 s. An irrational ratio never closes (Kronecker–Weyl: dense on the torus) — the mathematically correct "random". Hardware with no sync input: type the hertz.
EP.07 / 12
#lfo #torusknot #buchla #modularsynth #eurorack #hypnotictechno #musicmath #sounddesign #technoproducer #blender3d

---

## EP08 · LFSR SEQUENCER — `EP08-lfsr.mp4`

**Tag a schermo:** GF(2) · primitive polynomials · period 2ᵐ − 1 · Turing Machine module  
**Oggetto Blender:** `EP08_LFSR_x4_x3_1`  
**Audio:** kick 4/4 · rim sui bit di un LFSR a 4 bit x⁴+x³+1 (periodo 15) · nell'ultima frase x⁴+x²+1 (periodo 6)

**Formule a schermo:**
- sₙ₊₄ = sₙ₊₃ ⊕ sₙ   ⇔   x⁴ + x³ + 1 over GF(2)
- primitive ⇒ period 2^m − 1  ·  (x²+x+1)² ⇒ period 6

**Le 4 frasi (7.5 s ciascuna):**
1. A 4-bit shift register with feedback s[n+4] = s[n+3] ⊕ s[n]: the "Turing Machine" module, written in GF(2).
2. x⁴ + x³ + 1 is primitive over GF(2): the sequence has maximal period 2⁴ − 1 = 15, visiting every non-zero state exactly once.
3. 15 against a 16-step bar: the pattern slides one step per bar and closes after lcm(15,16) = 240 steps = 15 bars. Polyrhythm from a polynomial.
4. Flip one tap to x⁴ + x² + 1 = (x² + x + 1)²: not primitive, the period collapses to 6. The polynomial is the groove.

**VO (it, ~25 s):** Un registro a scorrimento a quattro bit con retroazione s di n più quattro uguale s di n più tre xor s di n: il modulo Turing Machine, scritto in GF due. x alla quarta più x al cubo più uno è primitivo: periodo massimo due alla quattro meno uno, quindici, ogni stato non nullo una volta sola. Quindici contro una battuta da sedici: il pattern scivola di uno step a battuta e si richiude dopo quindici battute. Cambia un tap, x alla quarta più x quadro più uno: non è primitivo, il periodo crolla a sei. Il polinomio è il groove.

**Caption:**
A 4-bit shift register with feedback s[n+4] = s[n+3] ⊕ s[n] — the Turing Machine module, written in GF(2). x⁴ + x³ + 1 is primitive: maximal period 2⁴ − 1 = 15, every non-zero state once. 15 against a 16-step bar slides one step per bar and closes after lcm(15,16) = 240 steps. Flip one tap to x⁴ + x² + 1 = (x² + x + 1)²: not primitive, period 6. The polynomial is the groove.
EP.08 / 12
#lfsr #galoisfield #turingmachine #modularsynth #eurorack #hypnotictechno #musicmath #generativemusic #technoproducer #blender3d

---

## EP09 · SWING IN MILLISECONDS — `EP09-swing-ms.mp4`

**Tag a schermo:** Δt = (s − ½)·T₈ · Weber–Fechner · JND · lane asynchrony  
**Oggetto Blender:** `EP09_groove_lanes`  
**Audio:** kick dritta 4/4 · hat a sedicesimi con swing 50 → 54 → 58 → 62 → 54 %

**Formule a schermo:**
- Δt = (s − ½)·T₈   ·   T₈ = 30/BPM s
- Weber: ΔT/T ≈ const ≈ 0.025   ⇒   JND₁₆ ≈ 5.8 ms @130

**Le 4 frasi (7.5 s ciascuna):**
1. Δt = (s − ½)·T₈. At 130 BPM T₈ = 230.77 ms: 54 % → +9.23 ms, 58 % → +18.46 ms, 62 % → +27.69 ms. MPC swing, decoded.
2. Weber–Fechner: the just-noticeable timing difference is ≈ 2.5 % of the interval, ≈ 5.8 ms on a 16th. 54 % is above threshold and below awareness.
3. Swing in % is tempo-relative; in ms it is not. 54 % at 140 BPM is 8.57 ms — the same setting is 7 % tighter. Think in milliseconds.
4. Kick at 50 %, hats at 54–56 %: the asynchrony between lanes is the groove, not the swing value. One lane swung alone is just late.

**VO (it, ~25 s):** Delta t uguale s meno un mezzo per T otto. A centotrenta l'ottavo dura duecentotrenta virgola settantasette millisecondi: cinquantaquattro per cento sono più nove virgola ventitré, cinquantotto più diciotto e quarantasei, sessantadue più ventisette e sessantanove. Weber-Fechner: la differenza di timing appena percepibile è circa il due e mezzo per cento dell'intervallo, cinque virgola otto millisecondi su un sedicesimo. Il cinquantaquattro sta sopra la soglia e sotto la coscienza. Lo swing in percentuale è relativo al tempo, in millisecondi no: a centoquaranta lo stesso cinquantaquattro è il sette per cento più stretto. Cassa a cinquanta, hat a cinquantaquattro: l'asincronia tra le corsie è il groove.

**Caption:**
Δt = (s − ½)·T₈. At 130 BPM T₈ = 230.77 ms: 54 % → +9.23 ms, 58 % → +18.46, 62 % → +27.69. Weber–Fechner: the JND for timing is ≈ 2.5 % of the interval ≈ 5.8 ms on a 16th — 54 % is above threshold, below awareness. Swing in % is tempo-relative, in ms it is not: 54 % at 140 BPM is 8.57 ms, 7 % tighter. Kick at 50, hats at 54–56: the asynchrony between lanes is the groove.
EP.09 / 12
#swing #groove #weberfechner #mpc #drumprogramming #hypnotictechno #musicmath #ableton #technoproducer #blender3d

---

## EP10 · FOURIER & THE CLICK — `EP10-fourier.mp4`

**Tag a schermo:** partial sums · Gibbs 8.949 % · δ(t) · Gabor uncertainty  
**Oggetto Blender:** `EP10_partial_sums`  
**Audio:** quadra su La2 costruita per somme parziali (1 → 15 armoniche) · dalla terza frase click a larga banda + kick

**Formule a schermo:**
- sq(x) = (4/π)·Σₖ sin((2k+1)x)/(2k+1)
- overshoot = (2/π)·Si(π) − 1 = 0.08949   ·   σt·σf ≥ 1/(4π)

**Le 4 frasi (7.5 s ciascuna):**
1. Square wave = 4/π·Σ sin((2k+1)x)/(2k+1): odd harmonics with amplitude 1/n. Each ribbon behind adds one more term.
2. Near the jump every partial sum overshoots by (2/π)·Si(π) − 1 = 8.949 %, for any finite N. Gibbs (1899): more harmonics narrow it, never remove it.
3. Limit case δ(t): flat spectrum, zero phase. Gabor: Δt·Δf ≥ 1/(4π). A click of σ = 1 ms spans σf ≥ 79.6 Hz — the only sound that hits every band.
4. Punch is bandwidth, not level: halve the transient length and the spectrum doubles in width. Don't EQ the click — shorten it.

**VO (it, ~25 s):** Un'onda quadra è quattro su pi greco per la somma dei seni delle armoniche dispari su n. Vicino al salto ogni somma parziale sbaglia per eccesso di due su pi greco per seno integrale di pi greco meno uno: otto virgola novecentoquarantanove per cento, per qualsiasi N finito. Gibbs, 1899: più armoniche lo restringono, non lo tolgono. Il caso limite è la delta: spettro piatto, fase zero. Gabor: sigma t per sigma f maggiore o uguale a uno su quattro pi greco. Un click di un millisecondo copre almeno ottanta hertz di larghezza. Il punch è banda, non livello: dimezza il transiente e lo spettro raddoppia. Non equalizzarlo, accorcialo.

**Caption:**
Square wave = (4/π)·Σ sin((2k+1)x)/(2k+1). Near the jump every partial sum overshoots by (2/π)·Si(π) − 1 = 8.949 % for any finite N — Gibbs (1899). Limit case δ(t): flat spectrum, zero phase. Gabor: σt·σf ≥ 1/(4π), so a 1 ms click spans ≥ 79.6 Hz — the only sound that hits every band. Punch is bandwidth, not level: halve the transient and the spectrum doubles. Shorten the click, don't EQ it.
EP.10 / 12
#fourier #gibbsphenomenon #dsp #transient #kickdrum #hypnotictechno #sounddesign #musicmath #technoproducer #blender3d

---

## EP11 · THE LOGISTIC MAP — `EP11-logistic.mp4`

**Tag a schermo:** period doubling · Feigenbaum δ, α · Lyapunov exponent · gen~  
**Oggetto Blender:** `EP11_logistic_cloud`  
**Audio:** sequencer a 16 note (dorico) pilotato dalla mappa logistica: r 2.9 → 3.2 → 3.5 → 3.5699 → 3.7 → 4 → sweep · kick 4/4

**Formule a schermo:**
- xₙ₊₁ = r xₙ(1 − xₙ)   ·   λ = lim (1/N)·Σ ln|r(1 − 2xₙ)|
- δ = lim (rₙ − rₙ₋₁)/(rₙ₊₁ − rₙ) = 4.6692…   ·   λ(r=4) = ln 2

**Le 4 frasi (7.5 s ciascuna):**
1. xₙ₊₁ = r·xₙ(1 − xₙ). Below r = 3 one fixed point. Then period 2, 4, 8 … at r = 3, 3.4495, 3.5441, 3.5644 — doubling faster and faster.
2. The bifurcation intervals shrink by δ = 4.6692… — Feigenbaum's constant, universal for every unimodal map. Accumulation at r∞ = 3.56995: chaos.
3. Lyapunov exponent λ = ⟨ln|r(1 − 2x)|⟩ > 0 is the definition of chaos. At r = 4, λ = ln 2: exactly one bit of unpredictability per step.
4. Quantise x to 16 notes and map r to a fader (2.9 → 3.99): one parameter sweeps from a loop to a melody to noise. gen~: History x; x = rx(1-x).

**VO (it, ~25 s):** x di n più uno uguale r x di n per uno meno x di n. Sotto r uguale tre un punto fisso; poi periodo due, quattro, otto a tre, tre virgola quattrocentoquarantanove, tre virgola cinquecentoquarantaquattro. Gli intervalli si restringono di delta uguale quattro virgola sei sei nove due, la costante di Feigenbaum, universale per ogni mappa unimodale. Accumulazione a tre virgola cinque sei nove nove cinque: caos. L'esponente di Lyapunov positivo è la definizione di caos; a r uguale quattro vale logaritmo di due, un bit di imprevedibilità per step. Quantizza x su sedici note e metti r su un fader: un parametro, da loop a melodia a rumore.

**Caption:**
x[n+1] = r·x(1−x). Below r = 3 a fixed point; then period 2, 4, 8 at r = 3, 3.4495, 3.5441 — intervals shrinking by Feigenbaum's δ = 4.6692…, universal for every unimodal map. Accumulation at r∞ = 3.56995: chaos. Lyapunov λ = ⟨ln|r(1−2x)|⟩ > 0 is the definition; at r = 4, λ = ln 2 = one bit of unpredictability per step. Quantise x to 16 notes, map r to a fader: loop → melody → noise.
gen~: History x; x = r*x*(1-x); out = floor(x*16).
EP.11 / 12
#logisticmap #chaostheory #feigenbaum #lyapunov #maxforlive #gen #generativemusic #hypnotictechno #musicmath #blender3d

---

## EP12 · RISSET RHYTHM — `EP12-risset.mp4`

**Tag a schermo:** τ₀·2^(t/T) · Shepard helix · Hann gains · constant mean log-tempo  
**Oggetto Blender:** `EP12_shepard_helix`  
**Audio:** tre layer di kick a ×1 ×2 ×4 con guadagni Hann su log-tempo, T = 7.5 s: accelerazione infinita (ritmo di Risset)

**Formule a schermo:**
- τₖ(t) = τ₀·2^(t/T + k)   ·   gₖ(t) = ½[1 − cos(2π(t/T + k)/3)]
- Σₖ gₖ = const   ·   state(t + T) = state(t) · shift(k)

**Le 4 frasi (7.5 s ciascuna):**
1. τ(t) = τ₀·2^(t/T): tempo rising exponentially, three layers an octave apart in tempo, gains on a raised cosine over log τ.
2. After T the tempo has doubled, but layer k+1 now carries exactly the amplitude layer k had — at τ₀. The state is periodic, the percept is not.
3. Shepard (1964) for pitch, Risset (1971) for rhythm: a circular dimension built from a log scale and a bell-shaped envelope. The helix is the geometry of the illusion.
4. T = 8 bars = 14.77 s. Three clips at ×1, ×2, ×4 with Hann gains offset by T/3. The mean log-tempo is constant: the floor never lands.

**VO (it, ~25 s):** Tau di t uguale tau zero per due alla t su T: il tempo sale esponenzialmente. Tre layer a distanza di un'ottava di tempo, guadagni a coseno rialzato sul logaritmo del tempo. Dopo T il tempo è raddoppiato, ma il layer successivo ha esattamente l'ampiezza che aveva il precedente, al tempo di partenza. Lo stato è periodico, la percezione no. Shepard nel sessantaquattro per l'altezza, Risset nel settantuno per il ritmo: una dimensione circolare costruita da una scala logaritmica e un inviluppo a campana. T uguale otto battute. Tre clip a uno, due, quattro con finestre di Hann sfalsate di T su tre. Il pavimento non arriva mai.

**Caption:**
τ(t) = τ₀·2^(t/T): tempo rising exponentially, three layers an octave apart in tempo, gains on a raised cosine over log τ. After T the tempo has doubled but layer k+1 carries exactly the amplitude layer k had — at τ₀. The state is periodic, the percept is not. Shepard (1964) for pitch, Risset (1971) for rhythm. T = 8 bars = 14.77 s, three clips at ×1 ×2 ×4, Hann gains offset by T/3. The floor never lands.
EP.12 / 12
#rissetrhythm #shepardtone #auditoryillusion #psychoacoustics #hypnotictechno #ableton #musicmath #sounddesign #technoproducer #blender3d

---

## Note di pubblicazione

- **Cover.** Il frame a t ≈ 3 s. `node render.js --stills` salva 4 fotogrammi chiave per episodio in `out/stills/`.
- **Ordine.** 01 → 12, 2–3 a settimana. Le frasi a schermo sono dense di proposito: il reel si riguarda.
- **Audio.** `python3 synth.py` rigenera i WAV; `node render.js` renderizza i video muti; il mux è in `render.js` (`--mux`) o a mano con ffmpeg.
- **Track/label.** Il campo `obj` nell'overlay del viewport è il nome del file .blend; se il reel gira sopra un tuo pezzo edito, metti il titolo lì (es. `Stelle_EP_kick`).
- **Commenti.** I numeri sono verificabili e citati (Bjorklund 2003, Gibbs 1899, Shepard 1964, Risset 1971, Hurwitz, Steinhaus, Feigenbaum). Chi li verifica è il pubblico giusto.
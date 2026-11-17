#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace polvere;

//==============================================================================
namespace
{
    const juce::StringArray scaleNames  { "Free", "Chromatic", "Octave", "Oct + 5th", "Minor", "Major",
                                          "Phrygian", "Dorian", "Pentatonic", "Hijaz" };
    const juce::StringArray windowNames { "Hann", "Gauss", "Tukey", "Triangle", "Expodec", "Rexpodec", "Random" };
    const juce::StringArray filterNames { "LP", "BP", "HP" };

    const std::vector<std::vector<int>> scaleDegrees {
        {}, {}, { 0 }, { 0, 7 },
        { 0, 2, 3, 5, 7, 8, 10 },  // minor
        { 0, 2, 4, 5, 7, 9, 11 },  // major
        { 0, 1, 3, 5, 7, 8, 10 },  // phrygian
        { 0, 2, 3, 5, 7, 9, 10 },  // dorian
        { 0, 3, 5, 7, 10 },        // minor pentatonic
        { 0, 1, 4, 5, 7, 8, 10 }   // hijaz
    };

    float frac01 (float x) noexcept { return x - std::floor (x); }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PolvereProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> ps;

    // testo dei valori leggibile
    auto attrs = [] (String label, float lo, float hi)
    {
        auto toText = [label, lo, hi] (float v, int) -> String
        {
            if (label == "ms")   return String (v, v < 100.0f ? 1 : 0);
            if (label == "Hz")   return v >= 1000.0f ? String (v / 1000.0f, 2) + "k" : String (v, v < 10.0f ? 2 : 0);
            if (label.isNotEmpty()) return String (v, 1);
            if (lo >= 0.0f && hi <= 2.0f) return String (roundToInt (v * 100.0f)) + " %";
            if (lo < 0.0f && hi <= 1.0f)  return String (roundToInt (v * 100.0f)) + " %";
            return String (v, 2);
        };
        return AudioParameterFloatAttributes().withLabel (label).withStringFromValueFunction (toText);
    };

    auto lin = [&] (const char* id, const char* name, float lo, float hi, float def, const char* label = "")
    {
        ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name,
            NormalisableRange<float> (lo, hi), def, attrs (label, lo, hi)));
    };
    auto skew = [&] (const char* id, const char* name, float lo, float hi, float centre, float def, const char* label = "")
    {
        NormalisableRange<float> r (lo, hi);
        r.setSkewForCentre (centre);
        ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def, attrs (label, lo, hi)));
    };
    auto choice = [&] (const char* id, const char* name, const StringArray& items, int def)
    {
        ps.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, items, def));
    };
    auto toggle = [&] (const char* id, const char* name, bool def)
    {
        ps.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def));
    };

    // GRAIN
    lin  ("pos",      "Position",       0.0f, 1.0f, 0.2f);
    lin  ("spray",    "Spray",          0.0f, 1.0f, 0.15f);
    lin  ("scan",     "Scan",          -1.0f, 1.0f, 0.15f);
    skew ("size",     "Grain Size",     5.0f, 2000.0f, 120.0f, 140.0f, "ms");
    lin  ("sizeRnd",  "Size Random",    0.0f, 1.0f, 0.3f);
    skew ("density",  "Density",        0.5f, 200.0f, 15.0f, 18.0f, "gr/s");
    lin  ("densRnd",  "Density Random", 0.0f, 1.0f, 0.4f);
    lin  ("chance",   "Chance",         0.0f, 1.0f, 1.0f);
    // PITCH
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "pitch", 1 }, "Pitch",
        NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("st").withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v)); })));
    lin  ("pitchRnd", "Pitch Random",   0.0f, 24.0f, 0.0f, "st");
    choice ("scale",  "Scale", scaleNames, 3);
    lin  ("detune",   "Detune Random",  0.0f, 100.0f, 8.0f, "ct");
    lin  ("reverse",  "Reverse Prob",   0.0f, 1.0f, 0.2f);
    toggle ("keytrack", "Key Track", true);
    // COLOR
    choice ("window", "Window", windowNames, 0);
    lin  ("pan",      "Pan Spread",     0.0f, 1.0f, 0.6f);
    lin  ("sample",   "Sample Select",  0.0f, 1.0f, 0.0f);
    lin  ("sampleRnd","Sample Random",  0.0f, 1.0f, 0.3f);
    skew ("cutoff",   "Cutoff",         30.0f, 20000.0f, 1200.0f, 20000.0f, "Hz");
    lin  ("cutRnd",   "Cutoff Random",  0.0f, 1.0f, 0.0f);
    lin  ("reso",     "Resonance",      0.0f, 1.0f, 0.15f);
    choice ("ftype",  "Filter Type", filterNames, 0);
    // MUTATE
    lin  ("mutate",   "Mutate",         0.0f, 1.0f, 0.25f);
    skew ("mutRate",  "Mutate Rate",    0.02f, 4.0f, 0.3f, 0.15f, "Hz");
    toggle ("drone",  "Drone", false);
    // AMP
    skew ("attack",   "Attack",         1.0f, 5000.0f, 200.0f, 30.0f, "ms");
    skew ("decay",    "Decay",          1.0f, 5000.0f, 300.0f, 500.0f, "ms");
    lin  ("sustain",  "Sustain",        0.0f, 1.0f, 0.8f);
    skew ("release",  "Release",        5.0f, 10000.0f, 800.0f, 1500.0f, "ms");
    // FX
    skew ("dlyTime",  "Delay Time",     10.0f, 2000.0f, 300.0f, 375.0f, "ms");
    lin  ("dlyFb",    "Delay Feedback", 0.0f, 0.95f, 0.4f);
    lin  ("dlyMix",   "Delay Mix",      0.0f, 1.0f, 0.2f);
    lin  ("verbSize", "Reverb Size",    0.0f, 1.0f, 0.75f);
    lin  ("verbMix",  "Reverb Mix",     0.0f, 1.0f, 0.3f);
    lin  ("width",    "Width",          0.0f, 2.0f, 1.2f);
    lin  ("gain",     "Output",        -36.0f, 12.0f, -6.0f, "dB");

    return { ps.begin(), ps.end() };
}

//==============================================================================
PolvereProcessor::PolvereProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "POLVERE", createLayout())
{
    formatManager.registerBasicFormats();

    pPos = p ("pos"); pSpray = p ("spray"); pScan = p ("scan"); pSize = p ("size"); pSizeRnd = p ("sizeRnd");
    pDensity = p ("density"); pDensRnd = p ("densRnd"); pChance = p ("chance");
    pPitch = p ("pitch"); pPitchRnd = p ("pitchRnd"); pScale = p ("scale"); pDetune = p ("detune");
    pReverse = p ("reverse"); pKeytrack = p ("keytrack");
    pWindow = p ("window"); pPan = p ("pan"); pSample = p ("sample"); pSampleRnd = p ("sampleRnd");
    pCutoff = p ("cutoff"); pCutRnd = p ("cutRnd"); pReso = p ("reso"); pFtype = p ("ftype");
    pMutate = p ("mutate"); pMutRate = p ("mutRate"); pDrone = p ("drone");
    pAttack = p ("attack"); pDecay = p ("decay"); pSustain = p ("sustain"); pRelease = p ("release");
    pDlyTime = p ("dlyTime"); pDlyFb = p ("dlyFb"); pDlyMix = p ("dlyMix");
    pVerbSize = p ("verbSize"); pVerbMix = p ("verbMix"); pWidth = p ("width"); pGain = p ("gain");

    rpSize = apvts.getParameter ("size"); rpDensity = apvts.getParameter ("density");
    rpPitchRnd = apvts.getParameter ("pitchRnd"); rpCutoff = apvts.getParameter ("cutoff");

    for (auto& m : modNorm) m.store (-1.0f);

    rng.seed ((uint32_t) juce::Time::getMillisecondCounter() ^ 0xA5A5F00Du);
    for (size_t i = 0; i < wander.size(); ++i)
    {
        wander[i].rateMul = 0.6f + 0.15f * (float) i;  // velocità diverse = movimento non sincronizzato
        wander[i].to = rng.bi();
        wander[i].phase = rng.uni();
    }
}

PolvereProcessor::~PolvereProcessor()
{
    cancelPendingUpdate();
    loaderPool.removeAllJobs (true, 10000);
}

bool PolvereProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void PolvereProcessor::prepareToPlay (double sampleRate, int)
{
    hostRate = sampleRate;
    for (auto& g : grains) g.active = false;
    for (auto& v : voices) { v.active = false; v.env.setSampleRate (sampleRate); v.env.reset(); }
    droneWasOn = false;

    delayBuf.setSize (2, (int) (sampleRate * 2.5) + 8);
    delayBuf.clear();
    delayWrite = 0;
    delaySmoothed.reset (sampleRate, 0.25);
    delaySmoothed.setCurrentAndTargetValue (pDlyTime->load() * 0.001f * (float) sampleRate);
    dampL = dampR = 0.0f;

    reverb.setSampleRate (sampleRate);
    reverb.reset();
    gainSmoothed.reset (sampleRate, 0.05);
    gainSmoothed.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pGain->load()));
}

//==============================================================================
float PolvereProcessor::quantise (float semis, int scale) const noexcept
{
    if (scale <= 0) return semis;
    if (scale == 1) return std::round (semis);

    const auto& deg = scaleDegrees[(size_t) scale];
    const float oct = std::floor (semis / 12.0f);
    const float rem = semis - oct * 12.0f;
    float best = 0.0f, bestDist = 1.0e9f;
    for (int d : deg)
        for (float cand : { (float) d, (float) d + 12.0f })
            if (std::abs (cand - rem) < bestDist) { bestDist = std::abs (cand - rem); best = cand; }
    return oct * 12.0f + best;
}

void PolvereProcessor::noteOn (int note, float velocity)
{
    int idx = -1;
    for (int i = 0; i < maxVoices; ++i)
        if (! voices[(size_t) i].active) { idx = i; break; }

    if (idx < 0) // voice stealing: la più vecchia non-drone
    {
        uint32_t oldest = UINT32_MAX;
        for (int i = 0; i < maxVoices; ++i)
            if (! voices[(size_t) i].drone && voices[(size_t) i].age < oldest) { oldest = voices[(size_t) i].age; idx = i; }
        if (idx < 0) idx = 0;
    }

    auto& v = voices[(size_t) idx];
    v.active = true;
    v.drone = false;
    v.note = note;
    v.velocity = velocity;
    v.samplesToNext = 0.0;
    v.age = ++voiceCounter;
    v.env.setSampleRate (hostRate);
    v.env.reset();
    v.env.noteOn();
}

void PolvereProcessor::noteOff (int note)
{
    for (auto& v : voices)
        if (v.active && ! v.drone && v.note == note)
            v.env.noteOff();
}

void PolvereProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())
        noteOff (m.getNoteNumber());
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        for (auto& v : voices)
            if (! v.drone) v.env.noteOff();
}

//==============================================================================
void PolvereProcessor::spawnGrain (int voiceIndex, int offset)
{
    if (activePool.empty()) return;
    if (rng.uni() >= frame.chance) return;

    int gi = -1;
    for (int i = 0; i < maxGrains; ++i)
        if (! grains[(size_t) i].active) { gi = i; break; }
    if (gi < 0) return;

    const auto& v = voices[(size_t) voiceIndex];
    const int n = (int) activePool.size();

    int sIdx = juce::jlimit (0, n - 1, (int) std::round (frame.sample * (float) (n - 1)));
    if (rng.uni() < frame.sampleRnd) sIdx = (int) (rng.uni() * (float) n) % n;

    const auto& s = *activePool[(size_t) sIdx];
    const int len = s.data.getNumSamples();
    if (len < 64) return;

    // durata
    const float sizeMs = juce::jlimit (3.0f, 4000.0f, frame.sizeMs * std::exp2 (rng.bi() * frame.sizeRnd * 2.0f));
    const int lengthOut = juce::jmax (16, (int) (sizeMs * 0.001f * (float) hostRate));

    // pitch
    float semis = frame.pitch + (frame.keytrack ? (float) (v.note - 60) : 0.0f);
    const float rnd = quantise (rng.bi() * frame.pitchRnd, frame.scale);
    semis += rnd + rng.bi() * frame.detune * 0.01f;
    const double ratio = std::exp2 ((double) semis / 12.0) * (s.sampleRate / hostRate);
    const bool rev = rng.uni() < frame.reverse;

    // posizione
    const float posN = frac01 (frame.pos + rng.bi() * frame.spray * 0.5f);

    auto& g = grains[(size_t) gi];
    g.active = true;
    g.voice = voiceIndex;
    g.sampleIndex = sIdx;
    g.readPos = (double) posN * (double) (len - 1);
    g.increment = rev ? -ratio : ratio;
    g.length = lengthOut;
    g.age = 0;
    g.delay = offset;
    g.window = frame.window >= numWindows ? (int) (rng.uni() * (float) numWindows) % numWindows : frame.window;

    // pan equal-power + normalizzazione per overlap
    const float pan = juce::jlimit (-1.0f, 1.0f, rng.bi() * frame.pan);
    const float angle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
    const float overlap = frame.density * sizeMs * 0.001f;
    const float amp = v.velocity * 1.6f / std::sqrt (juce::jmax (1.0f, overlap));
    g.gainL = std::cos (angle) * amp;
    g.gainR = std::sin (angle) * amp;

    // filtro per grano
    const float cutoff = juce::jlimit (30.0f, 20000.0f, frame.cutoff * std::exp2 (rng.bi() * frame.cutRnd * 3.0f));
    g.filterType = frame.ftype;
    g.filtered = ! (frame.ftype == 0 && cutoff >= 19000.0f) && ! (frame.ftype == 2 && cutoff <= 35.0f);
    if (g.filtered)
    {
        const float fc = juce::jmin (cutoff, (float) hostRate * 0.45f);
        const float gg = std::tan (juce::MathConstants<float>::pi * fc / (float) hostRate);
        g.fk = 1.0f / (0.6f + frame.reso * 11.0f);
        g.fa1 = 1.0f / (1.0f + gg * (gg + g.fk));
        g.fa2 = gg * g.fa1;
        g.fa3 = gg * g.fa2;
        g.ic1[0] = g.ic1[1] = g.ic2[0] = g.ic2[1] = 0.0f;
    }

    // evento per la GUI
    int s1, n1, s2, n2;
    eventFifo.prepareToWrite (1, s1, n1, s2, n2);
    if (n1 + n2 > 0)
    {
        auto& e = eventBuffer[(size_t) (n1 > 0 ? s1 : s2)];
        e.sampleIndex = sIdx; e.position = posN; e.pan = pan; e.pitch = semis;
        e.life = sizeMs * 0.001f; e.reverse = rev;
        eventFifo.finishedWrite (1);
    }
}

void PolvereProcessor::renderGrains (float* outL, float* outR, int numSamples)
{
    const int poolSize = (int) activePool.size();

    for (auto& g : grains)
    {
        if (! g.active) continue;
        const bool voiceDead = ! voices[(size_t) g.voice].active && envBuf[(size_t) g.voice][0] <= 0.0f;
        if (g.sampleIndex >= poolSize || voiceDead)
        {
            g.active = false;
            continue;
        }

        const auto& s = *activePool[(size_t) g.sampleIndex];
        const float* L = s.data.getReadPointer (0);
        const float* R = s.data.getReadPointer (1);
        const int len = s.data.getNumSamples();
        const float* env = envBuf[(size_t) g.voice].data();
        const float invLen = 1.0f / (float) g.length;

        for (int i = g.delay; i < numSamples; ++i)
        {
            if (g.age >= g.length) { g.active = false; break; }

            const float w = windows.lookup (g.window, (float) g.age * invLen) * env[i];

            const int i0 = (int) g.readPos;
            const float t = (float) (g.readPos - (double) i0);
            float l, r;
            if (i0 >= 1 && i0 + 2 < len)
            {
                l = hermite (L[i0 - 1], L[i0], L[i0 + 1], L[i0 + 2], t);
                r = hermite (R[i0 - 1], R[i0], R[i0 + 1], R[i0 + 2], t);
            }
            else
            {
                auto w0 = [len] (int k) { k %= len; return k < 0 ? k + len : k; };
                const int a = w0 (i0 - 1), b = w0 (i0), c = w0 (i0 + 1), d = w0 (i0 + 2);
                l = hermite (L[a], L[b], L[c], L[d], t);
                r = hermite (R[a], R[b], R[c], R[d], t);
            }

            if (g.filtered)
            {
                float* io[2] = { &l, &r };
                for (int ch = 0; ch < 2; ++ch)
                {
                    const float v0 = *io[ch];
                    const float v3 = v0 - g.ic2[ch];
                    const float v1 = g.fa1 * g.ic1[ch] + g.fa2 * v3;
                    const float v2 = g.ic2[ch] + g.fa2 * g.ic1[ch] + g.fa3 * v3;
                    g.ic1[ch] = 2.0f * v1 - g.ic1[ch];
                    g.ic2[ch] = 2.0f * v2 - g.ic2[ch];
                    *io[ch] = g.filterType == 0 ? v2 : g.filterType == 1 ? v1 : v0 - g.fk * v1 - v2;
                }
            }

            outL[i] += l * w * g.gainL;
            outR[i] += r * w * g.gainR;

            g.readPos += g.increment;
            if (g.readPos >= (double) len) g.readPos -= (double) len;
            else if (g.readPos < 0.0)      g.readPos += (double) len;
            ++g.age;
        }
        g.delay = 0;
    }
}

//==============================================================================
void PolvereProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);

    // handoff del pool dei sample (nessuna deallocazione sull'audio thread)
    {
        const juce::SpinLock::ScopedTryLockType lock (poolLock);
        if (lock.isLocked() && hasPendingPool)
        {
            std::swap (activePool, pendingPool);
            hasPendingPool = false;
            for (auto& g : grains) g.active = false;
        }
    }

    // drone
    const bool drone = pDrone->load() > 0.5f;
    if (drone && ! droneWasOn)
    {
        noteOn (60, 0.8f);
        for (auto& v : voices)
            if (v.age == voiceCounter) v.drone = true;
    }
    else if (! drone && droneWasOn)
    {
        for (auto& v : voices)
            if (v.drone) { v.env.noteOff(); v.drone = false; v.note = -1; }
    }
    droneWasOn = drone;

    const juce::ADSR::Parameters adsr { pAttack->load() * 0.001f, pDecay->load() * 0.001f,
                                        pSustain->load(), pRelease->load() * 0.001f };
    for (auto& v : voices) v.env.setParameters (adsr);

    auto* outL = buffer.getWritePointer (0);
    auto* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    std::array<float, subBlock> tmpR {};

    auto midiIt = midi.cbegin();
    const float mut = pMutate->load();
    const float mutRate = pMutRate->load();
    const float scanV = pScan->load();
    const double scanSpeed = (double) (scanV * scanV * scanV) * 0.25; // posizione/secondo

    for (int start = 0; start < numSamples; start += subBlock)
    {
        const int n = juce::jmin (subBlock, numSamples - start);

        for (; midiIt != midi.cend() && (*midiIt).samplePosition < start + n; ++midiIt)
            handleMidi ((*midiIt).getMessage());

        // --- modulazione "mutate" + scan
        const float secs = (float) n / (float) hostRate;
        for (auto& w : wander) w.advance (mutRate, secs, rng);
        scanPhase += scanSpeed * (double) secs;
        scanPhase -= std::floor (scanPhase);

        frame.pos      = frac01 (pPos->load() + (float) scanPhase + mut * 0.35f * wander[modPos].value);
        frame.spray    = juce::jlimit (0.0f, 1.0f, pSpray->load() + mut * 0.3f * (wander[modSpray].value * 0.5f + 0.5f));
        frame.sizeMs   = juce::jlimit (5.0f, 2000.0f, pSize->load() * std::exp2 (mut * 1.5f * wander[modSize].value));
        frame.sizeRnd  = pSizeRnd->load();
        frame.density  = juce::jlimit (0.5f, 200.0f, pDensity->load() * std::exp2 (mut * 1.5f * wander[modDensity].value));
        frame.densRnd  = pDensRnd->load();
        frame.chance   = pChance->load();
        frame.pitch    = pPitch->load();
        frame.pitchRnd = juce::jlimit (0.0f, 24.0f, pPitchRnd->load() + mut * 7.0f * (wander[modPitchRnd].value * 0.5f + 0.5f));
        frame.scale    = (int) pScale->load();
        frame.detune   = pDetune->load();
        frame.reverse  = pReverse->load();
        frame.keytrack = pKeytrack->load() > 0.5f;
        frame.window   = (int) pWindow->load();
        frame.pan      = pPan->load();
        frame.sample   = pSample->load();
        frame.sampleRnd= pSampleRnd->load();
        frame.cutoff   = juce::jlimit (30.0f, 20000.0f, pCutoff->load() * std::exp2 (mut * 3.0f * wander[modCutoff].value));
        frame.cutRnd   = pCutRnd->load();
        frame.reso     = pReso->load();
        frame.ftype    = (int) pFtype->load();

        // --- voci: inviluppo + scheduler dei grani
        for (int vi = 0; vi < maxVoices; ++vi)
        {
            auto& v = voices[(size_t) vi];
            auto& eb = envBuf[(size_t) vi];
            if (! v.active) { std::fill (eb.begin(), eb.end(), 0.0f); continue; }

            for (int i = 0; i < n; ++i)
                eb[(size_t) i] = v.env.getNextSample();

            const double mean = hostRate / (double) frame.density;
            while (v.samplesToNext < (double) n)
            {
                spawnGrain (vi, juce::jmax (0, (int) v.samplesToNext));
                const double regular = mean;
                const double poisson = -std::log (1.0 - (double) rng.uni() * 0.9999) * mean;
                v.samplesToNext += juce::jmax (1.0, regular + (poisson - regular) * (double) frame.densRnd);
            }
            v.samplesToNext -= (double) n;

            if (! v.env.isActive()) { v.active = false; v.drone = false; }
        }

        std::fill (tmpR.begin(), tmpR.end(), 0.0f);
        renderGrains (outL + start, outR != nullptr ? outR + start : tmpR.data(), n);
    }

    // --- stato per la GUI
    int gc = 0;
    for (auto& g : grains) gc += g.active ? 1 : 0;
    int vc = 0;
    for (auto& v : voices) vc += v.active ? 1 : 0;
    activeGrainCount.store (gc);
    activeVoiceCount.store (vc);
    scanPhaseDisplay.store (frame.pos);
    modNorm[modPos].store (frame.pos);
    modNorm[modSize].store (rpSize->convertTo0to1 (frame.sizeMs));
    modNorm[modDensity].store (rpDensity->convertTo0to1 (frame.density));
    modNorm[modPitchRnd].store (rpPitchRnd->convertTo0to1 (frame.pitchRnd));
    modNorm[modCutoff].store (rpCutoff->convertTo0to1 (frame.cutoff));
    modNorm[modSpray].store (frame.spray);

    if (outR == nullptr) return; // mono: niente FX stereo

    // --- FX: ping-pong delay
    const float fb = pDlyFb->load();
    const float dMix = pDlyMix->load();
    delaySmoothed.setTargetValue (pDlyTime->load() * 0.001f * (float) hostRate);
    const int dLen = delayBuf.getNumSamples();
    auto* dl = delayBuf.getWritePointer (0);
    auto* dr = delayBuf.getWritePointer (1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float d = juce::jlimit (1.0f, (float) dLen - 4.0f, delaySmoothed.getNextValue());
        float rp = (float) delayWrite - d;
        if (rp < 0.0f) rp += (float) dLen;
        const int r0 = (int) rp;
        const int r1 = (r0 + 1) % dLen;
        const float fr = rp - (float) r0;
        const float wetL = dl[r0] + fr * (dl[r1] - dl[r0]);
        const float wetR = dr[r0] + fr * (dr[r1] - dr[r0]);

        dampL += 0.4f * (wetR - dampL); // feedback incrociato + smorzamento
        dampR += 0.4f * (wetL - dampR);
        dl[delayWrite] = outL[i] + std::tanh (dampL * fb);
        dr[delayWrite] = outR[i] + std::tanh (dampR * fb);
        if (++delayWrite >= dLen) delayWrite = 0;

        outL[i] += wetL * dMix;
        outR[i] += wetR * dMix;
    }

    // --- riverbero
    juce::Reverb::Parameters rv;
    const float vMix = pVerbMix->load();
    rv.roomSize = 0.3f + 0.69f * pVerbSize->load();
    rv.damping = 0.45f;
    rv.width = 1.0f;
    rv.wetLevel = vMix * 0.25f;                 // juce::Reverb scala il wet x3
    rv.dryLevel = (1.0f - vMix * 0.5f) * 0.5f;  // ...e il dry x2
    reverb.setParameters (rv);
    reverb.processStereo (outL, outR, numSamples);

    // --- width, gain, soft clip
    const float width = pWidth->load();
    gainSmoothed.setTargetValue (juce::Decibels::decibelsToGain (pGain->load()));
    for (int i = 0; i < numSamples; ++i)
    {
        const float m = 0.5f * (outL[i] + outR[i]);
        const float s = 0.5f * (outL[i] - outR[i]) * width;
        const float gn = gainSmoothed.getNextValue();
        outL[i] = std::tanh ((m + s) * gn);
        outR[i] = std::tanh ((m - s) * gn);
    }
}

//==============================================================================
void PolvereProcessor::loadFiles (const juce::Array<juce::File>& files, bool replaceExisting)
{
    ++pendingJobs;
    sendChangeMessage();

    loaderPool.addJob ([this, files, replaceExisting]
    {
        SamplePool loaded;
        for (const auto& f : files)
        {
            if (loaded.size() >= 128) break;
            std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (f));
            if (reader == nullptr) continue;

            const auto maxLen = (juce::int64) (reader->sampleRate * 120.0); // max 2 minuti per sample
            const int len = (int) juce::jmin (reader->lengthInSamples, maxLen);
            if (len < 64) continue;

            auto s = std::make_shared<Sample>();
            s->name = f.getFileNameWithoutExtension();
            s->file = f;
            s->sampleRate = reader->sampleRate;
            s->data.setSize (2, len);
            s->data.clear();
            reader->read (&s->data, 0, len, 0, true, true);
            if (reader->numChannels == 1)
                s->data.copyFrom (1, 0, s->data, 0, 0, len);

            // normalizza a -1 dBFS per un pool omogeneo
            const float peak = s->data.getMagnitude (0, len);
            if (peak > 1.0e-4f) s->data.applyGain (0.89f / peak);

            s->peaks.assign (256, 0.0f);
            for (int b = 0; b < 256; ++b)
            {
                const int a = (int) ((juce::int64) b * len / 256);
                const int e = juce::jmax (a + 1, (int) ((juce::int64) (b + 1) * len / 256));
                s->peaks[(size_t) b] = juce::jmin (1.0f, s->data.getMagnitude (a, e - a));
            }
            loaded.push_back (std::move (s));
        }

        {
            const juce::ScopedLock sl (loadedLock);
            if (replaceExisting) { loadedQueue.clear(); replaceOnNextMerge = true; }
            for (auto& s : loaded) loadedQueue.push_back (std::move (s));
        }
        --pendingJobs;
        triggerAsyncUpdate();
    });
}

void PolvereProcessor::handleAsyncUpdate()
{
    SamplePool incoming;
    bool replace = false;
    {
        const juce::ScopedLock sl (loadedLock);
        incoming.swap (loadedQueue);
        replace = replaceOnNextMerge;
        replaceOnNextMerge = false;
    }

    if (replace) uiPool.clear();
    for (auto& s : incoming)
        if (uiPool.size() < 128) uiPool.push_back (std::move (s));

    publishPool();
}

void PolvereProcessor::clearSamples()
{
    loadFiles ({}, true);
}

void PolvereProcessor::removeSample (int index)
{
    if (juce::isPositiveAndBelow (index, (int) uiPool.size()))
    {
        uiPool.erase (uiPool.begin() + index);
        publishPool();
    }
}

void PolvereProcessor::publishPool()
{
    SamplePool copy = uiPool;
    SamplePool old;
    {
        const juce::SpinLock::ScopedLockType lock (poolLock);
        old.swap (pendingPool);
        pendingPool.swap (copy);
        hasPendingPool = true;
    }
    old.clear(); // deallocazione sul message thread
    sendChangeMessage();
}

//==============================================================================
void PolvereProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto existing = state.getChildWithName ("SAMPLES"); existing.isValid())
        state.removeChild (existing, nullptr);

    juce::ValueTree samples ("SAMPLES");
    for (const auto& s : uiPool)
    {
        juce::ValueTree t ("S");
        t.setProperty ("path", s->file.getFullPathName(), nullptr);
        samples.appendChild (t, nullptr);
    }
    state.appendChild (samples, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void PolvereProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr) return;

    auto tree = juce::ValueTree::fromXml (*xml);
    if (! tree.hasType (apvts.state.getType())) return;

    juce::Array<juce::File> files;
    if (auto samples = tree.getChildWithName ("SAMPLES"); samples.isValid())
    {
        for (const auto& c : samples)
        {
            juce::File f (c.getProperty ("path").toString());
            if (f.existsAsFile()) files.add (f);
        }
        tree.removeChild (samples, nullptr);
    }

    apvts.replaceState (tree);
    loadFiles (files, true);
}

//==============================================================================
juce::AudioProcessorEditor* PolvereProcessor::createEditor()
{
    return new PolvereEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PolvereProcessor();
}

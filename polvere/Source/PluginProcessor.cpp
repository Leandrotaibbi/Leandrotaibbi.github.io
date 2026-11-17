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
    const juce::StringArray rateNames   { "1/64", "1/32", "1/16T", "1/16", "1/8T", "1/8", "1/4", "1/2", "1 bar" };
    const float rateBeats[]             { 0.0625f, 0.125f, 1.0f / 6.0f, 0.25f, 1.0f / 3.0f, 0.5f, 1.0f, 2.0f, 4.0f };
    const juce::StringArray stutNames   { "1/8", "1/16", "1/32", "1/64", "Rnd" };
    const float stutBeats[]             { 0.5f, 0.25f, 0.125f, 0.0625f };
    const juce::StringArray modNames    { "Drift", "Lorenz", "Step S&H", "Logistic" };

    const std::vector<std::vector<int>> scaleDegrees {
        {}, {}, { 0 }, { 0, 7 },
        { 0, 2, 3, 5, 7, 8, 10 },  // minor
        { 0, 2, 4, 5, 7, 9, 11 },  // major
        { 0, 1, 3, 5, 7, 8, 10 },  // phrygian
        { 0, 2, 3, 5, 7, 9, 10 },  // dorian
        { 0, 3, 5, 7, 10 },        // minor pentatonic
        { 0, 1, 4, 5, 7, 8, 10 }   // hijaz
    };

    // parametri che non vengono toccati dal dado
    const juce::StringArray notDiceable { "play", "generate", "dice", "undo", "diceAmt", "drone", "keytrack",
                                          "gain", "steps", "rate", "attack" };

    float frac01 (float x) noexcept { return x - std::floor (x); }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PolvereProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> ps;

    auto attrs = [] (String label, float lo, float hi)
    {
        auto toText = [label, lo, hi] (float v, int) -> String
        {
            if (label == "ms")   return String (v, v < 100.0f ? 1 : 0);
            if (label == "Hz")   return v >= 1000.0f ? String (v / 1000.0f, 2) + "k" : String (v, v < 10.0f ? 2 : 0);
            if (label == "x")    return String (v, 2);
            if (label.isNotEmpty()) return String (v, 1);
            if (lo >= -1.0f && hi <= 2.0f) return String (roundToInt (v * 100.0f)) + " %";
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

    // PERFORMANCE / trigger (mappabili dal controller)
    toggle ("play",     "Play", true);
    toggle ("generate", "Generate Pattern", false);
    toggle ("dice",     "Dice Sound", false);
    toggle ("undo",     "Undo Dice", false);
    lin    ("diceAmt",  "Dice Amount", 0.05f, 1.0f, 1.0f);
    // SEQUENCER
    ps.push_back (std::make_unique<AudioParameterInt> (ParameterID { "steps", 1 }, "Steps", 1, maxSteps, 16));
    choice ("rate",     "Rate", rateNames, 3);
    lin    ("fill",     "Fill", 0.0f, 1.0f, 0.55f);
    lin    ("range",    "Note Range", 0.0f, 24.0f, 12.0f, "st");
    lin    ("ratchet",  "Ratchet", 0.0f, 1.0f, 0.15f);
    lin    ("evolve",   "Evolve", 0.0f, 1.0f, 0.12f);
    lin    ("plock",    "Step Lock", 0.0f, 1.0f, 0.35f);
    lin    ("gate",     "Gate", 0.05f, 2.0f, 0.5f, "x");
    lin    ("swing",    "Swing", 0.0f, 0.6f, 0.0f);
    // GRAIN
    lin  ("pos",      "Position",       0.0f, 1.0f, 0.2f);
    lin  ("spray",    "Spray",          0.0f, 1.0f, 0.15f);
    lin  ("scan",     "Scan",          -1.0f, 1.0f, 0.15f);
    skew ("size",     "Grain Size",     5.0f, 2000.0f, 120.0f, 110.0f, "ms");
    lin  ("sizeRnd",  "Size Random",    0.0f, 1.0f, 0.3f);
    skew ("density",  "Density",        0.5f, 200.0f, 15.0f, 30.0f, "gr/s");
    lin  ("densRnd",  "Density Random", 0.0f, 1.0f, 0.4f);
    lin  ("chance",   "Chance",         0.0f, 1.0f, 1.0f);
    // PITCH
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "pitch", 1 }, "Pitch",
        NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("st").withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v)); })));
    lin  ("pitchRnd", "Pitch Random",   0.0f, 24.0f, 0.0f, "st");
    choice ("scale",  "Scale", scaleNames, 6);
    lin  ("detune",   "Detune Random",  0.0f, 100.0f, 8.0f, "ct");
    lin  ("reverse",  "Reverse Prob",   0.0f, 1.0f, 0.2f);
    toggle ("keytrack", "Key Track", true);
    // COLOUR
    choice ("window", "Window", windowNames, 6);
    lin  ("pan",      "Pan Spread",     0.0f, 1.0f, 0.6f);
    lin  ("sample",   "Sample Select",  0.0f, 1.0f, 0.0f);
    lin  ("sampleRnd","Sample Random",  0.0f, 1.0f, 0.3f);
    skew ("cutoff",   "Cutoff",         30.0f, 20000.0f, 1200.0f, 20000.0f, "Hz");
    lin  ("cutRnd",   "Cutoff Random",  0.0f, 1.0f, 0.0f);
    lin  ("reso",     "Resonance",      0.0f, 1.0f, 0.15f);
    choice ("ftype",  "Filter Type", filterNames, 0);
    // CHAOS
    lin  ("mutate",   "Chaos Amount",   0.0f, 1.0f, 0.3f);
    skew ("mutRate",  "Chaos Rate",     0.02f, 4.0f, 0.3f, 0.2f, "Hz");
    choice ("modType","Chaos Type", modNames, 1);
    toggle ("drone",  "Drone", false);
    // GLITCH
    lin  ("stutter",  "Stutter",        0.0f, 1.0f, 0.1f);
    choice ("stutLen","Stutter Length", stutNames, 4);
    lin  ("crush",    "Crush",          0.0f, 1.0f, 0.0f);
    // AMP
    skew ("attack",   "Attack",         1.0f, 5000.0f, 200.0f, 4.0f, "ms");
    skew ("decay",    "Decay",          1.0f, 5000.0f, 300.0f, 400.0f, "ms");
    lin  ("sustain",  "Sustain",        0.0f, 1.0f, 0.6f);
    skew ("release",  "Release",        5.0f, 10000.0f, 800.0f, 600.0f, "ms");
    // SPACE
    skew ("dlyTime",  "Delay Time",     10.0f, 2000.0f, 300.0f, 375.0f, "ms");
    lin  ("dlyFb",    "Delay Feedback", 0.0f, 0.95f, 0.45f);
    lin  ("dlyMix",   "Delay Mix",      0.0f, 1.0f, 0.25f);
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
    pMutate = p ("mutate"); pMutRate = p ("mutRate"); pModType = p ("modType"); pDrone = p ("drone");
    pAttack = p ("attack"); pDecay = p ("decay"); pSustain = p ("sustain"); pRelease = p ("release");
    pPlay = p ("play"); pSteps = p ("steps"); pRate = p ("rate"); pFill = p ("fill"); pRange = p ("range");
    pRatchet = p ("ratchet"); pEvolve = p ("evolve"); pPlock = p ("plock"); pGate = p ("gate"); pSwing = p ("swing");
    pStutter = p ("stutter"); pStutLen = p ("stutLen"); pCrush = p ("crush");
    pDlyTime = p ("dlyTime"); pDlyFb = p ("dlyFb"); pDlyMix = p ("dlyMix");
    pVerbSize = p ("verbSize"); pVerbMix = p ("verbMix"); pWidth = p ("width"); pGain = p ("gain");

    rpSize = apvts.getParameter ("size"); rpDensity = apvts.getParameter ("density");
    rpPitchRnd = apvts.getParameter ("pitchRnd"); rpCutoff = apvts.getParameter ("cutoff");

    for (auto& m : modNorm) m.store (-1.0f);
    for (auto& c : ccMap) c.store (-1);
    for (auto& c : pendingCC) c.store (-1.0f);
    paramList = getParameters();

    rng.seed ((uint32_t) juce::Time::getMillisecondCounter() ^ 0xA5A5F00Du);
    for (size_t i = 0; i < wander.size(); ++i)
    {
        wander[i].rateMul = 0.6f + 0.15f * (float) i;
        wander[i].to = rng.bi();
        wander[i].phase = rng.uni();
        logistic[i] = 0.1f + 0.13f * (float) i;
    }

    generatePattern();
    startTimerHz (30);
}

PolvereProcessor::~PolvereProcessor()
{
    stopTimer();
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
    for (auto& e : events) e.active = false;
    droneWasOn = false;
    lastAbsStep = -1;
    wasPlaying = false;

    delayBuf.setSize (2, (int) (sampleRate * 2.5) + 8);
    delayBuf.clear();
    delayWrite = 0;
    delaySmoothed.reset (sampleRate, 0.25);
    delaySmoothed.setCurrentAndTargetValue (pDlyTime->load() * 0.001f * (float) sampleRate);
    dampL = dampR = 0.0f;

    stutBuf.setSize (2, (int) (sampleRate * 3.0));
    stutBuf.clear();
    stutWrite = stutPhase = stutRemaining = 0;
    stutLen = 64;

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

int PolvereProcessor::noteOn (int note, float velocity, float posOffset, float sizeMul)
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
    v.posOffset = posOffset;
    v.sizeMul = sizeMul;
    v.samplesToNext = 0.0;
    v.age = ++voiceCounter;
    v.env.setSampleRate (hostRate);
    v.env.reset();
    v.env.noteOn();
    return idx;
}

void PolvereProcessor::noteOffVoice (int voice, uint32_t age)
{
    if (! juce::isPositiveAndBelow (voice, maxVoices)) return;
    auto& v = voices[(size_t) voice];
    if (v.active && ! v.drone && v.age == age)
        v.env.noteOff();
}

void PolvereProcessor::releaseSequencerVoices()
{
    for (auto& e : events) e.active = false;
    for (auto& v : voices)
        if (v.active && ! v.drone) v.env.noteOff();
}

void PolvereProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
    {
        rootNote.store (m.getNoteNumber()); // le note in ingresso trasportano il pattern
    }
    else if (m.isController())
    {
        const int cc = m.getControllerNumber();
        const float val = (float) m.getControllerValue() / 127.0f;
        const int learning = learnParam.exchange (-1);
        if (learning >= 0)
        {
            for (auto& c : ccMap)
                if (c.load() == learning) c.store (-1);
            ccMap[(size_t) cc].store (learning);
        }
        if (ccMap[(size_t) cc].load() >= 0)
            pendingCC[(size_t) cc].store (val);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        releaseSequencerVoices();
    }
}

//==============================================================================
// SEQUENCER GENERATIVO
//==============================================================================
void PolvereProcessor::generateStep (int i) noexcept
{
    auto& s = pattern[(size_t) i];
    const float fill = pFill->load();
    const float range = pRange->load();
    const float plock = pPlock->load();
    const float ratchetP = pRatchet->load();

    // i tempi forti hanno più probabilità: l'ipnosi nasce dalla ripetizione
    const float bias = (i % 4 == 0) ? 0.2f : (i % 2 == 0 ? 0.05f : -0.08f);
    s.on = rng.uni() < juce::jlimit (0.0f, 1.0f, fill + bias);

    // melodia: metà delle note sulla root, il resto nel range quantizzato
    float semis = 0.0f;
    if (rng.uni() > 0.45f)
        semis = quantise (rng.bi() * range, juce::jmax (1, (int) pScale->load()));
    s.semis = semis;

    s.velocity = juce::jlimit (0.2f, 1.0f, 0.55f + 0.45f * rng.uni() + (i % 4 == 0 ? 0.15f : 0.0f));
    s.gate = pGate->load() * (0.5f + rng.uni());
    s.ratchet = rng.uni() < ratchetP ? 2 + (int) (rng.uni() * 3.0f) : 1;
    s.posOffset = rng.bi() * plock * 0.5f;
    s.sizeMul = std::exp2 (rng.bi() * plock * 2.0f);
}

void PolvereProcessor::generatePattern() noexcept
{
    for (int i = 0; i < maxSteps; ++i) generateStep (i);
    ++patternVersion;
}

void PolvereProcessor::scheduleEvent (const SeqEvent& e) noexcept
{
    for (auto& slot : events)
        if (! slot.active) { slot = e; slot.active = true; return; }
}

void PolvereProcessor::triggerStep (juce::int64 absStep, int offset, double stepSamples)
{
    const int steps = juce::jlimit (1, maxSteps, (int) pSteps->load());
    const int idx = (int) (((absStep % steps) + steps) % steps);

    if (idx == 0)
    {
        const float evolve = pEvolve->load();
        if (evolve > 0.0f)
        {
            for (int i = 0; i < steps; ++i)
                if (rng.uni() < evolve) generateStep (i);
            ++patternVersion;
        }
    }
    currentStep.store (idx);

    // stutter: ripete una fetta dell'uscita appena prodotta
    if (stutRemaining <= 0 && rng.uni() < pStutter->load() * 0.5f)
    {
        int li = (int) pStutLen->load();
        if (li >= 4) li = (int) (rng.uni() * 4.0f) % 4;
        const double beatSamples = stepSamples / (double) rateBeats[juce::jlimit (0, 8, (int) pRate->load())];
        const int maxLen = stutBuf.getNumSamples() / 3;
        stutLen = juce::jlimit (64, maxLen, (int) (beatSamples * stutBeats[li]));
        stutStart = stutWrite - stutLen;
        if (stutStart < 0) stutStart += stutBuf.getNumSamples();
        stutPhase = 0;
        stutRemaining = juce::jmin (maxLen, (int) (stepSamples * (rng.uni() < 0.35f ? 2.0 : 1.0)));
    }

    const auto& s = pattern[(size_t) idx];
    if (! s.on) return;

    const int r = juce::jmax (1, s.ratchet);
    const double sub = stepSamples / r;
    const int note = juce::jlimit (0, 127, rootNote.load() + (int) std::round (s.semis));
    for (int k = 0; k < r; ++k)
    {
        SeqEvent e;
        e.isOn = true;
        e.countdown = offset + (int) (k * sub);
        e.note = note;
        e.velocity = s.velocity * (k == 0 ? 1.0f : 0.7f);
        e.posOffset = s.posOffset;
        e.sizeMul = s.sizeMul;
        e.offAfter = juce::jmax (32, (int) (r > 1 ? sub * 0.6 : (double) s.gate * stepSamples));
        scheduleEvent (e);
    }
}

void PolvereProcessor::updateChaos (float secs, bool stepTick) noexcept
{
    const int type = (int) pModType->load();
    const float rate = pMutRate->load();

    switch (type)
    {
        case 1: // Lorenz: caos deterministico, mai uguale due volte
        {
            float t = secs * rate * 1.5f;
            while (t > 0.0f)
            {
                const float dt = juce::jmin (t, 0.005f);
                const float dx = 10.0f * (lorY - lorX);
                const float dy = lorX * (28.0f - lorZ) - lorY;
                const float dz = lorX * lorY - (8.0f / 3.0f) * lorZ;
                lorX += dx * dt; lorY += dy * dt; lorZ += dz * dt;
                t -= dt;
            }
            chaosOut = { lorX / 20.0f, lorY / 27.0f, (lorZ - 25.0f) / 22.0f, lorX * lorY / 300.0f, -lorY / 27.0f, (lorX - lorY) / 30.0f };
            break;
        }
        case 2: // Step S&H: salti a ogni step (o al rate se il sequencer è fermo)
        case 3: // Logistic map: caos a ogni step
        {
            shPhase += rate * secs;
            const bool tick = stepTick || shPhase >= 1.0f;
            if (shPhase >= 1.0f) shPhase -= std::floor (shPhase);
            if (tick)
            {
                for (size_t i = 0; i < chaosOut.size(); ++i)
                {
                    if (type == 2)
                        chaosOut[i] = rng.uni() < 0.6f ? rng.bi() : chaosOut[i];
                    else
                    {
                        const float rr = 3.7f + 0.29f * (float) i / 5.0f;
                        logistic[i] = juce::jlimit (0.001f, 0.999f, rr * logistic[i] * (1.0f - logistic[i]));
                        chaosOut[i] = logistic[i] * 2.0f - 1.0f;
                    }
                }
            }
            break;
        }
        default: // Drift: random walk lento
            for (size_t i = 0; i < wander.size(); ++i)
            {
                wander[i].advance (rate, secs, rng);
                chaosOut[i] = wander[i].value;
            }
            break;
    }
    for (auto& c : chaosOut) c = juce::jlimit (-1.0f, 1.0f, c);
}

//==============================================================================
// GRANI
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

    const float sizeMs = juce::jlimit (3.0f, 4000.0f, frame.sizeMs * v.sizeMul * std::exp2 (rng.bi() * frame.sizeRnd * 2.0f));
    const int lengthOut = juce::jmax (16, (int) (sizeMs * 0.001f * (float) hostRate));

    float semis = frame.pitch + (frame.keytrack ? (float) (v.note - 60) : 0.0f);
    semis += quantise (rng.bi() * frame.pitchRnd, frame.scale) + rng.bi() * frame.detune * 0.01f;
    const double ratio = std::exp2 ((double) semis / 12.0) * (s.sampleRate / hostRate);
    const bool rev = rng.uni() < frame.reverse;

    const float posN = frac01 (frame.pos + v.posOffset + rng.bi() * frame.spray * 0.5f);

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

    const float pan = juce::jlimit (-1.0f, 1.0f, rng.bi() * frame.pan);
    const float angle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
    const float overlap = frame.density * sizeMs * 0.001f;
    const float amp = v.velocity * 1.6f / std::sqrt (juce::jmax (1.0f, overlap));
    g.gainL = std::cos (angle) * amp;
    g.gainR = std::sin (angle) * amp;

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

    // handoff pool / pattern (nessuna deallocazione sull'audio thread)
    {
        const juce::SpinLock::ScopedTryLockType lock (poolLock);
        if (lock.isLocked() && hasPendingPool)
        {
            std::swap (activePool, pendingPool);
            hasPendingPool = false;
            for (auto& g : grains) g.active = false;
        }
    }
    {
        const juce::SpinLock::ScopedTryLockType lock (patternLock);
        if (lock.isLocked() && hasPendingPattern)
        {
            pattern = pendingPattern;
            hasPendingPattern = false;
            ++patternVersion;
        }
    }
    if (generateRequested.exchange (false))
        generatePattern();

    // trasporto: segue la DAW se è in play, altrimenti corre da solo
    double bpm = 120.0;
    bool hostPlaying = false;
    double hostPpq = 0.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = juce::jlimit (20.0, 400.0, *b);
            if (pos->getIsPlaying())
                if (auto q = pos->getPpqPosition()) { hostPlaying = true; hostPpq = *q; }
        }
    const double ppqPerSample = bpm / 60.0 / hostRate;
    const double basePpq = hostPlaying ? hostPpq : freePpq;
    const int rateIdx = juce::jlimit (0, 8, (int) pRate->load());
    const double stepBeats = rateBeats[rateIdx];
    const double stepSamples = stepBeats / ppqPerSample;
    const float swing = pSwing->load();

    const bool play = pPlay->load() > 0.5f;
    if (! play && wasPlaying)
    {
        releaseSequencerVoices();
        currentStep.store (-1);
    }
    if (play && ! wasPlaying) lastAbsStep = -1;
    wasPlaying = play;

    // drone
    const bool drone = pDrone->load() > 0.5f;
    if (drone && ! droneWasOn)
        voices[(size_t) noteOn (rootNote.load(), 0.7f)].drone = true;
    else if (! drone && droneWasOn)
        for (auto& v : voices)
            if (v.drone) { v.env.noteOff(); v.drone = false; v.note = -1; }
    droneWasOn = drone;

    const juce::ADSR::Parameters adsr { pAttack->load() * 0.001f, pDecay->load() * 0.001f,
                                        pSustain->load(), pRelease->load() * 0.001f };
    for (auto& v : voices) v.env.setParameters (adsr);

    auto* outL = buffer.getWritePointer (0);
    auto* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    std::array<float, subBlock> tmpR {};

    auto midiIt = midi.cbegin();
    const float mut = pMutate->load();
    const float scanV = pScan->load();
    const double scanSpeed = (double) (scanV * scanV * scanV) * 0.25;
    const float crush = pCrush->load();
    const int stutSize = stutBuf.getNumSamples();
    auto* sbL = stutBuf.getWritePointer (0);
    auto* sbR = stutBuf.getWritePointer (1);

    for (int start = 0; start < numSamples; start += subBlock)
    {
        const int n = juce::jmin (subBlock, numSamples - start);

        for (; midiIt != midi.cend() && (*midiIt).samplePosition < start + n; ++midiIt)
            handleMidi ((*midiIt).getMessage());

        // --- sequencer
        bool stepTick = false;
        if (play)
        {
            const double ppqNow = basePpq + (double) start * ppqPerSample;
            const double sPos = ppqNow / stepBeats;
            const auto absStep = (juce::int64) std::floor (sPos);
            const double fr = sPos - (double) absStep;
            const double threshold = (absStep & 1) ? swing * 0.5 : 0.0;
            if (absStep != lastAbsStep && fr >= threshold)
            {
                lastAbsStep = absStep;
                triggerStep (absStep, 0, stepSamples);
                stepTick = true;
            }
        }

        // --- eventi note (con ratchet e gate)
        for (auto& e : events)
        {
            if (! e.active) continue;
            if (e.countdown < n)
            {
                const int at = juce::jmax (0, e.countdown);
                e.active = false;
                if (e.isOn)
                {
                    const int vi = noteOn (e.note, e.velocity, e.posOffset, e.sizeMul);
                    voices[(size_t) vi].samplesToNext = (double) at;
                    SeqEvent off;
                    off.isOn = false;
                    off.voice = vi;
                    off.age = voices[(size_t) vi].age;
                    off.countdown = juce::jmax (0, at + e.offAfter - n);
                    scheduleEvent (off);
                }
                else
                    noteOffVoice (e.voice, e.age);
            }
            else
                e.countdown -= n;
        }

        // --- caos + scan
        const float secs = (float) n / (float) hostRate;
        updateChaos (secs, stepTick);
        scanPhase += scanSpeed * (double) secs;
        scanPhase -= std::floor (scanPhase);

        frame.pos      = frac01 (pPos->load() + (float) scanPhase + mut * 0.35f * chaosOut[modPos]);
        frame.spray    = juce::jlimit (0.0f, 1.0f, pSpray->load() + mut * 0.3f * (chaosOut[modSpray] * 0.5f + 0.5f));
        frame.sizeMs   = juce::jlimit (5.0f, 2000.0f, pSize->load() * std::exp2 (mut * 1.5f * chaosOut[modSize]));
        frame.sizeRnd  = pSizeRnd->load();
        frame.density  = juce::jlimit (0.5f, 200.0f, pDensity->load() * std::exp2 (mut * 1.5f * chaosOut[modDensity]));
        frame.densRnd  = pDensRnd->load();
        frame.chance   = pChance->load();
        frame.pitch    = pPitch->load();
        frame.pitchRnd = juce::jlimit (0.0f, 24.0f, pPitchRnd->load() + mut * 7.0f * (chaosOut[modPitchRnd] * 0.5f + 0.5f));
        frame.scale    = (int) pScale->load();
        frame.detune   = pDetune->load();
        frame.reverse  = pReverse->load();
        frame.keytrack = pKeytrack->load() > 0.5f;
        frame.window   = (int) pWindow->load();
        frame.pan      = pPan->load();
        frame.sample   = pSample->load();
        frame.sampleRnd= pSampleRnd->load();
        frame.cutoff   = juce::jlimit (30.0f, 20000.0f, pCutoff->load() * std::exp2 (mut * 3.0f * chaosOut[modCutoff]));
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
                const double poisson = -std::log (1.0 - (double) rng.uni() * 0.9999) * mean;
                v.samplesToNext += juce::jmax (1.0, mean + (poisson - mean) * (double) frame.densRnd);
            }
            v.samplesToNext -= (double) n;

            if (! v.env.isActive()) { v.active = false; v.drone = false; }
        }

        float* L = outL + start;
        float* R = outR != nullptr ? outR + start : tmpR.data();
        std::fill (tmpR.begin(), tmpR.end(), 0.0f);
        renderGrains (L, R, n);

        // --- crush (bit + sample rate) e stutter
        const float bits = 16.0f - crush * 13.0f;
        const float qStep = std::exp2 (bits - 1.0f);
        const int hold = 1 + (int) (crush * crush * 30.0f);
        for (int i = 0; i < n; ++i)
        {
            if (crush > 0.001f)
            {
                if (++crushCounter >= hold)
                {
                    crushCounter = 0;
                    crushHoldL = std::round (L[i] * qStep) / qStep;
                    crushHoldR = std::round (R[i] * qStep) / qStep;
                }
                L[i] = crushHoldL;
                R[i] = crushHoldR;
            }

            sbL[stutWrite] = L[i];
            sbR[stutWrite] = R[i];
            if (++stutWrite >= stutSize) stutWrite = 0;

            if (stutRemaining > 0)
            {
                int rp = stutStart + stutPhase;
                if (rp >= stutSize) rp -= stutSize;
                const float edge = (float) juce::jmin (stutPhase, stutLen - stutPhase);
                const float fade = juce::jmin (1.0f, edge / 48.0f);
                L[i] = sbL[rp] * fade;
                R[i] = sbR[rp] * fade;
                if (++stutPhase >= stutLen) stutPhase = 0;
                --stutRemaining;
            }
        }
    }

    freePpq = hostPlaying ? hostPpq + numSamples * ppqPerSample : freePpq + numSamples * ppqPerSample;
    stutterActive.store (stutRemaining > 0);

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

    if (outR == nullptr) return;

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

        dampL += 0.4f * (wetR - dampL);
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
    rv.wetLevel = vMix * 0.25f;
    rv.dryLevel = (1.0f - vMix * 0.5f) * 0.5f;
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
// MESSAGE THREAD: CC, trigger, dado, MIDI learn
//==============================================================================
void PolvereProcessor::timerCallback()
{
    // CC mappati -> parametri
    for (size_t cc = 0; cc < 128; ++cc)
    {
        const float v = pendingCC[cc].exchange (-1.0f);
        const int idx = ccMap[cc].load();
        if (v >= 0.0f && juce::isPositiveAndBelow (idx, paramList.size()))
        {
            auto* prm = paramList[idx];
            prm->beginChangeGesture();
            prm->setValueNotifyingHost (v);
            prm->endChangeGesture();
        }
    }

    // trigger sul fronte di salita (usa il controller in modalità momentanea)
    const bool gen  = apvts.getRawParameterValue ("generate")->load() > 0.5f;
    const bool dice = apvts.getRawParameterValue ("dice")->load() > 0.5f;
    const bool und  = apvts.getRawParameterValue ("undo")->load() > 0.5f;
    if (gen && ! lastGenerate) requestGenerate();
    if (dice && ! lastDice)    randomise (getDiceableIds(), apvts.getRawParameterValue ("diceAmt")->load());
    if (und && ! lastUndo)     undoRandomise();
    lastGenerate = gen; lastDice = dice; lastUndo = und;
}

juce::StringArray PolvereProcessor::getDiceableIds() const
{
    juce::StringArray ids;
    for (auto* prm : paramList)
        if (auto* wid = dynamic_cast<juce::AudioProcessorParameterWithID*> (prm))
            if (! notDiceable.contains (wid->paramID))
                ids.add (wid->paramID);
    return ids;
}

void PolvereProcessor::randomise (const juce::StringArray& ids, float amount)
{
    // range normalizzati "musicali"
    static const std::map<juce::String, juce::Range<float>> ranges {
        { "pos", { 0.0f, 1.0f } },     { "spray", { 0.0f, 0.8f } },   { "scan", { 0.3f, 0.7f } },
        { "size", { 0.15f, 0.85f } },  { "sizeRnd", { 0.0f, 0.8f } }, { "density", { 0.3f, 0.85f } },
        { "densRnd", { 0.0f, 1.0f } }, { "chance", { 0.55f, 1.0f } }, { "pitch", { 0.25f, 0.75f } },
        { "pitchRnd", { 0.0f, 0.5f } }, { "detune", { 0.0f, 0.4f } }, { "reverse", { 0.0f, 0.7f } },
        { "pan", { 0.2f, 1.0f } },     { "cutoff", { 0.35f, 1.0f } }, { "cutRnd", { 0.0f, 0.6f } },
        { "reso", { 0.0f, 0.6f } },    { "mutate", { 0.05f, 0.8f } }, { "mutRate", { 0.0f, 0.7f } },
        { "decay", { 0.2f, 0.8f } },   { "sustain", { 0.2f, 1.0f } }, { "release", { 0.15f, 0.75f } },
        { "dlyFb", { 0.0f, 0.8f } },   { "dlyMix", { 0.0f, 0.5f } },  { "verbMix", { 0.05f, 0.65f } },
        { "width", { 0.4f, 0.85f } },  { "verbSize", { 0.3f, 1.0f } }, { "fill", { 0.25f, 0.85f } },
        { "range", { 0.0f, 0.6f } },   { "ratchet", { 0.0f, 0.45f } }, { "evolve", { 0.0f, 0.4f } },
        { "gate", { 0.05f, 0.5f } },   { "swing", { 0.0f, 0.5f } },   { "stutter", { 0.0f, 0.5f } },
        { "crush", { 0.0f, 0.45f } },
    };

    std::vector<float> snapshot;
    for (auto* prm : paramList) snapshot.push_back (prm->getValue());
    undoStack.push_back (std::move (snapshot));
    if (undoStack.size() > 32) undoStack.erase (undoStack.begin());

    for (const auto& id : ids)
    {
        auto* prm = apvts.getParameter (id);
        if (prm == nullptr) continue;

        juce::Range<float> r { 0.0f, 1.0f };
        if (auto it = ranges.find (id); it != ranges.end()) r = it->second;

        float target = r.getStart() + msgRandom.nextFloat() * r.getLength();
        if (id == "pitch" && msgRandom.nextFloat() < 0.5f) target = 0.5f;
        if (id == "crush" && msgRandom.nextFloat() < 0.6f) target = 0.0f;
        const float cur = prm->getValue();
        const float v = juce::jlimit (0.0f, 1.0f, cur + (target - cur) * amount);

        prm->beginChangeGesture();
        prm->setValueNotifyingHost (v);
        prm->endChangeGesture();
    }
}

void PolvereProcessor::undoRandomise()
{
    if (undoStack.empty()) return;
    const auto snap = undoStack.back();
    undoStack.pop_back();
    for (int i = 0; i < juce::jmin ((int) snap.size(), paramList.size()); ++i)
    {
        auto* prm = paramList[i];
        if (auto* wid = dynamic_cast<juce::AudioProcessorParameterWithID*> (prm))
            if (wid->paramID == "generate" || wid->paramID == "dice" || wid->paramID == "undo") continue;
        prm->beginChangeGesture();
        prm->setValueNotifyingHost (snap[(size_t) i]);
        prm->endChangeGesture();
    }
}

int PolvereProcessor::indexOfParam (const juce::String& id) const
{
    for (int i = 0; i < paramList.size(); ++i)
        if (auto* wid = dynamic_cast<juce::AudioProcessorParameterWithID*> (paramList[i]))
            if (wid->paramID == id) return i;
    return -1;
}

void PolvereProcessor::startMidiLearn (const juce::String& paramId) { learnParam.store (indexOfParam (paramId)); }
void PolvereProcessor::cancelMidiLearn() { learnParam.store (-1); }

void PolvereProcessor::clearMidiMapping (const juce::String& paramId)
{
    const int idx = indexOfParam (paramId);
    for (auto& c : ccMap)
        if (c.load() == idx) c.store (-1);
}

int PolvereProcessor::getCCForParam (const juce::String& paramId) const
{
    const int idx = indexOfParam (paramId);
    if (idx < 0) return -1;
    for (int cc = 0; cc < 128; ++cc)
        if (ccMap[(size_t) cc].load() == idx) return cc;
    return -1;
}

juce::String PolvereProcessor::getLearningParam() const
{
    const int idx = learnParam.load();
    if (auto* wid = dynamic_cast<juce::AudioProcessorParameterWithID*> (juce::isPositiveAndBelow (idx, paramList.size()) ? paramList[idx] : nullptr))
        return wid->paramID;
    return {};
}

//==============================================================================
// SAMPLE POOL
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

            const auto maxLen = (juce::int64) (reader->sampleRate * 120.0);
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

void PolvereProcessor::clearSamples() { loadFiles ({}, true); }

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
    old.clear();
    sendChangeMessage();
}

//==============================================================================
// STATO
//==============================================================================
void PolvereProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    for (auto name : { "SAMPLES", "PATTERN", "MIDIMAP" })
        if (auto existing = state.getChildWithName (name); existing.isValid())
            state.removeChild (existing, nullptr);

    juce::ValueTree samples ("SAMPLES");
    for (const auto& s : uiPool)
    {
        juce::ValueTree t ("S");
        t.setProperty ("path", s->file.getFullPathName(), nullptr);
        samples.appendChild (t, nullptr);
    }
    state.appendChild (samples, nullptr);

    juce::ValueTree pat ("PATTERN");
    for (const auto& s : pattern)
    {
        juce::ValueTree t ("STEP");
        t.setProperty ("on", s.on, nullptr);
        t.setProperty ("semis", s.semis, nullptr);
        t.setProperty ("vel", s.velocity, nullptr);
        t.setProperty ("gate", s.gate, nullptr);
        t.setProperty ("ratchet", s.ratchet, nullptr);
        t.setProperty ("pos", s.posOffset, nullptr);
        t.setProperty ("size", s.sizeMul, nullptr);
        pat.appendChild (t, nullptr);
    }
    pat.setProperty ("root", rootNote.load(), nullptr);
    state.appendChild (pat, nullptr);

    juce::ValueTree map ("MIDIMAP");
    for (int cc = 0; cc < 128; ++cc)
    {
        const int idx = ccMap[(size_t) cc].load();
        if (auto* wid = dynamic_cast<juce::AudioProcessorParameterWithID*> (juce::isPositiveAndBelow (idx, paramList.size()) ? paramList[idx] : nullptr))
        {
            juce::ValueTree t ("CC");
            t.setProperty ("cc", cc, nullptr);
            t.setProperty ("param", wid->paramID, nullptr);
            map.appendChild (t, nullptr);
        }
    }
    state.appendChild (map, nullptr);

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

    if (auto pat = tree.getChildWithName ("PATTERN"); pat.isValid())
    {
        std::array<Step, maxSteps> loaded {};
        for (int i = 0; i < juce::jmin (maxSteps, pat.getNumChildren()); ++i)
        {
            const auto t = pat.getChild (i);
            auto& s = loaded[(size_t) i];
            s.on = t.getProperty ("on");
            s.semis = t.getProperty ("semis");
            s.velocity = t.getProperty ("vel");
            s.gate = t.getProperty ("gate");
            s.ratchet = t.getProperty ("ratchet");
            s.posOffset = t.getProperty ("pos");
            s.sizeMul = t.getProperty ("size");
        }
        rootNote.store ((int) pat.getProperty ("root", 60));
        {
            const juce::SpinLock::ScopedLockType lock (patternLock);
            pendingPattern = loaded;
            hasPendingPattern = true;
        }
        tree.removeChild (pat, nullptr);
    }

    if (auto map = tree.getChildWithName ("MIDIMAP"); map.isValid())
    {
        for (auto& c : ccMap) c.store (-1);
        for (const auto& t : map)
        {
            const int cc = t.getProperty ("cc");
            const int idx = indexOfParam (t.getProperty ("param").toString());
            if (juce::isPositiveAndBelow (cc, 128) && idx >= 0) ccMap[(size_t) cc].store (idx);
        }
        tree.removeChild (map, nullptr);
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

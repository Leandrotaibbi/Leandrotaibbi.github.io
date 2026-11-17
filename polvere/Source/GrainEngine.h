#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>
#include <memory>
#include <vector>

namespace polvere
{

//==============================================================================
// Un sample caricato nel pool (sempre stereo, mono duplicato)
struct Sample
{
    juce::String name;
    juce::File file;
    juce::AudioBuffer<float> data;
    double sampleRate = 44100.0;
    std::vector<float> peaks; // 256 valori di picco normalizzati per la GUI
};

using SamplePtr  = std::shared_ptr<Sample>;
using SamplePool = std::vector<SamplePtr>;

//==============================================================================
// PRNG xorshift veloce e real-time safe
struct FastRandom
{
    uint32_t state = 0x9E3779B9u;
    void seed (uint32_t s) noexcept { state = s != 0 ? s : 0x9E3779B9u; }
    inline uint32_t next() noexcept
    {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        return state;
    }
    inline float uni() noexcept  { return (float) (next() >> 8) * (1.0f / 16777216.0f); } // [0,1)
    inline float bi() noexcept   { return uni() * 2.0f - 1.0f; }                          // [-1,1)
};

//==============================================================================
enum WindowType { winHann = 0, winGauss, winTukey, winTriangle, winExpodec, winRexpodec, numWindows };

struct WindowTables
{
    static constexpr int size = 1024;
    std::array<std::array<float, size + 1>, numWindows> t {};

    WindowTables()
    {
        constexpr float twoPi = juce::MathConstants<float>::twoPi;
        for (int i = 0; i <= size; ++i)
        {
            const float x = (float) i / (float) size;
            t[winHann][(size_t) i]     = 0.5f - 0.5f * std::cos (twoPi * x);
            const float g = (x - 0.5f) / 0.16f;
            t[winGauss][(size_t) i]    = std::exp (-0.5f * g * g);
            const float a = 0.25f; // tukey alpha/2
            t[winTukey][(size_t) i]    = x < a ? 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * x / a)
                                       : x > 1.0f - a ? 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * (1.0f - x) / a)
                                       : 1.0f;
            t[winTriangle][(size_t) i] = 1.0f - std::abs (2.0f * x - 1.0f);
            auto expodec = [] (float y) { return y < 0.015f ? y / 0.015f : std::exp (-5.0f * (y - 0.015f) / 0.985f); };
            t[winExpodec][(size_t) i]  = expodec (x);
            t[winRexpodec][(size_t) i] = expodec (1.0f - x);
        }
        for (auto& w : t) { w[0] = 0.0f; w[size] = 0.0f; }
    }

    inline float lookup (int type, float phase) const noexcept
    {
        const float p = juce::jlimit (0.0f, 1.0f, phase) * (float) size;
        const int i = juce::jmin ((int) p, size - 1);
        const float f = p - (float) i;
        const auto& w = t[(size_t) type];
        return w[(size_t) i] + f * (w[(size_t) i + 1] - w[(size_t) i]);
    }
};

//==============================================================================
inline float hermite (float xm1, float x0, float x1, float x2, float t) noexcept
{
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b = w + a;
    return (((a * t) - b) * t + c) * t + x0;
}

//==============================================================================
struct Grain
{
    bool active = false;
    int voice = 0;
    int sampleIndex = 0;
    double readPos = 0.0;   // in frame del sample
    double increment = 1.0; // negativo = reverse
    int length = 0;         // in campioni host
    int age = 0;
    int delay = 0;          // offset di partenza nel sub-blocco
    int window = 0;
    float gainL = 0.0f, gainR = 0.0f;

    // filtro SVF TPT per grano
    bool filtered = false;
    int filterType = 0;
    float fa1 = 0, fa2 = 0, fa3 = 0, fk = 1;
    float ic1[2] { 0, 0 }, ic2[2] { 0, 0 };
};

struct Voice
{
    bool active = false;
    bool drone = false;
    int note = 60;
    float velocity = 1.0f;
    double samplesToNext = 0.0;
    uint32_t age = 0;
    float posOffset = 0.0f; // p-lock del sequencer
    float sizeMul = 1.0f;
    juce::ADSR env;
};

//==============================================================================
// Sequencer generativo
constexpr int maxSteps = 32;

struct Step
{
    bool on = false;
    float semis = 0.0f;     // offset dalla nota root (già quantizzato)
    float velocity = 0.8f;
    float gate = 0.6f;      // in multipli di step
    int ratchet = 1;        // 1..4 retrigger nello step
    float posOffset = 0.0f; // p-lock posizione
    float sizeMul = 1.0f;   // p-lock durata grani
};

struct SeqEvent
{
    bool active = false;
    bool isOn = true;
    int countdown = 0;      // campioni all'evento
    int note = 60;
    float velocity = 0.8f;
    float posOffset = 0.0f, sizeMul = 1.0f;
    int offAfter = 0;       // per gli eventi on: durata della nota
    int voice = -1;         // per gli eventi off
    uint32_t age = 0;
};

//==============================================================================
// Random walk liscio (interpolazione coseno tra punti casuali) — il "ballo"
struct Wander
{
    float from = 0, to = 0, phase = 0, rateMul = 1;
    float value = 0;

    void advance (float rateHz, float seconds, FastRandom& rng) noexcept
    {
        phase += rateHz * rateMul * seconds;
        while (phase >= 1.0f)
        {
            phase -= 1.0f;
            from = to;
            to = rng.bi();
        }
        const float s = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * phase);
        value = from + (to - from) * s;
    }
};

//==============================================================================
// Evento di grano per la visualizzazione
struct GrainEvent
{
    int sampleIndex = 0;
    float position = 0;  // 0..1
    float pan = 0;       // -1..1
    float pitch = 0;     // semitoni relativi
    float life = 0.1f;   // secondi
    bool reverse = false;
};

} // namespace polvere
